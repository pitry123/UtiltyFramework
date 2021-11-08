#pragma once
#include <core/communication.h>
#include <boost/asio.hpp>
#include <cstring>
#include <cstdint>

namespace communication
{
	namespace helpers
	{
		static inline bool convert_ip_address(const ::boost::asio::ip::address& from, core::communication::ip_address& to)
		{
			to = {};

			if (from.is_v4() == true)
			{
				to.type = core::communication::ip_address_type::IP_VER4;

#ifdef _WIN32
				memcpy_s(
					to.val,
					from.to_v4().to_bytes().size(),
					from.to_v4().to_bytes().data(),
					from.to_v4().to_bytes().size());
#else
				std::memcpy(
					to.val,
					from.to_v4().to_bytes().data(),
					from.to_v4().to_bytes().size());
#endif
			}
			else if (from.is_v6() == true)
			{
				to.type = core::communication::ip_address_type::IP_VER6;

#ifdef _WIN32
				memcpy_s(
					to.val,
					from.to_v6().to_bytes().size(),
					from.to_v6().to_bytes().data(),
					from.to_v6().to_bytes().size());
#else
				std::memcpy(
					to.val,
					from.to_v6().to_bytes().data(),
					from.to_v6().to_bytes().size());
#endif

				to.scope_id = static_cast<uint32_t>(from.to_v6().scope_id());
			}
			else
			{
				return false;
			}

			return true;
		}

		template <typename IP_PROTOCOL>
		static inline bool convert_ip_endpoint(const ::boost::asio::ip::basic_endpoint<IP_PROTOCOL>& from, core::communication::ip_endpoint& to)
		{
			to.port = static_cast<uint16_t>(from.port());
			return communication::helpers::convert_ip_address(from.address(), to.address);
		}
	}
}