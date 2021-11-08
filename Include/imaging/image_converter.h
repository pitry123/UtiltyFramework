#pragma once
#include <core/imaging.h>

namespace imaging
{
    class DLL_EXPORT image_converter : public core::imaging::image_algorithm_interface
    {
    public:
        virtual ~image_converter() = default;
        static bool create(core::imaging::pixel_format target_format, uint32_t target_width, uint32_t target_heigh, core::imaging::image_algorithm_interface** algo);
        static bool create(core::imaging::pixel_format target_format, core::imaging::image_algorithm_interface** algo);
    };
}
