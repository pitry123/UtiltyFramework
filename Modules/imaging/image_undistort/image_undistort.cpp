#include "image_undistort.h"
#include <utils/ref_count_ptr.hpp>

#include <cstddef>

#ifdef OPENCV_VERSION_4
    #include <opencv2/calib3d.hpp>
    #include <opencv2/imgproc/types_c.h>
#endif

static constexpr size_t POOLS_INITIAL_SIZE = 20;

imaging::image_undistort_impl::image_undistort_impl(
	uint32_t width,
	uint32_t height,
	float(&camera_matrix_chess)[3][3],
	float(&distCoeffs_chess)[5]) :
	m_buffer_pool(
		POOLS_INITIAL_SIZE,
		utils::ref_count_object_pool<utils::ref_count_buffer>::growing_mode::doubling,
		true,
		width * height * 4),
	m_image_pool(
		POOLS_INITIAL_SIZE,
		utils::ref_count_object_pool<utils::imaging::ref_count_image>::growing_mode::doubling,
		true)
{
#ifndef __clang_analyzer__
#ifdef USE_GPU
#ifndef OPENCV_VERSION_2
	// set below system variable in order to override OpenCL's default device
	/*if (_putenv("OPENCV_OPENCL_DEVICE=:GPU:0") != 0)
	{
		std::cerr << "Failed to set a desired OpenCL device" << std::endl;
	}*/

	cv::ocl::setUseOpenCL(true);
#endif
#endif

	cv::Mat camera_matrix = cv::Mat(3, 3, CV_32FC1, camera_matrix_chess);
	cv::Mat dist_coeffs = cv::Mat(1, 5, CV_32FC1, distCoeffs_chess);

    cv::initUndistortRectifyMap(camera_matrix, dist_coeffs, cv::Mat(), camera_matrix, cv::Size(static_cast<int>(width), static_cast<int>(height)), CV_32FC1, m_map1, m_map2);
#endif
}

bool imaging::image_undistort_impl::apply(core::imaging::image_interface* input, core::imaging::image_interface** output)
{
#ifndef __clang_analyzer__
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
		cv::cvtColor(m_gpu_convertion, m_gpu_source, CV_BGR2BGRA);
		output_format = core::imaging::pixel_format::BGRA;
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
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, source_buffer->data());
		source.copyTo(m_gpu_source);
		output_format = input_image_params.format;
	}
	
	cv::remap(m_gpu_source, m_gpu_target, m_map1, m_map2, cv::INTER_LINEAR);

	utils::ref_count_ptr<utils::ref_count_buffer> undistorted_buffer;
	if (m_buffer_pool.get_item(&undistorted_buffer) == false)
		throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
    cv::Mat target(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, undistorted_buffer->data());
	m_gpu_target.copyTo(target);

	utils::ref_count_ptr<utils::imaging::ref_count_image> instance;
	if (m_image_pool.get_item(&instance) == false)
		throw std::runtime_error("Failed to allocate output image. Out of memory?");

	instance->reset(core::imaging::image_params{ input_image_params.width,
			input_image_params.height,
			input_image_params.width * input_image_params.height * 4,
			output_format }, undistorted_buffer);	

	*output = instance;
	(*output)->add_ref();
	return true;
#else
	utils::ref_count_ptr<core::buffer_interface> converted_buffer;

	if (input_image_params.format == core::imaging::pixel_format::I420)
	{
        cv::Mat source(static_cast<int>(input_image_params.height + input_image_params.height / 2), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

		utils::ref_count_ptr<utils::ref_count_buffer> buffer;
		if (m_buffer_pool.get_item(&buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
		converted_buffer = buffer;
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

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
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

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
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

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
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

		cv::cvtColor(source, cpu_source, CV_YUV2RGBA_YUY2);
		output_format = core::imaging::pixel_format::RGBA;
	}
	else if (input_image_params.format == core::imaging::pixel_format::RGB)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC3, source_buffer->data());

		utils::ref_count_ptr<utils::ref_count_buffer> buffer;
		if (m_buffer_pool.get_item(&buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
		converted_buffer = buffer;
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

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
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

		cv::cvtColor(source, cpu_source, CV_BGR2BGRA);
		output_format = core::imaging::pixel_format::BGRA;
	}
	else if (input_image_params.format == core::imaging::pixel_format::GRAY8)
	{
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, source_buffer->data());

		utils::ref_count_ptr<utils::ref_count_buffer> buffer;
		if (m_buffer_pool.get_item(&buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
		converted_buffer = buffer;
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

		cv::cvtColor(source, cpu_source, CV_GRAY2RGBA);
		output_format = core::imaging::pixel_format::RGBA;
	}
	else if (input_image_params.format == core::imaging::pixel_format::GRAY16_LE)
	{
        cv::Mat temp(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_16UC1, source_buffer->data());
		utils::ref_count_ptr<utils::ref_count_buffer> grayscale_buffer;
		if (m_buffer_pool.get_item(&grayscale_buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
        cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC1, grayscale_buffer->data());
		cv::convertScaleAbs(temp, source, 255.0 / 65535.0);

		utils::ref_count_ptr<utils::ref_count_buffer> buffer;
		if (m_buffer_pool.get_item(&buffer) == false)
			throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");
		converted_buffer = buffer;
        cv::Mat cpu_source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());

		cv::cvtColor(source, cpu_source, CV_GRAY2RGBA);
		output_format = core::imaging::pixel_format::RGBA;
	}
	else
	{
		converted_buffer = source_buffer;
		output_format = input_image_params.format;
	}

	utils::ref_count_ptr<utils::ref_count_buffer> undistorted_buffer;
	if (m_buffer_pool.get_item(&undistorted_buffer) == false)
	throw std::runtime_error("Failed to allocate buffer for image un-distortion. Out of memory?");

    cv::Mat target(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, undistorted_buffer->data());
    cv::Mat source(static_cast<int>(input_image_params.height), static_cast<int>(input_image_params.width), CV_8UC4, converted_buffer->data());
	cv::remap(source, target, m_map1, m_map2, cv::INTER_LINEAR);

	utils::ref_count_ptr<utils::imaging::ref_count_image> instance;
	if (m_image_pool.get_item(&instance) == false)
	throw std::runtime_error("Failed to allocate output image. Out of memory?");

	instance->reset(core::imaging::image_params{ input_image_params.width,
			input_image_params.height,
			input_image_params.width * input_image_params.height * 4,
			output_format }, undistorted_buffer);

	*output = instance;
	(*output)->add_ref();
	return true;
#endif
#else
	return false;
#endif
}

bool imaging::image_undistort::create(
	uint32_t width, uint32_t height,
	float(&camera_matrix_chess)[3][3],
	float(&distCoeffs_chess)[5],
	core::imaging::image_algorithm_interface** algo)
{
	if (algo == nullptr)
		return false;

	utils::ref_count_ptr<core::imaging::image_algorithm_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<imaging::image_undistort_impl>(width, height, camera_matrix_chess, distCoeffs_chess);
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
