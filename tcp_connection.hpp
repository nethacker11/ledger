#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


using boost::asio::ip::tcp;
using boost::asio::io_service;

/* tcp_connection class taken from
 * http://www.bogotobogo.com/cplusplus/Boost/boost_AsynchIO_asio_tcpip_socket_server_client_timer_bind_handler_multithreading_synchronizing_network_D.php
 */

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

#endif
