#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/dispatcher.hpp>
#include <utils/signal.hpp>

namespace utils
{
	class timer : public utils::ref_count_base<core::ref_count_interface>
	{
	private:
		utils::ref_count_ptr<utils::dispatcher> m_dispatcher;
		double m_interval;
		bool m_running;
		int m_token;

		std::mutex m_mutex;

		timer(const timer& other);						// non construction-copyable
		timer(timer&& other);							// non construction-movable
		timer& operator=(const timer&) = delete;		// non copyable
		timer& operator=(timer&& other) = delete;		// non movable

	protected:
		virtual void on_timer_tick()
		{
			elapsed();
		}

		virtual void on_start(double interval, bool autoReset)
		{			
			m_dispatcher = utils::make_ref_count_ptr<utils::dispatcher>("Timer");

			m_interval = interval;
			m_token = m_dispatcher->register_timer(m_interval, [this, autoReset]()-> void
			{
				utils::scope_guard autoResetGurad([this, autoReset]() -> void
				{
					if (autoReset == true)
					{
						stop();
					}
				});

				on_timer_tick();
			});			
		}

		virtual void on_stop()
		{
			m_dispatcher->unregister_timer(m_token);
			m_dispatcher.release();
			m_interval = -1.0;
		}

	public:
		utils::signal<utils::timer> elapsed;

		timer() : 
			m_interval(-1.0), 
			m_running(false)
		{			
		}

		virtual ~timer()
		{
			stop();
		}

		double interval()
		{
			return m_interval;
		}

		void start(double interval, bool autoReset = false)
		{
			if (interval <= 0.0)
				throw std::out_of_range("interval");

			stop();
			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_running == false)
			{
				on_start(interval, autoReset);
				m_running = true;
			}
		}

		void stop()
		{
			std::lock_guard<std::mutex> locker(m_mutex);

			if (m_running == true)
			{
				on_stop();
				m_running = false;
			}
		}
	};
}