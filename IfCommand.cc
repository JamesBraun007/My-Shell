
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>
#include <algorithm>

#include "Command.hh"
#include "SimpleCommand.hh"
#include "IfCommand.hh"

void expandForWildcard(std::vector<std::string*> &simplerArgs);

IfCommand::IfCommand() {
    _condition = NULL;
    forLoop = false;
    whileLoop = false;
    _listCommands =  NULL;
    primaryArg = NULL;

}


// Run condition with command "test" and return the exit value.
int
IfCommand::runTest(SimpleCommand * condition) {
    // return 1;
    std::vector<std::string *> simplerArgs;
        
    for (size_t y = 0; y < condition->_arguments.size(); y++) {
        char * tempString = strdup(condition->_arguments[y]->c_str());
        std::string * curArg = new std::string(tempString);
        simplerArgs.push_back(curArg);
        free(tempString);
    }

    const char ** conditionArguments = (const char **)malloc((simplerArgs.size()+2) * sizeof(char *));
    conditionArguments[0] = "/usr/bin/test";
    conditionArguments[simplerArgs.size()+1] = NULL;

    for (unsigned long i = 1; i < simplerArgs.size() + 1; i++) {
        for (size_t n = 0; environ[n] != NULL; n++) {
            size_t equalIndex = std::string(environ[n]).find('=');
            if (equalIndex != std::string::npos) {
                std::string varName = std::string(environ[n], 0, equalIndex);
                std::string varValue = std::string(environ[n]).substr(equalIndex + 1);

                std::string nonExpanded = "${" + varName + "}";
                size_t findPattern = simplerArgs[i-1]->find(nonExpanded);

                while (findPattern != std::string::npos) {
                    simplerArgs[i-1]->replace(findPattern, nonExpanded.length(), varValue);
                    findPattern = simplerArgs[i-1]->find(nonExpanded, findPattern + varValue.length());
                }
            }
        }
        conditionArguments[i] = simplerArgs[i-1]->c_str();
    }

    // condition->print();

    int ret = fork();
    if (ret < 0) {
        perror("fork");
        exit(1);

    } else if (ret == 0) {
        execvp(conditionArguments[0], (char * const *) conditionArguments);
        perror("child");
        exit(1);
    } 

    for (unsigned long i = 1; i < simplerArgs.size() + 1; i++) {
       //  fprintf(stderr, " %s\n", conditionArguments[i]);
    }

    // Clean up allocated memory
    for (auto& argPtr : simplerArgs) {
        delete argPtr;
    }
    free(conditionArguments);
	
    // Block SIGCHLD signal
	sigset_t blockset, oldset;
	sigemptyset(&blockset);
	sigaddset(&blockset, SIGCHLD);
	sigprocmask(SIG_BLOCK, &blockset, &oldset);

    // honestly this has been stupid, and somehow this works for now. so im keeping it
    int status;
    if ( waitpid(ret, &status, 0) == -1 ) { 
        // perror("waitpid failed");
        // return EXIT_FAILURE;
        return 0;
    }
    	// unblocking SIGCHILD signal
	sigprocmask(SIG_SETMASK, &oldset, NULL);

    if ( WIFEXITED(status) ) {
        const int es = WEXITSTATUS(status);
        return es;
    }
    return status;
}

void 
IfCommand::insertCondition( SimpleCommand * condition ) {
    _condition = condition;
}

void 
IfCommand::insertListCommands( ListCommands * listCommands) {
    _listCommands = listCommands;
}

void 
IfCommand::iterateFor(SimpleCommand * forArguments) {

    std::vector<std::string *> simplerArgs;
        
    for (size_t y = 0; y < forArguments->_arguments.size(); y++) {
        char * tempString = strdup(forArguments->_arguments[y]->c_str());
        std::string * curArg = new std::string(tempString);
        simplerArgs.push_back(curArg);
        free(tempString);
    }

    // wildcard expansion
    expandForWildcard(simplerArgs);
    

    for (size_t i = 0; i < simplerArgs.size(); i++) {
        setenv(primaryArg->c_str(), simplerArgs[i]->c_str(), 1);
        _listCommands->execute();
        // printf("%s\n", forArguments->_arguments[i]->c_str());
    }


    for (auto& argPtr : simplerArgs) {
        delete argPtr;
    }
}

void 
IfCommand::clear() {
}

void 
IfCommand::print() {
    printf("IF [ \n"); 
    this->_condition->print();
    printf("   ]; then\n");
    this->_listCommands->print();
}
  
void 
IfCommand::execute() {
    // Run command if test is 0
    
    if (whileLoop) {
        while (runTest(this->_condition) == 0) {
            _listCommands->execute();
        }	
    } else if (forLoop) {
        iterateFor(this->_condition);

    } else {
        if (runTest(this->_condition) == 0) {
        _listCommands->execute();
        }
    }
}

void expandForWildcard(std::vector<std::string*> &simplerArgs) {
    for (size_t j = 0; j < simplerArgs.size(); j++) {
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
            std::sort(dirs.begin(), dirs.end());

            // checking if wildcards were expanded or not
            if (dirs.size() != 0 && *simplerArgs[j] != dirs[0]) {

                simplerArgs[j] = new std::string(dirs[0]);
                
                for (size_t u = 1; u < dirs.size(); u++) {
                simplerArgs.insert(simplerArgs.begin() + j + u, new std::string(dirs[u]));
                }

            } else {
                *simplerArgs[j] = std::string(OGarg);
            }

            free(OGarg);  
            
        }
    }
}



