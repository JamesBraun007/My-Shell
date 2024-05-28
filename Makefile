#
# CS252 - Shell Project
#
#Use GNU compiler
cc= gcc
CC= g++
ccFLAGS= -g -std=c11
CCFLAGS= -g -std=c++17
WARNFLAGS= -Wall -Wextra -pedantic

LEX=lex -l
YACC=yacc -y -d -t --debug

EDIT_MODE_ON=

ifdef EDIT_MODE_ON
	EDIT_MODE_OBJECTS=tty-raw-mode.o read-line.o
endif

all: git-commit shell

lex.yy.o: shell.l 
	$(LEX) -o lex.yy.cc shell.l
	$(CC) $(CCFLAGS) -c lex.yy.cc

y.tab.o: shell.y
	$(YACC) -o y.tab.cc shell.y
	$(CC) $(CCFLAGS) -c y.tab.cc

PipeCommand.o: PipeCommand.cc PipeCommand.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c PipeCommand.cc

SimpleCommand.o: SimpleCommand.cc SimpleCommand.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c SimpleCommand.cc

ListCommands.o: ListCommands.cc ListCommands.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c ListCommands.cc

Command.o: Command.cc Command.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c Command.cc

Shell.o: Shell.cc Shell.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c Shell.cc

IfCommand.o: IfCommand.cc IfCommand.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c IfCommand.cc

shell: y.tab.o lex.yy.o Shell.o PipeCommand.o SimpleCommand.o ListCommands.o Command.o IfCommand.o Shell.o $(EDIT_MODE_OBJECTS)
	$(CC) $(CCFLAGS) $(WARNFLAGS) -o shell lex.yy.o y.tab.o Shell.o PipeCommand.o SimpleCommand.o ListCommands.o Command.o IfCommand.o $(EDIT_MODE_OBJECTS)

tty-raw-mode.o: tty-raw-mode.c
	$(cc) $(ccFLAGS) $(WARNFLAGS) -c tty-raw-mode.c

read-line.o: read-line.c
	$(cc) $(ccFLAGS) $(WARNFLAGS) -c read-line.c

.PHONY: git-commit
git-commit:
	git checkout master >> .local.git.out || echo
	touch testall.out
	git add *.cc *.hh *.l *.y Makefile testall.out >> .local.git.out  || echo
	git commit -a -m  "Commit" >> .local.git.out || echo
	git push origin master

.PHONY: clean
clean:
	rm -f lex.yy.cc y.tab.cc y.tab.hh shell *.o

