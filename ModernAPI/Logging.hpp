#pragma once
#include <utils/logging.hpp>

namespace Logging
{
	using Severity = core::logging::severity;
	using ScopeLogger = utils::logging::scope_logger;

	class LogStream
	{
	private: 
		utils::logging::smart_log_stream m_stream;

	public:
		LogStream()
		{
		}

		LogStream(core::logging::log_stream* stream) :
			m_stream(stream)
		{
		}

		LogStream(const utils::logging::smart_log_stream& stream) :
			m_stream(stream)
		{
		}

		operator const utils::logging::smart_log_stream&() const
		{
			return m_stream;
		}

		explicit operator core::logging::log_stream*() const
		{
			return static_cast<core::logging::log_stream*>(m_stream);
		}

		::Logging::Severity Severity() const
		{
			return m_stream.severity();
		}

		::Logging::LogStream& operator<<(const std::stringstream& val)
		{
			m_stream.operator<<(val.str().c_str());
			return *this;
		}

		::Logging::LogStream& operator<<(const char* val)
		{
			m_stream.operator<<(val);
			return *this;
		}

		template <typename T>
		::Logging::LogStream& operator<<(const T& val)
		{
			m_stream.operator<<(val);
			return *this;
		}

		template <typename T>
		inline static std::string ToString(T val)
		{
			return utils::logging::smart_log_stream::to_string(val);
		}
	};

	class Logger final
	{
	private:
		utils::logging::smart_logger m_logger;

	public:
		Logger()
		{
			// Empty Logger
		}

		Logger(core::logging::logger* logger) :
			m_logger(logger)
		{
		}

		operator const utils::logging::smart_logger&() const
		{
			return m_logger;
		}

		explicit operator core::logging::logger*() const
		{
			return static_cast<core::logging::logger*>(m_logger);
		}

		bool Empty() const
		{
			return (m_logger.valid() == false);
		}

		const char* Name() const
		{
			return m_logger.name();
		}

		bool Enabled() const
		{
			return m_logger.enabled();
		}

		::Logging::Severity Filter() const
		{
			return m_logger.filter();
		}

		void Enabled(bool val)
		{
			m_logger.enabled(val);
		}

		void Filter(::Logging::Severity severity)
		{
			m_logger.filter(severity);
		}

		bool Log(::Logging::Severity severity, const char* message) const
		{
			return m_logger.log(severity, message);
		}

		::Logging::LogStream CreateStream(::Logging::Severity severity) const
		{
			return ::Logging::LogStream(m_logger.create_stream(severity));
		}
	};
}