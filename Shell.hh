#ifndef shell_hh
#define shell_hh

#include "ListCommands.hh"
#include "PipeCommand.hh"
#include "IfCommand.hh"

class Shell {

public:
  int _level; // Only outer level executes.
  bool _enablePrompt;
  ListCommands * _listCommands; 
  SimpleCommand *_simpleCommand;
  PipeCommand * _pipeCommand;
  IfCommand * _ifCommand;
  Command * _currentCommand;
  static Shell * TheShell;
  int _returnCode;
  pid_t _backgroundPID;
  const char * lastArg;
  const char * path;
  int numArgs;
  char ** argvCopy;
  std::vector<ListCommands*> listCommandStack;
  std::vector<SimpleCommand*> conditionStack;
  std::vector<IfCommand*> ifCommandStack;
  ListCommands * _currentListCommand;

  Shell();
  void execute();
  void print();
  void clear();
  void prompt();

};

#endif
