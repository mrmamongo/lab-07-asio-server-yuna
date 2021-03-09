//
// Created by lamp on 26.02.2021.
//

#ifndef TEMPLATE_SERVER_HPP
#define TEMPLATE_SERVER_HPP

#include <common.hpp>

class server_t {
  // Pairs: name / connection
  typedef std::pair<std::string, std::shared_ptr<connection>> client;

 public:
  explicit server_t(uint32_t port) : _logger("server_"), _port(port) {}
  ~server_t() {
    _context_thread.join();
    _handler_context_thread.join();
    _threads.join_all();
    _connections.clear();
  }

 public:
  void start_handler_thread() {
    _handler_context_thread = boost::thread([this] { _handler_context.run(); });
    LOG(_logger, l_info) << "Executing handler_thread"
                         << "karoche";
    _threads.add_thread(new boost::thread(&server_t::handler_thread, this));
  }

  void start_acceptor_thread() {
    _context_thread = boost::thread([this] { _context.run(); });
    LOG(_logger, l_info) << "Executing acceptor_thread"
                         << "karoche";
    _threads.add_thread(new boost::thread(&server_t::acceptor_thread, this));
  }

 private:
  [[noreturn]] void handler_thread() {
    LOG(_logger, l_info) << "Handler_thread started"
                         << "karoche";
    while (true) {
      // LOG(_logger, l_info) << "Handler thread updating..."
      //                     << "karoche";
      boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
      boost::recursive_mutex::scoped_lock lock(_mutex);
      if (!_connections.empty()) {
        bool found_null = false;
        for (auto& connection : _connections) {
          if (connection.second && !connection.second->is_connected()) {
            connection.first = DISCONNECTED;
            connection.second = nullptr;
            found_null = true;
            continue;
          }
          if (_clients_changed) {
            continue;
          }
          boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
          auto answer = connection.second->read();
          // std::cout << "\n\n" << answer.dump() << "\n\n";
          auto message_id = id_recognize(answer);
          if (message_id == c_disconnect) {
            LOG(_logger, l_debug)
                << "Client: " << connection.first << " disconnected"
                << "karoche";
            connection.first = DISCONNECTED;
            connection.second = nullptr;
            found_null = true;
            continue;
          } else if (message_id == c_ping) {
            LOG(_logger, l_debug) << "[" << connection.second->get_id() << "]"
                                  << " Client: " << connection.first << " PONG";
            pong(connection, message_recognize(answer));
          } else {
            connection.second->send(c_disconnect, "Incorrect pinging");
            LOG(_logger, l_error) << "Client: " << connection.first << " disconnected cause of incorrect pinging" << "karoche";
            connection.second = nullptr;
            connection.first = DISCONNECTED;
            found_null = true;
            continue;
          }

          // auto ping = connection.second->read();
        }
        _clients_changed = false;
        if (found_null) {
          _connections.erase(
              std::remove(
                  _connections.begin(), _connections.end(),
                  std::make_pair<std::string, std::shared_ptr<connection>>(
                      std::string{DISCONNECTED}, nullptr)),
              _connections.end());
        }
      }
    }
  }

  [[noreturn]] void acceptor_thread() {
    error_code ec;
    auto _acceptor =
        acceptor_t(_context, endpoint_t(boost::asio::ip::tcp::v4(), _port));
    LOG(_logger, l_info) << "Acceptor_thread started"
                         << "karoche";
    while (true) {
      LOG(_logger, l_debug)
          << "Acceptor thread updated, waiting for new connection..."
          << "karoche";
      auto socket = _acceptor.accept(ec);
      if (ec) {
        LOG(_logger, l_error)
            << "Acceptor error: " << ec.message() << "karoche";
        continue;
      }
      LOG(_logger, l_debug)
          << "Accepted connection: "
          << socket.remote_endpoint().address().to_string() << "karoche";
      auto connection_ = std::make_shared<connection>(
          connection::owner::server, _logger, _context, std::move(socket));
      connection_->connect_to_client(_id_counter++);
      std::string client_data =
          "[" + std::to_string(connection_->get_id()) + "][" +
          connection_->get_socket().remote_endpoint().address().to_string() +
          ":" +
          std::to_string(connection_->get_socket().remote_endpoint().port()) +
          "]";

      auto answer = connection_->read();  // Get the name of the client
      boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

      auto message_id = id_recognize(answer);

      if (message_id == INCORRECT) {
        connection_->send(s_deny, "Message id is incorrect");
        LOG(_logger, l_error)
            << client_data << "Client sent incorrect message: " << answer.dump()
            << "karoche";
      } else if (message_id == c_login || message_id == c_clients) {
        auto message = message_recognize(answer);  // gets name from the message
        auto num = find_name(message);
        std::stringstream out;
        if (num == -1) {
          out << "Welcome to the party, " + message + "!";
        } else {
          out << "Back again? So, welcome, " + message + "!";
        }
        if (message_id == c_login) {
          connection_->send(s_accept, out.str());
        } else {
          fill_clients(message, out);
          connection_->send(s_clients, out.str());
        }
        if (num == -1) {
          emplace_client(message, std::move(connection_));
        } else {
          boost::recursive_mutex::scoped_lock lock(_mutex);
          std::swap(_connections[num].second, connection_);
        }
      } else {
        connection_->send(s_deny, "Message id did not recognized");
        LOG(_logger, l_error)
            << client_data
            << "Client sent unrecognizable message: " << answer.dump()
            << "karoche";
      }
    }
  }

