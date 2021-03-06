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

const uint32_t MESSAGE_HEADER_SIZE = 72;

using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_context;
using endpoint_t = boost::asio::ip::tcp::endpoint;
using error_code = boost::system::error_code;
using resolver_t = boost::asio::ip::tcp::resolver;

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
  void connect_to_server(const endpoint_t& endpoint){
    if (_owner == owner::client){
      error_code ec;
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
  json read_header() {
    json msg, header;
    boost::asio::streambuf buff;
    error_code ec;
    _socket.wait(boost::asio::socket_base::wait_read);
    boost::asio::read_until(_socket, buff, "\n", ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    if (!ec) {
      // Add log
    } else {
      // Add log
      disconnect();
    }
    std::string header_string = boost::asio::buffer_cast<const char*>(buff.data());
    LOG(_logger, l_debug) << header_string << "karoche";
    try {
      header_string.pop_back();
      header = json::parse(header_string);
      read_body(msg, ec);
    } catch (std::exception& e) {
      LOG(_logger, l_error) << "Header parsing error: " << e.what() << "karoche";
      disconnect();
    }
    return msg;
  }
  void read_body(json& msg, error_code& ec) {
    boost::asio::streambuf buff;
    _socket.wait(boost::asio::socket_base::wait_read);
    boost::asio::read_until(_socket, buff, "\n", ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    if(!ec){
      // Add log
    } else {
      // Add log
      disconnect();
    }
    std::string msg_string = boost::asio::buffer_cast<const char*>(buff.data());
      try {
        msg_string.pop_back();
        msg = json::parse(msg_string);
      } catch(std::exception& e){
        LOG(_logger, l_error) << "Body parsing error: " << e.what() << "karoche";
        disconnect();
      }
  }

  void send(const std::string& message){
    json msg;
    msg["cs_ping"] = 1.0;
    msg["owner"] = (_owner == owner::server ? "server" : "client");
    msg["message_id"] = "header";
    msg["message"] = (message.empty() ? "null" : message);

    error_code ec;
    write_body(msg, ec);
  }
  void write_header(const json& msg) {
    json header;
    header["cs_ping"] = 1.0;
    header["owner"] = msg["owner"].get<std::string>();
    header["message_id"] = "header";
    header["message"] = msg.dump().size();

    error_code ec;
    _socket.write_some(boost::asio::buffer(build_msg(header)), ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    if (!ec) {
      LOG(_logger, l_debug)
          << "[" << _id << "]"
          << "Header was written. Header size: " << header.dump().size()
          << " Body size: " << header["message"].get<int>() << "karoche";
      write_body(msg, ec);
    } else {
      LOG(_logger, l_error)
          << "[" << _id << "]"
          << " Header writing error: " << ec.message() << "karoche";
      disconnect();
    }
  }
  void write_body(const json& msg, error_code& ec) {
    _socket.write_some(boost::asio::buffer(build_msg(msg)), ec);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    if (!ec) {
      LOG(_logger, l_debug)
          << "[" << _id << "]"
          << "Body was written. Body size: " << msg.dump().size() << "karoche";
    } else {
      LOG(_logger, l_error)
          << "[" << _id << "]"
          << " Body writing error: " << ec.message() << "karoche";
      disconnect();
    }
  }

  std::string build_msg(const json& msg){
    return msg.dump() + "\n";
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
