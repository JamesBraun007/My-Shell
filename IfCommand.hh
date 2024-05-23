#ifndef ifcommand_hh
#define ifcommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"
#include "ListCommands.hh"

// Command Data Structure

class IfCommand : public Command {
public:
  SimpleCommand * _condition;
  ListCommands * _listCommands; 


  IfCommand();
  void insertCondition( SimpleCommand * condition );
  void insertListCommands( ListCommands * listCommands);
  void iterateFor(SimpleCommand * forArguments);
  static int runTest(SimpleCommand * condition);

  std::string * primaryArg;
  bool forLoop;
  bool whileLoop;
  void clear();
  void print();
  void execute();

};

#endif
