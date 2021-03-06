//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_CLIENT_HPP
#define TEMPLATE_CLIENT_HPP

#include <common.hpp>

class client_t {
 public:
  client_t(const std::string& username): _username(username), _logger("client_")
  {
    _connection = (std::make_shared<connection>(connection::owner::client, _logger, _context));
  }

  ~client_t(){
    disconnect();
    if(_context_thread.joinable()){
      _context_thread.join();
    }
  }

 public:
  void connect(const std::string& host, uint32_t port) {
    _context_thread = boost::thread([this] { _context.run(); });
    resolver_t resolver(_context);
    auto ep = resolver.resolve(host, std::to_string(port));
    _connection->connect_to_server(*ep.begin());
    LOG(_logger, l_debug) << "Sending username..." << _username << "karoche";
    _connection->send(_username);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    auto answer = _connection->read();

    LOG(_logger, l_info)
        << "Connection established: "
        << _connection->get_socket().remote_endpoint().address().to_string()
        << ":" << _connection->get_socket().remote_endpoint().port()
        << "karoche";
    LOG(_logger, l_debug) << "Message from server: " << answer << "karoche";
  }



 private:
  void disconnect(){
    _connection->disconnect();
  }
 private:
  const std::string _username;
  std::shared_ptr<connection> _connection;

  io_context_t _context;
  boost::thread _context_thread;

  logger::logger _logger;
};


#endif  // TEMPLATE_CLIENT_HPP
