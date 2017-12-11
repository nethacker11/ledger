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
#include <boost/algorithm/string.hpp>
#include <boost/asio/signal_set.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using namespace std;
namespace pt = boost::property_tree;
using json = nlohmann::json;

static volatile sig_atomic_t stop = 0;

void send(json msg, tcp::socket &socket) {

  ostringstream os;
  os << msg;
  boost::asio::write(socket, boost::asio::buffer(os.str() + DELIM));
}

json recv(tcp::socket &socket) {

  boost::asio::streambuf recv_buf;
  size_t msg_len = boost::asio::read_until(socket, recv_buf, DELIM);

  boost::asio::streambuf::const_buffers_type bufs = recv_buf.data();
  string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());

  return json::parse(msg);
}

bool checkPayString(string &pay_string) {

  size_t str_len = pay_string.length();

  if (str_len > 3) {
    string currency = pay_string.substr(str_len - 3);
    string amount = pay_string.substr(0, str_len - 3);

    if (currency == "btc" || currency == "eth") {

      if (strtol(amount.c_str(), NULL, 10) > 0) {

        return true;
      }
    }
  } else {
    return false;
  }
  return false;
}

bool checkBalanceString(string &balance_string) {

  if (balance_string == "btc" || balance_string == "eth") {
    return true;
  }
  return false;
}

bool checkLogString(string &log_string) {

  if (log_string == "mine" || log_string == "all") {
    return true;
  }
  return false;
}

bool processText(std::vector<string> &parsed_text, tcp::socket &socket) {

  /* valid commands:
   * pay
   * balance
   * log
   */

  if (parsed_text.size() < 2) {
    cout << "insufficient arguments" << endl;
    return false;
  }

  json msg;
  bool valid = false;
  string command = parsed_text[0];

  //-------------------------------------------------pay

  if (command == "pay") {

    if (parsed_text.size() > 2) {
      if (checkPayString(parsed_text[2])) {
        //build msg

        valid = true;

        string currency = parsed_text[2].substr(parsed_text[2].length() - 3);
        string amount = parsed_text[2].substr(0, parsed_text[2].length() - 3);

        msg["command"] = "pay";
        msg["user"] = parsed_text[1];
        msg["currency"] = currency;
        msg["amount"] = stoi(amount);

      } else {
        // improper format
        cout << "pay command failed: invalid format" << endl;
      }
    } else {
      // improper format
      cout << "pay command failed: invalid format" << endl;
    }

  //-------------------------------------------------balance

  } else if (command == "balance") {
    
    if (checkBalanceString(parsed_text[1])) {
      // build msg

      valid = true;

      msg["command"] = "balance";
      msg["currency"] = parsed_text[1];

    } else {
      cout << "balance command failed: invalid format" << endl;
    }

  //-------------------------------------------------log

  } else if (command == "log") {

    if (checkLogString(parsed_text[1])) {
      // build msg

      valid = true;

      msg["command"] = "log";
      msg["user"] = parsed_text[1];
    } else {
      cout << "log command failed: invalid format" << endl;
    }

  } else {
    // invalid command
    cout << "command not recognized. valid commands:" << endl;
    cout << "pay, balance, log" << endl;
  }

  if (valid) {
    send(msg, socket);
  }

  return valid;
}


void login(string &name, tcp::socket &socket) {

  json login;

  login["name"] = name;
  login["command"] = "login";

  send(login, socket);
}

tcp::resolver::iterator networkSetup(io_service &io_service, tcp::socket &socket) {

  boost::asio::io_service::work work(io_service);

  tcp::resolver resolver(io_service);
  tcp::resolver::query query(ADDRESS, PORT_STR);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  return endpoint_iterator;
}

void sigHandler(int signum) {

  stop = 1;
}

int main(int argc, char **argv) {

  signal(SIGINT, sigHandler);

  if (argc != 2) {
    cout << "usage: wallet <name>" << endl;
    exit(1);
  }


  io_service io_service;
  tcp::socket socket(io_service);
  tcp::resolver::iterator endpoint_iterator = networkSetup(io_service, socket);

  try {
    boost::asio::connect(socket, endpoint_iterator);
  } catch (const boost::system::system_error &e) {

    cerr << "Could not connect to server" << endl;
    exit(1);
  }



  string name = argv[1];

  // aknowdledge login
  login(name, socket);
  json response = recv(socket);

  if (response["response"] == "success") {
    cout << "Successfully logged in" << endl;
  } else {
    cout << response["response"] << endl;
    exit(1);
  }
  
  
  string text;
  string prompt = "Command: ";

  std::vector<string> parsed_text;
  
  while (!stop) { 
  
    parsed_text.clear();

    cout << prompt;
    getline(cin, text);

    boost::split(parsed_text, text, boost::is_any_of(" "));

    if (processText(parsed_text, socket)) {
      json response = recv(socket);
      string response_msg = response["message"];
      cout << response_msg << endl;
    } else {
      continue;
    }

  }

  return 0;
}

