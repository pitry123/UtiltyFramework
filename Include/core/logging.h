/// @file	core/logging.h.
/// @brief	Declares the logging interface classes
#pragma once
#include <core/ref_count_interface.h>

#ifdef _WIN32
#undef ERROR
#endif

#ifdef VXWORKS
#undef ERROR
#endif

namespace core
{
	namespace logging
	{
		/// @enum	severity
		/// @brief	Values that represent logging severities
		enum severity
		{
			TRACE,
			DEBUG,
			INFO,
			WARNING,
			ERROR,
			FATAL
		};

		/// @class	log_stream
		/// @brief	An interface defining a logger stream.
		/// @date	14/05/2018
		class DLL_EXPORT log_stream : public core::ref_count_interface
		{
		public:
			/// @fn	virtual log_stream::~log_stream() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~log_stream() = default;

			/// @fn	virtual logging::severity log_stream::severity() const = 0;
			/// @brief	Gets the logger severity
			/// @date	14/05/2018
			/// @return	A logging::severity.
			virtual logging::severity severity() const = 0;

			/// @fn	virtual log_stream& operator<< (char c) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	c	A char.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (char c) = 0;

			/// @fn	virtual log_stream& operator<< (const char* p) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	p	A const char* (C style string).
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (const char* p) = 0;

			/// @fn	virtual log_stream& operator<< (bool value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A bool.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (bool value) = 0;

			/// @fn	virtual log_stream& operator<< (signed char value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A signed char.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (signed char value) = 0;

			/// @fn	virtual log_stream& operator<< (unsigned char value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An unsigned char.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (unsigned char value) = 0;

			/// @fn	virtual log_stream& operator<< (short value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A short.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (short value) = 0;

			/// @fn	virtual log_stream& operator<< (unsigned short value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An unsigned short.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (unsigned short value) = 0;

			/// @fn	virtual log_stream& operator<< (int value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An int.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (int value) = 0;

			/// @fn	virtual log_stream& operator<< (unsigned int value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An unsigned int.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (unsigned int value) = 0;

			/// @fn	virtual log_stream& operator<< (long value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A long.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (long value) = 0;

			/// @fn	virtual log_stream& operator<< (unsigned long value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An unsigned long.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (unsigned long value) = 0;

			/// @fn	virtual log_stream& operator<< (long long value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A long long.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (long long value) = 0;

			/// @fn	virtual log_stream& operator<< (unsigned long long value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	An unsigned long long.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (unsigned long long value) = 0;

			/// @fn	virtual log_stream& operator<< (float value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A float.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (float value) = 0;

			/// @fn	virtual log_stream& operator<< (double value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A double.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (double value) = 0;

			/// @fn	virtual log_stream& operator<< (long double value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A long double.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (long double value) = 0;

			/// @fn	virtual log_stream& operator<< (const void* value) = 0;
			/// @brief	Stream operator
			/// @date	14/05/2018
			/// @param	value	A buffer.
			/// @return	A reference to this log_straem.
			virtual log_stream& operator<< (const void* value) = 0;
		};

		/// @class	logger
		/// @brief	An interface defining a logger.
		/// @date	14/05/2018
		class DLL_EXPORT logger : public core::ref_count_interface
		{
		public:
			/// @fn	virtual logger::~logger() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~logger() = default;

			/// @fn	virtual const char* logger::name() const = 0;
			/// @brief	Gets the logger's name
			/// @date	14/05/2018
			/// @return	Null if it fails, else a pointer to a const char.
			virtual const char* name() const = 0;

			/// @fn	virtual bool logger::enabled() const = 0;
			/// @brief	Get whether the logging is enabled
			/// @date	14/05/2018
			/// @return	True if enabled, false if disabled.
			virtual bool enabled() const = 0;

			/// @fn	virtual core::logging::severity logger::filter() const = 0;
			/// @brief	Gets the current severity filter
			/// @date	14/05/2018
			/// @return	A core::logging::severity.
			virtual core::logging::severity filter() const = 0;

			/// @fn	virtual void logger::enabled(bool val) = 0;
			/// @brief	Sets enabled
			/// @date	14/05/2018
			/// @param	val	True to enable, false to disable.
			virtual void enabled(bool val) = 0;

			/// @fn	virtual void logger::filter(core::logging::severity severity) = 0;
			/// @brief	Sets the severity filter
			/// @date	14/05/2018
			/// @param	severity	The logging severity.
			virtual void filter(core::logging::severity severity) = 0;

			/// @fn	virtual bool logger::log(core::logging::severity severity, const char* message) const = 0;
			/// @brief	Logs a message
			/// @date	14/05/2018
			/// @param	severity	The severity.
			/// @param	message 	The message.
			/// @return	True if it succeeds, false if it fails.
			virtual bool log(core::logging::severity severity, const char* message) const = 0;

			/// @fn	virtual bool logger::create_stream(core::logging::severity severity, core::logging::log_stream** stream) const = 0;
			/// @brief	Creates a log_stream for this logger
			/// @date	14/05/2018
			/// @param 		   	severity	The severity.
			/// @param [out]	stream  	An address to a stream pointer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool create_stream(core::logging::severity severity, core::logging::log_stream** stream) const = 0;
		};
	}
}