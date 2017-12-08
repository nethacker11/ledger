#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#define PORT 8888

using boost::asio::ip::tcp;
using boost::asio::io_service;

/* Using tcp_connection class from 
 * http://www.bogotobogo.com/cplusplus/Boost/boost_AsynchIO_asio_tcpip_socket_server_client_timer_bind_handler_multithreading_synchronizing_network_D.php
*/

class Ledger {

public:
  vector<User*> users;
  vector<tcp::socket> connections;
  tcp::acceptor acceptor;

  Ledger(io_service& io_service) 
    : acceptor(io_service, tcp::endpoint(tcp::v4(), PORT)) {
    readConfig();
    startAccept();
  }

  void startAccept() {

    // Create socket to accept next client
    connections.push_back(tcp::socket(acceptor.get_io_service()));


    acceptor.async_accept(connections.back(), boost::bind(&Ledger::acceptHandler, this, &connections.back(), boost::asio::placeholders::error));
  }

  void acceptHandler(tcp::socket *socket, const boost::system::error_code& e) {

    if (!e) {
    cout << "accepted new connection" << endl;
    }

    std::string hello = "hi";

    boost::asio::write(*socket, boost::asio::buffer(hello));
    

    startAccept();

  }

  void readConfig();
  

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
