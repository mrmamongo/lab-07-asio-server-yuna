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
#include <boost/thread/recursive_mutex.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
 * These strings contain declared message headers
 * cause the Author is so lazy to declare true
 * enumerations and to convert them to strings
 */

// Client responses
const std::string c_login = "c_login";
const std::string c_clients = "c_clients";
const std::string c_ping = "c_ping";
const std::string c_disconnect = "c_disconnect";
// Server responses
const std::string s_accept = "s_accept";
const std::string s_deny = "s_deny";
const std::string s_clients = "s_clients";
const std::string s_ping = "s_ping";

// For incorrect responses
const std::string INCORRECT = "INCORRECT";
const std::string DISCONNECTED = "DISCONNECTED";


using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_service;
using endpoint_t = boost::asio::ip::tcp::endpoint;
using error_code = boost::system::error_code;
using resolver_t = boost::asio::ip::tcp::resolver;
using streambuf_t = boost::asio::streambuf;



class connection {
 public:
  enum class owner { client, server, empty };

  connection(owner parent, logger::logger& logger,
             io_context_t& context)
  :_logger(logger), _context(context), _socket(_context){
      _owner = parent;
  };

  connection(owner parent, logger::logger& logger,
             io_context_t& context, socket_t&& socket)
  :_logger(logger), _context(context), _socket(std::move(socket)){
      _owner = parent;
  };


  virtual ~connection(){
      disconnect();
  };

  socket_t& get_socket(){
    return _socket;
  }

  [[nodiscard]] size_t get_id() const { return _id; }

  public:
  void connect_to_server(const endpoint_t& endpoint, error_code& ec){
    if (_owner == owner::client){
      _socket.connect(endpoint, ec);
      if(!ec){
        LOG(_logger, l_info) << "Connected to server" << "karoche";
      } else {
        LOG(_logger, l_error) << "Connection error: " << ec.message() << "karoche";
      }
    }
  }
  void connect_to_client(uint32_t id){
    if (_owner == owner::server){
      if(is_connected()){
        _id = id;
      }
    }
  }
  void disconnect(){
    if (is_connected()){
      _socket.close();
    }
  }
  bool is_connected() {
    return _socket.is_open();
  }

 public:
  json read(){
    json body;
    error_code ec;
    read_body(body, ec);
    return body;
  }
  void send(const std::string& message_id, const std::string& message = std::string()){
    json msg;
    msg["cs_ping"] = 1.0;
    msg["owner"] = (_owner == owner::server ? "server" : "client");
    msg["message_id"] = message_id;
    msg["message"] = message;

    error_code ec;
    write_body(msg, ec);
  }
 private:
  void read_body(json& msg, error_code& ec) {
    streambuf_t buff;
    _socket.wait(boost::asio::socket_base::wait_read);
    boost::asio::read_until(_socket, buff, "\t", ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    if(!ec){
      LOG(_logger, l_debug) << (_owner == owner::client ? "" : std::string("[" + std::to_string(_id) + "]"))
                            << " Message read" << "karoche";
    } else {
      LOG(_logger, l_error) << (_owner == owner::client ? "" : std::string("[" + std::to_string(_id) + "]"))
          << "Message reading error: " << ec.message() << "karoche";
      disconnect();
    }
    std::string msg_string = boost::asio::buffer_cast<const char*>(buff.data());
      try {
        msg_string.pop_back();
        msg = json::parse(msg_string);
      } catch(std::exception& e){
        LOG(_logger, l_error)
            << (_owner == owner::client ? "" : std::string("[" + std::to_string(_id) + "]"))
            << "Body parsing error: " << e.what() << "karoche";
        disconnect();
      }
  }



  void write_body(const json& msg, error_code& ec) {
    _socket.wait(boost::asio::socket_base::wait_write);
    _socket.write_some(boost::asio::buffer(build_msg(msg)), ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

    if (!ec) {
      LOG(_logger, l_debug)
          << (_owner == owner::client ? "" : std::string("[" + std::to_string(_id) + "]"))
          << " Message was written. Body size: " << msg.dump().size() << "karoche";
    } else {
      LOG(_logger, l_error)
          << (_owner == owner::client ? "" : std::string("[" + std::to_string(_id) + "]"))
          << " Message writing error: " << ec.message() << "karoche";
      disconnect();
    }
  }

  std::string build_msg(const json& msg){
    return msg.dump() + "\t";
  }
 protected:
  logger::logger& _logger;

  io_context_t& _context;

  socket_t _socket;

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
