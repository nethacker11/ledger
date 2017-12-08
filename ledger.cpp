#include <iostream>
#include <string>
#include <sstream>

#include "user.hpp"
#include "ledger.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#define CONFIG_FILE "ledger.config"


namespace pt = boost::property_tree;

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
