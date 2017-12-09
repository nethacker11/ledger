#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "tcp_connection.hpp"

using namespace std;
using boost::asio::ip::tcp;


class User {

public:
  string name;
  int btc; 
  int eth;
  bool isConnected;
  boost::asio::streambuf buffer;
  tcp_connection::pointer connection;
  //tcp::socket *socket; // can't store the socket, is this a good idea?

  User(string name, int btc, int eth)
    : name(name), btc(btc), eth(eth), isConnected(false)
    {}
  
};
#endif
