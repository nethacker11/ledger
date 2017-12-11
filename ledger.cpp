#include <iostream>
#include <string>
#include <sstream>

#include "user.hpp"
#include "ledger.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>

#define CONFIG_FILE "ledger.config"


namespace pt = boost::property_tree;

using json = nlohmann::json;


void Ledger::send(json msg, tcp::socket &socket) {

  ostringstream os;
  os << msg;
  boost::asio::write(socket, boost::asio::buffer(os.str() + DELIM));
}

json Ledger::recv(tcp::socket &socket) {

  boost::asio::streambuf recv_buf;
  size_t msg_len = boost::asio::read_until(socket, recv_buf, DELIM);

  boost::asio::streambuf::const_buffers_type bufs = recv_buf.data();
  string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());

  return json::parse(msg);

}

void Ledger::processCommand(json msg, User *user) {

  json response;
  string command = msg["command"];

  if (command == "pay") {

    string recv_user = msg["user"];
    string currency = msg["currency"];
    int amount = msg["amount"];

    User *tmp = getUser(recv_user);

    if (tmp) {

      // Check for sufficient funds
      if (currency == "btc") {
        if (user->btc >= amount) {
          // execute pay
          user->btc -= amount;
          tmp->btc += amount;

          // log transaction
          user->transactions.push_back(user->name +  " paid " + recv_user
                                        + " " + std::to_string(amount) + " " + currency);

          response["message"] = "Pay successful";
        } else {
          response["message"] = "Insufficient funds";
        }
      } else {
        if (user->eth >= amount) {
          // execute pay
          user->eth -= amount;
          tmp->eth += amount;

          // log transaction
          user->transactions.push_back(user->name +  " paid " + recv_user
                                        + " " + std::to_string(amount) + " " + currency);

          response["message"] = "Pay successful";
        } else {
          response["message"] = "Insufficient funds";
        }
      }
    } else {
      response["message"] = "User not found";
    }
  } else if (command == "balance") {

    string currency = msg["currency"];
    int current_amount;
    if (currency == "btc") {
      current_amount = user->btc;
    } else {
      current_amount = user->eth;
    }
    response["message"] = to_string(current_amount);

  } else if (command == "log") {

    string response_msg;

    if (msg["user"] == "mine") {
 
      for ( auto &i: user->transactions ) {

        response_msg += i;
        response_msg += '\n';
      }
    } else {

      for ( auto &i: users ) {
        for ( auto &j: i->transactions ) {

          response_msg += j;
          response_msg += '\n';

        }
      }
    }

    if (response_msg.length() > 0) {
      response_msg.erase(response_msg.length()-1, 1);
    }
    response["message"] = response_msg;
  }
  send(response, user->connection->socket_);
}

void Ledger::HandleRead(User *user, const boost::system::error_code &e, size_t msg_len) {

  if (!e) {

    json j;

    boost::asio::streambuf::const_buffers_type bufs = user->buffer.data();
    string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());
    j = json::parse(msg);

    mutex.lock();
    processCommand(j, user);
    mutex.unlock();

    user->buffer.consume(msg_len + DELIM.length());

    boost::asio::async_read_until(user->connection->socket_, user->buffer, DELIM,
                     boost::bind(&Ledger::HandleRead, this, user,
                     boost::asio::placeholders::error,
                     boost::asio::placeholders::bytes_transferred));
  } else {
    // set flag so user can reconnect
    user->isConnected = false;
    cerr << user->name + " disconnected" << endl;
    
  }
}

void Ledger::startAccept() {

  tcp_connection::pointer new_connection = tcp_connection::create(acceptor.get_io_service());
  
    // Create socket to accept next client

    acceptor.async_accept(new_connection->socket_, boost::bind(&Ledger::acceptHandler, 
                          this, new_connection, boost::asio::placeholders::error));
}

void Ledger::acceptHandler(tcp_connection::pointer new_connection, const boost::system::error_code& e) {

  string msg;

  if (!e) {

    json login_msg = recv(new_connection->socket_);
    string name = login_msg["name"];


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
        
        cout << name + " logged in" << endl;

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

User *Ledger::getUser(string name) {

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

  boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
  signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

  io_service.run();
  
  // Free allocated memory
  for ( auto &i: ledger.users ) {
    delete(i);
  }

  return 0;
}
