#include <iostream>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;


class User {

public:
  string name;
  float btc; 
  float eth;
  //tcp::socket socket; // can't store the socket, is this a good idea?

  User(string name, float btc, float eth)
    : name(name), btc(btc), eth(eth)
    {}
  
};
