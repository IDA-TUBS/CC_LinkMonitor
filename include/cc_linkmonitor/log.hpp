#ifndef CC_LOG_h
#define CC_LOG_h

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/support/date_time.hpp>

#include <cstdlib>
#include <iostream>

/**
 * @brief Preprocessor wrapper for boost logging library
 * 
 */
#ifdef LOG_ON

#define CC_logTrace(msg) BOOST_LOG_TRIVIAL(trace) << msg; 
#define CC_logDebug(msg) BOOST_LOG_TRIVIAL(debug) << msg;
#define CC_logInfo(msg) BOOST_LOG_TRIVIAL(info) << msg;
#define CC_logWarning(msg) BOOST_LOG_TRIVIAL(warning) << msg;
#define CC_logError(msg) BOOST_LOG_TRIVIAL(error) << msg;
#define CC_logFatal(msg) BOOST_LOG_TRIVIAL(fatal) << msg;
#define AppLog(msg) BOOST_LOG_TRIVIAL(debug) << msg;

#else
#define CC_logTrace(msg)
#define CC_logDebug(msg)
#define CC_logInfo(msg)
#define CC_logWarning(msg)
#define CC_logError(msg)
#define CC_logFatal(msg)
#define AppLog(msg)
#endif

namespace cc_linkmonitor {

/**
 * @brief initialize general log messages on console
 * 
 */
void init_logging();

/**
 * @brief initialize logging to file. Only severity::trace is logged. Format is CSV. Intended for internal RM functions.
 * 
 * @param log_prefix log filename prefix
 * @param log_suffix log filename suffix
 */
void init_file_log(std::string log_prefix, std::string log_suffix);

/**
 * @brief initialize logging to file. Only severity::debug is logged. Format is CSV. Intended for (external) Applications using RM functions.
 * 
 * @param log_prefix log filename prefix
 * @param log_suffix log filename suffix
 */
void init_app_log(std::string log_prefix, std::string log_suffix);

/**
 * @brief initialize logging to file. Only severity::debug is logged. Format is CSV. Intended for (external) Applications using RM functions.
 * 
 * @param log_file Path to log file
 */
void init_app_log(std::string log_file);

};

#endif