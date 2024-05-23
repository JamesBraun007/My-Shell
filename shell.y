
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD 
%token NOTOKEN GREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT 
%token AMPERSAND PIPE LESS NEWLINE IF FI THEN LBRACKET RBRACKET SEMI RIGHTBRACKETSEMI
%token DO DONE WHILE FOR IN

%{
//#define yylex yylex
#include <cstdio>
#include "Shell.hh"

void yyerror(const char * s);
int yylex();
extern int yylex_destroy();
bool ifFlag = false;

%}

%%

goal: command_list;

arg_list:
        arg_list WORD { 
			std::string * word = $2;

			// allowing for escaped characters
			for (int i = 0; i < word->length(); i++) { 
				if ((*word)[i] == '\\' && (*word)[i] != '\n') {
					word->erase(i, 1);
				} 
			}

          Shell::TheShell->_simpleCommand->insertArgument( $2 ); }
		
		| /*empty string*/
	;

cmd_and_args:
  	WORD { 
		if (*$1 == "exit") {
			printf("Good bye!!\n\n");
			yylex_destroy();
			exit(0);
		}
          Shell::TheShell->_simpleCommand = new SimpleCommand(); 
          Shell::TheShell->_simpleCommand->insertArgument( $1 );
        } 
        arg_list
	;

pipe_list:
        cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( Shell::TheShell->_simpleCommand ); 
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	| pipe_list PIPE cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( Shell::TheShell->_simpleCommand ); 	
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	;

io_modifier:
	   GREATGREAT WORD 
	   {
		if (Shell::TheShell->_pipeCommand->_outFile) {
			printf("Ambiguous output redirect.\n");
		} else {
			Shell::TheShell->_pipeCommand->_outFile = $2;		
		}
	   }
	 | GREAT WORD 
	    {
		if (Shell::TheShell->_pipeCommand->_outFile) {
			printf("Ambiguous output redirect.\n");
		} else {
			Shell::TheShell->_pipeCommand->_outFile = $2;		
			Shell::TheShell->_pipeCommand->_overwrite = true;	
		}
	    }
	 | GREATGREATAMPERSAND WORD
	 	{
		Shell::TheShell->_pipeCommand->_errFile = $2;		
		Shell::TheShell->_pipeCommand->_outFile = $2;		
	 	}
	 | GREATAMPERSAND WORD 
	 	{
		Shell::TheShell->_pipeCommand->_errFile = $2;		
		Shell::TheShell->_pipeCommand->_outFile = $2;		
		Shell::TheShell->_pipeCommand->_overwrite = true;	
	 	}
	 | LESS WORD 
	 	{
		Shell::TheShell->_pipeCommand->_inFile = $2;		
	 	}
	 | TWOGREAT WORD
	 	{
		if (Shell::TheShell->_pipeCommand->_errFile) {
			printf("Ambiguous output redirect.\n");
		} else {
			Shell::TheShell->_pipeCommand->_errFile = $2;		
			Shell::TheShell->_pipeCommand->_overwrite = true;
		}
		}												
	;														

io_modifier_list:
	io_modifier_list io_modifier
	| /*empty*/
	;

background_optional: 
	AMPERSAND 
	{
		Shell::TheShell->_pipeCommand->_background = true;
	}
	| /*empty*/
	;

SEPARATOR:
	NEWLINE
	| SEMI
	;

command_line:
	 pipe_list io_modifier_list background_optional SEPARATOR 
         { 
			if (Shell::TheShell->_level > 0 && ifFlag  == false) {
				Shell::TheShell->listCommandStack[Shell::TheShell->_level - 1]->insertCommand(Shell::TheShell->_pipeCommand);
				Shell::TheShell->_pipeCommand = new PipeCommand();
			} else {
				Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_pipeCommand);
	    		Shell::TheShell->_pipeCommand = new PipeCommand();
			}
	     
         }
        | if_command SEPARATOR 
         {
	    Shell::TheShell->_listCommands->
		insertCommand(Shell::TheShell->_ifCommand);
         }
        | while_command SEPARATOR {
		//printf("while\n");
		Shell::TheShell->_ifCommand->whileLoop = true;
		if (Shell::TheShell->_level > 0) {
			Shell::TheShell->listCommandStack[Shell::TheShell->_level - 1]->insertCommand(Shell::TheShell->ifCommandStack[Shell::TheShell->_level]);
		} else {
			Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->ifCommandStack[Shell::TheShell->_level]); 
		}
		
		}
        | for_command SEPARATOR { 
		// printf("for\n"); 
		Shell::TheShell->_ifCommand->forLoop = true;
		if (Shell::TheShell->_level > 0) {
			Shell::TheShell->listCommandStack[Shell::TheShell->_level - 1]->insertCommand(Shell::TheShell->ifCommandStack[Shell::TheShell->_level]);
		} else {
			Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->ifCommandStack[Shell::TheShell->_level]); 
		}
		
		}
        | SEPARATOR /*accept empty cmd line*/
        | error SEPARATOR {yyerrok; 
		Shell::TheShell->clear(); 
		}
	;          /*error recovery*/

