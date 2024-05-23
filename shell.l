
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"


static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

"#".*\n {
  /* Discard comments starting with # */
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

">>" {
  return GREATGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"&" {
  return AMPERSAND;
}

"[" {
  return LBRACKET;
}

"]" {
  return RBRACKET;
}

"if" {
  return IF;
}

"then" {
  return THEN;
}

"fi" {
  return FI;
}

"];" {
  return RIGHTBRACKETSEMI;
}

"while" {
  return WHILE;
}

"do" {
  return DO;
}

"done" {
  return DONE;
}

"for" {
  return FOR;
}

"in" {
  return IN;
}

";" {
  return SEMI;
}

"2>" {
  return TWOGREAT;
}

"source"[[:space:]]*[^ \t\n;][^ \t\n;]* {
  std::string filename(yytext + 7); 

  FILE * fd = fopen(filename.c_str(), "r+");

  YY_BUFFER_STATE new_buffer = yy_create_buffer(fd, YY_BUF_SIZE);
  yypush_buffer_state(new_buffer);
  // yy_switch_to_buffer(new_buffer);

  yyparse();

  
  // yy_delete_buffer(new_buffer); 
  yypop_buffer_state();
  fclose(fd);

  // yy_switch_to_buffer(yy_create_buffer(stdin, YY_BUF_SIZE));
  
}


\"[^\"\n]*\" {
  /* searching for a string inside of quotes */
  yylval.cpp_string = new std::string(yytext + 1, yyleng - 2); 
  return WORD;
}

\`[^\n]*\` {
  /* searching for a string inside of backticks */
  yylval.cpp_string = new std::string(yytext); 
  return WORD;
}

\$\([^$\n()]+\) {
  /* searching for a string inside of $(...) */
  yylval.cpp_string = new std::string(yytext); 
  return WORD;
}

. { 
  /* handing singular characters */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}


([^ \;\t\n\>\<\|\&\\]|\\.)*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}
