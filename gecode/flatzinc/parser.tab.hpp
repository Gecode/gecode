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

#ifndef YY_YY_GECODE_FLATZINC_PARSER_TAB_HPP_INCLUDED
# define YY_YY_GECODE_FLATZINC_PARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    FZ_INT_LIT = 258,              /* FZ_INT_LIT  */
    FZ_BOOL_LIT = 259,             /* FZ_BOOL_LIT  */
    FZ_FLOAT_LIT = 260,            /* FZ_FLOAT_LIT  */
    FZ_ID = 261,                   /* FZ_ID  */
    FZ_U_ID = 262,                 /* FZ_U_ID  */
    FZ_STRING_LIT = 263,           /* FZ_STRING_LIT  */
    FZ_VAR = 264,                  /* FZ_VAR  */
    FZ_PAR = 265,                  /* FZ_PAR  */
    FZ_ANNOTATION = 266,           /* FZ_ANNOTATION  */
    FZ_ANY = 267,                  /* FZ_ANY  */
    FZ_ARRAY = 268,                /* FZ_ARRAY  */
    FZ_BOOL = 269,                 /* FZ_BOOL  */
    FZ_CASE = 270,                 /* FZ_CASE  */
    FZ_COLONCOLON = 271,           /* FZ_COLONCOLON  */
    FZ_CONSTRAINT = 272,           /* FZ_CONSTRAINT  */
    FZ_DEFAULT = 273,              /* FZ_DEFAULT  */
    FZ_DOTDOT = 274,               /* FZ_DOTDOT  */
    FZ_ELSE = 275,                 /* FZ_ELSE  */
    FZ_ELSEIF = 276,               /* FZ_ELSEIF  */
    FZ_ENDIF = 277,                /* FZ_ENDIF  */
    FZ_ENUM = 278,                 /* FZ_ENUM  */
    FZ_FLOAT = 279,                /* FZ_FLOAT  */
    FZ_FUNCTION = 280,             /* FZ_FUNCTION  */
    FZ_IF = 281,                   /* FZ_IF  */
    FZ_INCLUDE = 282,              /* FZ_INCLUDE  */
    FZ_INT = 283,                  /* FZ_INT  */
    FZ_LET = 284,                  /* FZ_LET  */
    FZ_MAXIMIZE = 285,             /* FZ_MAXIMIZE  */
    FZ_MINIMIZE = 286,             /* FZ_MINIMIZE  */
    FZ_OF = 287,                   /* FZ_OF  */
    FZ_SATISFY = 288,              /* FZ_SATISFY  */
    FZ_OUTPUT = 289,               /* FZ_OUTPUT  */
    FZ_PREDICATE = 290,            /* FZ_PREDICATE  */
    FZ_RECORD = 291,               /* FZ_RECORD  */
    FZ_SET = 292,                  /* FZ_SET  */
    FZ_SHOW = 293,                 /* FZ_SHOW  */
    FZ_SHOWCOND = 294,             /* FZ_SHOWCOND  */
    FZ_SOLVE = 295,                /* FZ_SOLVE  */
    FZ_STRING = 296,               /* FZ_STRING  */
    FZ_TEST = 297,                 /* FZ_TEST  */
    FZ_THEN = 298,                 /* FZ_THEN  */
    FZ_TUPLE = 299,                /* FZ_TUPLE  */
    FZ_TYPE = 300,                 /* FZ_TYPE  */
    FZ_VARIANT_RECORD = 301,       /* FZ_VARIANT_RECORD  */
    FZ_WHERE = 302                 /* FZ_WHERE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 610 "./gecode/flatzinc/parser.yxx"
 int iValue; char* sValue; bool bValue; double dValue;
         std::vector<int>* setValue;
         Gecode::FlatZinc::AST::SetLit* setLit;
         std::vector<double>* floatSetValue;
         std::vector<Gecode::FlatZinc::AST::SetLit>* setValueList;
         Gecode::FlatZinc::Option<Gecode::FlatZinc::AST::SetLit* > oSet;
         Gecode::FlatZinc::Option<std::pair<double,double>* > oPFloat;
         Gecode::FlatZinc::VarSpec* varSpec;
         Gecode::FlatZinc::Option<Gecode::FlatZinc::AST::Node*> oArg;
         std::vector<Gecode::FlatZinc::VarSpec*>* varSpecVec;
         Gecode::FlatZinc::Option<std::vector<Gecode::FlatZinc::VarSpec*>* > oVarSpecVec;
         Gecode::FlatZinc::AST::Node* arg;
         Gecode::FlatZinc::AST::Array* argVec;
       

#line 127 "gecode/flatzinc/parser.tab.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void *parm);


#endif /* !YY_YY_GECODE_FLATZINC_PARSER_TAB_HPP_INCLUDED  */
