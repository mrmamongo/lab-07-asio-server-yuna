//
// Created by lamp on 27.02.2021.
//

/*
 * This little logger is made to simplify the process
 * of logging for the /bmstu-iu8-cpp-sem-3/ - labs
 *
 * Copyright 2020 Lamp
 */

#ifndef TEMPLATE_LOGGER_HPP
#define TEMPLATE_LOGGER_HPP

#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
namespace logger{
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
typedef src::severity_logger<logging::trivial::severity_level> logger_t;

typedef logging::trivial::severity_level log_level_t;
#define l_info log_level_t::info
#define l_warning log_level_t::warning
#define l_error log_level_t::error
#define l_trace log_level_t::trace
#define l_debug log_level_t::debug
#define l_fatal log_level_t::fatal

class logger{
 public:
  explicit logger(const std::string& log_name = "log_"){
    initiate(log_name);
  }

  virtual ~logger() = default;

 public:
  template <typename log_data>
  void log(const log_data& value_to_log, const log_level_t& log_level){
    std::cout << "\n[LOG]"
              << "[" << to_string(log_level) << "]"
              << "[" << boost::this_thread::get_id() << "]: "
              << value_to_log << "\n";
    BOOST_LOG_SEV(_logger, log_level) << value_to_log;
  } // Logs a message and prints it on the console
 private:
   static std::string to_string(const log_level_t& level) {
    switch (level) {
      case log_level_t::info: {
        return "info";
      } break;
      case log_level_t::trace: {
        return "trace";
      } break;
      case log_level_t::debug: {
        return "debug";
      } break;
      case log_level_t::warning: {
        return "warning";
      } break;
      case log_level_t::error: {
        return "error";
      } break;
      case log_level_t::fatal: {
        return "fatal";
      } break;
    }
    return std::string();
  }

  static void add_common_attributes(){
    auto core = logging::core::get();
    core->add_global_attribute("TID", attrs::current_thread_id());
    core->add_global_attribute("TimeStamp", attrs::local_clock());
  }
  static void initiate(const std::string& log_name){
    add_common_attributes();
    logging::add_file_log(
        keywords::file_name = "logs/" + log_name + "%5N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation =
            sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%][%Severity%][%TID%]: %Message%");
  }
  static logger_t _logger;
};
logger_t logger::logger::_logger;
}
#endif  // TEMPLATE_LOGGER_HPP
