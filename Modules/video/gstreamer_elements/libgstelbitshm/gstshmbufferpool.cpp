#include "gstshmbufferpool.h"
#include "gstshmcommon.hpp"
#include "gstshmmeta.h"

#include <stdio.h>
#include <string.h>
#include <core/video.h>

#include <mutex>
#include <vector>
#include <algorithm>

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <libgstatime/gstatimemeta.h>

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4244) // conversion
#	pragma warning(disable: 4996) // deprecated
#endif

#define GLIB_VERSION_RULE() GLIB_CHECK_VERSION(2, 58, 0)

class gst_shared_memory_allocation_manager : public utils::ref_count_base<core::ref_count_interface>
{
private:
	utils::ref_count_ptr<shared_memory::shm_session> m_session_writer;

	std::mutex m_mutex;
	GstBufferPool* m_pool;
	std::string m_session_name;
	uint32_t m_pool_size;
	uint32_t m_alignment;
	uint32_t m_prefix;
	uint32_t m_padding;
	uint32_t m_buffer_size;		

	std::vector<utils::ref_count_ptr<gst_pool_item>> m_buffers;
	uint8_t* m_base_adress;
	size_t m_next_query_index;
	
	core::imaging::image_params m_image_params;
	core::video::video_params m_video_params;

	std::once_flag m_timestamps_meta_once_flag;
	const GstMetaInfo* m_timestamps_meta_info;

	void map_shared_memory()
	{
		if (m_session_writer == nullptr)
		{
			shared_memory::shm_pool_params pool_params{ 0, m_pool_size, m_buffer_size };
            m_session_writer = utils::make_ref_count_ptr<shared_memory::shm_session>(m_session_name.c_str(), &pool_params, static_cast<uint32_t>(1));
		}
		else
		{
			m_session_writer->remap();
		}

		m_buffers.clear();
		m_buffers.resize(m_pool_size);

		for (uint32_t i = 0; i < m_pool_size; i++)
		{
			utils::ref_count_ptr<shared_memory::shm_sharable_buffer_interface> shared_buffer;
			if (m_session_writer->query_buffer_unlocked(0, i, &shared_buffer) != shared_memory::error_codes::SHM_NO_ERROR)
                throw std::runtime_error("Unexpected!");

			if (i == 0)
				m_base_adress = shared_buffer->data();

			utils::ref_count_ptr<gst_pool_item> pool_item = utils::make_ref_count_ptr<gst_pool_item>(shared_buffer, m_alignment, m_pool);
			m_buffers[i] = pool_item;
		}
	}

	bool query_pool_item(gst_pool_item** pool_item)
	{
		if (pool_item == nullptr)
			return false;

		for (uint32_t i = 0; i < m_pool_size; i++)
		{			
			size_t index = ((m_next_query_index + i) % m_pool_size);
			if (m_buffers[index]->aquire() == false)
				continue;

			m_next_query_index = ((index + 1) % m_pool_size);

			*pool_item = m_buffers[index];
			(*pool_item)->add_ref();			

			return true;			
		}
		
		return false;
	}

	bool query_pool_item(GstBuffer* buffer, gst_pool_item** pool_item)
	{
		if (buffer == nullptr)
			return false;

		if (pool_item == nullptr)
			return false;

		if (GST_MINI_OBJECT_TYPE(buffer) == 0)
			return false;

		gst_buffer_map_wrapper mapper(buffer, GST_MAP_READ);
		uint8_t* data_ptr = mapper.data();
		if (data_ptr == nullptr)
			return false;				

		if (data_ptr < m_base_adress || data_ptr >= (m_base_adress + (m_pool_size * m_buffer_size)))
			return false;

        size_t offset = static_cast<size_t>(data_ptr - m_base_adress);
        size_t index = static_cast<size_t>((offset - (offset % m_buffer_size)) / m_buffer_size);
		if (index >= m_buffers.size())
			return false;

		*pool_item = m_buffers[index];
		(*pool_item)->add_ref();
		return true;
	}

public:
    static constexpr guint DATA_OFFSET = static_cast<guint>(sizeof(core::imaging::image_params) + sizeof(core::video::video_params) + sizeof(core::video::display_params) + sizeof(uint32_t));

