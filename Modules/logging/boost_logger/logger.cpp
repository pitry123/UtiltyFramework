#include "logger.h"
#include <utils/ref_count_ptr.hpp>
#include <utils/ref_count_object_pool.hpp>
#include <utils/strings.hpp>

#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/log/sinks/syslog_backend.hpp>

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4714) // __forceinline (used in boost) might raise warning when compiling with -GX/EHs/EHa or wihtout -Og/Ox/O1/O2
#endif

namespace logging
{
	static constexpr size_t STREAMS_POOL_INITIAL_SIZE = 64;
	static constexpr size_t LOGGERS_POOL_INITIAL_SIZE = 64;

	static constexpr char const* CONFIG_FILE_NAME = "boost_logger.xml";

	class boost_logger_config
	{
		friend class boost::serialization::access;

	public:
		class time_rotation
		{
			friend class boost::serialization::access;

		private:
			bool m_enabled;
			uint8_t m_hour;
			uint8_t m_minute;
			uint8_t m_second;

			template <class archive>
			void serialize(archive& ar, const unsigned int version)
			{
				using boost::serialization::make_nvp;
				ar & make_nvp("Enabled", m_enabled);
				ar & make_nvp("Hour", m_hour);
				ar & make_nvp("Minute", m_minute);
				ar & make_nvp("Second", m_second);
			}

		public:
			time_rotation() :
				m_enabled(false),
				m_hour(0),
				m_minute(0),
				m_second(0)
			{
			}

			bool Enabled()
			{
				if (m_enabled == false)
					return false;

				if (m_hour >= 24 || m_minute >= 59 || m_second >= 59)
					return false;

				return true;
			}

			uint8_t Hour()
			{
				return m_hour;
			}

			uint8_t Minute()
			{
				return m_minute;
			}

			uint8_t Second()
			{
				return m_second;
			}
		};

		class syslog_sink
		{
			friend class boost::serialization::access;

		private:
			bool m_enabled;
			std::string m_local_address;
			std::string m_target_address;
			uint16_t m_target_port;

			template <class archive>
			void serialize(archive& ar, const unsigned int version)
			{
				using boost::serialization::make_nvp;
				ar & make_nvp("Enabled", m_enabled);
				ar & make_nvp("LocalAddress", m_local_address);
				ar & make_nvp("TargetAddress", m_target_address);
				ar & make_nvp("TargetPort", m_target_port);
			}

		public:
			syslog_sink() :
				m_enabled(false),
				m_local_address(),
				m_target_address(),
				m_target_port(0)
			{
			}

			bool Enabled()
			{
				return m_enabled;
			}

			const char* LocalAddress()
			{
				return m_local_address.c_str();
			}

			const char* TargetAddress()
			{
				return m_target_address.c_str();
			}

			uint16_t TargetPort()
			{
				return m_target_port;
			}
		};

	private:
		static constexpr char const* DEFAULT_LOG_FILE_NAME = "Framework_2.0_%Y-%m-%d_%H-%M-%S.%N.log";
		static constexpr uint32_t DEFAULT_ROTATION_SIZE = 10;	//MBs
		static constexpr uint32_t DEFAULT_MIN_FREE_SPACE = 30;	//MBs
		static constexpr bool DEFAULT_ENABLED = true;
		static constexpr core::logging::severity DEFAULT_FILTER = core::logging::severity::TRACE;
		static constexpr bool DEFAULT_CONSOLE_SINK = false;

		std::string m_file_name;
		uint32_t m_rotation_size;
		time_rotation m_time_rotation;
		uint32_t m_min_free_space;
		bool m_enabled;
		core::logging::severity m_filter;
		bool m_console_sink;
		syslog_sink m_syslog_sink;

		template <class archive>
		void serialize(archive& ar, const unsigned int version)
		{
			using boost::serialization::make_nvp;
			ar & make_nvp("FileName", m_file_name);
			ar & make_nvp("RotationSize", m_rotation_size);
			ar & make_nvp("TimeRotation", m_time_rotation);
			ar & make_nvp("MinFreeSpace", m_min_free_space);
			ar & make_nvp("Enabled", m_enabled);
			ar & make_nvp("Filter", m_filter);
			ar & make_nvp("Console", m_console_sink);
			ar & make_nvp("SysLog", m_syslog_sink);
		}

