#pragma once
#include <imaging/image_utils.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_object_pool.hpp>
#include <utils/buffer_allocator.hpp>
#include <utils/imaging.hpp>

// opencv api
#define CV_IGNORE_DEBUG_BUILD_GUARD

// USE GPU accelaration (OpenCL) on
// OpenCV 3 and above
#define USE_GPU

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4996)
#include <opencv2/core/ocl.hpp>
#include <opencv2/opencv.hpp>
#pragma warning( pop )
#else
#ifndef __clang_analyzer__
#include <opencv2/core/version.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#if defined(CV_VERSION_EPOCH) && (CV_VERSION_EPOCH < 3)
    #define OPENCV_VERSION_2
#else
#if (CV_VERSION_MAJOR >= 4)
    #define OPENCV_VERSION_4
#endif
#endif

#ifdef OPENCV_VERSION_2
#ifdef USE_GPU
namespace cv
{
    using UMat = cv::Mat;
}
#endif
#else
    #include <opencv2/core/ocl.hpp>
#endif

#endif
#endif

namespace imaging
{
	class image_undistort_impl : public utils::ref_count_base<imaging::image_undistort>
	{
	private:
#ifndef __clang_analyzer__
#ifdef USE_GPU
		cv::UMat m_map1;
		cv::UMat m_map2;

		cv::UMat m_gpu_source;
		cv::UMat m_gpu_convertion;
		cv::UMat m_gpu_convertion_helper;
		cv::UMat m_gpu_target;
#else
		cv::Mat m_map1;
		cv::Mat m_map2;	
#endif
#endif

		utils::ref_count_object_pool<utils::ref_count_buffer> m_buffer_pool;
		utils::ref_count_object_pool<utils::imaging::ref_count_image> m_image_pool;

	public:
		image_undistort_impl(
			uint32_t width, uint32_t height,
			float(&camera_matrix_chess)[3][3],
			float(&distCoeffs_chess)[5]);
		virtual ~image_undistort_impl() = default;

		virtual bool apply(core::imaging::image_interface* input, core::imaging::image_interface** output) override;
	};
}
