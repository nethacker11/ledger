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

using json = nlohmann::json;


void Ledger::send(json msg, tcp::socket &socket) {

  ostringstream os;
  os << msg;
  boost::asio::write(socket, boost::asio::buffer(os.str() + DELIM));
}

void Ledger::processCommand(json msg, User *user) {

  json response;
  std::string command = msg["command"];

  if (command == "pay") {

    std::string recv_user = msg["user"];
    std::string currency = msg["currency"];
    int amount = msg["amount"];

    User *tmp = getUser(recv_user);

    if (tmp) {

      // Check for sufficient funds
      if (currency == "btc") {
        if (user->btc >= amount) {
          // execute pay
          user->btc -= amount;
          tmp->btc += amount;
        } else {
          response["message"] = "Insufficient funds";
        }
      } else {
        if (user->eth >= amount) {
          //execute pay
          user->eth -= amount;
          tmp->eth += amount;
        } else {
          response["message"] = "Insufficient funds";
        }
      }
    } else {
      response["message"] = "User not found";
    }
  }
  send(response, user->connection->socket_);
}

void Ledger::HandleRead(User *user, const boost::system::error_code &e, size_t msg_len) {

  if (!e) {

    cout << "in handle read" << endl;
    json j;

    /*
    cout << user->buffer.size() << endl;
    user->buffer.commit(msg_len);

    std::istream is(&user->buffer);
    std::string msg;
    is >> msg;
    */

    boost::asio::streambuf::const_buffers_type bufs = user->buffer.data();
    std::string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());
    j = json::parse(msg);

    cout << j.dump() << endl;
    processCommand(j, user);

    for ( auto &i: users ) {

      cout << i->name << endl;
      cout << "btc: " << i->btc << endl;
      cout << "eth: " << i->eth << endl;
    }


    user->buffer.consume(msg_len + DELIM.length());

    boost::asio::async_read_until(user->connection->socket_, user->buffer, DELIM,
                     boost::bind(&Ledger::HandleRead, this, user,
                     boost::asio::placeholders::error,
                     boost::asio::placeholders::bytes_transferred));
  } else {
    // set flag so user can reconnect
    user->isConnected = false;
    cerr << "Error reading from client, client disconnected" << endl;
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

    boost::asio::streambuf buf;
    boost::asio::read_until(new_connection->socket_, buf, DELIM); 

    std::istream is(&buf);
    std::string login;
    is >> login;

    json login_msg = json::parse(login);
    std::string name = login_msg["name"];

    cout << name << endl;

    User *tmp = getUser(name);
    json response;

    // Make sure user exists and is not logged in
    if (tmp) {

      if (!tmp->isConnected) {

        tmp->connection = new_connection;
        tmp->isConnected = true;

        boost::asio::async_read_until(tmp->connection->socket_, tmp->buffer, DELIM, 
            boost::bind(&Ledger::HandleRead, this, tmp,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));


        response["response"] = "success";

      } else {

        response["response"] = "User already logged in";
      }
    } else {

      response["response"] = "Username not recognized";
    }

    send(response, new_connection->socket_);

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
    int btc =  v.second.get<int>("btc");
    int eth =  v.second.get<int>("eth");

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