command_list :
     command_line 
	{ 
	   Shell::TheShell->execute();
	}
     | 
     command_list command_line 
	{
	    Shell::TheShell->execute();
	}
     ;  /* command loop*/

if_command:
    IF LBRACKET 
	{ 
	ifFlag = true;
	    Shell::TheShell->_level++; 
	    Shell::TheShell->_ifCommand = new IfCommand();
	} 
    arg_list RIGHTBRACKETSEMI THEN 
	{
	    Shell::TheShell->_ifCommand->insertCondition(Shell::TheShell->_simpleCommand);
	    Shell::TheShell->_simpleCommand = new SimpleCommand();
	}
    command_list FI 
	{ 
	    Shell::TheShell->_level--; 
	    Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
	    // Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_pipeCommand);
	    Shell::TheShell->_listCommands = new ListCommands();
	}
    ;

while_command:
    WHILE LBRACKET 
	{
	Shell::TheShell->_level++; 
	Shell::TheShell->_ifCommand = new IfCommand();
	Shell::TheShell->_ifCommand->whileLoop = true;
	Shell::TheShell->ifCommandStack.push_back(Shell::TheShell->_ifCommand);
	} 
	arg_list RIGHTBRACKETSEMI DO 
	{
	Shell::TheShell->ifCommandStack[Shell::TheShell->_level - 1]->insertCondition(Shell::TheShell->_simpleCommand);
	// Shell::TheShell->_ifCommand->insertCondition(Shell::TheShell->_simpleCommand);
	Shell::TheShell->_simpleCommand = new SimpleCommand();
	Shell::TheShell->_currentListCommand = new ListCommands();
	Shell::TheShell->listCommandStack.push_back(Shell::TheShell->_currentListCommand);
	
	}
	command_list DONE 
	{

	Shell::TheShell->ifCommandStack[Shell::TheShell->_level - 1]->insertListCommands(Shell::TheShell->listCommandStack[Shell::TheShell->_level - 1]);
	Shell::TheShell->_level--; 
	// Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
	// Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_pipeCommand);
	Shell::TheShell->_listCommands = new ListCommands();
	
	}
    ;

for_command:
    FOR WORD IN {
	Shell::TheShell->_level++; 
	Shell::TheShell->_ifCommand = new IfCommand();
	Shell::TheShell->_ifCommand->forLoop = true;
	Shell::TheShell->ifCommandStack.push_back(Shell::TheShell->_ifCommand);
		
	}
	arg_list SEMI DO {
	Shell::TheShell->ifCommandStack[Shell::TheShell->_level - 1]->insertCondition(Shell::TheShell->_simpleCommand);
	// Shell::TheShell->_ifCommand->insertCondition(Shell::TheShell->_simpleCommand);
	Shell::TheShell->_simpleCommand = new SimpleCommand();
	Shell::TheShell->_currentListCommand = new ListCommands();
	Shell::TheShell->listCommandStack.push_back(Shell::TheShell->_currentListCommand);

	Shell::TheShell->_ifCommand->primaryArg = $2;
	

	} command_list DONE {

	Shell::TheShell->ifCommandStack[Shell::TheShell->_level - 1]->insertListCommands(Shell::TheShell->listCommandStack[Shell::TheShell->_level - 1]);
	Shell::TheShell->_level--; 
	// Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
	// Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_pipeCommand);
	Shell::TheShell->_listCommands = new ListCommands();

	}
    ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
