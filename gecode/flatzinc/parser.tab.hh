/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FZ_INT_LIT = 258,
     FZ_BOOL_LIT = 259,
     FZ_FLOAT_LIT = 260,
     FZ_ID = 261,
     FZ_STRING_LIT = 262,
     FZ_VAR = 263,
     FZ_PAR = 264,
     FZ_ANNOTATION = 265,
     FZ_ANY = 266,
     FZ_ARRAY = 267,
     FZ_BOOL = 268,
     FZ_CASE = 269,
     FZ_COLONCOLON = 270,
     FZ_CONSTRAINT = 271,
     FZ_DEFAULT = 272,
     FZ_DOTDOT = 273,
     FZ_ELSE = 274,
     FZ_ELSEIF = 275,
     FZ_ENDIF = 276,
     FZ_ENUM = 277,
     FZ_FLOAT = 278,
     FZ_FUNCTION = 279,
     FZ_IF = 280,
     FZ_INCLUDE = 281,
     FZ_INT = 282,
     FZ_LET = 283,
     FZ_MAXIMIZE = 284,
     FZ_MINIMIZE = 285,
     FZ_OF = 286,
     FZ_SATISFY = 287,
     FZ_OUTPUT = 288,
     FZ_PREDICATE = 289,
     FZ_RECORD = 290,
     FZ_SET = 291,
     FZ_SHOW = 292,
     FZ_SHOWCOND = 293,
     FZ_SOLVE = 294,
     FZ_STRING = 295,
     FZ_TEST = 296,
     FZ_THEN = 297,
     FZ_TUPLE = 298,
     FZ_TYPE = 299,
     FZ_VARIANT_RECORD = 300,
     FZ_WHERE = 301
   };
#endif
/* Tokens.  */
#define FZ_INT_LIT 258
#define FZ_BOOL_LIT 259
#define FZ_FLOAT_LIT 260
#define FZ_ID 261
#define FZ_STRING_LIT 262
#define FZ_VAR 263
#define FZ_PAR 264
#define FZ_ANNOTATION 265
#define FZ_ANY 266
#define FZ_ARRAY 267
#define FZ_BOOL 268
#define FZ_CASE 269
#define FZ_COLONCOLON 270
#define FZ_CONSTRAINT 271
#define FZ_DEFAULT 272
#define FZ_DOTDOT 273
#define FZ_ELSE 274
#define FZ_ELSEIF 275
#define FZ_ENDIF 276
#define FZ_ENUM 277
#define FZ_FLOAT 278
#define FZ_FUNCTION 279
#define FZ_IF 280
#define FZ_INCLUDE 281
#define FZ_INT 282
#define FZ_LET 283
#define FZ_MAXIMIZE 284
#define FZ_MINIMIZE 285
#define FZ_OF 286
#define FZ_SATISFY 287
#define FZ_OUTPUT 288
#define FZ_PREDICATE 289
#define FZ_RECORD 290
#define FZ_SET 291
#define FZ_SHOW 292
#define FZ_SHOWCOND 293
#define FZ_SOLVE 294
#define FZ_STRING 295
#define FZ_TEST 296
#define FZ_THEN 297
#define FZ_TUPLE 298
#define FZ_TYPE 299
#define FZ_VARIANT_RECORD 300
#define FZ_WHERE 301




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 323 "gecode/flatzinc/parser.yxx"
{ int iValue; char* sValue; bool bValue; double dValue;
         std::vector<int>* setValue;
         Gecode::FlatZinc::AST::SetLit* setLit;
         std::vector<double>* floatSetValue;
         std::vector<Gecode::FlatZinc::AST::SetLit>* setValueList;
         Gecode::FlatZinc::Option<Gecode::FlatZinc::AST::SetLit* > oSet;
         Gecode::FlatZinc::VarSpec* varSpec;
         Gecode::FlatZinc::Option<Gecode::FlatZinc::AST::Node*> oArg;
         std::vector<Gecode::FlatZinc::VarSpec*>* varSpecVec;
         Gecode::FlatZinc::Option<std::vector<Gecode::FlatZinc::VarSpec*>* > oVarSpecVec;
         Gecode::FlatZinc::AST::Node* arg;
         Gecode::FlatZinc::AST::Array* argVec;
       }
/* Line 1489 of yacc.c.  */
#line 155 "gecode/flatzinc/parser.tab.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



