#pragma once
#include <map>
#include <mutex>
#include <chrono>
#include <queue>
#include <thread>
#include <condition_variable>
#include <tuple>
#include <functional>
#include <atomic>
#include <imaging/image_converter.h>
#include <utils/ref_count_ptr.hpp>
#include <utils/scope_guard.hpp>
#include <utils/video.hpp>
#include <utils/buffer_allocator.hpp>

//opengl api
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION // Silence OpenGL deprecated on macOS 10.14 (Mojave)

#include <GLFW/glfw3.h>
#include <cmath>

#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367

#ifndef GL_BGR_EXT
#define GL_BGR_EXT GL_BGR
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT GL_BGRA
#endif


namespace utils
{
	class viewer
	{
		using int_pair = std::pair<int, int>;

	public:
        viewer(size_t stream_count, uint32_t width, uint32_t height, std::function<void()> on_close_callback, std::string title = "") :
			m_stream_count(stream_count),
			m_width(width),
			m_height(height),
			m_user_on_close_callback(on_close_callback),
			m_title(title),
			m_window(nullptr),
			m_is_running(true),
			m_hasNewImages(false)
		{
			m_render_buffer.resize(stream_count);
			m_ui_thread = std::thread(&viewer::ui_refresh, this);
		}

		~viewer()
		{
			m_is_running = false;

			m_user_on_close_callback = nullptr;

			if (std::this_thread::get_id() == m_ui_thread.get_id())
			{
				m_ui_thread.detach();
			}
			else
			{
				m_render_thread_cv.notify_one();

				if (m_ui_thread.joinable() == true)
					m_ui_thread.join();
			}

			m_render_buffer.clear();
		}

        void show_frame(core::imaging::image_interface* image, size_t index)
		{
			if (image == nullptr)
				return;

			update_buffer(image, index);
		}

	private:
		void setup_window(uint32_t width, uint32_t height, std::string window_title)
		{
			if (m_window)
			{
				glfwDestroyWindow(m_window);
				glfwTerminate();
			}
			glfwInit();
            m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), window_title.c_str(), nullptr, nullptr);
			glfwMakeContextCurrent(m_window);
			glfwSetWindowUserPointer(m_window, this);
			/*glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) ->void
			{
			void* user_pointer = glfwGetWindowUserPointer(window);
			if (user_pointer == nullptr)
			return;

			viewer* user = static_cast<viewer*>(user_pointer);
			user->redraw();
			});*/

			glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) -> void
			{
                (void)window;
				/*void* user_pointer = glfwGetWindowUserPointer(window);
				if (user_pointer == nullptr)
				return;

				viewer* user = static_cast<viewer*>(user_pointer);
				user->redraw();*/
			});
		}

		void draw(const std::vector<utils::ref_count_ptr<core::imaging::image_interface>>& frames)
		{
			glfwMakeContextCurrent(m_window);
			int window_width, window_height;
			glfwGetWindowSize(m_window, &window_width, &window_height);

			if (frames.size() == 1)
			{
				glScissor(0, 0, window_width, window_height);
				glClearColor(0, 0, 0, 255);
				glClear(GL_COLOR_BUFFER_BIT);
			}

			glEnable(GL_TEXTURE_2D);
			glViewport(0, 0, window_width, window_height);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();

			glOrtho(0, window_width, window_height, 0, -1, +1);

			for (size_t i = 0; i < frames.size(); i++)
			{
				utils::ref_count_ptr<core::imaging::image_interface> frame = frames[i];
				if (frame == nullptr)
					continue;

				// Clear image section
				auto section_rect = calc_section(window_width, window_height, i);
				if (section_rect.second.first == 0 || section_rect.second.second == 0)
					break;

				glScissor(section_rect.first.first, section_rect.first.second, section_rect.second.first, section_rect.second.second);

                GLenum gl_format = 0;
                GLenum gl_channel_type = 0;
				int gl_internal_format = 0;
				utils::ref_count_ptr<core::imaging::image_interface> image_to_show = frame;

				core::imaging::image_params image_params = {};
				if (image_to_show->query_image_params(image_params) == false)
					continue;				

                int source_width = static_cast<int>(image_params.width);
                int source_height = static_cast<int>(image_params.height);

				switch (image_params.format)
				{
				case core::imaging::pixel_format::RGB:
					gl_format = GL_RGB;
					gl_channel_type = GL_UNSIGNED_BYTE;
					gl_internal_format = GL_RGB;
					break;
				case core::imaging::pixel_format::RGBA:
					gl_format = GL_RGBA;
					gl_channel_type = GL_UNSIGNED_INT_8_8_8_8_REV;
					gl_internal_format = GL_RGBA;
					break;
				case core::imaging::pixel_format::BGR:
					gl_format = GL_BGR_EXT;
					gl_channel_type = GL_UNSIGNED_BYTE;
					gl_internal_format = GL_RGB;
					break;
				case core::imaging::pixel_format::BGRA:
					gl_format = GL_BGRA_EXT;
					gl_channel_type = GL_UNSIGNED_INT_8_8_8_8_REV;
					gl_internal_format = GL_RGBA;
					break;
				case core::imaging::pixel_format::I420:
				case core::imaging::pixel_format::NV12:
				case core::imaging::pixel_format::YUY2:
				case core::imaging::pixel_format::UYVY:
				case core::imaging::pixel_format::GRAY8:
				case core::imaging::pixel_format::GRAY16_LE:
				{
					if (m_image_converter == nullptr)
					{
						if (imaging::image_converter::create(core::imaging::pixel_format::RGBA, &m_image_converter) == false)
							throw std::runtime_error("Failed to create image pixel-format converter");
					}

					utils::ref_count_ptr<core::imaging::image_interface> converted_image;
					if (m_image_converter->apply(image_to_show, &converted_image) == false)
						throw std::runtime_error("Failed to convert frame's pixel-format");

					image_to_show = converted_image;  
					
					gl_format = GL_RGBA;
					gl_internal_format = GL_RGBA;
					gl_channel_type = GL_UNSIGNED_INT_8_8_8_8_REV;
					break;
				}				
				default:
					throw std::runtime_error("format is not supported");
				}

				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (image_to_show->query_buffer(&buffer) == false)
					continue;

				auto rect = calc_window_size(section_rect.second.first, section_rect.second.second, frame);

				auto x_entry = section_rect.first.first + rect.first.first;
				auto y_entry = section_rect.first.second + rect.first.second;
				auto width = rect.second.first;
				auto height = rect.second.second;

				if (width == 0 || height == 0)
					return;

				glBindTexture(GL_TEXTURE_2D, m_textures_ids[i]);

				if (m_textureInitialized[i] == false)
				{
                    glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, source_width, source_height, 0, gl_format, gl_channel_type, buffer->data());
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

					m_textureInitialized[i] = true;
				}
				else
				{
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, source_width, source_height, gl_format, gl_channel_type, buffer->data());
				}

				glBegin(GL_QUADS);

				glTexCoord2f(0, 0); glVertex2f(static_cast<GLfloat>(x_entry), static_cast<GLfloat>(y_entry));
				glTexCoord2f(0, 1); glVertex2f(static_cast<GLfloat>(x_entry), static_cast<GLfloat>(y_entry + height));
				glTexCoord2f(1, 1); glVertex2f(static_cast<GLfloat>(x_entry + width), static_cast<GLfloat>(y_entry + height));
				glTexCoord2f(1, 0); glVertex2f(static_cast<GLfloat>(x_entry + width), static_cast<GLfloat>(y_entry));

				glEnd();
			}

			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
			glfwSwapBuffers(m_window);
		}

		void ui_refresh()
		{
			setup_window(m_width, m_height, m_title);
			m_textures_ids.resize(m_stream_count);
			m_textureInitialized.resize(m_stream_count);
			m_converted_image_buffers.resize(m_stream_count);

            glGenTextures(static_cast<int>(m_stream_count), m_textures_ids.data());

            for (size_t i = 0; i < m_stream_count; i++)
			{
				m_textureInitialized[i] = false;
			}

			auto pred = [&]() -> bool
			{
				return ((m_is_running == false) || (m_hasNewImages == true));
			};

			std::vector<utils::ref_count_ptr<core::imaging::image_interface>> cloned_pointers;
			cloned_pointers.resize(m_stream_count);

			auto interval = std::chrono::microseconds(16666);
			auto time_to_wait = interval;

			while (m_is_running)
			{
				utils::scope_guard pointers_clear([&cloned_pointers]()
				{
					for (size_t i = 0; i < cloned_pointers.size(); ++i)
					{
						cloned_pointers[i].release();
					}
				});

				std::unique_lock<std::mutex> locker(m_render_mutex);
				/*bool render = */m_render_thread_cv.wait_for(locker, time_to_wait, pred);

				auto now = std::chrono::high_resolution_clock::now();

				if (m_is_running == false)
					break;

				bool render = false;

				if (m_hasNewImages == true)
				{
					for (size_t i = 0; i < m_render_buffer.size(); ++i)
					{
						if (m_render_buffer[i] == nullptr)
							continue;

						cloned_pointers[i] = m_render_buffer[i];
						m_render_buffer[i].release();
						render = true;
					}

					m_hasNewImages = false;
				}

				locker.unlock();

				if (render == true)
				{
					draw(cloned_pointers);
				}

				glfwPollEvents();
				if (glfwWindowShouldClose(m_window) > 0)
				{
					m_is_running = false;
				}

				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - now);
				if (duration > interval)
					time_to_wait = std::chrono::microseconds(0);
				else
					time_to_wait = std::chrono::duration_cast<std::chrono::microseconds>(interval - duration);

			}

			if (m_user_on_close_callback != nullptr)
			{
				m_user_on_close_callback();
			}

			glfwDestroyWindow(m_window);
			glfwTerminate();
		}

        void update_buffer(core::imaging::image_interface* image, size_t index)
		{
			if (m_is_running)
			{
				std::unique_lock<std::mutex> locker(m_render_mutex);
				m_render_buffer[index] = image;
				m_hasNewImages = true;
				locker.unlock();

				//m_render_thread_cv.notify_one();
			}
		}

		std::pair<viewer::int_pair, viewer::int_pair> calc_window_size(int window_width, int window_height, const core::imaging::image_interface* frame)
		{
			core::imaging::image_params image_params = {};
			if (frame->query_image_params(image_params) == false)
				return std::pair<int_pair, int_pair>(int_pair(0, 0), int_pair(0, 0));

            double scale_width = window_width / static_cast<double>(image_params.width);
            double scale_height = window_height / static_cast<double>(image_params.height);

			double width = scale_width < scale_height ? window_width : image_params.width * scale_height;
			double height = scale_height < scale_width ? window_height : image_params.height * scale_width;

			int x_entry = static_cast<int>((window_width - width) / 2);
			int y_entry = static_cast<int>((window_height - height) / 2);

			return std::pair<int_pair, int_pair>(int_pair(x_entry, y_entry), int_pair(static_cast<int>(width), static_cast<int>(height)));
		}

		std::pair<viewer::int_pair, viewer::int_pair> calc_section(int window_width, int window_height, size_t index)
		{
			if (window_width == 0 || window_height == 0)
				return std::pair<viewer::int_pair, viewer::int_pair>({ 0, 0 }, { 0, 0 });
			int x_count;
			int y_count;
			int base = static_cast<int>(std::ceil(std::sqrt(m_stream_count)));
			if (window_width > window_height)
			{
				x_count = base;
				y_count = static_cast<int>((static_cast<double>(m_stream_count) / static_cast<double>(x_count)) + 0.5);
			}
			else
			{
				y_count = base;
				x_count = static_cast<int>((static_cast<double>(m_stream_count) / static_cast<double>(y_count)) + 0.5);
			}

			int width = window_width / x_count;
			int height = window_height / y_count;

            int x = ((static_cast<int>(index) % x_count) * width);
            int y = ((static_cast<int>(index) / x_count) * height);


			return std::pair<viewer::int_pair, viewer::int_pair>({ x, y }, { width,height });
		}

        size_t m_stream_count;
		uint32_t m_width;
		uint32_t m_height;
		std::function<void()> m_user_on_close_callback;
		std::string m_title;

		std::vector<utils::ref_count_ptr<core::imaging::image_interface>> m_render_buffer;
		std::condition_variable m_render_thread_cv;
		std::thread m_ui_thread;
		std::mutex m_render_mutex;
		GLFWwindow* m_window;
		std::atomic<bool> m_is_running;
		bool m_hasNewImages;

		std::vector<bool> m_textureInitialized;
		std::vector<GLuint> m_textures_ids;
		std::vector<utils::ref_count_ptr<core::buffer_interface>> m_converted_image_buffers;
		utils::ref_count_ptr<core::imaging::image_algorithm_interface> m_image_converter;
	};
}
