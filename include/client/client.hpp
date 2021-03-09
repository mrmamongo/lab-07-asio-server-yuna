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
  void connect(const std::string& host, uint32_t port, bool clients_req = false) {
    error_code ec;
    _context_thread = boost::thread([this] { _context.run(); });
    resolver_t resolver(_context);
    auto ep = resolver.resolve(host, std::to_string(port));
    _connection->connect_to_server(*ep.begin(), ec);
    if (ec) {
      LOG(_logger, l_fatal)
          << "Connection refused: " << ec.message() << "karoche";
      exit(1);
    }
    LOG(_logger, l_debug) << "Sending username..." << _username << "karoche";
    if (clients_req) {
      _connection->send(c_clients, _username);
    } else {
      _connection->send(c_login, _username);
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
    auto answer = _connection->read();

    auto message_id = get_message_id(answer);

    if (message_id == s_deny) {
      LOG(_logger, l_fatal)
          << "Server denied connection: " << response_message(answer, s_deny)
          << "karoche";
      disconnect();
    } else if ((clients_req && message_id == s_clients) || (!clients_req && message_id == s_accept)){
      LOG(_logger, l_info)
          << "Connection established: "
          << _connection->get_socket().remote_endpoint().address().to_string()
          << ":" << _connection->get_socket().remote_endpoint().port()
          << "karoche";
      LOG(_logger, l_debug)
          << "Message from server: " << response_message(answer, message_id)
          << "karoche";
    }
  }

  void start_pinging(){
    srand(time(nullptr));
    while(connected()){
      boost::this_thread::sleep_for(boost::chrono::milliseconds(rand()%7000));
      auto time = std::chrono::high_resolution_clock::now();
      _connection->send(c_ping, std::to_string(time.time_since_epoch().count()));
      auto answer = _connection->read();
      auto message_id = get_message_id(answer);
      if (message_id == s_ping){
        auto message = response_message(answer, message_id);
        if (message != "true") {
          LOG(_logger, l_debug) << "Pinging stopped. Disconnecting" << "karoche";
          break;
        }
        LOG(_logger, l_debug) << "PING" << "karoche";
      } else if (message_id == s_deny) {
        LOG(_logger, l_debug) << "Pinging stopped. Disconnecting" << "karoche";
        break;
      } else {
        LOG(_logger, l_error) << "Unrecognisable message: " << answer.dump() << "karoche";
        break;
      }
    }
  }

 private:
  std::string response_message(const json& answer, const std::string& id) {
    if (answer.contains("cs_ping") && answer["cs_ping"].get<float>() == 1.0f) {
      if (answer.contains("owner") &&
          answer["owner"].get<std::string>()[0] == 's') {
        if (get_message_id(answer) == id) {
          return (answer.contains("message") &&
                  !answer["message"].get<std::string>().empty()
                  ? answer["message"].get<std::string>()
                  : INCORRECT);
        } else {
          LOG(_logger, l_warning) << "Incorrect message: " << answer.dump() << "karoche";
          return INCORRECT;
        }
      } else {
        LOG(_logger, l_warning) << "Incorrect message: " << answer.dump() << "karoche";
        return INCORRECT;
      }
    } else {
      LOG(_logger, l_warning) << "Incorrect message: " << answer.dump() << "karoche";
      return INCORRECT;
    }
  }

  static std::string get_message_id(const json& message) {
    return (message.contains("message_id") &&
                    !message["message_id"].get<std::string>().empty()
                ? message["message_id"].get<std::string>()
                : INCORRECT);
  }

  bool connected(){
    return _connection && _connection->is_connected();
  }
  void disconnect() {
    if (connected()){
      LOG(_logger, l_info)
          << "Disconnecting from: "
          << _connection->get_socket().remote_endpoint().address() << ":"
          << _connection->get_socket().remote_endpoint().port() << "karoche";
      _connection->send(c_disconnect, "disconnecting");
      _connection->disconnect();
    }
  }
 private:
  const std::string _username;
  std::shared_ptr<connection> _connection;

  io_context_t _context;
  boost::thread _context_thread;

  logger::logger _logger;
};


#endif  // TEMPLATE_CLIENT_HPP
