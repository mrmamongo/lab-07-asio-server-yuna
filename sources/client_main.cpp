//
// Created by lamp on 26.02.2021.
//

#include <client.hpp>
/*
 * json {
 * "cs_ping" : <VERSION> // string
 * "owner": server/client
 * }
 */

int main(int argc, char** argv){
  switch (argc){
    default:
    case 1:{ // Empty call
      client_t client("Guest");
      client.connect("127.0.0.1", 2281337, true);
      client.start_pinging();
    } break;
    case 2:{ // Call with username
      client_t client(argv[1]);
      client.connect("127.0.0.1", 2281337);
      client.start_pinging();
    } break;
    case 3:{ // Call with username and clients request
      client_t client(argv[1]);
      client.connect("127.0.0.1", 2281337, true);
      client.start_pinging();
    } break;
  }
}
