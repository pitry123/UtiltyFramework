#include "image_converter.h"

#ifdef OPENCV_VERSION_4
    #include <opencv2/imgproc/types_c.h>
#endif

imaging::image_converter_impl::image_converter_impl(
		core::imaging::pixel_format format,
		uint32_t width,
		uint32_t height) :
	m_buffer_pool(POOLS_INITIAL_SIZE,
		utils::ref_count_object_pool<utils::ref_count_buffer>::growing_mode::doubling,
		true,
		BUFFER_MAX_SIZE),
	m_image_pool(POOLS_INITIAL_SIZE,
		utils::ref_count_object_pool<utils::imaging::ref_count_image>::growing_mode::doubling,
		true),
	m_format(format),
	m_width(width),
	m_height(height)
{
    if (format != core::imaging::pixel_format::RGB &&
        format != core::imaging::pixel_format::BGR &&
        format != core::imaging::pixel_format::RGBA &&
        format != core::imaging::pixel_format::BGRA &&
		format != core::imaging::pixel_format::GRAY8)
    {
        throw std::invalid_argument("format: target is not supported");
    }
}

bool imaging::image_converter_impl::apply(
	core::imaging::image_interface* input, 
	core::imaging::image_interface** output)
{
	if (input == nullptr)
		return false;

	if (output == nullptr)
		return false;

	core::imaging::image_params input_image_params;
	if (input->query_image_params(input_image_params) == false)
		return false;

	utils::ref_count_ptr<core::buffer_interface> source_buffer;
	if (input->query_buffer(&source_buffer) == false)
		return false;

	core::imaging::pixel_format output_format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;

#ifdef USE_GPU

	if (m_format == core::imaging::pixel_format::RGB)
	{
		if (input_image_params.format == core::imaging::pixel_format::I420)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGB_I420);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::NV12)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGB_NV12);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::YUY2)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGB_YUY2);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::UYVY)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGB_Y422);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGB)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
			source.copyTo(m_gpu_source);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGBA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGBA2RGB);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGR)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGR2RGB);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGRA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGRA2RGB);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2RGB);
			output_format = core::imaging::pixel_format::RGB;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion_helper);
			cv::convertScaleAbs(m_gpu_convertion_helper, m_gpu_convertion, 255.0 / 65535.0);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2RGB);
			output_format = core::imaging::pixel_format::RGB;
		}
		else
		{
			return false;
		}

		if (output_format != core::imaging::pixel_format::RGB)
			return false;
	}
	else if (m_format == core::imaging::pixel_format::BGR)
	{
		if (input_image_params.format == core::imaging::pixel_format::I420)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGR_I420);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::NV12)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGR_NV12);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::YUY2)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGR_YUY2);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::UYVY)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGR_Y422);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGB)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
			source.copyTo(m_gpu_source);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGRA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGRA2BGR);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGR)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
			source.copyTo(m_gpu_source);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGBA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGBA2BGR);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2BGR);
			output_format = core::imaging::pixel_format::BGR;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
			source.copyTo(m_gpu_convertion_helper);
			cv::convertScaleAbs(m_gpu_convertion_helper, m_gpu_convertion, 255.0 / 65535.0);
			cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2BGR);
			output_format = core::imaging::pixel_format::BGR;
		}
		else
		{
			return false;
		}

		if (output_format != core::imaging::pixel_format::BGR)
			return false;
	}
    else if (m_format == core::imaging::pixel_format::RGBA)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGBA_I420);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGBA_NV12);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGBA_YUY2);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2RGBA_Y422);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
            source.copyTo(m_gpu_source);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGB2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGR2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGRA2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion_helper);
            cv::convertScaleAbs(m_gpu_convertion_helper, m_gpu_convertion, 255.0 / 65535.0);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::RGBA)
            return false;
    }
    else if (m_format == core::imaging::pixel_format::BGRA)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGRA_I420);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGRA_NV12);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGRA_YUY2);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2BGRA_Y422);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
            source.copyTo(m_gpu_source);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGB2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGR2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGBA2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            source.copyTo(m_gpu_convertion_helper);
            cv::convertScaleAbs(m_gpu_convertion_helper, m_gpu_convertion, 255.0 / 65535.0);
            cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_GRAY2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::BGRA)
            return false;
    }
	else if (m_format == core::imaging::pixel_format::GRAY8)
	{
	if (input_image_params.format == core::imaging::pixel_format::I420)
	{
        cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2GRAY_I420);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::NV12)
	{
        cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2GRAY_NV12);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::YUY2)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2GRAY_YUY2);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::UYVY)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_YUV2GRAY_Y422);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::RGB)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGB2GRAY);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::BGRA)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGRA2GRAY);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::BGR)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGR2GRAY);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::RGBA)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
		source.copyTo(m_gpu_convertion);
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_RGBA2GRAY);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());
		source.copyTo(m_gpu_source);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
        cv::Mat target(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1);
		cv::convertScaleAbs(source, target, 255.0 / 65535.0);
		target.copyTo(m_gpu_source);
		output_format = core::imaging::pixel_format::GRAY8;
	}
	else
	{
		return false;
	}

	if (output_format != core::imaging::pixel_format::GRAY8)
		return false;
	}
	else
	{
		return false;
	}

    utils::ref_count_ptr<utils::ref_count_buffer> target_buffer;
    if (m_buffer_pool.get_item(&target_buffer) == false)
        throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");

    bool resizing = (m_width > 0 && m_height > 0 &&
                    (m_width != input_image_params.width || m_height != input_image_params.height));

    uint32_t width = input_image_params.width;;
    uint32_t height = input_image_params.height;
    if (resizing == true)
    {
        width = m_width;
        height = m_height;
    }

    cv::Mat target;
    uint32_t bpp = 0;
    if (m_format == core::imaging::pixel_format::BGR ||
        m_format == core::imaging::pixel_format::RGB)
    {
        target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC3, target_buffer->data());
		bpp = 3;
    }
	else if(m_format == core::imaging::pixel_format::BGRA ||
		m_format == core::imaging::pixel_format::RGBA)
    {
        target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC4, target_buffer->data());
		bpp = 4;
    }
	else // GRAY8
	{
		target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC1, target_buffer->data());
		bpp = 1;
	}	

    if (resizing)
    {
        // Resize the image to the target width hight
        cv::resize(m_gpu_source, m_gpu_after_resize, cv::Size(static_cast<int>(width), static_cast<int>(height)), 0.0, 0.0, cv::INTER_LINEAR);
        m_gpu_after_resize.copyTo(target);        
    }
    else
    {
        m_gpu_source.copyTo(target);
    }
	
	utils::ref_count_ptr<utils::imaging::ref_count_image> instance;
	if (m_image_pool.get_item(&instance) == false)
		throw std::runtime_error("Failed to allocate output image. Out of memory?");

	instance->reset(core::imaging::image_params{ 
        width,
        height,
        width * height * bpp,
		output_format }, 
        target_buffer);

	*output = instance;
	(*output)->add_ref();
	return true;

