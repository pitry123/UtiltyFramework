#pragma once
#include <http/curl_http_client.h>
#include <utils/http.hpp>
#include <Utils.hpp>

namespace Http
{
	class HttpResult : public Common::CoreObjectWrapper<core::http::async_result_interface>
	{
	private:
		utils::http::async_result_wrapper* wrapper() const
		{
			return static_cast<utils::http::async_result_wrapper*>(static_cast<void*>(m_core_object));
		}

	public:
		HttpResult()
		{
			// Empty result
		}

		HttpResult(core::http::async_result_interface* result) :
			Common::CoreObjectWrapper<core::http::async_result_interface>(
				utils::make_ref_count_ptr<utils::http::async_result_wrapper>(result))
		{
			if (result == nullptr)
				throw std::invalid_argument("result");
		}	

		bool Wait(uint32_t timeout = 0)
		{
			ThrowOnEmpty("HttpResult");
			return m_core_object->wait(timeout);
		}

		const char* GetResult()
		{
			ThrowOnEmpty("HttpResult");
			return m_core_object->get_result();
		}

		Utils::SignalToken operator+=(const std::function<void(const char*)>& func)
		{
			ThrowOnEmpty("HttpResult");
			return wrapper()->operator+=(func);
		}

		bool operator-=(Utils::SignalToken token)
		{
			ThrowOnEmpty("HttpResult");
			return wrapper()->operator-=(token);
		}		

		virtual void UnderlyingObject(core::http::async_result_interface** core_object) const override
		{
			ThrowOnEmpty("HttpResult");
			wrapper()->underlying_object(core_object);
		}
	};

	class HttpClient : public Common::CoreObjectWrapper<core::http::http_client_interface>
	{
	private:
		static HttpClient Create(const char* content_type)
		{
			utils::ref_count_ptr<core::http::http_client_interface> instance;
			if (http::curl_http_client::create(content_type, &instance) == false)
				throw std::runtime_error("Failed to create http client");

			return HttpClient(instance);
		}

	public:		
		HttpClient(core::http::http_client_interface* http_client) : 
			Common::CoreObjectWrapper<core::http::http_client_interface>(http_client)
		{
		}

		HttpClient() :
			HttpClient(static_cast<core::http::http_client_interface*>(Create(nullptr)))
		{			
		}

		HttpClient(const char* content_type) :
			HttpClient(static_cast<core::http::http_client_interface*>(Create(content_type)))
		{
		}

		HttpClient(std::nullptr_t)
		{
			// Empty Client
		}

		int Get(std::string uri, Http::HttpResult& result, long TimeOut = 0) const
		{
			ThrowOnEmpty("HttpClient");

			utils::ref_count_ptr<core::http::async_result_interface> core_result;
			int res = m_core_object->http_get(uri.c_str(), TimeOut, &core_result);
			if (res == 0)
				result = HttpResult(core_result);

			return res;
		}

		int Delete(std::string uri, Http::HttpResult& result, long TimeOut = 0) const
		{
			ThrowOnEmpty("HttpClient");

			utils::ref_count_ptr<core::http::async_result_interface> core_result;
			int res = m_core_object->http_delete(uri.c_str(), TimeOut, &core_result);
			if (res == 0)
				result = HttpResult(core_result);

			return res;
		}

		int Post(std::string uri, std::string data, Http::HttpResult& result, long TimeOut = 0) const
		{
			ThrowOnEmpty("HttpClient");

			utils::ref_count_ptr<core::http::async_result_interface> core_result;
			int res = m_core_object->http_post(uri.c_str(), data.c_str(), TimeOut, &core_result);
			if (res == 0)
				result = HttpResult(core_result);

			return res;
		}		

		int Put(std::string uri, std::string data, Http::HttpResult& result, long TimeOut = 0) const
		{
			ThrowOnEmpty("HttpClient");

			utils::ref_count_ptr<core::http::async_result_interface> core_result;
			int res = m_core_object->http_put(uri.c_str(), data.c_str(), TimeOut, &core_result);
			if (res == 0)
				result = HttpResult(core_result);

			return res;
		}
	}; 
}