#ifndef simplecommand_hh
#define simplecommand_hh

#include <string>
#include <vector>

class SimpleCommand {
public:

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();
  void clear();
};

#endif
