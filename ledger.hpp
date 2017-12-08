#include <vector>
#include <ctime>
#include <string>
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


    acceptor.async_accept(connections.back(), boost::bind(&Ledger::acceptHandler, this, boost::asio::placeholders::error));
  }

  void acceptHandler(const boost::system::error_code& e) {

    if (!e) {
    cout << "accepted new connection" << endl;
    }

    startAccept();

  }

  void readConfig();
  

};

std::string make_daytime_string()
{
    std:: time_t now = std::time(0);
      return std::ctime(&now);
}


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

  // Call boost::asio::async_write() to serve the data to the client. 
  // We are using boost::asio::async_write(), 
  // rather than ip::tcp::socket::async_write_some(), 
  // to ensure that the entire block of data is sent.

  void start()
  {
    // The data to be sent is stored in the class member m_message 
    // as we need to keep the data valid 
    // until the asynchronous operation is complete.
    m_message = make_daytime_string();

    // When initiating the asynchronous operation, 
    // and if using boost::bind(), 
    // we must specify only the arguments 
    // that match the handler's parameter list. 
    // In this code, both of the argument placeholders 
    // (boost::asio::placeholders::error 
    // and boost::asio::placeholders::bytes_transferred) 
    // could potentially have been removed, 
    // since they are not being used in handle_write().

    boost::asio::async_write(socket_, boost::asio::buffer(m_message),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }
  // handle_write() is responsible for any further actions 
  // for this client connection.
  void handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/)
  {
  }

  tcp::socket socket_;
  std::string m_message;
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