	gst_shared_memory_allocation_manager(GstBufferPool* pool, const char* session_name, uint32_t pool_size, uint32_t alignment, uint32_t prefix, uint32_t padding, int32_t buffer_size, uint32_t minimum_buffer_size, GstVideoInfo& info) :
		m_pool(pool),
		m_session_name(session_name),
		m_pool_size(pool_size),
		m_alignment(alignment),
		m_prefix(prefix),
		m_padding(padding),
        m_buffer_size((std::max)(static_cast<uint32_t>(buffer_size) + DATA_OFFSET + alignment + prefix + padding, minimum_buffer_size)),
		m_base_adress(nullptr),
		m_next_query_index(0)
	{
        // Unused
        (void)m_prefix;
        (void)m_padding;

		switch (info.finfo->format)
		{
		case GST_VIDEO_FORMAT_RGB:
			m_image_params.format = core::imaging::pixel_format::RGB;
			break;
		case GST_VIDEO_FORMAT_RGBA:
			m_image_params.format = core::imaging::pixel_format::RGBA;
			break;
		case GST_VIDEO_FORMAT_BGR:
			m_image_params.format = core::imaging::pixel_format::BGR;
			break;
		case GST_VIDEO_FORMAT_BGRA:
			m_image_params.format = core::imaging::pixel_format::BGRA;
			break;
		case GST_VIDEO_FORMAT_I420:
			m_image_params.format = core::imaging::pixel_format::I420;
			break;
		case GST_VIDEO_FORMAT_NV12:
			m_image_params.format = core::imaging::pixel_format::NV12;
			break;
		case GST_VIDEO_FORMAT_YUY2:
			m_image_params.format = core::imaging::pixel_format::YUY2;
			break;
		case GST_VIDEO_FORMAT_UYVY:
			m_image_params.format = core::imaging::pixel_format::UYVY;
			break;
		case GST_VIDEO_FORMAT_GRAY8:
			m_image_params.format = core::imaging::pixel_format::GRAY8;
			break;
		case GST_VIDEO_FORMAT_GRAY16_LE:
			m_image_params.format = core::imaging::pixel_format::GRAY16_LE;
			break;
		default:
			m_image_params.format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;
			break;
		}

        m_image_params.width = static_cast<uint32_t>(info.width);
        m_image_params.height = static_cast<uint32_t>(info.height);
		m_image_params.size = static_cast<uint32_t>(info.size);

        m_video_params.framerate.numerator = static_cast<uint32_t>(info.fps_n);
        m_video_params.framerate.denominator = static_cast<uint32_t>(info.fps_d);
		
		switch (info.interlace_mode)
		{
		case GST_VIDEO_INTERLACE_MODE_PROGRESSIVE:
			m_video_params.interlace_mode = core::video::interlace_mode::PROGRESSIVE;
			break;
		case GST_VIDEO_INTERLACE_MODE_INTERLEAVED:
			m_video_params.interlace_mode = core::video::interlace_mode::INTERLEAVED;
			break;
		case GST_VIDEO_INTERLACE_MODE_MIXED:
			m_video_params.interlace_mode = core::video::interlace_mode::MIXED;
			break;
		case GST_VIDEO_INTERLACE_MODE_FIELDS:
			m_video_params.interlace_mode = core::video::interlace_mode::FIELDS;
			break;
		default:
			m_video_params.interlace_mode = core::video::interlace_mode::UNDEFINED_INTERLACE_MODE;
			break;
		}		
		
		map_shared_memory();
	}

