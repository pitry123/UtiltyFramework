#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <shared_memory_streaming.hpp>

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gstbuffer.h>

class gst_buffer_map_wrapper
{
private:
	GstBuffer * m_buffer;
	GstMapInfo m_info;
	bool m_mapped;

public:

	gst_buffer_map_wrapper(GstBuffer* buffer, GstMapFlags flags) :
		m_buffer(buffer),
		m_mapped(false)
	{
		if (m_buffer == nullptr)
			return;

		if (gst_buffer_map(m_buffer, &m_info, flags))
			m_mapped = true;
	}

	~gst_buffer_map_wrapper()
	{
		if (m_mapped == true)
			gst_buffer_unmap(m_buffer, &m_info);
	}

	uint8_t* data()
	{
		if (m_mapped == false)
			return nullptr;

		return m_info.data;
	}
};

class gst_pool_item : public utils::ref_count_base<core::ref_count_interface>
{
private:
	static constexpr uint32_t HEADER_SIZE =
		sizeof(core::imaging::image_params) +
		sizeof(core::video::display_params) +
		sizeof(core::video::video_params) + 
		sizeof(uint32_t);

	GstBuffer * m_gst_buffer;
	utils::ref_count_ptr<shared_memory::shm_sharable_buffer_interface> m_shared_buffer;

	uint32_t m_alignment_offset;
	bool m_taken;	

public:
	gst_pool_item(shared_memory::shm_sharable_buffer_interface* shared_buffer, uint32_t alignment, GstBufferPool* gst_pool) :
		m_shared_buffer(shared_buffer),
		m_alignment_offset(0),
		m_taken(false)
	{
		if (alignment != 0)
		{
			size_t shared_buffer_size = m_shared_buffer->size();
			uint32_t max_data_size = static_cast<uint32_t>(shared_buffer_size - (HEADER_SIZE + alignment));
			void* unaligned_ptr = (m_shared_buffer->data() + HEADER_SIZE);
			void* aligned_ptr = unaligned_ptr;
			
			size_t bounds = ((m_shared_buffer->data() + shared_buffer_size) - static_cast<uint8_t*>(unaligned_ptr));
			if (std::align(alignment, max_data_size, aligned_ptr, bounds) == nullptr || aligned_ptr == nullptr)
				throw std::runtime_error("Failed to align data");

			m_alignment_offset = static_cast<uint32_t>((static_cast<uint8_t*>(aligned_ptr) - static_cast<uint8_t*>(unaligned_ptr)));

		}

		gsize gst_buffer_offset = HEADER_SIZE + m_alignment_offset;
		gpointer gst_buffer_ptr = (gpointer)(m_shared_buffer->data() + gst_buffer_offset);
		gsize gst_buffer_size = m_shared_buffer->size() - gst_buffer_offset;

		GstBuffer* gst_buffer = gst_buffer_new_wrapped_full(
			static_cast<GstMemoryFlags>(0),
			gst_buffer_ptr,
			gst_buffer_size,
			0,
			gst_buffer_size,
			NULL,
			NULL);

		if (gst_buffer == nullptr)
			throw std::runtime_error("WTF!");

		gst_buffer->pool = gst_pool;
		m_gst_buffer = gst_buffer;
	}

	~gst_pool_item()
	{
		gst_buffer_unref(m_gst_buffer);
	}

	GstBuffer* gst_buffer()
	{
		return m_gst_buffer;
	}

	bool aquire()
	{
		if (m_taken == true)
			return false;

		if (m_shared_buffer->lock_unique() == false)
			return false;

		m_taken = true;
		return true;
	}

	bool free()
	{
		if (m_shared_buffer->unlock() == false)
			return false;

		m_taken = false;
		return true;
	}

	bool publish(const core::imaging::image_params& image_params, const core::video::display_params& display_params, const core::video::video_params& video_params)
	{
		uint8_t* ptr = m_shared_buffer->data();
		std::memcpy(ptr, &image_params, sizeof(image_params));
		ptr += sizeof(image_params);
		
		std::memcpy(ptr, &display_params, sizeof(display_params));
		ptr += sizeof(display_params);

		std::memcpy(ptr, &video_params, sizeof(video_params));
		ptr += sizeof(video_params);

		std::memcpy(ptr, &m_alignment_offset, sizeof(uint32_t));

		shared_memory::error_codes error = m_shared_buffer->publish();
		if (error != shared_memory::error_codes::SHM_NO_ERROR)
			return false;

		return true;
	}
};
