#include <iostream>
#include <string>
#include <sstream>

#include "user.hpp"
#include "ledger.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#define CONFIG_FILE "ledger.config"


namespace pt = boost::property_tree;

void Ledger::HandleRead(const boost::system::error_code &e) {

    cout << "in handle read" << endl;
  if (!e) {
    cout << "in handle read" << endl;
/*
    cout << "in handle read" << endl;
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, DELIM); 

    std::istream is(&buf);
    std::string name;
    is >> name;

    cout << name << endl;
    boost::asio::streambuf buf;

    std::istream is(&buf);
    std::string msg;
    is >> msg;

    cout << msg << endl;

    */

    b.consume(5);

    boost::asio::async_read_until(connections[0]->socket_, b, DELIM,
                     boost::bind(&Ledger::HandleRead, this,
                     boost::asio::placeholders::error));
  } else {
    cerr << "Error reading from client" << endl;
    cerr << e.message() << endl;
  }
}

void Ledger::startAccept() {

  tcp_connection::pointer new_connection = tcp_connection::create(acceptor.get_io_service());
  
    // Create socket to accept next client
    connections.push_back(new_connection);


    acceptor.async_accept(new_connection->socket_, boost::bind(&Ledger::acceptHandler, 
                          this, new_connection, boost::asio::placeholders::error));
}

void Ledger::acceptHandler(tcp_connection::pointer new_connection, const boost::system::error_code& e) {

  std::string msg;

  cout << "in accept handler" << endl;

  if (!e) {


    boost::asio::async_read_until(new_connection->socket_, b, DELIM, boost::bind(&Ledger::HandleRead, this, boost::asio::placeholders::error));
    cout << "passed async_read" << endl;
    /*
    User *tmp = getUser(name);

    // Make sure user exists and is not logged in
    if (tmp) {

      if (!tmp->isConnected) {
        cout << "found user" << endl;
        //tmp->socket = socket;
        //tmp->isConnected = true;
        async_read_until(*socket, tmp->buffer, DELIM,
                         boost::bind(&Ledger::HandleRead, this, tmp, _1, _2));
      } else {
        msg = "User already logged in";
        boost::asio::write(*socket, boost::asio::buffer(msg));
      }

    } else {

      msg = "Username not recognized";
      boost::asio::write(*socket, boost::asio::buffer(msg));

    }
  */
  }

    startAccept();
}

User *Ledger::getUser(std::string name) {

  for ( auto &i: users ) {

    if (name.compare(i->name) == 0) {
      return i;
    }
  }

  return NULL;
}


void Ledger::readConfig() {

  pt::ptree root;
  pt::read_json(CONFIG_FILE, root);

  // Populate users vector from config file
  BOOST_FOREACH(pt::ptree::value_type &v, root.get_child("Users")) {

    string name = v.first;
    float btc =  v.second.get<float>("btc");
    float eth =  v.second.get<float>("eth");

    users.push_back(new User(name, btc, eth));
  }
}


int main(int argc, char **argv) {

  io_service io_service;
  boost::asio::io_service::work work(io_service);

  Ledger ledger(io_service);
  cout << "ledger created" << endl;

  io_service.run();

  

  // Free allocated memory
  for ( auto &i: ledger.users ) {
    delete(i);
  }

  return 0;
}
