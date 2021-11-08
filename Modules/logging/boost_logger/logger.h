#pragma once
#include <logging/boost_logger.h>
#include <utils/ref_count_base.hpp>

#include <string>
#include <vector>
#include <atomic>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core/record.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>

namespace logging
{
	class loggers_pool;

	class log_stream_impl : public utils::ref_count_base<core::logging::log_stream>
	{
	private:
		// Well... That's a complicated one.
		// We want to be able to create a pool of log_stream_impl so we won't have
		// to perform a dynamic allocation each time a user is requesting an instance from the logger.
		// We are wrapping 'boost::log::record' and 'boost::log::aux::record_pump<::boost::log::trivial::logger::logger_type>'
		// as the logger's stream objects.
		// We need to construct the objects each time the user requests a stream
		// and destruct them when the stream is returned to the pool (objects does not have a proper reset API).
		// Issue is we can't do the above since record_pump is NOT copy or move assignable. It also doesn't support default constructor.
		// To overcome the issue, I'm allocating a buffer the size of both objects.
		// I'm placing the 1st object at the beggining of the allocation and construting
		// the 2nd object (record_pump) with 'placement new' into the pre-allocated buffer address + 1st object's size.
		// I'm also making sure to LITERALLY call the destructor of both objects when we need to clean (e.g. stream returned to pool
		// or destructed).
		class record_wrapper
		{
		private:
			enum state
			{
				CLEARED,
				INITIATED				
			};

			std::vector<uint8_t> m_allocation;
			state m_state;

		public:
			using boost_record = boost::log::record;
			using boost_record_pump = boost::log::aux::record_pump<::boost::log::trivial::logger::logger_type>;

			record_wrapper() :
				m_allocation(sizeof(boost_record) + sizeof(boost_record_pump)),
				m_state(state::CLEARED)
			{
			}

			~record_wrapper()
			{
				clear();
			}

			void init(::boost::log::trivial::severity_level severity)
			{
				if (m_state == state::INITIATED)
					clear();

				record() = (::boost::log::trivial::logger::get()).open_record(::boost::log::keywords::severity = severity);
				new (&(pump())) ::boost::log::aux::record_pump<::boost::log::trivial::logger::logger_type>(::boost::log::trivial::logger::get(), record());
				m_state = state::INITIATED;
			}

			void clear()
			{
				if (m_state == state::CLEARED)
					return;

				pump().~boost_record_pump();
				record().~boost_record();
				m_state = state::CLEARED;
			}

			boost_record& record()
			{
				return *(static_cast<boost_record*>(static_cast<void*>(m_allocation.data())));
			}

			boost_record_pump& pump()
			{
				return *(static_cast<boost_record_pump*>(static_cast<void*>(m_allocation.data() + sizeof(boost_record))));
			}
		};

		core::logging::severity m_severity;
		mutable record_wrapper m_record_wrapper;

		template <typename T>
		core::logging::log_stream& stream_operator_handler(T& val)
		{		
			m_record_wrapper.pump().stream() << val;
			return *this;
		}	

	public:
		log_stream_impl();
		virtual ~log_stream_impl() noexcept;
		virtual int release() const override;

		virtual core::logging::severity severity() const override;
		virtual core::logging::log_stream& operator<<(char c) override;
		virtual core::logging::log_stream& operator<<(const char* p) override;
		virtual core::logging::log_stream& operator<<(bool value) override;
		virtual core::logging::log_stream& operator<<(signed char value) override;
		virtual core::logging::log_stream& operator<<(unsigned char value) override;
		virtual core::logging::log_stream& operator<<(short value) override;
		virtual core::logging::log_stream& operator<<(unsigned short value) override;
		virtual core::logging::log_stream& operator<<(int value) override;
		virtual core::logging::log_stream& operator<<(unsigned int value) override;
		virtual core::logging::log_stream& operator<<(long value) override;
		virtual core::logging::log_stream& operator<<(unsigned long value) override;
		virtual core::logging::log_stream& operator<<(long long value) override;
		virtual core::logging::log_stream& operator<<(unsigned long long value) override;
		virtual core::logging::log_stream& operator<<(float value) override;
		virtual core::logging::log_stream& operator<<(double value) override;
		virtual core::logging::log_stream& operator<<(long double value) override;
		virtual core::logging::log_stream& operator<<(const void* value) override;

		void init(core::logging::severity severity);
	};

	class boost_logger_impl :
		public utils::ref_count_base<core::logging::logger>
	{
		friend class loggers_pool;

	private:
        static constexpr char const* UNDEFINED_NAME = "Undefined";

		char m_name[64];
		std::atomic<bool> m_enabled;
		std::atomic<core::logging::severity> m_filter;

		void set_name(const char* name);

	public:
		boost_logger_impl(const char* name, core::logging::severity filter);
		virtual const char* name() const override;
		virtual bool enabled() const override;
		virtual core::logging::severity filter() const override;
        virtual void enabled(bool val) override;
		virtual void filter(core::logging::severity severity) override;
		virtual bool log(core::logging::severity severity, const char* message) const override;
		virtual bool create_stream(core::logging::severity severity, core::logging::log_stream** stream) const override;
	};
}

