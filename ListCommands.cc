
#include "ListCommands.hh"

ListCommands::ListCommands() {
}

void 
ListCommands::insertCommand( Command * command )
{
    _commands.push_back(command);
}

void 
ListCommands::execute() 
{
    for (size_t i = 0; i < _commands.size(); i++) {
      _commands[i]->execute();
    }
}

void 
ListCommands::clear()
{
    for (size_t i = 0; i < _commands.size(); i++) {
      _commands[i]->clear();
    }
    _commands.clear();
}

void 
ListCommands::print()
{
    for (size_t i = 0; i < _commands.size(); i++) {
      _commands[i]->print();
    }
}

