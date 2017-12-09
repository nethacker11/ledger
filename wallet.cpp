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

json recv(tcp::socket &socket) {

  boost::asio::streambuf recv_buf;
  size_t msg_len = boost::asio::read_until(socket, recv_buf, DELIM);

  boost::asio::streambuf::const_buffers_type bufs = recv_buf.data();
  std::string msg(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + msg_len - DELIM.length());

  return json::parse(msg);
}

bool checkPayString(std::string pay_string) {

  size_t str_len = pay_string.length();

  if (str_len > 3) {
    std::string currency = pay_string.substr(str_len - 3);
    std::string amount = pay_string.substr(0, str_len - 3);

    if (currency == "btc" || currency == "eth") {

      if (strtol(amount.c_str(), NULL, 10) != 0) {

        return true;
      }
    }
  } else {
    return false;
  }
  return false;
}

bool checkBalanceString(std::string balance_string) {

  if (balance_string == "btc" || balance_string == "eth") {
    return true;
  }
  return false;
}

bool checkLogString(std::string log_string) {

  if (log_string == "mine" || log_string == "all") {
    return true;
  }
  return false;
}

bool processText(std::vector<std::string> parsed_text, tcp::socket &socket) {

  /* valid commands:
   * pay
   * balance
   * log
   */

  json msg;
  bool valid = false;
  std::string command = parsed_text[0];


  if (command == "pay") {

    if (parsed_text.size() > 2) {
      if (checkPayString(parsed_text[2])) {
        //build msg

        valid = true;

        std::string currency = parsed_text[2].substr(parsed_text[2].length() - 3);
        std::string amount = parsed_text[2].substr(0, parsed_text[2].length() - 3);

        msg["command"] = "pay";
        msg["user"] = parsed_text[1];
        msg["currency"] = currency;
        msg["amount"] = stoi(amount);

      } else {
        // improper format
        cout << "pay string failed" << endl;
      }
    } else {
      // improper format
      cout << "pay string failed" << endl;
    }

  } else if (command == "balance") {
    
    if (checkBalanceString(parsed_text[1])) {
      // build msg

      valid = true;

      msg["command"] = "balance";
      msg["currency"] = parsed_text[1];

    } else {
      cout << "balance string failed" << endl;
    }

  } else if (command == "log") {

    if (checkLogString(parsed_text[1])) {
      // build msg

      valid = true;

      msg["command"] = "log";
      msg["user"] = parsed_text[1];
    } else {
      cout << "log string failed" << endl;
    }

  } else {
    // invalid command
    cout << "invalid command" << endl;
  }

  if (valid) {
    send(msg, socket);
  }

  return valid;
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

  // aknowdledge login
  login(name, socket);
  json response = recv(socket);

  if (response["response"] == "success") {
    cout << "Successfully logged in" << endl;
  } else {
    cout << response["response"] << endl;
    exit(1);
  }
  
  

  std::string text;
  std::string prompt = "Command: ";

  std::vector<std::string> parsed_text;
  
  for (;;) { 
  
    parsed_text.clear();

    cout << prompt;
    getline(cin, text);

    boost::split(parsed_text, text, boost::is_any_of(" "));

    // Every command needs 2 arguments,
    // but we will allow for more than
    // 2 and ignore the rest
    if (parsed_text.size() > 1) {
      if (processText(parsed_text, socket)) {
      json response = recv(socket);
      std::string response_msg = response["message"];
      cout << response_msg << endl;
      } else {
        continue;
      }
    } else {
      cout << "error: invalid command" << endl;
    }
    
  }





  return 0;
}


  /*
  cout << msg_len << endl;
  recv_buf.commit(msg_len);

  std::istream is(&recv_buf);
  std::string msg;
  is >> msg;

  */
