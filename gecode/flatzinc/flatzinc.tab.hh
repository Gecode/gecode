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
     INT_LIT = 258,
     BOOL_LIT = 259,
     FLOAT_LIT = 260,
     ID = 261,
     STRING_LIT = 262,
     VAR = 263,
     PAR = 264,
     ANNOTATION = 265,
     ANY = 266,
     ARRAY = 267,
     BOOL = 268,
     CASE = 269,
     COLONCOLON = 270,
     CONSTRAINT = 271,
     DEFAULT = 272,
     DOTDOT = 273,
     ELSE = 274,
     ELSEIF = 275,
     ENDIF = 276,
     ENUM = 277,
     FLOAT = 278,
     FUNCTION = 279,
     IF = 280,
     INCLUDE = 281,
     INT = 282,
     LET = 283,
     MAXIMIZE = 284,
     MINIMIZE = 285,
     OF = 286,
     SATISFY = 287,
     OUTPUT = 288,
     PREDICATE = 289,
     RECORD = 290,
     SET = 291,
     SHOW = 292,
     SHOWCOND = 293,
     SOLVE = 294,
     STRING = 295,
     TEST = 296,
     THEN = 297,
     TUPLE = 298,
     TYPE = 299,
     VARIANT_RECORD = 300,
     WHERE = 301
   };
#endif
/* Tokens.  */
#define INT_LIT 258
#define BOOL_LIT 259
#define FLOAT_LIT 260
#define ID 261
#define STRING_LIT 262
#define VAR 263
#define PAR 264
#define ANNOTATION 265
#define ANY 266
#define ARRAY 267
#define BOOL 268
#define CASE 269
#define COLONCOLON 270
#define CONSTRAINT 271
#define DEFAULT 272
#define DOTDOT 273
#define ELSE 274
#define ELSEIF 275
#define ENDIF 276
#define ENUM 277
#define FLOAT 278
#define FUNCTION 279
#define IF 280
#define INCLUDE 281
#define INT 282
#define LET 283
#define MAXIMIZE 284
#define MINIMIZE 285
#define OF 286
#define SATISFY 287
#define OUTPUT 288
#define PREDICATE 289
#define RECORD 290
#define SET 291
#define SHOW 292
#define SHOWCOND 293
#define SOLVE 294
#define STRING 295
#define TEST 296
#define THEN 297
#define TUPLE 298
#define TYPE 299
#define VARIANT_RECORD 300
#define WHERE 301




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 320 "gecode/flatzinc/flatzinc.yxx"
{ int iValue; char* sValue; bool bValue; double dValue;
         std::vector<int>* setValue;
         AST::SetLit* setLit;
         std::vector<double>* floatSetValue;
         std::vector<AST::SetLit>* setValueList;
         Option<AST::SetLit* > oSet;
         VarSpec* varSpec;
         Option<AST::Node*> oArg;
         std::vector<VarSpec*>* varSpecVec;
         Option<std::vector<VarSpec*>* > oVarSpecVec;
         AST::Node* arg;
         AST::Array* argVec;
       }
/* Line 1529 of yacc.c.  */
#line 155 "gecode/flatzinc/flatzinc.tab.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



