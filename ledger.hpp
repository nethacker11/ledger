#ifndef LEDGER_H
#define LEDGER_H

#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "networking.hpp"
#include "tcp_connection.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;
using boost::asio::io_service;
using json = nlohmann::json;

class Ledger {

public:
  vector<User*> users;
  boost::asio::streambuf b;
  vector<tcp_connection::pointer> connections;

  tcp::acceptor acceptor;

  Ledger(io_service& io_service) 
    : acceptor(io_service, tcp::endpoint(tcp::v4(), PORT)) {
    readConfig();
    startAccept();
  }

  void readConfig();
  void startAccept();
  void acceptHandler(tcp_connection::pointer new_connection, const boost::system::error_code& e);
  void HandleRead(User *user, const boost::system::error_code &e, size_t msg_len);
  void processCommand(json msg, User *user);
  void send(json msg, tcp::socket &socket);
  json recv(tcp::socket &socket);
  User *getUser(std::string name);

};

#endif
