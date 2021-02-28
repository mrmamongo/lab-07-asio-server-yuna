//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_SERVER_HPP
#define TEMPLATE_SERVER_HPP

#include <common.hpp>

class server_t : connection{
 public:
  server_t(uint32_t port): connection(owner::server, "server_"){
    _acceptor = acceptor_t(*_pcontext, boost::asio::ip::tcp::v4(), port);
  }
 private:

};


#endif  // TEMPLATE_SERVER_HPP
