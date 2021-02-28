// Copyright 2020 Lamp
#include <logger.hpp>

int main(){
  logger::logger log;
  log.log("Hello, this is info message", logger::l_info);
  log.log("Hello, this is trace message", logger::l_trace);
  log.log("Hello, this is debug message", logger::l_debug);
  log.log("Hello, this is warning message", logger::l_warning);
  log.log("Hello, this is error message", logger::l_error);
  log.log("Hello, this is fatal message", logger::l_fatal);
}