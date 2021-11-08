/// @file	logging/boost_logger.h.
/// @brief	Declares the boost logger class
#pragma once
#include <core/logging.h>

#ifdef _WIN32
#undef ERROR
#endif

namespace logging
{	
	/// @class	boost_logger
	/// @brief	Implementation of core::logging::logger based on the Boost C++ library logging capabilities
	/// @date	15/05/2018
	class DLL_EXPORT boost_logger : public core::logging::logger
	{
	public:
		/// @fn	virtual boost_logger::~boost_logger() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~boost_logger() = default;

		/// @fn	static bool boost_logger::create(const char* name, core::logging::severity filter, core::logging::logger** logger);
		/// @brief	Static factory: Creates a new logger instance
		/// @date	15/05/2018
		/// @param 		   	name  	The logger name.
		/// @param 		   	filter	Specifies the severity filter.
		/// @param [out]	logger	An address of a pointer to core::logging::logger
		/// @return	True if it succeeds, false if it fails.
		static bool create(const char* name, core::logging::severity filter, core::logging::logger** logger);
	};
}