#pragma once
#include <video/publishers/gstreamer_rtsp_launch.h>

#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

// gstreamer includes
#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

namespace video
{
	namespace publishers
	{
		template <class T>
		class gstreamer_base_rtsp_server : public T
		{
		private:
			static gboolean timeout(GstRTSPServer * server)
			{
				GstRTSPSessionPool* pool = gst_rtsp_server_get_session_pool(server);
				gst_rtsp_session_pool_cleanup(pool);
				g_object_unref(pool);

				return TRUE;
			}

			static void	rtsp_media_constructed(GstRTSPMediaFactory* gstrtspmediafactory, GstRTSPMedia* media, gpointer user_data)
			{

			}

			static void rtsp_media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data)
			{
				/* get the element used for providing the streams of the media */
				GstElement* element = gst_rtsp_media_get_element(media);
				static_cast<video::publishers::gstreamer_base_rtsp_server<T>*>(user_data)->on_pipeline_constructed(element);
			}

			std::string m_url;
			uint16_t m_port;

			std::string m_minmum_multicast_address;
			std::string m_maximum_multicast_address;
			uint16_t m_minmum_multicast_port;			
			uint16_t m_maximum_multicast_port;

			std::mutex m_state_mutex;
			core::video::video_state m_state;

			std::atomic<bool> m_running;
			bool m_stop_feed;

			std::thread m_working_thread;

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GMainLoop* m_working_loop;

			inline void start_unsafe()
			{
				on_start();

				m_state = core::video::PLAYING;

				enum init_state
				{
					Unknown,
					Success,
					Failed
				};

				std::mutex mutex;
				std::condition_variable server_creation_sync;
				init_state start_state = init_state::Unknown;

				m_working_thread = std::thread([&]()
				{
					GstRTSPServer* server;
					GstRTSPMountPoints* mounts;
					GstRTSPMediaFactory* factory;
					GstRTSPAddressPool* pool;

					int argc = 0;
					gst_init(&argc, nullptr);

					m_working_loop = g_main_loop_new(NULL, FALSE);

					/* create a server instance */
					server = gst_rtsp_server_new();

					/* get the mount points for this server, every server has a default object
					* that be used to map uri mount points to media factories */
					mounts = gst_rtsp_server_get_mount_points(server);

					/* make a media factory for a test stream. The default media factory can use
					* gst-launch syntax to create pipelines.
					* any launch line works as long as it contains elements named pay%d. Each
					* element with pay%d names will be a stream */
					factory = gst_rtsp_media_factory_new();

					gst_rtsp_media_factory_set_launch(factory, pipeline_description().c_str());
					gst_rtsp_media_factory_set_shared(factory, TRUE);

					if (m_minmum_multicast_address.empty() == false && 
						m_maximum_multicast_address.empty() == false &&
						m_minmum_multicast_port > 0 &&
						m_maximum_multicast_port > m_minmum_multicast_port)
					{
						///* make a new address pool */
						pool = gst_rtsp_address_pool_new();
						gst_rtsp_address_pool_add_range(pool,
							m_minmum_multicast_address.c_str(), m_maximum_multicast_address.c_str(), m_minmum_multicast_port, m_maximum_multicast_port, 16);

						gst_rtsp_media_factory_set_address_pool(factory, pool);
						
						/* make sure multicast is available */
						GstRTSPLowerTrans protocols = gst_rtsp_media_factory_get_protocols(factory);
						protocols = static_cast<GstRTSPLowerTrans>(protocols | GstRTSPLowerTrans::GST_RTSP_LOWER_TRANS_UDP_MCAST);
						gst_rtsp_media_factory_set_protocols(factory, protocols);

						/* allow multicast only */
						/*gst_rtsp_media_factory_set_protocols(factory,
							GST_RTSP_LOWER_TRANS_UDP_MCAST);*/

						g_object_unref(pool);
					}

					// Set the server's listening port
					// The max value of a uint16_t is 65k, which is 5 chars
					char port_buf[6] = {};
					unsigned int local_port = static_cast<unsigned int>(m_port);
#ifdef _WIN32
					sprintf_s(port_buf, "%u", local_port);
#else
					sprintf(port_buf, "%u", local_port);
#endif
					gst_rtsp_server_set_service(server, port_buf);

					/* notify when our media is ready, This is called whenever someone asks for
					* the media and a new pipeline with our appsrc is created */
					g_signal_connect(factory, "media-configure", (GCallback)rtsp_media_configure, this);
					g_signal_connect(factory, "media-constructed", (GCallback)rtsp_media_constructed, this);

					/* attach the test factory to the /test url */
					gst_rtsp_mount_points_add_factory(mounts, (std::string("/") + m_url).c_str(), factory);

					/* don't need the ref to the mapper anymore */
					g_object_unref(mounts);

					g_timeout_add_seconds(2, (GSourceFunc)timeout, server);

					/* attach the server to the default main context */
					auto gst_server_id = gst_rtsp_server_attach(server, NULL);
					if (gst_server_id == 0)
					{
						g_print("failed to attach the server\n");
						std::unique_lock<std::mutex> init_locker(mutex);
						start_state = init_state::Failed;
						init_locker.unlock();

						server_creation_sync.notify_one();
						return;
					}

					std::unique_lock<std::mutex> locker(mutex);
					start_state = init_state::Success;
					locker.unlock();
					server_creation_sync.notify_one();

					/* start serving */
					g_print("stream ready at rtsp://0.0.0.0:%d/%s\n", static_cast<int>(m_port), m_url.c_str());
					g_main_loop_run(m_working_loop);

					if (gst_server_id != 0)
						g_source_remove(gst_server_id);

					g_object_unref(server);
					on_pipeline_completed();
				});

				std::unique_lock<std::mutex> init_locker(mutex);
				server_creation_sync.wait(init_locker, [&]()
				{
					return (start_state != init_state::Unknown);
				});

				init_locker.unlock();

				if (start_state != init_state::Success)
				{
					stop_unsafe();
					return;
				}

				m_running = true;
				on_started();
			}