	bool aquire(GstBuffer** buffer)
	{
		if (buffer == nullptr)
			return false;

		utils::ref_count_ptr<gst_pool_item> pool_item;		
		
		std::unique_lock<std::mutex> locker(m_mutex);
		int retry_counter = 5;
		bool found = false;
		while(retry_counter > 0)
		{
			found = query_pool_item(&pool_item);
			if (found == true)
				break;

            const gchar* message = "Pool is being abused! Remapping...\n";
            g_print("%s", message);
            GST_WARNING_OBJECT(m_pool, "%s", message);
			
			--retry_counter;
			map_shared_memory();
		}		
		locker.unlock();

        if (found == false)
		{
			const char* message = "Failed to map shared memory. Buffer allocation failed...\n";
            g_print("%s", message);
            GST_ERROR_OBJECT(m_pool, "%s", message);
			return false;
		}

		GstBuffer* pool_buf = pool_item->gst_buffer();

		// We add the pool item as metadata.
		// The metadata assures we'll preserve ownership on the shared memory until the buffer is freed.
		// That's important since we might re-map the whole shared allocation if the buffer pool is not sufficient
		// or being abused by a client.
		GST_META_SHM_BUFFER_ADD(pool_buf, static_cast<void*>(pool_item));

		*buffer = pool_buf;
		return true;
	}

	bool free(GstBuffer* buffer)
	{
		utils::ref_count_ptr<gst_pool_item> pool_item;

		std::unique_lock<std::mutex> locker(m_mutex);
		
		if (query_pool_item(buffer, &pool_item) == false)
			return false;

		locker.unlock();

		if (pool_item->free() == false)
			return false;		

		return true;
	}

	bool publish(GstBuffer* source_buffer, GstBuffer* target_buffer)
	{
		std::call_once(m_timestamps_meta_once_flag, [&]()
		{
			m_timestamps_meta_info = gst_meta_get_info("ATime");
		});

		utils::ref_count_ptr<gst_pool_item> pool_item;

		std::unique_lock<std::mutex> locker(m_mutex);

		if (query_pool_item(target_buffer, &pool_item) == false)
			return false;

		locker.unlock();

		core::video::display_params display_params;
		display_params.dts = source_buffer->dts;
		display_params.pts = source_buffer->pts;
		display_params.duration = source_buffer->duration;
		display_params.frame_id = 0;
		display_params.timestamp = 0;

		if (m_timestamps_meta_info != nullptr)
		{
			// Try to get timestamp		
			GstMeta* meta = gst_buffer_get_meta(source_buffer, m_timestamps_meta_info->api);
			if (meta != nullptr)
				display_params.timestamp = static_cast<uint64_t>(reinterpret_cast<ATimeMeta*>(meta)->absoluteTime);
		}
		
		bool retval = pool_item->publish(m_image_params, display_params, m_video_params);
		if (retval == false)
		{
			const char* message = "Pool is being abused! Remapping...\n";
            g_print("%s", message);
            GST_WARNING_OBJECT(m_pool, "%s", message);

			map_shared_memory();
		}

		return retval;
	}
};

GST_DEBUG_CATEGORY_STATIC(gst_shm_buffer_pool_debug);
#define GST_CAT_DEFAULT gst_shm_buffer_pool_debug

/**
* gst_buffer_pool_config_set_video_alignment:
* @config: a #GstStructure
* @align: a #GstVideoAlignment
*
* Set the video alignment in @align to the bufferpool configuration
* @config
*/
void gst_shm_buffer_pool_config_set_video_alignment(GstStructure * config, GstVideoAlignment * align)
{
	g_return_if_fail(config != NULL);
	g_return_if_fail(align != NULL);

	gst_structure_set(config,
		"padding-top", G_TYPE_UINT, align->padding_top,
		"padding-bottom", G_TYPE_UINT, align->padding_bottom,
		"padding-left", G_TYPE_UINT, align->padding_left,
		"padding-right", G_TYPE_UINT, align->padding_right,
		"stride-align0", G_TYPE_UINT, align->stride_align[0],
		"stride-align1", G_TYPE_UINT, align->stride_align[1],
		"stride-align2", G_TYPE_UINT, align->stride_align[2],
		"stride-align3", G_TYPE_UINT, align->stride_align[3], NULL);
}

