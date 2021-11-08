// curl_http_client.cpp : Defines the exported functions for the DLL application.
//

#include "curl_http_client.h"
#include <utils/http.hpp>
#include <utils/scope_guard.hpp>

#include <cstddef>
#include <string>
#include <mutex>
#include <condition_variable>
#include <cstring>

#define GIGABYTE 1073741824

namespace http
{
	class curl_http_async_result : public utils::http::async_result_base<utils::ref_count_base<core::http::async_result_interface>>
	{
		friend class curl_http_client_impl;

	private:
		std::mutex m_mutex;		
		std::condition_variable m_wait_handle;
		
		std::string m_result;
		bool m_completed;		

		void set_result(std::string&& result)
		{
			m_result = m_result + std::move(result);

			std::unique_lock<std::mutex> locker(m_mutex);
			m_completed = true;
			locker.unlock();

			m_wait_handle.notify_all();
			raise_callbacks(m_result.c_str());
		}

	public:
		curl_http_async_result() :
			m_completed(false)
		{
		}

		virtual bool wait(uint32_t timeout) override
		{
			auto pred = [&]()
			{
				return (m_completed == true);
			};

			std::unique_lock<std::mutex> locker(m_mutex);
			if (timeout == 0)
			{				
				m_wait_handle.wait(locker, pred);
			}
			else
			{
				if (m_wait_handle.wait_for(locker, std::chrono::milliseconds(timeout), pred) == false)
					return false;
			}

			return true;
		}

		virtual const char* get_result() override
		{
			if (wait(0) == false)
				return nullptr;


			return m_result.length() > 0 ? m_result.c_str() : "";
		}

		virtual bool register_callback(core::http::async_result_callback_interface* callback) override
		{
			if (callback == nullptr)
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_completed == true)
				callback->on_result(m_result.c_str());