			inline void stop_unsafe()
			{
				if (m_state != core::video::video_state::PLAYING)
					return;

				on_stop();

				m_state = core::video::video_state::STOPPED;
				g_main_quit(m_working_loop);
				if (m_working_thread.joinable() == true)
					m_working_thread.join();

				g_main_loop_unref(m_working_loop);

				m_running = false;
				on_pipeline_destroyed();				
				on_stopped();
			}

		protected:
			virtual std::string pipeline_description() = 0;

			bool running()
			{
				return m_running.load();
			}

			virtual void on_start()
			{

			}

			virtual void on_started()
			{

			}

			virtual void on_stop()
			{

			}

			virtual void on_stopped()
			{

			}

			virtual void on_pipeline_constructed(GstElement* pipeline)
			{
				
			}

			virtual void on_pipeline_completed()
			{
				
			}

			virtual void on_pipeline_destroyed()
			{

			}

		public:
			gstreamer_base_rtsp_server(
				const char* stream_name,
				uint16_t port, 
				const char* minmum_multicast_address,
				const char* maximum_multicast_address,
				uint16_t minmum_multicast_port,
				uint16_t maximum_multicast_port) :
				m_url(stream_name), m_port(port),
				m_minmum_multicast_address((minmum_multicast_address == nullptr ? "" : minmum_multicast_address)),				
				m_maximum_multicast_address((maximum_multicast_address == nullptr ? "" : maximum_multicast_address)),
				m_minmum_multicast_port(minmum_multicast_port),
				m_maximum_multicast_port(maximum_multicast_port)
			{
			}

			gstreamer_base_rtsp_server(const char* stream_name, uint16_t port) :
				gstreamer_base_rtsp_server(stream_name, port, nullptr, nullptr, 0, 0)
			{
			}

			virtual ~gstreamer_base_rtsp_server()
			{
				stop();
			}

			virtual core::video::video_state state() override
			{
				return m_state;
			}

			virtual void start() override
			{
				std::lock_guard<std::mutex> locker(m_state_mutex);
				stop_unsafe();
				start_unsafe();
			}

			virtual void stop() override
			{
				std::lock_guard<std::mutex> locker(m_state_mutex);
				stop_unsafe();
			}

			virtual void pause() override
			{
				// Currently we treat pause as stop
				stop();
			}

			virtual bool add_error_callback(core::video::video_error_callback* callback) override
			{
				(void)callback;

				// Currently not supported
				return false;
			}

			virtual bool remove_error_callback(core::video::video_error_callback* callback) override
			{
				(void)callback;

				// Currently not supported
				return false;
			}
		};
	}
}

