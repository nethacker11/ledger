#include <iostream>
#include <string>
#include <sstream>
#include "user.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

namespace pt = boost::property_tree;


int main(int argc, char **argv) {

  vector<User*> users;

  pt::ptree root;
  pt::read_json("ledger.config", root);

  BOOST_FOREACH(pt::ptree::value_type &v, root.get_child("Users")) {

    string name = v.first;
    float btc =  v.second.get<float>("btc");
    float eth =  v.second.get<float>("eth");

    users.push_back(new User(name, btc, eth));
    //pt::ptree &subtree = v.second; // value (or a subnode)
  }

  for( auto &i : users ) {
    cout << i->name << endl;
    cout << i->btc << endl;
  }

  std::stringstream s;
  pt::json_parser::write_json(s, root);

  cout << s.str() << endl;


  return 0;
}
