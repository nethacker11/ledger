#include <iostream>
#include <string>
#include <sstream>

#include "json.hpp"
#include "networking.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using namespace std;
namespace pt = boost::property_tree;
using json = nlohmann::json;


/*

   { 
     "name" : "phil"
     "command" : "login", 
   }

   {
     "name" : "phil"
     "command" : "pay"

     {
        "name" : "hannah",
        "currency" : "btc",
        "amount" : 333
     }
  }
  
*/

void send(json msg, tcp::socket &socket) {

  ostringstream os;
  os << msg;
  boost::asio::write(socket, boost::asio::buffer(os.str() + DELIM));
}

json makeMsg(std::string command, std::string data) {

  json msg;

  if (command == "login") {

    msg["name"] = data;
    msg["command"] = "login";
  }

  return msg;
}

void login(std::string name, tcp::socket &socket) {

  json login = makeMsg("login", name);
  send(login, socket);
}

int main(int argc, char **argv) {


  if (argc != 2) {
    cout << "usage: wallet <name>" << endl;
    exit(1);
  }


  io_service io_service;
  boost::asio::io_service::work work(io_service);

  tcp::socket socket(io_service);

  tcp::resolver resolver(io_service);
  tcp::resolver::query query(ADDRESS, PORT_STR);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  
  try {
    boost::asio::connect(socket, endpoint_iterator);
  } catch (const boost::system::system_error &e) {

    cerr << "Could not connect to server" << endl;
    exit(1);
  }

  std::string name = argv[1];

  login(name, socket);

  boost::asio::streambuf recv_buf;
  size_t msg_len = boost::asio::read_until(socket, recv_buf, DELIM);

  boost::asio::streambuf::const_buffers_type bufs = recv_buf.data();
  std::string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());

  /*
  cout << msg_len << endl;
  recv_buf.commit(msg_len);

  std::istream is(&recv_buf);
  std::string msg;
  is >> msg;

  */
  json tmp = json::parse(msg);

  cout << tmp["response"] << endl;
  
  
  for (;;) { }





  return 0;
}
