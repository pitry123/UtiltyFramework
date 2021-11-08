#pragma once
#include <core/framework.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/signal.hpp>

#include <sstream>

#define LOG_AS(logger, severity) logger.create_stream(severity)
#define LOG_TRACE(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::TRACE)
#define LOG_DEBUG(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::DEBUG)
#define LOG_INFO(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::INFO)
#define LOG_WARNING(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::WARNING)
#define LOG_ERROR(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::ERROR)
#define LOG_FATAL(logger) static_cast<const utils::logging::smart_logger&>(logger).create_stream(core::logging::severity::FATAL)

#define LOG_FUNC(logger) \
::Logging::ScopeLogger arg_B6F22A29CC8647CFB8BC11BE9EACC910(logger, core::logging::severity::DEBUG, __FUNCTION__);

#define LOG_SCOPE(logger, scope) \
::Logging::ScopeLogger arg_1FB885016F0E4AF2ACD7DC2836E1DC9A(logger, core::logging::severity::DEBUG, scope);

#define LOG_TO_STRING(val) std::stringstream(utils::logging::smart_log_stream::to_string(val))

namespace utils
{
	namespace logging
	{
		class smart_logger_hook : public utils::ref_count_base<core::framework::logger_hook_interface>
		{
		public:
			utils::signal<utils::logging::smart_logger_hook, core::logging::logger*> logger_added;

			virtual void on_logger_created(core::logging::logger* logger) override
			{
				logger_added(logger);
			}
		};

		class smart_log_stream
		{
		private:
			utils::ref_count_ptr<core::logging::log_stream> m_stream;

		public:
			smart_log_stream()
			{
			}

			smart_log_stream(core::logging::log_stream* stream) :
				m_stream(stream)
			{
			}

			explicit operator core::logging::log_stream*() const
			{
				return static_cast<core::logging::log_stream*>(m_stream);
			}

			core::logging::severity severity() const
			{
				if (m_stream == nullptr)
					return core::logging::severity::FATAL; // Assume worst case

				return m_stream->severity();
			}

			utils::logging::smart_log_stream& operator<<(const std::stringstream& val)
			{
				if (m_stream != nullptr)
					m_stream->operator<<(val.str().c_str());
				
				return *this;
			}

			utils::logging::smart_log_stream& operator<<(const char* val)
			{
				if (m_stream != nullptr)
					m_stream->operator<<(val);
				
				return *this;
			}

			template <typename T>
			utils::logging::smart_log_stream& operator<<(const T& val)
			{
				if (m_stream != nullptr)
					m_stream->operator<<(val);
				
				return *this;
			}

			template <typename T>
			inline static std::string to_string(T val)
			{
				std::stringstream retval;
				retval << val;
				return retval.str();
			}
		};

		class smart_logger
		{
		private:
			utils::ref_count_ptr<core::logging::logger> m_logger;

		public:
			smart_logger()
			{
				// Empty Logger
			}

			smart_logger(core::logging::logger* logger) :
				m_logger(logger)
			{
			}

			explicit operator core::logging::logger*() const
			{
				return static_cast<core::logging::logger*>(m_logger);
			}

			bool valid() const
			{
				return (m_logger != nullptr);
			}

			const char* name() const
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				return m_logger->name();
			}

			bool enabled() const
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				return m_logger->enabled();
			}

			core::logging::severity filter() const
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				return m_logger->filter();
			}

			void enabled(bool val)
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				m_logger->enabled(val);
			}

			void filter(core::logging::severity severity)
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				m_logger->filter(severity);
			}

			bool log(core::logging::severity severity, const char* message) const
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				return m_logger->log(severity, message);
			}

			utils::logging::smart_log_stream create_stream(core::logging::severity severity) const
			{
				if (m_logger == nullptr)
					throw std::runtime_error("logger is nullptr");

				utils::ref_count_ptr<core::logging::log_stream> instance;
				if (m_logger->create_stream(severity, &instance) == false)
					return utils::logging::smart_log_stream();

				return utils::logging::smart_log_stream(instance);
			}
		};

		class scope_logger final
		{
		private:
			const char* ENTERING = ">> ";
			const char* EXITING = "<< ";

			const utils::logging::smart_logger& m_logger;
			core::logging::severity m_severity;
			const char* m_scope;

		public:
			scope_logger(const utils::logging::smart_logger& logger, core::logging::severity severity, const char* scope) :
				m_logger(logger),
				m_severity(severity),
				m_scope(scope)
			{
				if (scope == nullptr)
					throw std::invalid_argument("scope");

				LOG_AS(m_logger, m_severity) << ENTERING << m_scope;
			}

			~scope_logger()
			{
				LOG_AS(m_logger, m_severity) << EXITING << m_scope;
			}
		};
	}
}