/**
* gst_buffer_pool_config_get_video_alignment:
* @config: a #GstStructure
* @align: a #GstVideoAlignment
*
* Get the video alignment from the bufferpool configuration @config in
* in @align
*
* Returns: %TRUE if @config could be parsed correctly.
*/
gboolean gst_shm_buffer_pool_config_get_video_alignment(GstStructure * config,	GstVideoAlignment * align)
{
	g_return_val_if_fail(config != NULL, FALSE);
	g_return_val_if_fail(align != NULL, FALSE);

	return gst_structure_get(config,
		"padding-top", G_TYPE_UINT, &align->padding_top,
		"padding-bottom", G_TYPE_UINT, &align->padding_bottom,
		"padding-left", G_TYPE_UINT, &align->padding_left,
		"padding-right", G_TYPE_UINT, &align->padding_right,
		"stride-align0", G_TYPE_UINT, &align->stride_align[0],
		"stride-align1", G_TYPE_UINT, &align->stride_align[1],
		"stride-align2", G_TYPE_UINT, &align->stride_align[2],
		"stride-align3", G_TYPE_UINT, &align->stride_align[3], NULL);
}

/* bufferpool */
struct _GstShmBufferPoolPrivate
{
	GstCaps* caps;
	GstVideoInfo info;
	GstVideoAlignment video_align;	
	gboolean add_videometa;
	gboolean need_alignment;
	GstAllocationParams params;

	GMutex mutex;
	gchar* session_name;
	guint minimum_pool_size;
	guint minimum_buffer_size;

	gst_shared_memory_allocation_manager* allocation_manager;
};

static void gst_shm_buffer_pool_finalize(GObject * object);

#if GLIB_VERSION_RULE()
#define GST_SHM_BUFFER_POOL_GET_PRIVATE(obj)  \
    (GstShmBufferPoolPrivate*)(gst_shm_buffer_pool_get_instance_private(obj))
#else
#define GST_SHM_BUFFER_POOL_GET_PRIVATE(obj)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GST_TYPE_SHM_BUFFER_POOL, GstShmBufferPoolPrivate))
#endif

#if GLIB_VERSION_RULE()
#define gst_shm_buffer_pool_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstShmBufferPool, gst_shm_buffer_pool, GST_TYPE_BUFFER_POOL,
    G_ADD_PRIVATE(GstShmBufferPool)
    GST_DEBUG_CATEGORY_INIT(gst_shm_buffer_pool_debug, "elbitshmpool", 0,
        "elbitshmpool object"))
#else
#define gst_shm_buffer_pool_parent_class parent_class
G_DEFINE_TYPE(GstShmBufferPool, gst_shm_buffer_pool, GST_TYPE_BUFFER_POOL)
#endif

static const gchar** shared_memory_pool_get_options(GstBufferPool * pool)
{
	static const gchar *options[] = 
	{	GST_BUFFER_POOL_OPTION_VIDEO_META,
		GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT, NULL
	};

	return options;
}

