//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_COMMON_HPP
#define TEMPLATE_COMMON_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <vector>
#include <deque>
#include <iostream>
#include <logger.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class message_types {
  server_accept = 0,
  server_deny = 1,
  server_ping = 2,
  client_login = 10,
  client_ping = 11,
  client_list = 12
};

/* message_types
 * server_accept, server deny, client_login
 * server_ping, client_ping
 * client_list, server_list
 */

/*
 * json {
 * "cs_ping" : <VERSION> // string
 * "owner": server/client
 * "message_id": message_types // in integers
 * }
 */

using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_context;
using endpoint_t = boost::asio::ip::tcp::endpoint;

class connection {
 public:
  enum class owner { client, server, empty };

  connection() {
    _owner = owner::empty;
    _socket = socket_t(*_pcontext);
  }

  connection(owner parent, const std::string& log_name){
      _owner = parent;
      _logger = std::make_unique<logger::logger>(log_name);
  };

  virtual ~connection() = 0;

  [[nodiscard]] size_t get_id() const { return _id; }

 protected:
  virtual void connect() = 0;
  virtual void disconnect() = 0;

  void is_connected() {
    if (_owner == owner::client){
      _socket.close();
    }
  }

  virtual void send() = 0;
  virtual void read() = 0;
 protected:
  std::unique_ptr<logger::logger> _logger;

  std::unique_ptr<io_context_t> _pcontext;
  std::thread _context_thread;

  /*
   * Client uses _socket to establish connection to server
   *
   * Server uses _acceptor to accept connections from clients
   */
  union {
    socket_t _socket;
    acceptor_t _acceptor;
  };

  /*
   * Client uses this variable as his unique id
   *
   * Server uses this variable as counter for new clients
   */
  unsigned short _id = 0;

 private:
  owner _owner;
};

#endif  // TEMPLATE_COMMON_HPP