  std::string id_recognize(const json& message) {
    if (message.contains("cs_ping") &&
        message["cs_ping"].get<float>() == 1.0f) {
      if (message.contains("owner") &&
          message["owner"].get<std::string>() == "client") {
        if (message.contains("message_id") && !message["message_id"].empty() &&
            message["message_id"].is_string()) {
          auto message_id = message["message_id"].get<std::string>();
          if (message_id[0] == 'c') {
            return message_id;
          } else {
            LOG(_logger, l_warning)
                << "Incorrect message: " << message.dump() << "karoche";
            return INCORRECT;
          }
        } else {
          LOG(_logger, l_warning)
              << "Incorrect message: " << message.dump() << "karoche";
          return INCORRECT;
        }
      } else {
        LOG(_logger, l_warning)
            << "Incorrect message, check the version. Message: "
            << message.dump() << "karoche";
        return INCORRECT;
      }
    } else {
      LOG(_logger, l_warning)
          << "Incorrect message, check the owner of the connection. Message: "
          << message.dump() << "karoche";
      return INCORRECT;
    }
  }

  void pong(client& cl, std::string message) {
    auto time =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    try {
      if (time - atol(message.c_str()) < 5001000000) {
        cl.second->send(s_ping, "true");
      }
    } catch (std::exception& e) {
      LOG(_logger, l_fatal)
          << "String to long conversion error."
          << " Disconnecting client: " << cl.first << "karoche";
      cl.second->disconnect();
      cl.first = DISCONNECTED;
      cl.second = nullptr;
    }
  }

  std::string message_recognize(const json& message){
    if(message.contains("message") && !message["message"].empty() && message["message"].is_string()) {
      auto msg = message["message"].get<std::string>();
      return msg;
    } else {
      LOG(_logger, l_warning) << "Incorrect message: " << message.dump() << "karoche";
      return INCORRECT;
    }
  }

  void emplace_client(const std::string& name, std::shared_ptr<connection>&& conn) {
    // Sending the client to the vector of the clients
    // TODO: connect the DB that contains clients' names and their ip(?). SQLite?
    boost::recursive_mutex::scoped_lock lock(_mutex);
    _connections.emplace_back(std::make_pair(name, std::move(conn)));
    _clients_changed = true;
  }

  void fill_clients(const std::string& name, std::stringstream& out) {
    for (auto& clnt : _connections) {
      if (name != clnt.first && clnt.second != nullptr)
        out << clnt.first << " - "
            << clnt.second->get_socket().remote_endpoint().address().to_string()
            << ":" << clnt.second->get_socket().remote_endpoint().port()
            << "\n";
    }
  }

 private:

  int find_name(const std::string& name){
    size_t i = 0;
    for (; i < _connections.size(); ++i){
      if (_connections[i].first == name){
        return i;
      }
    }
    return -1;
  }
 private:

  std::vector<client> _connections;

  io_context_t _context;
  io_context_t _handler_context;
  boost::thread _context_thread;
  boost::thread _handler_context_thread;
  boost::thread_group _threads;

  logger::logger _logger;

  uint32_t _id_counter = 1000;
  const uint32_t _port = 1601;

  boost::recursive_mutex _mutex;

  bool _clients_changed = false;
};


#endif  // TEMPLATE_SERVER_HPP
