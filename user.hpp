#include <iostream>
#include <string>

using namespace std;


class User {

public:
  string name;
  float btc; 
  float eth;

  User(string name, float btc, float eth)
    : name(name), btc(btc), eth(eth)
    {}
  
  User() {}
};
