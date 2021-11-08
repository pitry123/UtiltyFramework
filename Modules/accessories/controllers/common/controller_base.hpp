#pragma once
#include <utils/database.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/scope_guard.hpp>

#include <mutex>

namespace accessories
{
	namespace controllers
	{
		template <class T>
		class controller_base : public utils::database::database_dispatcher_base<utils::ref_count_base<T>>
		{
		private:
			std::mutex m_mutex;
			utils::timer_token m_timer_token;
			double m_polling_interval_milliseconds;

		protected:
			utils::ref_count_ptr<core::database::table_interface> m_status_table;
			utils::ref_count_ptr<core::database::table_interface> m_commands_table;

			virtual void on_start()
			{
			}

			virtual void on_stop()
			{
			}

			virtual void on_timer() = 0;						

		private:
			void stop_unsafe()
			{
				if (m_timer_token == utils::timer_token_undefined)
					return;

				utils::ref_count_ptr<utils::dispatcher> context;
				if (this->query_context(&context) == false)
					return;

				context->unregister_timer(m_timer_token);
				m_timer_token = utils::timer_token_undefined;

				on_stop();
			}

			void start_unsafe()
			{
				utils::ref_count_ptr<utils::dispatcher> context;
				if (this->query_context(&context) == false)
					return;

				on_start();

				m_timer_token = context->register_timer(m_polling_interval_milliseconds, [this]()
				{
					on_timer();
				});
			}

		public:
			controller_base(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				double polling_interval_milliseconds,
				const char* controller_id) :
				utils::database::database_dispatcher_base<utils::ref_count_base<T>>(controller_id),
				m_status_table(status_table),
				m_commands_table(commands_table),
				m_timer_token(utils::timer_token_undefined),
				m_polling_interval_milliseconds(polling_interval_milliseconds)
			{
				if (status_table == nullptr)
					throw std::invalid_argument("status_table");

				if (commands_table == nullptr)
					throw std::invalid_argument("commands_table");

				if (polling_interval_milliseconds <= 0.0)
					throw std::invalid_argument("polling_interval_milliseconds");
			}

			virtual ~controller_base()
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				stop_unsafe();
			}

			virtual void start() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				stop_unsafe();
				start_unsafe();
			}

			virtual void stop() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				stop_unsafe();
			}
		};
	}
}