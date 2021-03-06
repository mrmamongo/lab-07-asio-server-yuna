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

int main(){
  client_t client("Nick");
  client.connect("127.0.0.1", 1680);
}
