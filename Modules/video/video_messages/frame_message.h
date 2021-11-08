#pragma once
#include <video/messages/frame_message.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <utils/video.hpp>
#include <utils/buffer_allocator.hpp>

namespace video
{
	namespace messages
	{
		class frame_message_impl : public utils::ref_count_base<video::messages::frame_message>
		{
		private:
			utils::ref_count_ptr<core::video::frame_interface> m_frame;
			utils::ref_count_ptr<core::buffer_allocator> m_buffer_allocator;

		public:
			frame_message_impl()
			{
			}

			frame_message_impl(core::buffer_allocator* buffer_allocator) :
				m_buffer_allocator(buffer_allocator)
			{
			}

			frame_message_impl(core::video::frame_interface* frame, core::buffer_allocator* buffer_allocator) :
				m_frame(frame),
				m_buffer_allocator(buffer_allocator)
			{
				if (frame == nullptr)
					throw std::invalid_argument("frame");
			}

			frame_message_impl(const frame_message_impl& other) :
				m_frame(other.m_frame),
				m_buffer_allocator(other.m_buffer_allocator)
			{
			}

            virtual core::guid id() const override
			{
				return video::messages::frame_message::ID();
			}

            virtual bool clone(core::messaging::message_interface** msg) override
			{
				if (msg == nullptr)
					return false;

				utils::ref_count_ptr<core::messaging::message_interface> instance;
				try
				{
					instance = utils::make_ref_count_ptr<frame_message_impl>(*this);
				}
				catch (...)
				{
					return false;
				}

				if (instance == nullptr)
					return false;

				*msg = instance;
				(*msg)->add_ref();
				return true;
			}

			uint64_t data_size() const override
			{
				utils::video::frame_binary_serializer serializer(m_frame);
				return serializer.data_size();
			}

			virtual bool serialize(core::stream_interface& stream) const override
			{
				utils::video::frame_binary_serializer serializer(m_frame);
				return serializer.serialize(stream);
			}

			virtual bool deserialize(core::stream_interface& stream) override
			{
				utils::video::frame_binary_serializer serializer(m_buffer_allocator);

				if (serializer.deserialize(stream) == false)
					return false;

				utils::ref_count_ptr<core::video::frame_interface> frame;
				if (serializer.query_frame(&frame) == false)
					return false;

				m_frame = frame;
				return true;
			}

			virtual bool query_frame(core::video::frame_interface** frame) override
			{
				if (frame == nullptr)
					return false;

				if (m_frame == nullptr)
					return false;

				*frame = m_frame;
				(*frame)->add_ref();
				return true;
			}
		};
	}
}

