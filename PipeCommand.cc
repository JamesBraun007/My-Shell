/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <regex.h>
#include<algorithm>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>

#include "PipeCommand.hh"
#include "Shell.hh"

#define MAXFILENAME 9999

void expandWildcard(char * prefix, char *suffix,  std::vector<std::string> &dirs);


PipeCommand::PipeCommand() {
    // Initialize a new vector of Simple PipeCommands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _overwrite = false;
}

void PipeCommand::insertSimpleCommand( SimpleCommand * simplePipeCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simplePipeCommand);
}

void PipeCommand::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simplePipeCommand : _simpleCommands) {
        delete simplePipeCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    // if _outFile and _errFile point to same place, only need to delete one (this is still wrong somehow)
    if (_outFile == _errFile && _outFile) {
        delete _outFile;
        _outFile = NULL;
        _errFile = NULL;
        
    } else {

        if ( _outFile ) {
        delete _outFile;
        }
        _outFile = NULL;

        if ( _errFile ) {
            delete _errFile;
        }
        _errFile = NULL;

    }

    _background = false;

    _overwrite = false;
}

void PipeCommand::print() {
    printf("\n\n");
    //printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple PipeCommands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simplePipeCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simplePipeCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

extern "C" void handleZombie(int sig) {
    int wstatus;
    pid_t pid;

   while((pid = waitpid(-1, &wstatus, WNOHANG)) != -1) {
        if (pid != -1) {
            // printf("%d exited\n", pid);

            Shell::TheShell->_backgroundPID = pid;
        }
   }
    
    Shell::TheShell->prompt();
}

extern "C" void ctrlC(int sig) {
    //Shell::TheShell->clear();
    printf("\n");
    Shell::TheShell->prompt();
}

void PipeCommand::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::TheShell->prompt();
        return;
    }

    // print();

    // handling ctrl-c 
    struct sigaction sa;
    sa.sa_handler = ctrlC;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &sa, NULL)){
       perror("sigaction");
       exit(2);
    }

   // handling zombie processes here    
     
   	struct sigaction siga;
    siga.sa_handler = handleZombie;
   	sigemptyset(&siga.sa_mask);
   	siga.sa_flags = SA_RESTART;

   	sigaction(SIGCHLD, &siga, NULL);
   	

    // saves the current stdin and stdout into two new file descriptors
    int tempin = dup(0);
    int tempout = dup(1);
    int temperr = dup(2);

    // checking if an input file was provided
    int fdin;
    if (_inFile) {
        fdin = open(_inFile->c_str(), O_RDONLY, 0644);
        
        if (fdin == -1) {
            std::cerr << "Error opening input file." << std::endl;
        }

    } else {
        fdin = dup(tempin);
    }

    int fderr;

    if (_errFile) {
        if (_overwrite) {
            fderr = open(_errFile->c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        } else {
            fderr = open(_errFile->c_str(), O_CREAT | O_RDWR | O_APPEND, 0644);
        }
    } else {
        fderr = dup(temperr);
    }

    dup2(fderr, 2);
    close(fderr);    

    // iterating through commands
    int fdout;
    int ret;
    
    for (unsigned long i = 0; i < _simpleCommands.size(); i++) {
        bool isFork = true;


        SimpleCommand * s = _simpleCommands[i];
        std::vector<std::string *> simplerArgs;
        
        for (size_t y = 0; y < s->_arguments.size(); y++) {
            char * tempString = strdup(s->_arguments[y]->c_str());
            std::string * curArg = new std::string(tempString);
            simplerArgs.push_back(curArg);
            free(tempString);
        }
        
        const char ** args = (const char **) malloc((simplerArgs.size() + 1) * sizeof(char*));     // mallocing space for argument string plus null char

    
        dup2(fdin, 0);  // redirecting input
        close(fdin);    

        // checking if last command
        if (i == _simpleCommands.size() - 1) {

        if (_outFile) {
            if (_overwrite) {
                fdout = open(_outFile->c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
            } else {
                fdout = open(_outFile->c_str(), O_CREAT | O_RDWR | O_APPEND, 0644);
            }
        } else {
            fdout = dup(tempout);
        }

        
        if (fdout == -1) {
            std::cerr << "Error opening/creating output file." << std::endl;
        }

        } else {

            // setting up pipes between commands
            int fdpipe[2];
            pipe(fdpipe);
            fdout = fdpipe[1];
            fdin = fdpipe[0];
        }

        dup2(fdout, 1);
        close(fdout);

        // iterating through arguments of each command
        for (unsigned long j = 0; j < simplerArgs.size(); j++) {

            if (*(simplerArgs[j]) != "${_}" && *(simplerArgs[j]) != "${SHELL}") {
                // looking if env var expansion is needed
                for (size_t n = 0; environ[n] != NULL; n++) {

                    size_t equalIndex = std::string(environ[n]).find('=');

                    if (equalIndex != std::string::npos) {
                        std::string varName = std::string(environ[n], 0, equalIndex);
                        std::string varValue = std::string(environ[n]).substr(equalIndex + 1);

                        std::string nonExpanded = "${" + varName + "}";
                        size_t findPattern = simplerArgs[j]->find(nonExpanded);

                        // looking for multiple occurences of ${var} in one argument and expanding 
                        while (findPattern != std::string::npos) {
                            simplerArgs[j]->replace(findPattern, nonExpanded.length(), varValue);
                            findPattern = simplerArgs[j]->find(nonExpanded, findPattern + varValue.length());
                        }

                        // handling tilde expansions
                        if (*simplerArgs[j] == "~" && varName == "HOME") {
                            *simplerArgs[j] = varValue;
                        }

                        if (simplerArgs[j]->find("~/") != std::string::npos && varName == "HOME") {
                            size_t tildeIndex = simplerArgs[j]->find("~/");
                            std::string temp = simplerArgs[j]->substr(tildeIndex + 1, simplerArgs[j]->length() - tildeIndex);
                            *simplerArgs[j] = varValue + temp;

                        } else if (simplerArgs[j]->find("~") != std::string::npos && simplerArgs[j]->length() > 1) {
                            size_t tildeIndex = simplerArgs[j]->find("~");
                            simplerArgs[j]->replace(tildeIndex, 1, "/homes/");
                        }
                    }
                }
            }

            if (simplerArgs[j]->find("*") != std::string::npos || (simplerArgs[j]->find("?") != std::string::npos && simplerArgs[j]->find("${") == std::string::npos)) {

                char * OGarg = strdup(simplerArgs[j]->c_str());
                char wildcard = (*simplerArgs[j])[0];

                // converting * wildcard to regex
                size_t starIndex = simplerArgs[j]->find("*");
                while (starIndex != std::string::npos) {
                    simplerArgs[j]->replace(starIndex, 1, ".*");
                    starIndex = simplerArgs[j]->find("*", starIndex + 2);
                }

                // converting ? wildcard to regex
               
                size_t questIndex = simplerArgs[j]->find("?");
                while (questIndex != std::string::npos) {
                    simplerArgs[j]->replace(questIndex, 1, ".");
                    questIndex = simplerArgs[j]->find("?", questIndex + 1);
                }
                
                // printf("%s\n", simplerArgs[j]->c_str());

                std::vector<std::string> dirs; 
                // if a path is detected, need to expand each component
                if (simplerArgs[j]->find("/") != std::string::npos) {
                    
                    char * tempSuffix = strdup(simplerArgs[j]->c_str());
                    char * tempPrefix = strdup("/");

                    expandWildcard(tempPrefix, tempSuffix, dirs);
                    
                    free(tempSuffix);
                    free(tempPrefix);

                // otherwise just expand within current directory
                } else {

                    const char * regularExp = simplerArgs[j]->c_str();
                    char regExpComplete[1024];
                    sprintf(regExpComplete, "^%s$", regularExp );

                    regex_t re;	
                    int result = regcomp( &re, regExpComplete,  REG_EXTENDED|REG_NOSUB);
                    if (result != 0) {
                        printf("Bad regular expression\n");
                        exit(1);
                    }

                    DIR * dir = opendir(".");
                    if (dir == NULL) {
                        perror("opendir");
                        exit(1);
                    }

                    struct dirent *entry;

                    while ((entry = readdir(dir)) != NULL) {
                        
                        regmatch_t match;
                        result = regexec(&re, entry->d_name, 1, &match, 0);

                        // checking if user wants to list hidden files
                        if (result == 0) {
                            if (entry->d_name[0] == '.' && wildcard == '.') {
                                dirs.push_back(entry->d_name);

                            } else if (entry->d_name[0] != '.' && wildcard != '.') {
                                dirs.push_back(entry->d_name);
                            }
                            // dirs.push_back(entry->d_name);
                        }    
                    }
                    closedir(dir);
		            regfree(&re);
                }

                std::sort(dirs.begin(), dirs.end());
                for (size_t d = 0; d < dirs.size(); d++) {
                    if (dirs[d].find("//") != std::string::npos) {
                        dirs[d].replace(0, 2, "/");
                    }
                }

                // checking if wildcards were expanded or not
                if (dirs.size() != 0 && *simplerArgs[j] != dirs[0]) {

                    simplerArgs[j] = new std::string(dirs[0]);
                    
                    for (size_t u = 1; u < dirs.size(); u++) {
                    simplerArgs.insert(simplerArgs.begin() + j + u, new std::string(dirs[u]));
                    }

                    // need to insert contents of dirs back into args 
                    args = (const char **) realloc(args, (simplerArgs.size() + 1) * sizeof(char*));
                    args[simplerArgs.size()] = NULL;
                } else {
                    *simplerArgs[j] = std::string(OGarg);
                }

                free(OGarg);  // hope this is ok to free 
            }

            if (simplerArgs[j]->length() >= 4) {
                if (simplerArgs[j]->at(0) == '$' && simplerArgs[j]->at(3) == '}' && isdigit(simplerArgs[j]->at(2))) {
                    char digit = simplerArgs[j]->at(2);
                    int argIndex = digit - '0';
                    *simplerArgs[j] = Shell::TheShell->argvCopy[argIndex + 1];
                }
            }

            if (*simplerArgs[j] == "${#}") {
                *simplerArgs[j] = std::to_string(Shell::TheShell->numArgs);
            }

            // expand to PID of shell prcocess
            if (*simplerArgs[j] == "${$}") {
                pid_t shellPID = getpid();
                *simplerArgs[j] = std::to_string(shellPID);
            }

            // expand to return code of the last executed simple command (excluding background)
            if (*simplerArgs[j] == "${?}") {
                *simplerArgs[j] = std::to_string(Shell::TheShell->_returnCode);
            }

            // expand to PID of the last process run in the background
            if (*simplerArgs[j] == "${!}") {
                *simplerArgs[j] = std::to_string(Shell::TheShell->_backgroundPID);
            }

            // expand to The last argument in the fully expanded previous command
            if (*(simplerArgs[j]) == "${_}") {
                *simplerArgs[j] = Shell::TheShell->lastArg;
            }

            // expand to The path of your shell executable
            if (*simplerArgs[j] == "${SHELL}") {
                *simplerArgs[j] = realpath(Shell::TheShell->path, NULL);
            }   

            // retrieving strings enclosed in $(...) and '...' to use in subshell
            size_t startIndex = simplerArgs[j]->find("$(");
            size_t endIndex = simplerArgs[j]->find(")", startIndex + 1);
            std::string subShellString;
            bool isSubShell = false;

            if (startIndex != std::string::npos && endIndex != std::string::npos) {
                subShellString = simplerArgs[j]->substr(startIndex + 2, endIndex - startIndex - 2);
                isSubShell = true;

            } else {
                startIndex = simplerArgs[j]->find("`");
                endIndex = simplerArgs[j]->find("`", startIndex + 1);

                if (startIndex != std::string::npos && endIndex != std::string::npos) {
                    subShellString = simplerArgs[j]->substr(startIndex + 1, endIndex - startIndex - 1);
                    isSubShell = true;
                }
            }

            // if a subshell command was found, then do subshell stuff
            if (isSubShell) {
                
                int pin[2];
                int pout[2];
                pipe(pin);
                pipe(pout);

                write(pin[1], subShellString.c_str(), subShellString.size());
                write(pin[1], "\n", 1);
                write(pin[1], "exit\n", 5);

                close(pin[1]);
                
                int pid = fork();
                // child process
                if (pid == 0) {
                    dup2(pin[0], 0);
                    dup2(pout[1], 1);
                    
                    const char *self[] = {"/proc/self/exe", NULL};       
                    execvp(self[0], (char * const *)self);
                    
                } 

                // after subshell runs, parent reads output from pout[0] and writes into a buffer
                char * buf = (char *)malloc(9999 * sizeof(char));
                read(pout[0], buf, 9999);    

                close(pout[0]);
                close(pin[0]);
                close(pout[1]);

                std::string buffer(buf);
                std::vector<std::string> newArgs;
                int startSubstring = 0;

                // separate buffer string into different arguments if needed
                for (size_t n = 0; n < buffer.size(); n++) {
                    if (buffer[n] == ' ' || buffer[n] == '\n' || buffer[n] == '\t') {
                        newArgs.push_back(buffer.substr(startSubstring, n - startSubstring));
                        startSubstring = n + 1;
                    }
                }

                // removing empty strings and newlines from array
                for (size_t t = 0; t < newArgs.size(); t++) {
                    size_t nIndex = newArgs[t].find('\n');
                    if (nIndex != std::string::npos) {
                        newArgs[t] = newArgs[t].substr(0, nIndex - 1);
                    }
                    if (newArgs[t] == "") {
                        newArgs.erase(newArgs.begin() + t);
                        t--;
                    }
                }

                simplerArgs[j] = new std::string(newArgs[0]);

                for (size_t t = 1; t < newArgs.size(); t++) {
                    simplerArgs.insert(simplerArgs.begin() + j + t, new std::string(newArgs[t]));
                }

                // assigning all strings as arguments to c-style args array
                args = (const char **) realloc(args, (simplerArgs.size() + 1) * sizeof(char*));
                args[simplerArgs.size()] = NULL;
            } 

            //printf("%s\n", simplerArgs[j]->c_str());
            args[j] = simplerArgs[j]->c_str();

            if (j == simplerArgs.size() - 1) {
                // getting last arg of most recent command
                Shell::TheShell->lastArg = strdup(args[simplerArgs.size()-1]);

                // setting end of args to be NULL for execvp functionality                  
                args[simplerArgs.size()] = NULL;
            }
        }


        // checking if user is running setenv
        if (simplerArgs[0]->compare("setenv") == 0) {
  		isFork = false;
            setenv(simplerArgs[1]->c_str(), simplerArgs[2]->c_str(), 1);        
        }
        
        // checking if user is running unsetenv
        if (simplerArgs[0]->compare("unsetenv") == 0) {
            isFork = false;
            unsetenv(simplerArgs[1]->c_str());                                    
        }


        // checking if user is running cd
        if (simplerArgs[0]->compare("cd") == 0) {
            isFork = false;
            if (simplerArgs[1] == NULL) {
                chdir("/homes/braun43");
            } else {
                if (chdir(simplerArgs[1]->c_str()) != 0) {
                    fprintf(stderr, "cd: can't cd to %s\n", simplerArgs[1]->c_str());
                }
            }
        }  

        // if built in function was not called then fork 
        if (isFork) {
            ret = fork();
            if (ret == 0) {
		// checking if user is running printenv functions
       	   	 if (simplerArgs[0]->compare("printenv") == 0) {
           	     for (size_t n = 0; environ[n] != NULL; n++) {
                         printf("%s\n", environ[n]);
                     }
		     exit(0);
                  }

                execvp(args[0], (char * const *)args);
                perror("execvp");
                exit(1);
            } 
        }

        free(args);
        for (auto& argPtr : simplerArgs) {
            delete argPtr;
        }
    }

    dup2(fderr, 2);
    close(fderr);

    // restoring stdin, stdout, stderr
    dup2(tempin, 0);
    dup2(tempout, 1);
    dup2(temperr, 2);

    close(tempin);
    close(tempout);
    close(temperr);

    if (!_background) {
        int status;
        waitpid(ret, &status, 0); 

        // getting return code of most recent command
        if (WIFEXITED(status)) {
            Shell::TheShell->_returnCode = WEXITSTATUS(status); 
        } 
    } 
      
    // Clear to prepare for next command
    // clear();

    // Print new prompt
    // Shell::TheShell->prompt();
}

// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
char ** 
PipeCommand::expandEnvVarsAndWildcards(SimpleCommand * simpleCommandNumber)
{
    simpleCommandNumber->print();
    return NULL;
}

void expandWildcard(char * prefix, char *suffix,  std::vector<std::string> &dirs) {
    if (suffix[0] == 0) {
        // suffix is empty, argument has been fully expanded and will be inserted into argument
        dirs.push_back(strdup(prefix));
        return;
    }

    // Obtain the next component in the suffix
    // Also advance suffix.
    char * s = strchr(suffix, '/');
    char component[MAXFILENAME];

    if (s != NULL){ // Copy up to the first “/”
        strncpy(component,suffix, s-suffix);
        suffix = s + 1;
    } else {
        strcpy(component, suffix);
        suffix = suffix + strlen(suffix);       
    }

    char newPrefix[MAXFILENAME];
    if (strchr(component, '?') == NULL && strchr(component, '*') == NULL) {
        // component does not have wildcards
        if (strcmp(prefix, "/") == 0) {
            sprintf(newPrefix,"%s%s", prefix, component);
        } else {
            sprintf(newPrefix,"%s/%s", prefix, component);
        }
        expandWildcard(newPrefix, suffix, dirs);
        return;
    }

    const char * regularExp = component;
    char regExpComplete[1024];
    sprintf(regExpComplete, "^%s$", regularExp );

    regex_t re;	
    int result = regcomp( &re, regExpComplete,  REG_EXTENDED|REG_NOSUB);
    if (result != 0) {
        printf("Bad regular expression\n");
        exit(1);
    }

    char * dir = prefix;
    DIR * d = opendir(dir);

    // if directory doesn't open but prefix isnt empty, then done expanding
    if (d == NULL && strlen(prefix) > 0) {
        // regmatch_t match;
        // result = regexec(&re, prefix, 1, &match, 0);
        // if (result == 0) {
        //     if (prefix[0] != '.') {
        //         expandWildcard(prefix, suffix, dirs);
        //     }
        // }

    } else if (d == NULL) {
        perror("opendir");
        exit(1);
    } else {

        struct dirent *entry;

        while ((entry = readdir(d)) != NULL) {
            regmatch_t match;
            result = regexec(&re, entry->d_name, 1, &match, 0);

            if (result == 0) {
                // might need to allow hidden files for robustness
                if (entry->d_name[0] != '.') { 
                    sprintf(newPrefix,"%s/%s", prefix, entry->d_name);
                    expandWildcard(newPrefix, suffix, dirs);
                }
            }    
        }
    }
    closedir(d);
    regfree(&re);
    
}


