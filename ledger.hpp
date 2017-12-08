#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "networking.hpp"

using boost::asio::ip::tcp;
using boost::asio::io_service;

class tcp_connection
  // Using shared_ptr and enable_shared_from_this 
  // because we want to keep the tcp_connection object alive 
  // as long as there is an operation that refers to it.
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new tcp_connection(io_service));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  tcp::socket socket_;
};

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
  void HandleRead(const boost::system::error_code &e, size_t msg_len);
  User *getUser(std::string name);

};





/*
class Server {

public:
  vector<tcp::socket> connections;
  tcp::acceptor acceptor;

  Server(io_service& io_service) 
    : acceptor(io_service, tcp::endpoint(tcp::v4(), PORT)) {
      startAccept();
    }

  void startAccept() {

    // Create socket to accept next client
    connections.push_back(tcp::socket(acceptor.get_io_service()));

    acceptor.async_accept(




  }

};
*/
