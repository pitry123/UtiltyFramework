#pragma once
#include <http/curl_http_client.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <curl/curl.h>

#include <memory>

namespace http
{
	class curl_http_client_impl : public utils::ref_count_base<http::curl_http_client>
	{
	private:
		struct custom_curl_deleter
		{
			void operator()(CURL* p)
			{
				curl_easy_cleanup(p);
			}
		};

		using unique_curl = std::unique_ptr<CURL, custom_curl_deleter>;

		static size_t result_callback(void *contents, size_t size, size_t nmemb, void *userp);

		unique_curl easy_create();

		std::string m_content_type;

	public:	
		curl_http_client_impl(const char* content_type);

		virtual int http_get(const char* url, long timeout,core::http::async_result_interface** result) override;
		virtual int http_delete(const char* url, long timeout, core::http::async_result_interface** result) override;
		virtual int http_post(const char* url, const char* data, long timeout, core::http::async_result_interface** result) override;
		virtual int http_put(const char* url, const char* data, long timeout, core::http::async_result_interface** result) override;
	};
}

