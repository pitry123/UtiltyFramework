#pragma once
#include <core/http.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/callback_handler.hpp>
#include <utils/signal.hpp>

namespace utils
{
	namespace http
	{
		template <typename T>
		class async_result_base : public T
		{
		private:
			utils::callback_handler<core::http::async_result_callback_interface> m_callbacks;			

		protected:
			void raise_callbacks(const char* result)
			{
				m_callbacks.raise_callbacks([&](core::http::async_result_callback_interface* callback)
				{
					callback->on_result(result);
				});
			}

		public:
			virtual ~async_result_base() = default;

			virtual bool register_callback(core::http::async_result_callback_interface* callback) override 
			{
				return m_callbacks.add_callback(callback);
			}

			virtual bool unregister_callback(core::http::async_result_callback_interface* callback) override
			{
				return m_callbacks.remove_callback(callback);
			}
		};

		
		class smart_async_result_callback : public utils::ref_count_base<core::http::async_result_callback_interface>
		{
		private:
			std::function<void(const char*)> m_func;

		public:
			smart_async_result_callback(const std::function<void(const char*)>& func) :
				m_func(func)
			{
			}

			virtual ~smart_async_result_callback() = default;

			virtual void on_result(const char* res) override
			{
				if (m_func == nullptr)
					return;

				m_func(res);
			}
		};
		
		class async_result_wrapper : public utils::ref_count_base<core::http::async_result_interface>
		{
		private:
			std::mutex m_mutex;

			utils::ref_count_ptr<core::http::async_result_interface> m_async_result;
			utils::ref_count_ptr<utils::http::smart_async_result_callback> m_callback;
			std::string m_result;

			utils::signal<async_result_wrapper, const char*> m_result_signal;

		public:
			async_result_wrapper(core::http::async_result_interface* async_result) :
				m_async_result(async_result),
				m_callback(utils::make_ref_count_ptr<utils::http::smart_async_result_callback>(
					[this](const char* res)
			{
				std::lock_guard<std::mutex> locker(m_mutex);

				m_result = res;
				m_result_signal(res);
			}))
			{
				if (async_result == nullptr)
					throw std::invalid_argument("async_result");

				m_async_result->register_callback(m_callback);
			}

			virtual ~async_result_wrapper()
			{
				m_async_result->unregister_callback(m_callback);
			}

			signal_token operator+=(const std::function<void(const char*)>& func)
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				
				utils::signal_token retval = (m_result_signal += func);				
				if (m_result.empty() == false)
					func(m_result.c_str());

				return retval;
			}

			bool operator-=(signal_token token)
			{
				return (m_result_signal -= token);
			}

			void underlying_object(core::http::async_result_interface** obj) const
			{
				if (obj == nullptr)
					throw std::invalid_argument("obj");

				*obj = m_async_result;
				(*obj)->add_ref();
			}

			virtual bool wait(uint32_t timeout) override
			{
				return m_async_result->wait(timeout);
			}

			virtual const char* get_result() override
			{
				return m_async_result->get_result();
			}

			virtual bool register_callback(core::http::async_result_callback_interface* callback) override
			{
				return m_async_result->register_callback(callback);
			}

			virtual bool unregister_callback(core::http::async_result_callback_interface* callback) override
			{
				return m_async_result->unregister_callback(callback);
			}
		};
	}
}