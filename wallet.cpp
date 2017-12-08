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

int main(int argc, char **argv) {


  if (argc != 2) {
    cout << "usage: wallet <name>" << endl;
    exit(1);
  }

  json j;

  j["name"] = "phil";
  


  io_service io_service;
  boost::asio::io_service::work work(io_service);

  tcp::socket socket(io_service);

  tcp::resolver resolver(io_service);
  tcp::resolver::query query(ADDRESS, PORT_STR);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  
  boost::asio::connect(socket, endpoint_iterator);

  std::string msg = "hello";

  boost::asio::write(socket, boost::asio::buffer(j.dump()));





  return 0;
}
