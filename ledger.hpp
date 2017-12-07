#include <vector>


class Ledger {

public:
  vector<User*> users;

  Ledger() {
    readConfig();
  }

  void readConfig();

};
