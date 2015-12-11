/*
 * Copyright (c) 2015, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SRC_COMPONENTS_INCLUDE_UTILS_LOGGER_H_
#define SRC_COMPONENTS_INCLUDE_UTILS_LOGGER_H_

#ifdef ENABLE_LOG

#include <errno.h>
#include <string>
#include <sstream>
#include <cstdint>

#if defined(LOG4CXX_LOGGER)
#include <log4cxx/logger.h>
#include <log4cxx/spi/loggingevent.h>
#elif defined(WIN_NATIVE)
#include <windows.h>
#elif defined(QT_PORT)
// Qt includes goes here
#else
#error Unsupported case for logging includes
#endif

namespace logger {
  bool init_logger(const std::string& file_name);
  void deinit_logger();

  bool logs_enabled();
  void set_logs_enabled(bool state);
} // namespace logger

#define INIT_LOGGER_WITH_CFG(file_name) logger::init_logger(file_name)

#if defined(LOG4CXX_LOGGER)
#define INIT_LOGGER() INIT_LOGGER_WITH_CFG("log4cxx.properties")
#else
// win and qt loggers don't use config file
#define INIT_LOGGER() INIT_LOGGER_WITH_CFG("")
#endif

#define DEINIT_LOGGER() logger::deinit_logger()

#define CREATE_LOGGERPTR_GLOBAL(logger_var, logger_name) \
    namespace { \
      CREATE_LOGGERPTR_LOCAL(logger_var, logger_name); \
    }

#define LOG4CXX_ERROR_WITH_ERRNO(logger, message) \
    LOG4CXX_ERROR(logger, message << ", error code " << errno << " (" << strerror(errno) << ")")

#define LOG4CXX_WARN_WITH_ERRNO(logger, message) \
    LOG4CXX_WARN(logger, message << ", error code " << errno << " (" << strerror(errno) << ")")

#if defined(OS_POSIX)

namespace logger {
  bool push_log(log4cxx::LoggerPtr logger,
                log4cxx::LevelPtr level,
                const std::string& entry,
                log4cxx_time_t timeStamp,
                const log4cxx::spi::LocationInfo& location,
                const log4cxx::LogString& threadName);
  log4cxx_time_t time_now();
}

#define CREATE_LOGGERPTR_LOCAL(logger_var, logger_name) \
    log4cxx::LoggerPtr logger_var = log4cxx::LoggerPtr(log4cxx::Logger::getLogger(logger_name));

#define LOG4CXX_IS_TRACE_ENABLED(logger) logger->isTraceEnabled()

#define LOG_WITH_LEVEL(loggerPtr, logLevel, logEvent) \
do { \
     std::stringstream accumulator; \
     accumulator << logEvent; \
     logger::push_log(loggerPtr, logLevel, accumulator.str(), logger::time_now(), \
         LOG4CXX_LOCATION, ::log4cxx::spi::LoggingEvent::getCurrentThreadName()); \
} while (false)

#undef LOG4CXX_TRACE
#define LOG4CXX_TRACE(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getTrace(), logEvent)

#define LOG4CXX_AUTO_TRACE_WITH_NAME_SPECIFIED(loggerPtr, auto_trace) \
    logger::AutoTrace auto_trace(loggerPtr, LOG4CXX_LOCATION)
#define LOG4CXX_AUTO_TRACE(loggerPtr) LOG4CXX_AUTO_TRACE_WITH_NAME_SPECIFIED(loggerPtr, SDL_local_auto_trace_object)

#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getDebug(), logEvent)

#undef LOG4CXX_INFO
#define LOG4CXX_INFO(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getInfo(), logEvent)

#undef LOG4CXX_WARN
#define LOG4CXX_WARN(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getWarn(), logEvent)

#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getError(), logEvent)

#undef LOG4CXX_FATAL
#define LOG4CXX_FATAL(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, ::log4cxx::Level::getFatal(), logEvent)

#elif defined(WIN_NATIVE)

namespace logger {
  bool push_log(const std::string& logger,
                uint32_t level,
                SYSTEMTIME time,
                const std::string& entry);
  SYSTEMTIME time_now();
}

#define CREATE_LOGGERPTR_LOCAL(logger_var, logger_name) \
    std::string logger_var(logger_name);

#define LOG_WITH_LEVEL(loggerPtr, logLevel, logEvent, line) \
do { \
     std::stringstream accumulator; \
     accumulator << __FILE__ << ":" << line << " " << __FUNCTION__ << ": " << logEvent; \
     logger::push_log(loggerPtr, logLevel, logger::time_now(), accumulator.str()); \
} while (false)

#define LOG_WITH_LEVEL_EXT(loggerPtr, logLevel, logEvent, line, file, function) \
do { \
     std::stringstream accumulator; \
     accumulator << file << ":" << line << " " << function << ": " << logEvent; \
     logger::push_log(loggerPtr, logLevel, logger::time_now(), accumulator.str()); \
} while (false)

namespace logger {
  class AutoTrace {
   public:
    AutoTrace(const std::string& logger_ptr,
              unsigned long line_number,
              const char* file_name,
              const char* function_name)
     : logger_ptr_(logger_ptr),
       line_number_(line_number),
       file_name_(file_name),
       function_name_(function_name) {
      LOG_WITH_LEVEL_EXT(logger_ptr_, 0, "Enter", line_number_, file_name_, function_name_);
    };
    ~AutoTrace() {
      LOG_WITH_LEVEL_EXT(logger_ptr_, 0, "Exit", line_number_, file_name_, function_name_);
    };
   private:
    const std::string   logger_ptr_;
    const unsigned long line_number_;
    const char*         file_name_;
    const char*         function_name_;
  };
}  // namespace logger

#define LOG4CXX_IS_TRACE_ENABLED(logger) true

#undef LOG4CXX_TRACE
#define LOG4CXX_TRACE(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 0, logEvent, __LINE__)

#define LOG4CXX_AUTO_TRACE(loggerPtr) \
    logger::AutoTrace auto_trace(loggerPtr, __LINE__, __FILE__, __FUNCTION__);

#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 1, logEvent, __LINE__)

#undef LOG4CXX_INFO
#define LOG4CXX_INFO(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 2, logEvent, __LINE__)

#undef LOG4CXX_WARN
#define LOG4CXX_WARN(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 3, logEvent, __LINE__)

#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 4, logEvent, __LINE__)

#undef LOG4CXX_FATAL
#define LOG4CXX_FATAL(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 5, logEvent, __LINE__)

#elif defined(QT_PORT) // logging macroses for the Qt case

namespace logger {
  bool push_log(
    const std::string& logger,
    const uint32_t level,
    const std::string& entry,
    const char* file,
    const unsigned long line);
} // namespace logger

#define CREATE_LOGGERPTR_LOCAL(logger_var, logger_name) \
    std::string logger_var(logger_name);

#define LOG4CXX_IS_TRACE_ENABLED(logger)

#define LOG_WITH_LEVEL(loggerPtr, logLevel, logEvent, file, line)   \
do {                                                                \
     std::stringstream accumulator;                                 \
     accumulator << logEvent;                                       \
     logger::push_log(                                              \
       loggerPtr,                                                   \
       logLevel,                                                    \
       accumulator.str(),                                           \
       file,                                                        \
       line);                                                       \
} while (false)

#undef LOG4CXX_TRACE
#define LOG4CXX_TRACE(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 0, logEvent, __FILE__, __LINE__)

#define LOG4CXX_AUTO_TRACE_WITH_NAME_SPECIFIED(loggerPtr, auto_trace)
#define LOG4CXX_AUTO_TRACE(loggerPtr)

#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 1, logEvent, __FILE__, __LINE__)

#undef LOG4CXX_INFO
#define LOG4CXX_INFO(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 2, logEvent, __FILE__, __LINE__)

#undef LOG4CXX_WARN
#define LOG4CXX_WARN(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 3, logEvent, __FILE__, __LINE__)

#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 4, logEvent, __FILE__, __LINE__)

#undef LOG4CXX_FATAL
#define LOG4CXX_FATAL(loggerPtr, logEvent) LOG_WITH_LEVEL(loggerPtr, 5, logEvent, __FILE__, __LINE__)

#endif // end of cases when logging is enabled
  
#else  // ENABLE_LOG is OFF

#define CREATE_LOGGERPTR_GLOBAL(logger_var, logger_name)
#define CREATE_LOGGERPTR_LOCAL(logger_var, logger_name)
#define INIT_LOGGER(file_name)
#define DEINIT_LOGGER(file_name)
#define LOG4CXX_IS_TRACE_ENABLED(logger) false

#undef LOG4CXX_TRACE
#define LOG4CXX_TRACE(x, y)

#define LOG4CXX_AUTO_TRACE_WITH_NAME_SPECIFIED(loggerPtr, auto_trace)
#define LOG4CXX_AUTO_TRACE(loggerPtr)

#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(x, y)

#undef LOG4CXX_INFO
#define LOG4CXX_INFO(x, y)

#undef LOG4CXX_WARN
#define LOG4CXX_WARN(x, y)

#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(x, y)

#undef LOG4CXX_ERROR_WITH_ERRNO
#define LOG4CXX_ERROR_WITH_ERRNO(x, y)

#undef LOG4CXX_WARN_WITH_ERRNO
#define LOG4CXX_WARN_WITH_ERRNO(x, y)

#undef LOG4CXX_FATAL
#define LOG4CXX_FATAL(x, y)

#endif  // ENABLE_LOG

#endif  // SRC_COMPONENTS_INCLUDE_UTILS_LOGGER_H_
