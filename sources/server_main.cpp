//
// Created by lamp on 26.02.2021.
//

#include <server.hpp>

int main(){
  server_t server(2281337);
  server.start_acceptor_thread();
  server.start_handler_thread();
}