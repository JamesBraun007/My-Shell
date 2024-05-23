#ifndef listcommand_hh
#define listcommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"

class ListCommands : public Command {
public:
  std::vector<Command *> _commands;

  ListCommands();
  void insertCommand( Command * Command );
  void execute();
  void clear();
  void print();
};

#endif
