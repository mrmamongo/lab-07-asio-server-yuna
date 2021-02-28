//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_CLIENT_HPP
#define TEMPLATE_CLIENT_HPP

#include <common.hpp>

logger::logger_t logger::logger::_logger;

class client_t : public connection {
 public:
  client_t(const std::string& username)
      : connection(owner::client, "client"), _username(username){
    _socket = socket_t(*_pcontext);
  }

  ~client_t() override{
    disconnect();
  }

 public:



 private:
  void disconnect() override{
    if(_socket.is_open()){
      _logger->log("Closing connection to server...", logger::l_info);
      _socket.close();
    }
  }

  void connect() override{

  }

  void send() override{

  }

  void read() override{

  }
 private:
  const std::string _username;
};


#endif  // TEMPLATE_CLIENT_HPP