static gboolean shared_memory_pool_set_config(GstBufferPool * pool, GstStructure * config)
{
	GstShmBufferPool *vpool = GST_SHM_BUFFER_POOL_CAST(pool);
	GstShmBufferPoolPrivate *priv = vpool->priv;
	GstVideoInfo info;
	GstCaps *caps;
	guint size, min_buffers, max_buffers;
	gint width, height;

	if (!gst_buffer_pool_config_get_params(config, &caps, &size, &min_buffers,
		&max_buffers))
		goto wrong_config;

	if (caps == NULL)
		goto no_caps;

	/* now parse the caps from the config */
	if (!gst_video_info_from_caps(&info, caps))
		goto wrong_caps;

	if (priv->allocation_manager != NULL &&
		memcmp(&info, &(priv->info), sizeof(GstVideoInfo)) == 0)
		return TRUE;

	if (size < info.size)
		size = static_cast<guint>(info.size);

	if (size == 0)
		goto wrong_size;

	width = info.width;
	height = info.height;

	GST_LOG_OBJECT(pool, "%dx%d, caps %" GST_PTR_FORMAT, width, height, caps);

	if (priv->caps)
		gst_caps_unref(priv->caps);
	priv->caps = gst_caps_ref(caps);

	/* enable metadata based on config of the pool */
	priv->add_videometa =
		gst_buffer_pool_config_has_option(config,
			GST_BUFFER_POOL_OPTION_VIDEO_META);

	/* parse extra alignment info */
	priv->need_alignment = gst_buffer_pool_config_has_option(config,
		GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

	if (priv->need_alignment && priv->add_videometa) 
	{
		guint max_align, n;
		gst_buffer_pool_config_get_video_alignment(config, &priv->video_align);

		/* ensure GstAllocationParams alignment is compatible with video alignment */
		max_align = (guint)(priv->params.align);
		for (n = 0; n < GST_VIDEO_MAX_PLANES; ++n)
			max_align |= priv->video_align.stride_align[n];

		for (n = 0; n < GST_VIDEO_MAX_PLANES; ++n)
			priv->video_align.stride_align[n] = max_align;

#if (GST_VERSION_MINOR >= 12)
        /* apply the alignment to the info */
        if (!gst_video_info_align(&info, &priv->video_align))
            goto failed_to_align;
#else
        gst_video_info_align(&info, &priv->video_align);
#endif

		gst_buffer_pool_config_set_video_alignment(config, &priv->video_align);

		if (priv->params.align < max_align) {
			GST_WARNING_OBJECT(pool, "allocation params alignment %u is smaller "
				"than the max specified video stride alignment %u, fixing",
				(guint)priv->params.align, max_align);
			priv->params.align = max_align;
		}
	}

	priv->info = info;

	if (priv->allocation_manager != nullptr)
		priv->allocation_manager->release();

	max_buffers = (std::max)(priv->minimum_pool_size, max_buffers);
	priv->allocation_manager = new gst_shared_memory_allocation_manager(
		pool,
		priv->session_name,
		max_buffers,
		static_cast<uint32_t>(priv->params.align),
		static_cast<uint32_t>(priv->params.prefix),
        static_cast<uint32_t>(priv->params.padding),
        static_cast<int32_t>(size),
		priv->minimum_buffer_size,
		info);

	gst_buffer_pool_config_set_params(config, caps, size, min_buffers, max_buffers);	
	if (!GST_BUFFER_POOL_CLASS(parent_class)->set_config(pool, config))
		return FALSE;

	return TRUE;

	/* ERRORS */
wrong_config:
	{
		GST_WARNING_OBJECT(pool, "invalid config");
		return FALSE;
	}
no_caps:
	{
		GST_WARNING_OBJECT(pool, "no caps in config");
		return FALSE;
	}
wrong_caps:
	{
		GST_WARNING_OBJECT(pool,
			"failed getting geometry from caps %" GST_PTR_FORMAT, caps);
		return FALSE;
	}
wrong_size:
	{
		GST_WARNING_OBJECT(pool,
			"Provided size is to small for the caps: %u", size);
		return FALSE;
    }
#if (GST_VERSION_MINOR >= 12)
failed_to_align:
    {
        GST_WARNING_OBJECT(pool, "Failed to align");
        return FALSE;
    }
#endif
}

static GstFlowReturn shared_memory_pool_alloc(GstBufferPool * pool, GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
	return GST_FLOW_OK;
}

GstFlowReturn shared_memory_pool_aquire_buffer(
	GstBufferPool* pool,
	GstBuffer** buffer,
	GstBufferPoolAcquireParams *params)
{
	GstShmBufferPool *spool = GST_SHM_BUFFER_POOL_CAST(pool);
	GstShmBufferPoolPrivate *priv = spool->priv;

	if (priv->allocation_manager == NULL)
	{
		if (priv->caps != NULL)
		{
			GstStructure* config = gst_buffer_pool_get_config(pool);
			gst_buffer_pool_config_add_option(config, GST_SHM_BUFFER_POOL_OPTION_VIDEO_META);
			gst_buffer_pool_config_set_params(config, priv->caps, 0, 0, 0);
			if (!shared_memory_pool_set_config(pool, config))
				return GST_FLOW_ERROR;
		}
		else
		{
			return GST_FLOW_ERROR;
		}
	}

	if (priv->allocation_manager->aquire(buffer) == false)
		return GST_FLOW_NOT_LINKED;

	(*buffer)->pool = pool;

	return GST_FLOW_OK;
}

void shared_memory_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer)
{
	GstShmBufferPool *spool = GST_SHM_BUFFER_POOL_CAST(pool);
	GstShmBufferPoolPrivate *priv = spool->priv;

	if (priv->allocation_manager->free(buffer) == false)
	{
        g_print("Cleaning leftovers after shared memeory pool was re-mapped...\n");
		return;
	}

	//return GST_BUFFER_POOL_CLASS(parent_class)->release_buffer(pool, buffer);
}