			return utils::http::async_result_base<utils::ref_count_base<core::http::async_result_interface>>::register_callback(callback);
		}		
	};

	size_t curl_http_client_impl::result_callback(void *contents, size_t size, size_t nmemb, void *userp)
	{		
		curl_http_async_result* async_result = static_cast<curl_http_async_result*>(userp);
		if (async_result == nullptr)
			return 0;		

		const size_t realsize = size * nmemb;
		async_result->set_result(contents == nullptr ?
			std::move(std::string()) :
			std::move(std::string(static_cast<const char*>(contents), realsize)));

		return realsize;
	}

	curl_http_client_impl::unique_curl curl_http_client_impl::easy_create()
	{
		return curl_http_client_impl::unique_curl(curl_easy_init());
	}

	curl_http_client_impl::curl_http_client_impl(const char* content_type) :
		m_content_type((content_type == nullptr)?"application/json":content_type)
	{
		const long ret_code = curl_global_init(CURL_GLOBAL_ALL);
		if (ret_code != CURLE_OK)
			throw std::runtime_error("Failed to initialize curl");
	}

	int curl_http_client_impl::http_get(const char* url, long timeout, core::http::async_result_interface** result)
	{
		curl_http_client_impl::unique_curl curl = easy_create();
		if (curl == nullptr)
			return CURLE_FAILED_INIT;
		
		// We assume the the func callback will release the current ref (which is 1 after the creation and detach)
		core::http::async_result_interface* user_p =
			utils::make_ref_count_ptr<http::curl_http_async_result>().detach(); // ref count is now 1

		utils::scope_guard releaser([&]()
		{
			user_p->release();
		});

		// int retval = Call CURL API with '&curl_http_client_impl::result_callback' and user_p
		long ret_op_code = 0;
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_URL, url);
		if (ret_op_code != CURLE_OK) 
			return static_cast<int>(ret_op_code);
		
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, timeout);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, result_callback);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, user_p);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);
	
		if (result != nullptr)
		{
			*result = user_p;
			(*result)->add_ref(); // ref count is now 2
		}

		// Perform operation.
		return static_cast<int>(curl_easy_perform(curl.get()));
	}

	int curl_http_client_impl::http_post(const char* url, const char* data, long timeout, core::http::async_result_interface** result)
	{
		curl_http_client_impl::unique_curl curl = easy_create();
		if (curl == nullptr)
			return CURLE_FAILED_INIT;

		// We assume the the func callback will release the current ref (which is 1 after the creation and detach)
		core::http::async_result_interface* user_p =
			utils::make_ref_count_ptr<http::curl_http_async_result>().detach(); // ref count is now 1

		utils::scope_guard releaser([&]()
		{
			user_p->release();
		});

		// int retval = Call CURL API with '&curl_http_client_impl::result_callback' and user_p
		long ret_op_code = 0;
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_URL, url);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, timeout);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		if (((unsigned long)strlen(data)) >= GIGABYTE)
			ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE_LARGE, (long)strlen(data));
		else
			ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, (long)strlen(data));

		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, data);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, result_callback);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, user_p);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		struct curl_slist *headers = NULL;
		std::string content_type_header = "Content-Type: " + m_content_type;
		headers = curl_slist_append(headers, content_type_header.c_str());
		/* pass our list of custom made headers */
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		if (result != nullptr)
		{
			*result = user_p;
			(*result)->add_ref(); // ref count is now 2
		}

		// Perform operation.
		return static_cast<int>(curl_easy_perform(curl.get()));
	}
	
	int curl_http_client_impl::http_put(const char* url, const char* data, long timeout, core::http::async_result_interface** result)
	{
		curl_http_client_impl::unique_curl curl = easy_create();
		if (curl == nullptr)
			return CURLE_FAILED_INIT;

		// We assume the the func callback will release the current ref (which is 1 after the creation and detach)
		core::http::async_result_interface* user_p =
			utils::make_ref_count_ptr<http::curl_http_async_result>().detach(); // ref count is now 1

		utils::scope_guard releaser([&]()
		{
			user_p->release();
		});

		// int retval = Call CURL API with '&curl_http_client_impl::result_callback' and user_p
		long ret_op_code = 0;
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_URL, url);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, timeout);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		if (((unsigned long)strlen(data)) >= GIGABYTE)
			ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE_LARGE, (long)strlen(data));
		else
			ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, (long)strlen(data));

		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, data);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, result_callback);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, user_p);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		struct curl_slist *headers = NULL;
		std::string content_type_header = "Content-Type: " + m_content_type;
		headers = curl_slist_append(headers, content_type_header.c_str());
		/* pass our list of custom made headers */
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		if (result != nullptr)
		{
			*result = user_p;
			(*result)->add_ref(); // ref count is now 2
		}

		// Perform operation.
		return static_cast<int>(curl_easy_perform(curl.get()));
	}

	int curl_http_client_impl::http_delete(const char* url, long timeout, core::http::async_result_interface** result)
	{
		curl_http_client_impl::unique_curl curl = easy_create();
		if (curl == nullptr)
			return CURLE_FAILED_INIT;

		// We assume the the func callback will release the current ref (which is 1 after the creation and detach)
		core::http::async_result_interface* user_p =
			utils::make_ref_count_ptr<http::curl_http_async_result>().detach(); // ref count is now 1

		utils::scope_guard releaser([&]()
		{
			user_p->release();
		});

		// int retval = Call CURL API with '&curl_http_client_impl::result_callback' and user_p
		long ret_op_code = 0;
		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, timeout);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_URL, url);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, result_callback);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		ret_op_code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, user_p);
		if (ret_op_code != CURLE_OK)
			return static_cast<int>(ret_op_code);

		if (result != nullptr)
		{
			*result = user_p;
			(*result)->add_ref(); // ref count is now 2
		}

		// Perform operation.
		return static_cast<int>(curl_easy_perform(curl.get()));
	}
}

bool http::curl_http_client::create(const char* content_type, core::http::http_client_interface** client)
{
	if (client == nullptr)
		return false;

	utils::ref_count_ptr<core::http::http_client_interface> instance;

	try
	{
		instance = utils::make_ref_count_ptr<curl_http_client_impl>(content_type);
	}	
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*client = instance;
	(*client)->add_ref();	
	return true;
}