#else //USE_GPU

    utils::ref_count_ptr<core::buffer_interface> converted_buffer;

    if (m_format == core::imaging::pixel_format::RGB)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGB_I420);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGB_NV12);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGB_YUY2);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGB_YUY2);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            converted_buffer = source_buffer;
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGBA2RGB);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGR2RGB);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGRA2RGB);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2RGB);
            output_format = core::imaging::pixel_format::RGB;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
            if (m_buffer_pool.get_item(&grayscale_buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, grayscale_buffer->data());
            cv::convertScaleAbs(temp, source, 255.0 / 65535.0);

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2RGB);
            output_format = core::imaging::pixel_format::RGB;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::RGB)
            return false;
    }
    else if (m_format == core::imaging::pixel_format::BGR)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGR_I420);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGR_NV12);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGR_YUY2);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGR_YUY2);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            converted_buffer = source_buffer;
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGBA2BGR);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGB2BGR);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGRA2BGR);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2BGR);
            output_format = core::imaging::pixel_format::BGR;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
            if (m_buffer_pool.get_item(&grayscale_buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, grayscale_buffer->data());
            cv::convertScaleAbs(temp, source, 255.0 / 65535.0);

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2BGR);
            output_format = core::imaging::pixel_format::BGR;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::BGR)
            return false;
    }
    if (m_format == core::imaging::pixel_format::RGBA)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGBA_I420);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGBA_NV12);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGBA_YUY2);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2RGBA_YUY2);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            converted_buffer = source_buffer;
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGB2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGR2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGRA2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
            if (m_buffer_pool.get_item(&grayscale_buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, grayscale_buffer->data());
            cv::convertScaleAbs(temp, source, 255.0 / 65535.0);

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2RGBA);
            output_format = core::imaging::pixel_format::RGBA;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::RGBA)
            return false;
    }
    if (m_format == core::imaging::pixel_format::BGRA)
    {
        if (input_image_params.format == core::imaging::pixel_format::I420)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGRA_I420);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::NV12)
        {
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGRA_NV12);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::YUY2)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGRA_YUY2);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::UYVY)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_YUV2BGRA_YUY2);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGRA)
        {
            converted_buffer = source_buffer;
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGB)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGB2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::BGR)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_BGR2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::RGBA)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_RGBA2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
        {
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
        {
            cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
            utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
            if (m_buffer_pool.get_item(&grayscale_buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, grayscale_buffer->data());
            cv::convertScaleAbs(temp, source, 255.0 / 65535.0);

            utils::ref_count_ptr<utils::ref_count_buffer> buffer;
            if (m_buffer_pool.get_item(&buffer) == false)
                throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
            converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());

            cv::cvtColor(source, cpu_source, CV_GRAY2BGRA);
            output_format = core::imaging::pixel_format::BGRA;
        }
        else
        {
            return false;
        }

        if (output_format != core::imaging::pixel_format::BGRA)
            return false;
    }
	if (m_format == core::imaging::pixel_format::GRAY8)
	{
		if (input_image_params.format == core::imaging::pixel_format::I420)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_YUV2GRAY_I420);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::NV12)
		{
            cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_YUV2GRAY_NV12);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::YUY2)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_YUV2GRAY_YUY2);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::UYVY)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC2, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_YUV2GRAY_YUY2);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGRA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_BGRA2GRAY);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGB)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_RGB2GRAY);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::BGR)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_BGR2GRAY);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::RGBA)
		{
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());

			utils::ref_count_ptr<utils::ref_count_buffer> buffer;
			if (m_buffer_pool.get_item(&buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
			converted_buffer = buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());

			cv::cvtColor(source, cpu_source, CV_RGBA2GRAY);
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
		{
		converted_buffer = source_buffer;
		output_format = core::imaging::pixel_format::GRAY8;
		}
		else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
		{
            cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
			utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
			if (m_buffer_pool.get_item(&grayscale_buffer) == false)
				throw std::runtime_error("Failed to allocate buffer for image conversion. Out of memory?");
            cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, grayscale_buffer->data());
			cv::convertScaleAbs(temp, source, 255.0 / 65535.0);
			converted_buffer = grayscale_buffer;
            cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());
			output_format = core::imaging::pixel_format::GRAY8;
		}
		else
		{
			return false;
		}

		if (output_format != core::imaging::pixel_format::GRAY8)
			return false;
	}

	bool resizing = 
		(m_width > 0 && m_height > 0 && 
		(m_width != input_image_params.width || m_height != input_image_params.height));

	uint32_t width = input_image_params.width;
	uint32_t height = input_image_params.height;
	if (resizing == true)
	{
		width = m_width;
		height = m_height;
	}
    
    utils::ref_count_ptr<core::buffer_interface> target_buffer;
    if (resizing == true)
    {
		utils::ref_count_ptr<utils::ref_count_buffer> resize_buffer;
		if (m_buffer_pool.get_item(&resize_buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");

        cv::Mat cpu_source;
        cv::Mat cpu_target;
        if (m_format == core::imaging::pixel_format::RGB ||
            m_format == core::imaging::pixel_format::BGR)
        {
            cpu_source = cv::Mat(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, converted_buffer->data());
            cpu_target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC3, resize_buffer->data());
        }
        else if (m_format == core::imaging::pixel_format::RGBA ||
			m_format == core::imaging::pixel_format::BGRA)
        {
            cpu_source = cv::Mat(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());
            cpu_target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC4, resize_buffer->data());
        }
		else // GRAY8
		{
            cpu_source = cv::Mat(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, converted_buffer->data());
			cpu_target = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC1, resize_buffer->data());
		}

        cv::resize(cpu_source, cpu_target, cv::Size(static_cast<int>(width), static_cast<int>(height)), 0.0, 0.0, cv::INTER_LINEAR);
		target_buffer = resize_buffer;
    }
    else
    {
        target_buffer = converted_buffer;
    }
	
	utils::ref_count_ptr<utils::imaging::ref_count_image> instance;
	if (m_image_pool.get_item(&instance) == false)
		throw std::runtime_error("Failed to allocate output image. Out of memory?");

	cv::Mat target;
    uint32_t bpp = 0;
	if (m_format == core::imaging::pixel_format::BGR ||
		m_format == core::imaging::pixel_format::RGB)
	{
		bpp = 3;
	}
	else if (m_format == core::imaging::pixel_format::BGRA ||
		m_format == core::imaging::pixel_format::RGBA)
	{
		bpp = 4;
	}
	else // GRAY8
	{
		bpp = 1;
	}

	instance->reset(core::imaging::image_params{ 
        width,
        height,
        width * height * bpp,
		output_format }, 
        target_buffer);


	*output = instance;
	(*output)->add_ref();
	return true;

#endif //USE_GPU
}

bool imaging::image_converter::create(core::imaging::pixel_format target_format,
    uint32_t target_width,
    uint32_t target_heigh,
    core::imaging::image_algorithm_interface** algo)
{
	if (algo == nullptr)
		return false;

	utils::ref_count_ptr<core::imaging::image_algorithm_interface> instance;
	try
	{
        instance = utils::make_ref_count_ptr<imaging::image_converter_impl>(target_format, target_width, target_heigh);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*algo = instance;
	(*algo)->add_ref();
	return true;
}

bool imaging::image_converter::create(core::imaging::pixel_format target_format,
    core::imaging::image_algorithm_interface** algo)
{
    return imaging::image_converter::create(target_format, 0, 0, algo);
}
