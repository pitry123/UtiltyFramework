/// @file	core/imaging.h.
/// @brief	Declares the image interface class
#pragma once
#include <core/buffer_interface.h>

namespace core
{
	namespace imaging
	{
		/// @enum	pixel_format
		/// @brief	Values that represent pixel formats
		enum pixel_format
		{
			RGB,
			RGBA,
			BGR,
			BGRA,

			I420,
			NV12,
			YUY2,
            UYVY,

			GRAY8,
			GRAY16_LE,

			UNDEFINED_PIXEL_FORMAT
		};

#pragma pack(1)
		/// @struct	image_params
		/// @brief	An image parameters structure.
		/// @date	14/05/2018
		struct DLL_EXPORT image_params
		{
			/// @brief	The image width
			uint32_t width;
			/// @brief	The image height
			uint32_t height;
			/// @brief	The image size in bytes
			uint32_t size;
			/// @brief	The pixel-format
			pixel_format format; //pixel_format t:int
		};
#pragma pack()

		/// @class	image_interface
		/// @brief	An interface defining an image.
		/// @date	14/05/2018
		class DLL_EXPORT image_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual image_interface::~image_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~image_interface() = default;

			/// @fn	virtual bool image_interface::query_image_params(core::imaging::image_params& image_params) const = 0;
			/// @brief	Queries the image parameters
			/// @date	14/05/2018
			/// @param	[out]	image_params	The result image parameters.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_image_params(core::imaging::image_params& image_params) const = 0;

			/// @fn	virtual bool image_interface::query_buffer(core::buffer_interface** buffer) const = 0;
			/// @brief	Queries the image buffer
			/// @date	14/05/2018
			/// @param [out]	buffer	The buffer interface pointer address.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_buffer(core::buffer_interface** buffer) const = 0;
		};

		class DLL_EXPORT image_algorithm_interface : public core::ref_count_interface
		{
		public:
			virtual ~image_algorithm_interface() = default;
			virtual bool apply(core::imaging::image_interface* input, core::imaging::image_interface** output) = 0;
		};
	}
}