GstBufferPool* gst_shm_buffer_pool_new(const gchar* session_name, guint minimum_pool_size, guint minimum_buffer_size, GstCaps* caps)
{
	GstShmBufferPool* pool = GST_SHM_BUFFER_POOL_CAST(g_object_new(GST_TYPE_SHM_BUFFER_POOL, NULL));
	gst_object_ref_sink(pool);

	GstShmBufferPoolPrivate* priv = pool->priv;

	if (session_name == NULL)
	{
		GST_ERROR_OBJECT(pool, "session-name is invalid");		
		shared_memory::guid_generator generator;
		std::stringstream session_name_stream;
		session_name_stream << generator.generate();

		const char* message =
			"\nelbitshmsink: You did NOT supply a session-name so we generated one for you.\n" \
			"Note that this is not a common usecase.\n" \
			"Unless you know what you're doing,\n" \
            "please set the 'session-name' property before starting the pipeline containing 'elbitshmsink'.\n";

        std::string sname = session_name_stream.str();
        sname.erase(std::remove_if(sname.begin(), sname.end(), static_cast<int(&)(int)>(std::isspace)), sname.end());

        g_print("%s", message);
        GST_WARNING_OBJECT(pool, "%s", message);
        g_print("Session name is - %s\n\n", sname.c_str());

		priv->session_name = g_strdup(sname.c_str());
	}
	else
    {
        g_print("elbitshmsink: Session name is - %s\n", session_name);
		priv->session_name = g_strdup(session_name);
	}

	
	priv->minimum_pool_size = minimum_pool_size;
	priv->minimum_buffer_size = minimum_buffer_size;
	if (caps != NULL)
		priv->caps = gst_caps_ref(caps);

	GST_LOG_OBJECT(pool, "new shared memory buffer pool %p", pool);

	return GST_BUFFER_POOL_CAST(pool);
}

