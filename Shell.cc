
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <sys/wait.h>

#include "Command.hh"
#include "Shell.hh"

int yyparse(void);

Shell * Shell::TheShell;

Shell::Shell() {
    this->argvCopy = NULL;
    this->numArgs = 0;
    this->path = "";
    this->lastArg = "";
    this->_backgroundPID = 0;
    this->_returnCode = 0;
    this->_level = 0;
    this->_enablePrompt = true;
    this->_listCommands = new ListCommands(); 
    this->_simpleCommand = new SimpleCommand();
    this->_pipeCommand = new PipeCommand();
    this->_currentCommand = this->_pipeCommand;
    std::vector<ListCommands*> listCommandStack;
    std::vector<SimpleCommand*> conditionStack;
    std::vector<IfCommand*> ifCommandStack;
    ListCommands * _currentListCommand;
    if ( !isatty(0)) {
	this->_enablePrompt = false;
    }
}

void Shell::prompt() {
    if (_enablePrompt) {
	printf("\rmyshell>");
	fflush(stdout);
    }
}

void Shell::print() {
    printf("\n--------------- Command Table ---------------\n");
    this->_listCommands->print();
}

void Shell::clear() {
    this->_listCommands->clear();
    this->_simpleCommand->clear();
    this->_pipeCommand->clear();
    this->_currentCommand->clear();
    this->_level = 0;
    this->_backgroundPID = 0;
    this->_returnCode = 0;
    this->lastArg = "";
    this->path = "";
    this->argvCopy = NULL;
    this->numArgs = 0;
}


void Shell::execute() {
  if (this->_level == 0 ) {
    //this->print();
    this->_listCommands->execute();
    this->_listCommands->clear();
    this->prompt();
  }
}

void yyset_in (FILE *  in_str );

int 
main(int argc, char **argv) {

  char * input_file = NULL;
  if ( argc > 1 ) {
    input_file = argv[1];
    FILE * f = fopen(input_file, "r");
    if (f==NULL) {
	fprintf(stderr, "Cannot open file %s\n", input_file);
        perror("fopen");
        exit(1);
    }
    yyset_in(f);
  }  

  Shell::TheShell = new Shell();
  Shell::TheShell->path = argv[0];
  Shell::TheShell->numArgs = argc - 2;
  Shell::TheShell->argvCopy = argv;


  if (input_file != NULL) {
    // No prompt if running a script
    Shell::TheShell->_enablePrompt = false;
  }
  else {
    Shell::TheShell->prompt();
  }
  yyparse();
}


