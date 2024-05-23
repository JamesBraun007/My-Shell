/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Y_TAB_HH_INCLUDED
# define YY_YY_Y_TAB_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 15 "shell.y"

#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif

#line 57 "y.tab.hh"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    WORD = 258,                    /* WORD  */
    NOTOKEN = 259,                 /* NOTOKEN  */
    GREAT = 260,                   /* GREAT  */
    GREATGREAT = 261,              /* GREATGREAT  */
    GREATAMPERSAND = 262,          /* GREATAMPERSAND  */
    GREATGREATAMPERSAND = 263,     /* GREATGREATAMPERSAND  */
    TWOGREAT = 264,                /* TWOGREAT  */
    AMPERSAND = 265,               /* AMPERSAND  */
    PIPE = 266,                    /* PIPE  */
    LESS = 267,                    /* LESS  */
    NEWLINE = 268,                 /* NEWLINE  */
    IF = 269,                      /* IF  */
    FI = 270,                      /* FI  */
    THEN = 271,                    /* THEN  */
    LBRACKET = 272,                /* LBRACKET  */
    RBRACKET = 273,                /* RBRACKET  */
    SEMI = 274,                    /* SEMI  */
    RIGHTBRACKETSEMI = 275,        /* RIGHTBRACKETSEMI  */
    DO = 276,                      /* DO  */
    DONE = 277,                    /* DONE  */
    WHILE = 278,                   /* WHILE  */
    FOR = 279,                     /* FOR  */
    IN = 280                       /* IN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define WORD 258
#define NOTOKEN 259
#define GREAT 260
#define GREATGREAT 261
#define GREATAMPERSAND 262
#define GREATGREATAMPERSAND 263
#define TWOGREAT 264
#define AMPERSAND 265
#define PIPE 266
#define LESS 267
#define NEWLINE 268
#define IF 269
#define FI 270
#define THEN 271
#define LBRACKET 272
#define RBRACKET 273
#define SEMI 274
#define RIGHTBRACKETSEMI 275
#define DO 276
#define DONE 277
#define WHILE 278
#define FOR 279
#define IN 280

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 24 "shell.y"

  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;

#line 133 "y.tab.hh"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_HH_INCLUDED  */