void gst_shm_buffer_pool_publish(GstBufferPool* pool, GstBuffer* buffer)
{
	if (pool == NULL)
		return;

	if (buffer == NULL)
		return;

	GstBuffer* source_buffer = buffer;
	GstShmBufferPool* spool = GST_SHM_BUFFER_POOL_CAST(pool);
	GstShmBufferPoolPrivate* priv = spool->priv;

	if (priv->allocation_manager == NULL)
	{
		if (priv->caps != NULL)
		{
			GstStructure* config = gst_buffer_pool_get_config(pool);
			gst_buffer_pool_config_add_option(config, GST_SHM_BUFFER_POOL_OPTION_VIDEO_META);
			gst_buffer_pool_config_set_params(config, priv->caps, 0, 0, 0);
			if (!gst_buffer_pool_set_config(pool, config))
				return;
		}
		else
		{
			return;
		}
	}

	gboolean buffer_copy = FALSE;
	if (pool != buffer->pool)
	{
		// The connected source does not respect our sink's allocation proposal!
		// We know that because the buffer's pool is not the same as ours.
        // We need to aquire a buffer from the pool, mem-copy the data into it and return it to the pool after publishing

        static gsize once_flag = 0;

        if (g_once_init_enter(&once_flag))
		{
			const gchar* message = 
				"elbitshmsink: Warning! The connected source does not respect our sink's allocation proposal!\n" \
				"We know that because the buffer's pool is not the same as ours.\n" \
				"nWe need to aquire a buffer from the pool, mem - copy the data into it and return it to the pool after publishing\n";

            g_print("%s", message);
            GST_WARNING_OBJECT(pool, "%s", message);

            g_once_init_leave (&once_flag, 1);
        }

		GstBuffer* pool_buf = NULL;		
		if (priv->allocation_manager->aquire(&pool_buf) == false)
			return;

		GstMapInfo write_map;
		if (gst_buffer_map(pool_buf, &write_map, GstMapFlags::GST_MAP_WRITE) == FALSE)
			return;

		GstMapInfo read_map;
		if (gst_buffer_map(buffer, &read_map, GstMapFlags::GST_MAP_READ) == FALSE)
		{
			gst_buffer_unmap(pool_buf, &write_map);
			return;
		}

		memcpy(write_map.data, read_map.data, read_map.size);
		
		gst_buffer_unmap(buffer, &read_map);
		gst_buffer_unmap(pool_buf, &write_map);

		pool_buf->dts = buffer->dts;
		pool_buf->pts = buffer->pts;
		pool_buf->duration = buffer->duration;
		pool_buf->offset = buffer->offset;
		pool_buf->offset_end = buffer->offset_end;

		buffer = pool_buf;
		buffer_copy = TRUE;
	}

	priv->allocation_manager->publish(source_buffer, buffer);

	if (buffer_copy)
		priv->allocation_manager->free(buffer);
}

static void gst_shm_buffer_pool_class_init(GstShmBufferPoolClass * klass)
{
	GObjectClass *gobject_class = (GObjectClass *)klass;
	GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *)klass;

#if !GLIB_VERSION_RULE()
    g_type_class_add_private(klass, sizeof(GstShmBufferPoolPrivate));
#endif

	gobject_class->finalize = gst_shm_buffer_pool_finalize;

	gstbufferpool_class->get_options = shared_memory_pool_get_options;
	gstbufferpool_class->set_config = shared_memory_pool_set_config;
	gstbufferpool_class->alloc_buffer = shared_memory_pool_alloc;
	gstbufferpool_class->acquire_buffer = shared_memory_pool_aquire_buffer;
    gstbufferpool_class->release_buffer = shared_memory_pool_release_buffer;

#if !GLIB_VERSION_RULE()
    GST_DEBUG_CATEGORY_INIT(gst_shm_buffer_pool_debug, "elbitshmpool", 0,
        "elbitshmpool object");
#endif
}

static void gst_shm_buffer_pool_init(GstShmBufferPool * pool)
{
	pool->priv = GST_SHM_BUFFER_POOL_GET_PRIVATE(pool);
	
	g_mutex_init(&pool->priv->mutex);
	pool->priv->allocation_manager = NULL;
}

static void gst_shm_buffer_pool_finalize(GObject * object)
{
	GstShmBufferPool *pool = GST_SHM_BUFFER_POOL_CAST(object);
	GstShmBufferPoolPrivate *priv = pool->priv;

	GST_LOG_OBJECT(pool, "finalize buffer pool %p", pool);

	g_mutex_clear(&priv->mutex);

	if (priv->caps)
		gst_caps_unref(priv->caps);

	if (priv->session_name != NULL)
		g_free(priv->session_name);

	if (priv->allocation_manager != NULL)
		priv->allocation_manager->release();

	//G_OBJECT_CLASS(gst_shm_buffer_pool_parent_class)->finalize(object);
}

// Intel C++ does not properly keep warning state for function templates,
// so popping warning state at the end of translation unit leads to warnings in the middle.
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif
