//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_SERVER_HPP
#define TEMPLATE_SERVER_HPP

#include <common.hpp>

class server_t{
 public:
  explicit server_t(uint32_t port)
      : _logger("server_"), _port(port){
  }
  ~server_t(){
    _context_thread.join();
    _threads.join_all();
    _connections.clear();
  }
 public:
  void start_handler_thread(){
    _context_thread = boost::thread([this]{
                        _context.run();
                      });
    LOG(_logger, l_info) << "Executing handler thread" << "karoche";
    _threads.add_thread(new boost::thread(&server_t::handler_thread, this));
  }

 private:
  [[noreturn]] void handler_thread() {
    error_code ec;
    auto _acceptor =
        acceptor_t(_context, endpoint_t(boost::asio::ip::tcp::v4(), _port));
    LOG(_logger, l_info) << "Handler thread started"
                         << "karoche";
    while (true) {

      auto socket = _acceptor.accept(ec);
      if (ec){
        LOG(_logger, l_error) << "Acceptor error: " << ec.message() << "karoche";
      }
      LOG(_logger, l_debug) << "Accepted connection: " << socket.remote_endpoint().address().to_string() << "karoche";
      auto connection_ = std::make_shared<connection>(connection::owner::server,
                                                      _logger, _context, std::move(socket));
      connection_->connect_to_client(_id_counter++);

      connection_->get_socket().wait(boost::asio::socket_base::wait_read);
      auto answer = connection_->read();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
      auto name = response_name(answer);
      connection_->send("Welcome to the party, " + name + "!");

      LOG(_logger, l_info)
          << "Client: [" << connection_->get_id() << "]["
          << connection_->get_socket().remote_endpoint().address().to_string()
          << ":" << connection_->get_socket().remote_endpoint().port()
          << "] connected successfully. Username: " << name
          << "karoche";

      {
        std::lock_guard<std::mutex> lock(_mutex);
        _connections.emplace_back(name, connection_);
      }
    }
  }

  std::string response_name(const json& answer){
    if (answer.contains("message") && answer["message"].is_string() && !answer["message"].get<std::string>().empty()){
      return answer["message"].get<std::string>();
    } else {
      return std::string();
    }
  }
 private:
  std::vector<std::pair<std::string, std::shared_ptr<connection>>> _connections; // Pairs: name / connection

  io_context_t _context;
  boost::thread _context_thread;
  boost::thread_group _threads;

  logger::logger _logger;

  uint32_t _id_counter = 1000;
  const uint32_t _port = 1601;

  std::mutex _mutex;
};


#endif  // TEMPLATE_SERVER_HPP
