#pragma once
#include <core/imaging.h>
#include <utils/ref_count_base.hpp>
#include <utils/buffer_allocator.hpp>

namespace utils
{
	namespace imaging
	{
		class ref_count_image : public utils::ref_count_base<core::imaging::image_interface>
		{
		private:
			core::imaging::image_params m_image_params;
			utils::ref_count_ptr<core::buffer_interface> m_buffer;

		public:
			ref_count_image() :
				m_image_params({})
			{
			}

			ref_count_image(const core::imaging::image_params& image_params, core::buffer_interface* buffer) :
				m_image_params(image_params),
				m_buffer(buffer)
			{
				if (buffer == nullptr || buffer->size() < image_params.size)
					throw std::invalid_argument("buffer");
			}

			ref_count_image(const core::imaging::image_params& image_params) :
				ref_count_image(image_params, utils::make_ref_count_ptr<utils::ref_count_buffer>(image_params.size))
			{
			}

			virtual bool query_image_params(core::imaging::image_params& image_params) const override
			{
				image_params = m_image_params;
				return true;
			}

			virtual bool query_buffer(core::buffer_interface** buffer) const override
			{
				if (buffer == nullptr)
					return false;

				*buffer = m_buffer;
				(*buffer)->add_ref();
				return true;
			}

			void reset(const core::imaging::image_params& image_params, core::buffer_interface* buffer)
			{
				m_image_params = image_params;
				m_buffer = buffer;
			}

			bool is_set()
			{
				return (m_buffer != nullptr);
			}
		};
	}
}