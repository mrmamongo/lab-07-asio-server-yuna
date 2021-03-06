//
// Created by lamp on 26.02.2021.
//

#include <server.hpp>

int main(){
  server_t server(1680);
  server.start_handler_thread();
}