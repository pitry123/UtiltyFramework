#pragma once
#include <core/ref_count_interface.h>
#include <cstdint>

namespace core
{
	namespace http
	{
		class DLL_EXPORT async_result_callback_interface : public core::ref_count_interface
		{
		public:
			virtual ~async_result_callback_interface() = default;
			virtual void on_result(const char* result) = 0;
		};

		class DLL_EXPORT async_result_interface : public core::ref_count_interface
		{		
		public:
			virtual ~async_result_interface() = default;

			virtual bool wait(uint32_t timeout) = 0;
			virtual const char* get_result() = 0;		

			virtual bool register_callback(core::http::async_result_callback_interface* callback) = 0;
			virtual bool unregister_callback(core::http::async_result_callback_interface* callback) = 0;
		};

		class DLL_EXPORT http_client_interface : public core::ref_count_interface
		{
		public:
			virtual ~http_client_interface() = default;

			virtual int http_get(const char* url, long timeout,core::http::async_result_interface** result) = 0;
			virtual int http_delete(const char* url, long timeout, core::http::async_result_interface** result) = 0;
			virtual int http_post(const char* url, const char* data, long timeout, core::http::async_result_interface** result) = 0;
			virtual int http_put(const char* url, const char* data, long timeout, core::http::async_result_interface** result) = 0;
		};
	}
}