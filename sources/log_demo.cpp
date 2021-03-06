// Copyright 2020 Lamp
#include <logger.hpp>

#include "../include/common/common.hpp"

int main(){
  logger::logger log;
  LOG(log, l_info) << "This is " << "separated " << "info message" << "karoche";
  json message_header;
  message_header["cs_ping"] = 1.0;
  message_header["owner"] = "server";
  message_header["message_id"] = "header";
  message_header["message"] = 100500;

  LOG(log, l_info) << message_header.dump() << "karoche";
}