#pragma once
#include <core/imaging.h>

#include <Common.hpp>

namespace Imaging
{
	using PixelFormat = core::imaging::pixel_format;

	template <typename T>
	class ImageTemplate : public Common::CoreObjectWrapper<T>
	{
	public:
		ImageTemplate()
		{
			// Empty ImageTemplate
		}

		ImageTemplate(T* image) :
			Common::CoreObjectWrapper<T>(image)
		{
		}

		virtual ~ImageTemplate() = default;

		size_t Size() const
		{
            this->ThrowOnEmpty("Imaging::ImageTemplate");

			utils::ref_count_ptr<core::buffer_interface> buffer;
            if (this->m_core_object->query_buffer(&buffer) == false)
				return 0;

			return buffer->size();
		}

		uint8_t* Buffer() const
		{
            this->ThrowOnEmpty("Imaging::ImageTemplate");

			utils::ref_count_ptr<core::buffer_interface> buffer;
            if (this->m_core_object->query_buffer(&buffer) == false)
				return nullptr;

			return buffer->data();
		}

		uint32_t Width() const
		{
            this->ThrowOnEmpty("Imaging::ImageTemplate");

			core::imaging::image_params image_params = {};
            if (this->m_core_object->query_image_params(image_params) == false)
				return 0;

			return image_params.width;
		}

		uint32_t Height() const
		{
            this->ThrowOnEmpty("Imaging::ImageTemplate");

			core::imaging::image_params image_params = {};
            if (this->m_core_object->query_image_params(image_params) == false)
				return 0;

			return image_params.height;
		}

		Imaging::PixelFormat Format() const
		{
            this->ThrowOnEmpty("Imaging::ImageTemplate");

			core::imaging::image_params image_params = {};
            if (this->m_core_object->query_image_params(image_params) == false)
				return core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;

			return image_params.format;
		}
	};	


	class Image : public ImageTemplate<core::imaging::image_interface>
	{
	public:
		Image()
		{
			// Empty ImageTemplate
		}

		Image(core::imaging::image_interface* image) :
			ImageTemplate<core::imaging::image_interface>(image)
		{
		}
	};

	class ImageAlgorithm : public Common::CoreObjectWrapper<core::imaging::image_algorithm_interface>
	{
	public:
		ImageAlgorithm()
		{
			// Empty ImageAlgorithm
		}

		ImageAlgorithm(core::imaging::image_algorithm_interface* algorithm) :
			Common::CoreObjectWrapper<core::imaging::image_algorithm_interface>(algorithm)
		{
		}

		Imaging::Image Apply(const Imaging::Image& input)
		{
			ThrowOnEmpty("Imaging::ImageAlgorithm");

			utils::ref_count_ptr<core::imaging::image_interface> output;
			if (m_core_object->apply(static_cast<core::imaging::image_interface*>(input), &output) == false)
				return Imaging::Image(); // Empty Image

			return Imaging::Image(output);
		}
	};
}