	public:
		boost_logger_config() :
			m_file_name(DEFAULT_LOG_FILE_NAME),
			m_rotation_size(DEFAULT_ROTATION_SIZE),
			m_time_rotation({}),
			m_min_free_space(DEFAULT_MIN_FREE_SPACE),
			m_enabled(DEFAULT_ENABLED),
			m_filter(DEFAULT_FILTER),
			m_console_sink(DEFAULT_CONSOLE_SINK),
			m_syslog_sink()
		{
		}

		bool save(const char* config_file_name)
		{
			try
			{
				std::fstream ofs(config_file_name, std::ios::out | std::ios::binary);
				unsigned int flags = boost::archive::no_header;
				boost::archive::xml_oarchive xml(ofs, flags);
				xml << boost::serialization::make_nvp("BoostLogger", *this);
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		static boost_logger_config load(const char* config_file_name)
		{
			boost_logger_config retval;

			try
			{
				std::fstream ifs(config_file_name, std::ios::in | std::ios::binary);
				if (ifs.is_open() == false)
				{
					// Fallback to default values
					return boost_logger_config();
				}

				unsigned int flags = boost::archive::no_header;
				boost::archive::xml_iarchive xml(ifs, flags);
				xml >> boost::serialization::make_nvp("BoostLogger", retval);
			}
			catch (...)
			{
				// Fallback to default values
				return boost_logger_config();
			}
			std::size_t fileLocationPosition = retval.m_file_name.find_last_of("/\\");
			if (fileLocationPosition == std::string::npos) // No match
				return retval;

			std::string pathToFileLocation = retval.m_file_name.substr(0, fileLocationPosition + 1);
			std::string fileName = retval.m_file_name.substr(fileLocationPosition + 1);
			auto_expand_environment_variables(pathToFileLocation);
			retval.m_file_name = pathToFileLocation + fileName;
			return retval;
		}

		boost_logger_config(const char* file_name,
			uint32_t rotation_size,
			const boost_logger_config::time_rotation& time_rotation,
			uint32_t min_free_space,
			bool enabled,
			core::logging::severity filter,
			bool console_sink) :
			m_file_name(file_name == nullptr ? DEFAULT_LOG_FILE_NAME : file_name),
			m_rotation_size(rotation_size),
			m_time_rotation(time_rotation),
			m_min_free_space(min_free_space),
			m_enabled(enabled),
			m_filter(filter),
			m_console_sink(console_sink)
		{
		}

		const char* file_name()
		{
			return m_file_name.c_str();
		}

		uint32_t rotation_size()
		{
			if (m_rotation_size == 0)
				return DEFAULT_ROTATION_SIZE;

			return m_rotation_size;
		}

		time_rotation& time_based_rotation()
		{
			return m_time_rotation;
		}

		uint32_t min_free_space()
		{
			uint32_t retval = m_min_free_space;

			if (retval == 0)
				retval = DEFAULT_MIN_FREE_SPACE;

			if (retval < rotation_size())
				retval = rotation_size();

			return retval;
		}

		bool enabled()
		{
			return m_enabled;
		}

		core::logging::severity filter()
		{
			return m_filter;
		}

		bool console_sink()
		{
			return m_console_sink;
		}

		syslog_sink& syslog()
		{
			return m_syslog_sink;
		}
	};

	::boost::log::trivial::severity_level convert(core::logging::severity severity)
	{
		switch (severity)
		{
		case core::logging::severity::TRACE:
			return ::boost::log::trivial::severity_level::trace;
		case core::logging::severity::DEBUG:
			return ::boost::log::trivial::severity_level::debug;
		case core::logging::severity::INFO:
			return ::boost::log::trivial::severity_level::info;
		case core::logging::severity::WARNING:
			return ::boost::log::trivial::severity_level::warning;
		case core::logging::severity::ERROR:
			return ::boost::log::trivial::severity_level::error;
		case core::logging::severity::FATAL:
			return ::boost::log::trivial::severity_level::fatal;
		default:
			break;
		}

		throw std::runtime_error("Unexpected severity value");
	}

	core::logging::severity convert(::boost::log::trivial::severity_level severity)
	{
		switch (severity)
		{
		case ::boost::log::trivial::severity_level::trace:
			return core::logging::severity::TRACE;
		case ::boost::log::trivial::severity_level::debug:
			return core::logging::severity::DEBUG;
		case ::boost::log::trivial::severity_level::info:
			return core::logging::severity::INFO;
		case ::boost::log::trivial::severity_level::warning:
			return core::logging::severity::WARNING;
		case ::boost::log::trivial::severity_level::error:
			return core::logging::severity::ERROR;
		case ::boost::log::trivial::severity_level::fatal:
			return core::logging::severity::FATAL;
		default:
			break;
		}

		throw std::runtime_error("Unexpected severity value");
	}

	class loggers_pool : private utils::ref_count_object_pool<logging::boost_logger_impl>
	{
	public:
		loggers_pool(size_t size, core::logging::severity default_filter) :
			utils::ref_count_object_pool<logging::boost_logger_impl>(
				size,
				utils::ref_count_object_pool<logging::boost_logger_impl>::growing_mode::doubling,
				false,
				nullptr,
				default_filter)
		{
		}

		bool get_logger(const char* name, core::logging::severity filter, logging::boost_logger_impl** logger) const
		{
			if (utils::ref_count_object_pool<logging::boost_logger_impl>::get_item(logger) == false)
				return false;

			(*logger)->set_name(name);
			(*logger)->filter(filter);
			return true;
		}
	};

	class streams_pool : private utils::ref_count_object_pool<logging::log_stream_impl>
	{
	public:
		streams_pool(size_t size) :
			utils::ref_count_object_pool<logging::log_stream_impl>(
				size,
				utils::ref_count_object_pool<logging::log_stream_impl>::growing_mode::doubling,
				false)
		{
		}

		bool get_stream(core::logging::severity severity, logging::log_stream_impl** stream) const
		{
			if (utils::ref_count_object_pool<logging::log_stream_impl>::get_item(stream) == false)
				return false;

			(*stream)->init(severity);
			return true;
		}
	};

	class LOGGER
	{
	private:
		core::logging::severity m_filter;
		loggers_pool m_loggers_pool;
		streams_pool m_streams_pool;

		core::logging::severity init()
		{
			logging::boost_logger_config config =
				logging::boost_logger_config::load(CONFIG_FILE_NAME);

			/* init boost log
			* 1. Add common attributes
			* 2. set log filter to trace
			*/

			::boost::log::core::get()->set_logging_enabled(config.enabled());

			boost::log::add_common_attributes();
			boost::log::trivial::severity_level filter = convert(config.filter());
			boost::log::core::get()->set_filter(
				boost::log::trivial::severity >= filter
			);

			/* log formatter:
			* [TimeStamp] [ThreadId] [Severity Level] Log message
			*/

			auto fmtTimeStamp = boost::log::expressions::
				format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");

			auto fmtThreadId = boost::log::expressions::
				attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");

			auto fmtSeverity = boost::log::expressions::
				attr<boost::log::trivial::severity_level>("Severity");

			/*boost::log::formatter logFmt =
				boost::log::expressions::format("[%1%] (%2%) [%3%] %4%")
				% fmtTimeStamp % fmtThreadId % fmtSeverity % boost::log::expressions::smessage;*/

			boost::log::formatter logFmt =
				boost::log::expressions::stream
				<< "[" << fmtTimeStamp << "] "
				<< "(" << fmtThreadId << ") "
				<< "[" << std::setw(7) << std::left << fmtSeverity << "] "
				<< boost::log::expressions::smessage;


			if (config.console_sink() == true)
			{
				/* console sink */
				auto consoleSink = boost::log::add_console_log(std::clog);
				consoleSink->set_formatter(logFmt);
			}

			if (config.syslog().Enabled() == true)
			{
				// Create a new backend
				boost::shared_ptr<::boost::log::sinks::syslog_backend > backend =
					boost::make_shared<::boost::log::sinks::syslog_backend >(
						::boost::log::keywords::use_impl = ::boost::log::sinks::syslog::udp_socket_based,
						::boost::log::keywords::facility = ::boost::log::sinks::syslog::local0);

				// Setup the local and target address and port to send syslog messages to
				backend->set_local_address(config.syslog().LocalAddress());
				backend->set_target_address(config.syslog().TargetAddress(), config.syslog().TargetPort());

				// Create and fill in another level translator for "MyLevel" attribute of type string
				::boost::log::sinks::syslog::custom_severity_mapping<boost::log::trivial::severity_level> mapping("Severity");
				mapping[boost::log::trivial::severity_level::trace] = ::boost::log::sinks::syslog::debug;
				mapping[boost::log::trivial::severity_level::debug] = ::boost::log::sinks::syslog::debug;
				mapping[boost::log::trivial::severity_level::info] = ::boost::log::sinks::syslog::info;
				mapping[boost::log::trivial::severity_level::warning] = ::boost::log::sinks::syslog::warning;
				mapping[boost::log::trivial::severity_level::error] = ::boost::log::sinks::syslog::error;
				mapping[boost::log::trivial::severity_level::fatal] = ::boost::log::sinks::syslog::critical;
				backend->set_severity_mapper(mapping);

				// Wrap it into the frontend and register in the core.				
				typedef ::boost::log::sinks::synchronous_sink<::boost::log::sinks::syslog_backend> sink_t;
				boost::shared_ptr<sink_t> sink = boost::make_shared<sink_t>(backend);
				::boost::log::core::get()->add_sink(sink);
			}

			/* fs sink */
			if (config.time_based_rotation().Enabled() == true)
			{
				auto fsSink = boost::log::add_file_log(
					boost::log::keywords::file_name = config.file_name(),
					boost::log::keywords::rotation_size = config.rotation_size() * 1024 * 1024,
					boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(config.time_based_rotation().Hour(), config.time_based_rotation().Minute(), config.time_based_rotation().Second()),
					boost::log::keywords::min_free_space = config.min_free_space() * 1024 * 1024,
					boost::log::keywords::open_mode = std::ios_base::app);

				fsSink->set_formatter(logFmt);
				fsSink->locked_backend()->auto_flush(true);
			}
			else
			{
				auto fsSink = boost::log::add_file_log(
					boost::log::keywords::file_name = config.file_name(),
					boost::log::keywords::rotation_size = config.rotation_size() * 1024 * 1024,
					boost::log::keywords::min_free_space = config.min_free_space() * 1024 * 1024,
					boost::log::keywords::open_mode = std::ios_base::app);

				fsSink->set_formatter(logFmt);
				fsSink->locked_backend()->auto_flush(true);
			}

			return config.filter();
		}

	public:
		static LOGGER instance;

		LOGGER() :
			m_filter(init()),
			m_loggers_pool(LOGGERS_POOL_INITIAL_SIZE, m_filter),
			m_streams_pool(STREAMS_POOL_INITIAL_SIZE)
		{
		}

		bool log(core::logging::severity severity, const char* message)
		{
			if (m_filter > severity)
				return false;

			if (::boost::log::core::get()->get_logging_enabled() == false)
				return false;

			switch (severity)
			{
			case core::logging::severity::TRACE:
				BOOST_LOG_TRIVIAL(trace) << message;
				break;
			case core::logging::severity::DEBUG:
				BOOST_LOG_TRIVIAL(debug) << message;
				break;
			case core::logging::severity::INFO:
				BOOST_LOG_TRIVIAL(info) << message;
				break;
			case core::logging::severity::WARNING:
				BOOST_LOG_TRIVIAL(warning) << message;
				break;
			case core::logging::severity::ERROR:
				BOOST_LOG_TRIVIAL(error) << message;
				break;
			case core::logging::severity::FATAL:
				BOOST_LOG_TRIVIAL(fatal) << message;
				break;
			default:
				return false;
			}

			return true;
		}

		bool create_logger(const char* name, core::logging::severity filter, core::logging::logger** logger) const
		{
			if (logger == nullptr)
				return false;

			utils::ref_count_ptr<logging::boost_logger_impl> pool_instance;
			if (m_loggers_pool.get_logger(name, filter, &pool_instance) == false)
				return false;

			if (pool_instance == nullptr)
				return false;

			*logger = pool_instance;
			(*logger)->add_ref();
			return true;
		}

		bool create_stream(core::logging::severity severity, core::logging::log_stream ** stream) const
		{
			if (stream == nullptr)
				return false;

			if (m_filter > severity)
				return false;

			if (::boost::log::core::get()->get_logging_enabled() == false)
				return false;

			utils::ref_count_ptr<logging::log_stream_impl> pool_instance;
			if (m_streams_pool.get_stream(severity, &pool_instance) == false)
				return false;

			if (pool_instance == nullptr)
				return false;

			*stream = pool_instance;
			(*stream)->add_ref();
			return true;
		}
	};

	LOGGER LOGGER::instance;
}

logging::log_stream_impl::log_stream_impl()
{
}

logging::log_stream_impl::~log_stream_impl() noexcept
{
}

inline int logging::log_stream_impl::release() const
{
	int count = utils::ref_count_base<core::logging::log_stream>::release();
	if (count == 1)
		m_record_wrapper.clear();

	return count;
}

core::logging::severity logging::log_stream_impl::severity() const
{
	return m_severity;
}

core::logging::log_stream& logging::log_stream_impl::operator<<(char c)
{
	return stream_operator_handler(c);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(const char* p)
{
	return stream_operator_handler(p);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(bool value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(signed char value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(unsigned char value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(short value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(unsigned short value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(int value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(unsigned int value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(long value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(unsigned long value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(long long value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(unsigned long long value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(float value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(double value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(long double value)
{
	return stream_operator_handler(value);
}

core::logging::log_stream& logging::log_stream_impl::operator<<(const void * value)
{
	return stream_operator_handler(value);
}

void logging::log_stream_impl::init(core::logging::severity severity)
{
	m_severity = severity;
	m_record_wrapper.init(convert(severity));
}

void logging::boost_logger_impl::set_name(const char * name)
{
	if (name == nullptr)
		name = UNDEFINED_NAME;

#ifdef _WIN32
	strncpy_s(m_name, name, sizeof(m_name) - 1);
#else
	std::memset(m_name, 0, sizeof(m_name));
	std::strncpy(m_name, name, sizeof(m_name) - 1);
#endif
}

logging::boost_logger_impl::boost_logger_impl(const char * name, core::logging::severity filter) :
	m_enabled(true),
	m_filter(filter)
{
	set_name(name);
}

const char* logging::boost_logger_impl::name() const
{
	return m_name;
}

bool logging::boost_logger_impl::enabled() const
{
	return m_enabled;
}

core::logging::severity logging::boost_logger_impl::filter() const
{
	return m_filter;
}

void logging::boost_logger_impl::enabled(bool val)
{
	m_enabled = val;
}

void logging::boost_logger_impl::filter(core::logging::severity severity)
{
	m_filter = severity;
}

bool logging::boost_logger_impl::log(core::logging::severity severity, const char * message) const
{
	utils::ref_count_ptr<core::logging::log_stream> stream;
	if (create_stream(severity, &stream) == false)
		return false;

	stream->operator<<(message);
	return true;
}

bool logging::boost_logger_impl::create_stream(core::logging::severity severity, core::logging::log_stream ** stream) const
{
	if (stream == nullptr)
		return false;

	if (m_filter > severity)
		return false;

	if (LOGGER::instance.create_stream(severity, stream) == false)
		return false;

	(*stream)->operator<<("[") << m_name << "]: ";
	return true;
}

bool logging::boost_logger::create(const char* name, core::logging::severity filter, core::logging::logger** logger)
{
	return logging::LOGGER::instance.create_logger(name, filter, logger);
}
