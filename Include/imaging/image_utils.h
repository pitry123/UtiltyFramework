#pragma once
#include <core/imaging.h>

namespace imaging
{    
    class DLL_EXPORT image_undistort : public core::imaging::image_algorithm_interface
    {
    public:
        virtual ~image_undistort() = default;				
		static bool create(
			uint32_t width, uint32_t height,
			float (&camera_matrix_chess)[3][3],
			float (&distCoeffs_chess)[5], 
			core::imaging::image_algorithm_interface** algo);
    };
}