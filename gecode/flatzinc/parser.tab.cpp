/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



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
     FZ_U_ID = 262,
     FZ_STRING_LIT = 263,
     FZ_VAR = 264,
     FZ_PAR = 265,
     FZ_ANNOTATION = 266,
     FZ_ANY = 267,
     FZ_ARRAY = 268,
     FZ_BOOL = 269,
     FZ_CASE = 270,
     FZ_COLONCOLON = 271,
     FZ_CONSTRAINT = 272,
     FZ_DEFAULT = 273,
     FZ_DOTDOT = 274,
     FZ_ELSE = 275,
     FZ_ELSEIF = 276,
     FZ_ENDIF = 277,
     FZ_ENUM = 278,
     FZ_FLOAT = 279,
     FZ_FUNCTION = 280,
     FZ_IF = 281,
     FZ_INCLUDE = 282,
     FZ_INT = 283,
     FZ_LET = 284,
     FZ_MAXIMIZE = 285,
     FZ_MINIMIZE = 286,
     FZ_OF = 287,
     FZ_SATISFY = 288,
     FZ_OUTPUT = 289,
     FZ_PREDICATE = 290,
     FZ_RECORD = 291,
     FZ_SET = 292,
     FZ_SHOW = 293,
     FZ_SHOWCOND = 294,
     FZ_SOLVE = 295,
     FZ_STRING = 296,
     FZ_TEST = 297,
     FZ_THEN = 298,
     FZ_TUPLE = 299,
     FZ_TYPE = 300,
     FZ_VARIANT_RECORD = 301,
     FZ_WHERE = 302
   };
#endif
/* Tokens.  */
#define FZ_INT_LIT 258
#define FZ_BOOL_LIT 259
#define FZ_FLOAT_LIT 260
#define FZ_ID 261
#define FZ_U_ID 262
#define FZ_STRING_LIT 263
#define FZ_VAR 264
#define FZ_PAR 265
#define FZ_ANNOTATION 266
#define FZ_ANY 267
#define FZ_ARRAY 268
#define FZ_BOOL 269
#define FZ_CASE 270
#define FZ_COLONCOLON 271
#define FZ_CONSTRAINT 272
#define FZ_DEFAULT 273
#define FZ_DOTDOT 274
#define FZ_ELSE 275
#define FZ_ELSEIF 276
#define FZ_ENDIF 277
#define FZ_ENUM 278
#define FZ_FLOAT 279
#define FZ_FUNCTION 280
#define FZ_IF 281
#define FZ_INCLUDE 282
#define FZ_INT 283
#define FZ_LET 284
#define FZ_MAXIMIZE 285
#define FZ_MINIMIZE 286
#define FZ_OF 287
#define FZ_SATISFY 288
#define FZ_OUTPUT 289
#define FZ_PREDICATE 290
#define FZ_RECORD 291
#define FZ_SET 292
#define FZ_SHOW 293
#define FZ_SHOWCOND 294
#define FZ_SOLVE 295
#define FZ_STRING 296
#define FZ_TEST 297
#define FZ_THEN 298
#define FZ_TUPLE 299
#define FZ_TYPE 300
#define FZ_VARIANT_RECORD 301
#define FZ_WHERE 302




/* Copy the first part of user declarations.  */
#line 40 "gecode/flatzinc/parser.yxx"

#define YYPARSE_PARAM parm
#define YYLEX_PARAM static_cast<ParserState*>(parm)->yyscanner
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/parser.hh>
#include <iostream>
#include <fstream>

#ifdef HAVE_MMAP
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

using namespace std;

int yyparse(void*);
int yylex(YYSTYPE*, void* scanner);
int yylex_init (void** scanner);
int yylex_destroy (void* scanner);
int yyget_lineno (void* scanner);
void yyset_extra (void* user_defined ,void* yyscanner );

extern int yydebug;

using namespace Gecode;
using namespace Gecode::FlatZinc;

void yyerror(void* parm, const char *str) {
  ParserState* pp = static_cast<ParserState*>(parm);
  pp->err << "Error: " << str
          << " in line no. " << yyget_lineno(pp->yyscanner)
          << std::endl;
  pp->hadError = true;
}

void yyassert(ParserState* pp, bool cond, const char* str)
{
  if (!cond) {
    pp->err << "Error: " << str
            << " in line no. " << yyget_lineno(pp->yyscanner)
            << std::endl;
    pp->hadError = true;
  }
}

/*
 * The symbol tables
 *
 */

AST::Node* getArrayElement(ParserState* pp, string id, int offset) {
  if (offset > 0) {
    SymbolEntry e;
    if (pp->symbols.get(id,e)) {
      switch (e.t) {
      case ST_INTVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::IntVar(pp->arrays[e.i+offset]);
      case ST_BOOLVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::BoolVar(pp->arrays[e.i+offset]);
      case ST_SETVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::SetVar(pp->arrays[e.i+offset]);
      case ST_INTVALARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::IntLit(pp->arrays[e.i+offset]);
      case ST_SETVALARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::SetLit(pp->setvals[pp->arrays[e.i+1]+offset-1]);
      default:
        break;
      }
    }
  }
error:
  pp->err << "Error: array access to " << id << " invalid"
          << " in line no. "
          << yyget_lineno(pp->yyscanner) << std::endl;
  pp->hadError = true;
  return new AST::IntVar(0); // keep things consistent
}
AST::Node* getVarRefArg(ParserState* pp, string id, bool annotation = false) {
  SymbolEntry e;
  if (pp->symbols.get(id, e)) {
    switch (e.t) {
    case ST_INTVAR: return new AST::IntVar(e.i);
    case ST_BOOLVAR: return new AST::BoolVar(e.i);
    case ST_SETVAR: return new AST::SetVar(e.i);
    default: break;
    }
  }
  
  if (annotation)
    return new AST::Atom(id);
  pp->err << "Error: undefined variable " << id
          << " in line no. "
          << yyget_lineno(pp->yyscanner) << std::endl;
  pp->hadError = true;
  return new AST::IntVar(0); // keep things consistent
}

void addDomainConstraint(ParserState* pp, std::string id, AST::Node* var,
                         Option<AST::SetLit* >& dom) {
  if (!dom())
    return;
  AST::Array* args = new AST::Array(2);
  args->a[0] = var;
  args->a[1] = dom.some();
  pp->domainConstraints.push_back(new ConExpr(id, args));
}

/*
 * Initialize the root gecode space
 *
 */

void initfg(ParserState* pp) {
  if (!pp->hadError)
    pp->fg->init(pp->intvars.size(),
                 pp->boolvars.size(),
                 pp->setvars.size());

  for (unsigned int i=0; i<pp->intvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newIntVar(static_cast<IntVarSpec*>(pp->intvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }
    if (pp->intvars[i].first[0] != '[') {
      delete pp->intvars[i].second;
      pp->intvars[i].second = NULL;
    }
  }
  for (unsigned int i=0; i<pp->boolvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newBoolVar(
          static_cast<BoolVarSpec*>(pp->boolvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }
    if (pp->boolvars[i].first[0] != '[') {
      delete pp->boolvars[i].second;
      pp->boolvars[i].second = NULL;
    }
  }
  for (unsigned int i=0; i<pp->setvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newSetVar(static_cast<SetVarSpec*>(pp->setvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }      
    if (pp->setvars[i].first[0] != '[') {
      delete pp->setvars[i].second;
      pp->setvars[i].second = NULL;
    }
  }
  for (unsigned int i=pp->domainConstraints.size(); i--;) {
    if (!pp->hadError) {
      try {
        assert(pp->domainConstraints[i]->args->a.size() == 2);
        pp->fg->postConstraint(*pp->domainConstraints[i], NULL);
        delete pp->domainConstraints[i];
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());        
      }
    }
  }
}

void fillPrinter(ParserState& pp, Gecode::FlatZinc::Printer& p) {
  p.init(pp.getOutput());
}

AST::Node* arrayOutput(AST::Call* ann) {
  AST::Array* a = NULL;
  
  if (ann->args->isArray()) {
    a = ann->args->getArray();
  } else {
    a = new AST::Array(ann->args);
  }
  
  std::ostringstream oss;
  
  oss << "array" << a->a.size() << "d(";
  for (unsigned int i=0; i<a->a.size(); i++) {
    AST::SetLit* s = a->a[i]->getSet();
    if (s->empty())
      oss << "{}, ";
    else if (s->interval)
      oss << s->min << ".." << s->max << ", ";
    else {
      oss << "{";
      for (unsigned int j=0; j<s->s.size(); j++) {
        oss << s->s[j];
        if (j<s->s.size()-1)
          oss << ",";
      }
      oss << "}, ";
    }
  }

  if (!ann->args->isArray()) {
    a->a[0] = NULL;
    delete a;
  }
  return new AST::String(oss.str());
}

/*
 * The main program
 *
 */

namespace Gecode { namespace FlatZinc {

  FlatZincSpace* parse(const std::string& filename, Printer& p, std::ostream& err,
                       FlatZincSpace* fzs) {
#ifdef HAVE_MMAP
    int fd;
    char* data;
    struct stat sbuf;
    fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
      err << "Cannot open file " << filename << endl;
      return NULL;
    }
    if (stat(filename.c_str(), &sbuf) == -1) {
      err << "Cannot stat file " << filename << endl;
      return NULL;      
    }
    data = (char*)mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd,0);
    if (data == (caddr_t)(-1)) {
      err << "Cannot mmap file " << filename << endl;
      return NULL;      
    }

    if (fzs == NULL) {
      fzs = new FlatZincSpace();
    }
    ParserState pp(data, sbuf.st_size, err, fzs);
#else
    std::ifstream file;
    file.open(filename.c_str());
    if (!file.is_open()) {
      err << "Cannot open file " << filename << endl;
      return NULL;
    }
    std::string s = string(istreambuf_iterator<char>(file),
                           istreambuf_iterator<char>());
    if (fzs == NULL) {
      fzs = new FlatZincSpace();
    }
    ParserState pp(s, err, fzs);
#endif
    yylex_init(&pp.yyscanner);
    yyset_extra(&pp, pp.yyscanner);
    // yydebug = 1;
    yyparse(&pp);
    fillPrinter(pp, p);
    
    if (pp.yyscanner)
      yylex_destroy(pp.yyscanner);
    return pp.hadError ? NULL : pp.fg;
  }

  FlatZincSpace* parse(std::istream& is, Printer& p, std::ostream& err,
                       FlatZincSpace* fzs) {
    std::string s = string(istreambuf_iterator<char>(is),
                           istreambuf_iterator<char>());

    if (fzs == NULL) {
      fzs = new FlatZincSpace();
    }
    ParserState pp(s, err, fzs);
    yylex_init(&pp.yyscanner);
    yyset_extra(&pp, pp.yyscanner);
    // yydebug = 1;
    yyparse(&pp);
    fillPrinter(pp, p);
    
    if (pp.yyscanner)
      yylex_destroy(pp.yyscanner);
    return pp.hadError ? NULL : pp.fg;
  }

}}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 347 "gecode/flatzinc/parser.yxx"
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
/* Line 193 of yacc.c.  */
#line 511 "gecode/flatzinc/parser.tab.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 524 "gecode/flatzinc/parser.tab.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   344

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  67
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNRULES -- Number of states.  */
#define YYNSTATES  336

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      49,    50,     2,     2,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    52,    48,
       2,    55,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    53,     2,    54,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,     2,    57,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    10,    12,    15,    19,    20,    22,
      25,    29,    30,    32,    35,    39,    45,    46,    49,    51,
      55,    59,    66,    74,    77,    79,    81,    85,    87,    89,
      91,    95,    97,   101,   103,   105,   112,   119,   126,   135,
     142,   149,   158,   172,   186,   200,   216,   232,   248,   264,
     282,   284,   286,   291,   292,   295,   297,   301,   302,   304,
     308,   310,   312,   317,   318,   321,   323,   327,   331,   333,
     335,   340,   341,   344,   346,   350,   354,   356,   358,   363,
     364,   367,   369,   373,   377,   378,   381,   382,   385,   386,
     389,   390,   393,   400,   404,   409,   411,   415,   419,   421,
     426,   428,   433,   437,   441,   442,   445,   447,   451,   452,
     455,   457,   461,   462,   465,   467,   471,   472,   475,   477,
     481,   483,   487,   489,   493,   494,   497,   499,   501,   503,
     505,   507,   512,   513,   516,   518,   522,   524,   529,   531,
     533,   534,   536,   539,   543,   548,   550,   552,   556,   558,
     562,   564,   566,   568,   570,   572,   577
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      59,     0,    -1,    60,    62,    64,    98,    48,    -1,    -1,
      61,    -1,    66,    48,    -1,    61,    66,    48,    -1,    -1,
      63,    -1,    75,    48,    -1,    63,    75,    48,    -1,    -1,
      65,    -1,    97,    48,    -1,    65,    97,    48,    -1,    35,
       6,    49,    67,    50,    -1,    -1,    68,    79,    -1,    69,
      -1,    68,    51,    69,    -1,    70,    52,     6,    -1,    13,
      53,    72,    54,    32,    71,    -1,    13,    53,    72,    54,
      32,     9,    71,    -1,     9,    71,    -1,    71,    -1,    99,
      -1,    37,    32,    99,    -1,    14,    -1,    24,    -1,    73,
      -1,    72,    51,    73,    -1,    28,    -1,     3,    19,     3,
      -1,     6,    -1,     7,    -1,     9,    99,    52,    74,   119,
     113,    -1,     9,   100,    52,    74,   119,   113,    -1,     9,
     101,    52,    74,   119,   113,    -1,     9,    37,    32,    99,
      52,    74,   119,   113,    -1,    99,    52,    74,   119,    55,
     114,    -1,    14,    52,    74,   119,    55,   114,    -1,    37,
      32,    99,    52,    74,   119,    55,   114,    -1,    13,    53,
       3,    19,     3,    54,    32,     9,    99,    52,    74,   119,
      93,    -1,    13,    53,     3,    19,     3,    54,    32,     9,
     100,    52,    74,   119,    94,    -1,    13,    53,     3,    19,
       3,    54,    32,     9,   101,    52,    74,   119,    95,    -1,
      13,    53,     3,    19,     3,    54,    32,     9,    37,    32,
      99,    52,    74,   119,    96,    -1,    13,    53,     3,    19,
       3,    54,    32,    99,    52,    74,   119,    55,    53,   103,
      54,    -1,    13,    53,     3,    19,     3,    54,    32,    14,
      52,    74,   119,    55,    53,   105,    54,    -1,    13,    53,
       3,    19,     3,    54,    32,    24,    52,    74,   119,    55,
      53,   107,    54,    -1,    13,    53,     3,    19,     3,    54,
      32,    37,    32,    99,    52,    74,   119,    55,    53,   109,
      54,    -1,     3,    -1,    74,    -1,    74,    53,     3,    54,
      -1,    -1,    78,    79,    -1,    76,    -1,    78,    51,    76,
      -1,    -1,    51,    -1,    53,    77,    54,    -1,     5,    -1,
      74,    -1,    74,    53,     3,    54,    -1,    -1,    83,    79,
      -1,    81,    -1,    83,    51,    81,    -1,    53,    82,    54,
      -1,     4,    -1,    74,    -1,    74,    53,     3,    54,    -1,
      -1,    87,    79,    -1,    85,    -1,    87,    51,    85,    -1,
      53,    86,    54,    -1,   102,    -1,    74,    -1,    74,    53,
       3,    54,    -1,    -1,    91,    79,    -1,    89,    -1,    91,
      51,    89,    -1,    53,    90,    54,    -1,    -1,    55,    80,
      -1,    -1,    55,    88,    -1,    -1,    55,    84,    -1,    -1,
      55,    92,    -1,    17,     6,    49,   111,    50,   119,    -1,
      40,   119,    33,    -1,    40,   119,   118,   117,    -1,    28,
      -1,    56,   103,    57,    -1,     3,    19,     3,    -1,    14,
      -1,    56,   106,    79,    57,    -1,    24,    -1,    56,   108,
      79,    57,    -1,    56,   103,    57,    -1,     3,    19,     3,
      -1,    -1,   104,    79,    -1,     3,    -1,   104,    51,     3,
      -1,    -1,   106,    79,    -1,     4,    -1,   106,    51,     4,
      -1,    -1,   108,    79,    -1,     5,    -1,   108,    51,     5,
      -1,    -1,   110,    79,    -1,   102,    -1,   110,    51,   102,
      -1,   112,    -1,   111,    51,   112,    -1,   114,    -1,    53,
     115,    54,    -1,    -1,    55,   114,    -1,     4,    -1,     3,
      -1,     5,    -1,   102,    -1,    74,    -1,    74,    53,   114,
      54,    -1,    -1,   116,    79,    -1,   114,    -1,   116,    51,
     114,    -1,    74,    -1,    74,    53,     3,    54,    -1,    31,
      -1,    30,    -1,    -1,   120,    -1,    16,   121,    -1,   120,
      16,   121,    -1,     6,    49,   122,    50,    -1,   123,    -1,
     121,    -1,   122,    51,   121,    -1,   124,    -1,    53,   122,
      54,    -1,     4,    -1,     3,    -1,     5,    -1,   102,    -1,
      74,    -1,    74,    53,   124,    54,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   447,   447,   449,   451,   454,   455,   459,   460,   464,
     465,   467,   469,   472,   473,   480,   482,   484,   487,   488,
     491,   494,   495,   496,   497,   500,   501,   502,   503,   506,
     507,   510,   511,   517,   517,   520,   552,   583,   588,   620,
     646,   657,   670,   735,   793,   800,   861,   882,   902,   909,
     932,   936,   951,   975,   976,   980,   982,   985,   985,   987,
     991,   993,  1008,  1031,  1032,  1036,  1038,  1042,  1046,  1048,
    1063,  1086,  1087,  1091,  1093,  1096,  1099,  1101,  1116,  1139,
    1140,  1144,  1146,  1149,  1154,  1155,  1160,  1161,  1166,  1167,
    1172,  1173,  1177,  1191,  1204,  1226,  1228,  1230,  1236,  1238,
    1251,  1252,  1259,  1261,  1268,  1269,  1273,  1275,  1280,  1281,
    1285,  1287,  1292,  1293,  1297,  1299,  1304,  1305,  1309,  1311,
    1319,  1321,  1325,  1327,  1332,  1333,  1337,  1339,  1341,  1343,
    1345,  1415,  1429,  1430,  1434,  1436,  1444,  1458,  1480,  1481,
    1489,  1490,  1494,  1496,  1500,  1504,  1508,  1510,  1514,  1516,
    1520,  1522,  1524,  1526,  1528,  1602,  1613
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FZ_INT_LIT", "FZ_BOOL_LIT",
  "FZ_FLOAT_LIT", "FZ_ID", "FZ_U_ID", "FZ_STRING_LIT", "FZ_VAR", "FZ_PAR",
  "FZ_ANNOTATION", "FZ_ANY", "FZ_ARRAY", "FZ_BOOL", "FZ_CASE",
  "FZ_COLONCOLON", "FZ_CONSTRAINT", "FZ_DEFAULT", "FZ_DOTDOT", "FZ_ELSE",
  "FZ_ELSEIF", "FZ_ENDIF", "FZ_ENUM", "FZ_FLOAT", "FZ_FUNCTION", "FZ_IF",
  "FZ_INCLUDE", "FZ_INT", "FZ_LET", "FZ_MAXIMIZE", "FZ_MINIMIZE", "FZ_OF",
  "FZ_SATISFY", "FZ_OUTPUT", "FZ_PREDICATE", "FZ_RECORD", "FZ_SET",
  "FZ_SHOW", "FZ_SHOWCOND", "FZ_SOLVE", "FZ_STRING", "FZ_TEST", "FZ_THEN",
  "FZ_TUPLE", "FZ_TYPE", "FZ_VARIANT_RECORD", "FZ_WHERE", "';'", "'('",
  "')'", "','", "':'", "'['", "']'", "'='", "'{'", "'}'", "$accept",
  "model", "preddecl_items", "preddecl_items_head", "vardecl_items",
  "vardecl_items_head", "constraint_items", "constraint_items_head",
  "preddecl_item", "pred_arg_list", "pred_arg_list_head", "pred_arg",
  "pred_arg_type", "pred_arg_simple_type", "pred_array_init",
  "pred_array_init_arg", "var_par_id", "vardecl_item", "int_init",
  "int_init_list", "int_init_list_head", "list_tail",
  "int_var_array_literal", "float_init", "float_init_list",
  "float_init_list_head", "float_var_array_literal", "bool_init",
  "bool_init_list", "bool_init_list_head", "bool_var_array_literal",
  "set_init", "set_init_list", "set_init_list_head",
  "set_var_array_literal", "vardecl_int_var_array_init",
  "vardecl_bool_var_array_init", "vardecl_float_var_array_init",
  "vardecl_set_var_array_init", "constraint_item", "solve_item",
  "int_ti_expr_tail", "bool_ti_expr_tail", "float_ti_expr_tail",
  "set_literal", "int_list", "int_list_head", "bool_list",
  "bool_list_head", "float_list", "float_list_head", "set_literal_list",
  "set_literal_list_head", "flat_expr_list", "flat_expr",
  "non_array_expr_opt", "non_array_expr", "non_array_expr_list",
  "non_array_expr_list_head", "solve_expr", "minmax", "annotations",
  "annotations_head", "annotation", "annotation_list", "annotation_expr",
  "ann_non_array_expr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    59,    40,
      41,    44,    58,    91,    93,    61,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    58,    59,    60,    60,    61,    61,    62,    62,    63,
      63,    64,    64,    65,    65,    66,    67,    67,    68,    68,
      69,    70,    70,    70,    70,    71,    71,    71,    71,    72,
      72,    73,    73,    74,    74,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      76,    76,    76,    77,    77,    78,    78,    79,    79,    80,
      81,    81,    81,    82,    82,    83,    83,    84,    85,    85,
      85,    86,    86,    87,    87,    88,    89,    89,    89,    90,
      90,    91,    91,    92,    93,    93,    94,    94,    95,    95,
      96,    96,    97,    98,    98,    99,    99,    99,   100,   100,
     101,   101,   102,   102,   103,   103,   104,   104,   105,   105,
     106,   106,   107,   107,   108,   108,   109,   109,   110,   110,
     111,   111,   112,   112,   113,   113,   114,   114,   114,   114,
     114,   114,   115,   115,   116,   116,   117,   117,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   124,   124,   124,   124,   124
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     0,     1,     2,     3,     0,     1,     2,
       3,     0,     1,     2,     3,     5,     0,     2,     1,     3,
       3,     6,     7,     2,     1,     1,     3,     1,     1,     1,
       3,     1,     3,     1,     1,     6,     6,     6,     8,     6,
       6,     8,    13,    13,    13,    15,    15,    15,    15,    17,
       1,     1,     4,     0,     2,     1,     3,     0,     1,     3,
       1,     1,     4,     0,     2,     1,     3,     3,     1,     1,
       4,     0,     2,     1,     3,     3,     1,     1,     4,     0,
       2,     1,     3,     3,     0,     2,     0,     2,     0,     2,
       0,     2,     6,     3,     4,     1,     3,     3,     1,     4,
       1,     4,     3,     3,     0,     2,     1,     3,     0,     2,
       1,     3,     0,     2,     1,     3,     0,     2,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     1,     1,     1,
       1,     4,     0,     2,     1,     3,     1,     4,     1,     1,
       0,     1,     2,     3,     4,     1,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     4,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     7,     4,     0,     0,     1,     0,     0,
       0,     0,    95,     0,   104,    11,     8,     0,     0,     0,
       5,    16,     0,    98,   100,     0,   104,     0,     0,     0,
       0,     0,     0,   106,     0,    57,     0,     0,    12,     0,
       0,     9,     0,     6,     0,     0,    27,    28,     0,     0,
      57,    18,     0,    24,    25,    97,     0,   110,   114,    57,
      57,     0,     0,     0,     0,    33,    34,   140,     0,    96,
      58,   105,     0,   140,     0,     0,    13,    10,   140,    23,
       0,     0,    15,    58,    17,     0,     0,    58,     0,    58,
       0,   140,   140,   140,     0,     0,     0,   141,     0,   107,
       0,     0,     2,    14,     0,     0,    31,     0,    29,    26,
      19,    20,     0,   111,    99,   115,   101,   124,   124,   124,
       0,   151,   150,   152,    33,   156,     0,   104,   154,   153,
     142,   145,   148,     0,     0,   140,   127,   126,   128,   132,
     130,   129,     0,   120,   122,   139,   138,    93,     0,     0,
       0,     0,     0,   140,     0,    35,    36,    37,     0,     0,
       0,   146,     0,     0,     0,    40,   143,     0,   134,     0,
      57,     0,   140,     0,   136,    94,    39,    32,    30,     0,
     124,   125,     0,   103,     0,     0,   149,   102,     0,     0,
     123,    58,   133,     0,    92,   121,     0,     0,    21,    38,
       0,     0,     0,     0,     0,   144,   147,   155,    41,   135,
     131,     0,    22,     0,     0,     0,     0,     0,     0,     0,
       0,   137,     0,     0,     0,     0,   140,   140,     0,   140,
       0,   140,   140,   140,     0,     0,     0,     0,     0,    84,
      86,    88,     0,     0,   140,     0,   140,     0,    42,     0,
      43,     0,    44,   108,   112,     0,   104,    90,    53,    85,
      71,    87,    63,    89,     0,    57,     0,    57,     0,     0,
       0,    45,    50,    51,    55,     0,    57,    68,    69,    73,
       0,    57,    60,    61,    65,     0,    57,    47,   109,    48,
     113,   116,    46,    79,    91,     0,    59,    58,    54,     0,
      75,    58,    72,     0,    67,    58,    64,     0,   118,     0,
      57,    77,    81,     0,    57,    76,     0,    56,     0,    74,
       0,    66,    49,    58,   117,     0,    83,    58,    80,    52,
      70,    62,   119,     0,    82,    78
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,    15,    16,    37,    38,     5,    49,
      50,    51,    52,    53,   107,   108,   140,    17,   274,   275,
     276,    71,   259,   284,   285,   286,   263,   279,   280,   281,
     261,   312,   313,   314,   294,   248,   250,   252,   271,    39,
      74,    54,    28,    29,   141,    34,    35,   264,    59,   266,
      60,   309,   310,   142,   143,   155,   144,   169,   170,   175,
     148,    96,    97,   161,   162,   131,   132
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -127
static const yytype_int16 yypact[] =
{
     -30,     2,    25,    71,   -30,   -14,    -2,  -127,    32,   187,
       4,     3,  -127,    28,    69,    60,    71,    31,    42,    35,
    -127,    45,    95,  -127,  -127,    68,    16,    62,    64,    66,
     136,    59,     7,  -127,    96,    80,   146,   102,    60,   108,
     109,  -127,    59,  -127,   239,   105,  -127,  -127,   129,   110,
     118,  -127,   119,  -127,  -127,  -127,     7,  -127,  -127,   121,
     122,    59,    59,    59,   151,  -127,  -127,   158,   123,  -127,
     173,  -127,   130,   158,   133,   137,  -127,  -127,   158,  -127,
      33,     7,  -127,    45,  -127,   185,   147,   194,   143,   198,
     152,   158,   158,   158,   205,    85,   155,   197,    59,  -127,
     106,    89,  -127,  -127,   159,   201,  -127,    51,  -127,  -127,
    -127,  -127,    59,  -127,  -127,  -127,  -127,   162,   162,   162,
     168,   207,  -127,  -127,   183,  -127,    85,    69,   186,  -127,
    -127,  -127,  -127,   160,    85,   158,   207,  -127,  -127,   160,
     188,  -127,    56,  -127,  -127,  -127,  -127,  -127,    59,   160,
     233,    33,   212,   158,   160,  -127,  -127,  -127,   213,   244,
      85,  -127,    74,   192,   140,  -127,  -127,   195,  -127,   200,
     204,   160,   158,   106,   199,  -127,  -127,  -127,  -127,   112,
     162,  -127,   209,  -127,   100,    85,  -127,  -127,   202,   160,
    -127,   160,  -127,   203,  -127,  -127,   248,   239,  -127,  -127,
     245,   206,   208,   229,   219,  -127,  -127,  -127,  -127,  -127,
    -127,   218,  -127,   243,   225,   226,   227,    59,    59,     7,
      59,  -127,     7,    59,    59,    59,   158,   158,   228,   158,
     231,   158,   158,   158,   230,   232,    59,   234,    59,   235,
     236,   237,   240,   241,   158,   246,   158,   247,  -127,   249,
    -127,   250,  -127,   277,   279,   242,    69,   251,    46,  -127,
     128,  -127,    34,  -127,   253,   121,   254,   122,   252,   255,
     257,  -127,  -127,   258,  -127,   259,   261,  -127,   262,  -127,
     260,   265,  -127,   264,  -127,   266,   267,  -127,  -127,  -127,
    -127,     0,  -127,    20,  -127,   283,  -127,    46,  -127,   285,
    -127,   128,  -127,   295,  -127,    34,  -127,   207,  -127,   268,
     270,   271,  -127,   269,   274,  -127,   272,  -127,   273,  -127,
     275,  -127,  -127,     0,  -127,   301,  -127,    20,  -127,  -127,
    -127,  -127,  -127,   276,  -127,  -127
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,   315,  -127,
    -127,   256,  -127,   -42,  -127,   177,   -31,   316,    36,  -127,
    -127,   -46,  -127,    26,  -127,  -127,  -127,    37,  -127,  -127,
    -127,     8,  -127,  -127,  -127,  -127,  -127,  -127,  -127,   296,
    -127,     6,   141,   142,   -89,  -126,  -127,  -127,    83,  -127,
      86,  -127,  -127,  -127,   164,  -102,  -121,  -127,  -127,  -127,
    -127,   -49,  -127,   -88,   184,  -127,   179
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      67,   163,    79,   307,    84,     1,   129,   130,     6,    18,
       8,    78,   165,    88,    90,    27,   156,   157,   168,    33,
      57,    58,    18,   307,   101,     7,    65,    66,   176,   104,
      91,    92,    93,   181,    20,    12,   105,   129,    68,   282,
      65,    66,   117,   118,   119,   129,   166,    21,     8,   272,
     193,    22,    65,    66,    44,    31,   127,    30,    45,    46,
      32,   106,    86,    14,   128,    65,    66,   135,   208,    47,
     209,   129,    33,    12,     8,   129,   127,    36,   199,    41,
       9,   153,    48,    43,    10,    11,   167,   109,   121,   122,
     123,   124,    66,   125,    42,   128,   129,   206,    55,    12,
      56,    14,   151,   128,   180,   152,   172,   173,    13,   136,
     137,   138,    65,    66,    61,     8,    62,   174,    63,   145,
     146,   197,   147,   194,   192,   185,    46,    14,   186,   128,
     269,    70,   277,   128,    65,    66,    47,   198,   126,    64,
      12,   127,    73,   121,   122,   123,    65,    66,   125,    48,
     205,   185,    72,    69,   128,   212,    76,    77,    80,   139,
      82,    81,   127,   136,   137,   138,    65,    66,    14,    83,
      94,    85,    87,    89,    95,    98,    99,   234,   235,   100,
     237,   102,   239,   240,   241,   103,   226,   227,   204,   229,
       8,   111,   231,   232,   233,   255,   127,   257,   113,   112,
     114,    23,   308,   115,   315,   244,   214,   246,   120,   116,
     133,    24,     8,   134,   149,    12,   127,   154,   200,   288,
     150,   290,   158,   201,    25,   228,   159,   273,   230,   278,
     298,   283,   160,   202,   332,   302,   177,    12,   315,   164,
     306,   171,     8,    26,   179,   182,   203,   183,     8,   187,
     189,   211,   196,    46,   190,   191,   207,   210,   217,    23,
     218,   219,   311,    47,   324,    14,   273,    12,   328,    24,
     278,   220,   221,    12,   283,   222,    48,   223,   224,   225,
     236,    57,   213,   238,    58,   242,   316,   243,   318,   245,
     247,   249,   251,   253,   254,    14,   311,   268,   320,   256,
     258,    26,   260,   262,   333,   291,   270,   287,   289,   292,
     293,   295,   297,   296,   300,   299,   301,   303,   305,    19,
     304,   323,   322,   326,   325,   327,   329,   330,   178,   331,
     335,   321,    40,   317,    75,   334,   265,   195,   319,   110,
     267,   215,   216,   188,   184
};

static const yytype_uint16 yycheck[] =
{
      31,   127,    44,     3,    50,    35,    95,    95,     6,     3,
       3,    42,   133,    59,    60,     9,   118,   119,   139,     3,
       4,     5,    16,     3,    73,     0,     6,     7,   149,    78,
      61,    62,    63,   154,    48,    28,     3,   126,    32,     5,
       6,     7,    91,    92,    93,   134,   134,    49,     3,     3,
     171,    19,     6,     7,     9,    52,    56,    53,    13,    14,
      32,    28,    56,    56,    95,     6,     7,    98,   189,    24,
     191,   160,     3,    28,     3,   164,    56,    17,   180,    48,
       9,   112,    37,    48,    13,    14,   135,    81,     3,     4,
       5,     6,     7,     8,    52,   126,   185,   185,     3,    28,
      32,    56,    51,   134,   153,    54,    50,    51,    37,     3,
       4,     5,     6,     7,    52,     3,    52,   148,    52,    30,
      31,     9,    33,   172,   170,    51,    14,    56,    54,   160,
     256,    51,     4,   164,     6,     7,    24,   179,    53,     3,
      28,    56,    40,     3,     4,     5,     6,     7,     8,    37,
      50,    51,     6,    57,   185,   197,    48,    48,    53,    53,
      50,    32,    56,     3,     4,     5,     6,     7,    56,    51,
      19,    52,    51,    51,    16,    52,     3,   226,   227,    49,
     229,    48,   231,   232,   233,    48,   217,   218,   182,   220,
       3,     6,   223,   224,   225,   244,    56,   246,     4,    52,
      57,    14,   291,     5,   293,   236,   200,   238,     3,    57,
      55,    24,     3,    16,    55,    28,    56,    55,     9,   265,
      19,   267,    54,    14,    37,   219,    19,   258,   222,   260,
     276,   262,    49,    24,   323,   281,     3,    28,   327,    53,
     286,    53,     3,    56,    32,    32,    37,     3,     3,    57,
      55,     3,    53,    14,    54,    51,    54,    54,    52,    14,
      52,    32,   293,    24,   310,    56,   297,    28,   314,    24,
     301,    52,    54,    28,   305,    32,    37,    52,    52,    52,
      52,     4,    37,    52,     5,    55,     3,    55,     3,    55,
      55,    55,    55,    53,    53,    56,   327,    55,     3,    53,
      53,    56,    53,    53,     3,    53,    55,    54,    54,    54,
      53,    53,    51,    54,    54,    53,    51,    53,    51,     4,
      54,    51,    54,    54,    53,    51,    54,    54,   151,    54,
      54,   305,    16,   297,    38,   327,   253,   173,   301,    83,
     254,   200,   200,   164,   160
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    35,    59,    60,    61,    66,     6,     0,     3,     9,
      13,    14,    28,    37,    56,    62,    63,    75,    99,    66,
      48,    49,    19,    14,    24,    37,    56,    99,   100,   101,
      53,    52,    32,     3,   103,   104,    17,    64,    65,    97,
      75,    48,    52,    48,     9,    13,    14,    24,    37,    67,
      68,    69,    70,    71,    99,     3,    32,     4,     5,   106,
     108,    52,    52,    52,     3,     6,     7,    74,    99,    57,
      51,    79,     6,    40,    98,    97,    48,    48,    74,    71,
      53,    32,    50,    51,    79,    52,    99,    51,    79,    51,
      79,    74,    74,    74,    19,    16,   119,   120,    52,     3,
      49,   119,    48,    48,   119,     3,    28,    72,    73,    99,
      69,     6,    52,     4,    57,     5,    57,   119,   119,   119,
       3,     3,     4,     5,     6,     8,    53,    56,    74,   102,
     121,   123,   124,    55,    16,    74,     3,     4,     5,    53,
      74,   102,   111,   112,   114,    30,    31,    33,   118,    55,
      19,    51,    54,    74,    55,   113,   113,   113,    54,    19,
      49,   121,   122,   103,    53,   114,   121,   119,   114,   115,
     116,    53,    50,    51,    74,   117,   114,     3,    73,    32,
     119,   114,    32,     3,   122,    51,    54,    57,   124,    55,
      54,    51,    79,   114,   119,   112,    53,     9,    71,   113,
       9,    14,    24,    37,    99,    50,   121,    54,   114,   114,
      54,     3,    71,    37,    99,   100,   101,    52,    52,    32,
      52,    54,    32,    52,    52,    52,    74,    74,    99,    74,
      99,    74,    74,    74,   119,   119,    52,   119,    52,   119,
     119,   119,    55,    55,    74,    55,    74,    55,    93,    55,
      94,    55,    95,    53,    53,   119,    53,   119,    53,    80,
      53,    88,    53,    84,   105,   106,   107,   108,    55,   103,
      55,    96,     3,    74,    76,    77,    78,     4,    74,    85,
      86,    87,     5,    74,    81,    82,    83,    54,    79,    54,
      79,    53,    54,    53,    92,    53,    54,    51,    79,    53,
      54,    51,    79,    53,    54,    51,    79,     3,   102,   109,
     110,    74,    89,    90,    91,   102,     3,    76,     3,    85,
       3,    81,    54,    51,    79,    53,    54,    51,    79,    54,
      54,    54,   102,     3,    89,    54
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (parm, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, parm); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *parm)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parm)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *parm;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (parm);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *parm)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parm)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *parm;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parm);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, void *parm)
#else
static void
yy_reduce_print (yyvsp, yyrule, parm)
    YYSTYPE *yyvsp;
    int yyrule;
    void *parm;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , parm);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parm); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *parm)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parm)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    void *parm;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parm);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void *parm);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *parm)
#else
int
yyparse (parm)
    void *parm;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 7:
#line 459 "gecode/flatzinc/parser.yxx"
    { initfg(static_cast<ParserState*>(parm)); ;}
    break;

  case 8:
#line 461 "gecode/flatzinc/parser.yxx"
    { initfg(static_cast<ParserState*>(parm)); ;}
    break;

  case 35:
#line 521 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, !(yyvsp[(2) - (6)].oSet)() || !(yyvsp[(2) - (6)].oSet).some()->empty(), "Empty var int domain");
        bool print = (yyvsp[(5) - (6)].argVec)->hasAtom("output_var");
        bool introduced = (yyvsp[(5) - (6)].argVec)->hasAtom("var_is_introduced");
        yyassert(pp,
          pp->symbols.put((yyvsp[(4) - (6)].sValue), se_iv(pp->intvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[(4) - (6)].sValue)), new AST::IntVar(pp->intvars.size()));
        }
        if ((yyvsp[(6) - (6)].oArg)()) {
          AST::Node* arg = (yyvsp[(6) - (6)].oArg).some();
          if (arg->isInt()) {
            pp->intvars.push_back(varspec((yyvsp[(4) - (6)].sValue),
              new IntVarSpec(arg->getInt(),introduced)));
          } else if (arg->isIntVar()) {
            pp->intvars.push_back(varspec((yyvsp[(4) - (6)].sValue),
              new IntVarSpec(Alias(arg->getIntVar()),introduced)));
          } else {
            yyassert(pp, false, "Invalid var int initializer");
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "int_in",
                                new AST::IntVar(pp->intvars.size()-1), (yyvsp[(2) - (6)].oSet));
          delete arg;
        } else {
          pp->intvars.push_back(varspec((yyvsp[(4) - (6)].sValue), new IntVarSpec((yyvsp[(2) - (6)].oSet),introduced)));
        }
        delete (yyvsp[(5) - (6)].argVec); free((yyvsp[(4) - (6)].sValue));
      ;}
    break;

  case 36:
#line 553 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[(5) - (6)].argVec)->hasAtom("output_var");
        bool introduced = (yyvsp[(5) - (6)].argVec)->hasAtom("var_is_introduced");
        yyassert(pp,
          pp->symbols.put((yyvsp[(4) - (6)].sValue), se_bv(pp->boolvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[(4) - (6)].sValue)), new AST::BoolVar(pp->boolvars.size()));
        }
        if ((yyvsp[(6) - (6)].oArg)()) {
          AST::Node* arg = (yyvsp[(6) - (6)].oArg).some();
          if (arg->isBool()) {
            pp->boolvars.push_back(varspec((yyvsp[(4) - (6)].sValue),
              new BoolVarSpec(arg->getBool(),introduced)));            
          } else if (arg->isBoolVar()) {
            pp->boolvars.push_back(varspec((yyvsp[(4) - (6)].sValue),
              new BoolVarSpec(Alias(arg->getBoolVar()),introduced)));
          } else {
            yyassert(pp, false, "Invalid var bool initializer");
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "int_in",
                                new AST::BoolVar(pp->boolvars.size()-1), (yyvsp[(2) - (6)].oSet));
          delete arg;
        } else {
          pp->boolvars.push_back(varspec((yyvsp[(4) - (6)].sValue), new BoolVarSpec((yyvsp[(2) - (6)].oSet),introduced)));
        }
        delete (yyvsp[(5) - (6)].argVec); free((yyvsp[(4) - (6)].sValue));
      ;}
    break;

  case 37:
#line 584 "gecode/flatzinc/parser.yxx"
    { ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, false, "Floats not supported");
        delete (yyvsp[(5) - (6)].argVec); free((yyvsp[(4) - (6)].sValue));
      ;}
    break;

  case 38:
#line 589 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[(7) - (8)].argVec)->hasAtom("output_var");
        bool introduced = (yyvsp[(7) - (8)].argVec)->hasAtom("var_is_introduced");
        yyassert(pp,
          pp->symbols.put((yyvsp[(6) - (8)].sValue), se_sv(pp->setvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[(6) - (8)].sValue)), new AST::SetVar(pp->setvars.size()));
        }
        if ((yyvsp[(8) - (8)].oArg)()) {
          AST::Node* arg = (yyvsp[(8) - (8)].oArg).some();
          if (arg->isSet()) {
            pp->setvars.push_back(varspec((yyvsp[(6) - (8)].sValue),
              new SetVarSpec(arg->getSet(),introduced)));
          } else if (arg->isSetVar()) {
            pp->setvars.push_back(varspec((yyvsp[(6) - (8)].sValue),
              new SetVarSpec(Alias(arg->getSetVar()),introduced)));
            delete arg;
          } else {
            yyassert(pp, false, "Invalid var set initializer");
            delete arg;
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "set_subset",
                                new AST::SetVar(pp->setvars.size()-1), (yyvsp[(4) - (8)].oSet));
        } else {
          pp->setvars.push_back(varspec((yyvsp[(6) - (8)].sValue), new SetVarSpec((yyvsp[(4) - (8)].oSet),introduced)));
        }
        delete (yyvsp[(7) - (8)].argVec); free((yyvsp[(6) - (8)].sValue));
      ;}
    break;

  case 39:
#line 621 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, !(yyvsp[(1) - (6)].oSet)() || !(yyvsp[(1) - (6)].oSet).some()->empty(), "Empty int domain");
        yyassert(pp, (yyvsp[(6) - (6)].arg)->isInt(), "Invalid int initializer");
        int i = -1;
        bool isInt = (yyvsp[(6) - (6)].arg)->isInt(i);
        if ((yyvsp[(1) - (6)].oSet)() && isInt) {
          AST::SetLit* sl = (yyvsp[(1) - (6)].oSet).some();
          if (sl->interval) {
            yyassert(pp, i >= sl->min && i <= sl->max, "Empty int domain");
          } else {
            bool found = false;
            for (unsigned int j=0; j<sl->s.size(); j++)
              if (sl->s[j] == i) {
                found = true;
                break;
              }
            yyassert(pp, found, "Empty int domain");
          }
        }
        yyassert(pp,
          pp->symbols.put((yyvsp[(3) - (6)].sValue), se_i(i)),
          "Duplicate symbol");
        delete (yyvsp[(4) - (6)].argVec); free((yyvsp[(3) - (6)].sValue));        
      ;}
    break;

  case 40:
#line 647 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[(6) - (6)].arg)->isBool(), "Invalid bool initializer");
        if ((yyvsp[(6) - (6)].arg)->isBool()) {
          yyassert(pp,
            pp->symbols.put((yyvsp[(3) - (6)].sValue), se_b((yyvsp[(6) - (6)].arg)->getBool())),
            "Duplicate symbol");
        }
        delete (yyvsp[(4) - (6)].argVec); free((yyvsp[(3) - (6)].sValue));        
      ;}
    break;

  case 41:
#line 658 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, !(yyvsp[(3) - (8)].oSet)() || !(yyvsp[(3) - (8)].oSet).some()->empty(), "Empty set domain");
        yyassert(pp, (yyvsp[(8) - (8)].arg)->isSet(), "Invalid set initializer");
        AST::SetLit* set = (yyvsp[(8) - (8)].arg)->getSet();
        pp->setvals.push_back(*set);
        yyassert(pp,
          pp->symbols.put((yyvsp[(5) - (8)].sValue), se_s(pp->setvals.size()-1)),
          "Duplicate symbol");
        delete set;
        delete (yyvsp[(6) - (8)].argVec); free((yyvsp[(5) - (8)].sValue));        
      ;}
    break;

  case 42:
#line 672 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[(3) - (13)].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          bool print = (yyvsp[(12) - (13)].argVec)->hasCall("output_array");
          vector<int> vars((yyvsp[(5) - (13)].iValue));
          yyassert(pp, !(yyvsp[(9) - (13)].oSet)() || !(yyvsp[(9) - (13)].oSet).some()->empty(),
                   "Empty var int domain");
          if (!pp->hadError) {
            if ((yyvsp[(13) - (13)].oVarSpecVec)()) {
              vector<VarSpec*>* vsv = (yyvsp[(13) - (13)].oVarSpecVec).some();
              yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[(5) - (13)].iValue)),
                       "Initializer size does not match array dimension");
              if (!pp->hadError) {
                for (int i=0; i<(yyvsp[(5) - (13)].iValue); i++) {
                  IntVarSpec* ivsv = static_cast<IntVarSpec*>((*vsv)[i]);
                  if (ivsv->alias) {
                    vars[i] = ivsv->i;
                  } else {
                    vars[i] = pp->intvars.size();
                    pp->intvars.push_back(varspec((yyvsp[(11) - (13)].sValue), ivsv));
                  }
                  if (!pp->hadError && (yyvsp[(9) - (13)].oSet)()) {
                    Option<AST::SetLit*> opt =
                      Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[(9) - (13)].oSet).some()));                    
                    addDomainConstraint(pp, "int_in",
                                        new AST::IntVar(vars[i]),
                                        opt);
                  }
                }
              }
              delete vsv;
            } else {
              IntVarSpec* ispec = new IntVarSpec((yyvsp[(9) - (13)].oSet),!print);
              string arrayname = "["; arrayname += (yyvsp[(11) - (13)].sValue);
              for (int i=0; i<(yyvsp[(5) - (13)].iValue)-1; i++) {
                vars[i] = pp->intvars.size();
                pp->intvars.push_back(varspec(arrayname, ispec));
              }          
              vars[(yyvsp[(5) - (13)].iValue)-1] = pp->intvars.size();
              pp->intvars.push_back(varspec((yyvsp[(11) - (13)].sValue), ispec));
            }
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[(12) - (13)].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[(5) - (13)].iValue); i++)
              output->a.push_back(new AST::IntVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[(11) - (13)].sValue)), a);
          }
          int iva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(11) - (13)].sValue), se_iva(iva)),
            "Duplicate symbol");
        }
        delete (yyvsp[(12) - (13)].argVec); free((yyvsp[(11) - (13)].sValue));
      ;}
    break;

  case 43:
#line 737 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[(12) - (13)].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[(3) - (13)].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          vector<int> vars((yyvsp[(5) - (13)].iValue));
          if ((yyvsp[(13) - (13)].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[(13) - (13)].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[(5) - (13)].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[(5) - (13)].iValue); i++) {
                BoolVarSpec* bvsv = static_cast<BoolVarSpec*>((*vsv)[i]);
                if (bvsv->alias)
                  vars[i] = bvsv->i;
                else {
                  vars[i] = pp->boolvars.size();
                  pp->boolvars.push_back(varspec((yyvsp[(11) - (13)].sValue), (*vsv)[i]));
                }
                if (!pp->hadError && (yyvsp[(9) - (13)].oSet)()) {
                  Option<AST::SetLit*> opt =
                    Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[(9) - (13)].oSet).some()));                    
                  addDomainConstraint(pp, "int_in",
                                      new AST::BoolVar(vars[i]),
                                      opt);
                }
              }
            }
            delete vsv;
          } else {
            for (int i=0; i<(yyvsp[(5) - (13)].iValue); i++) {
              vars[i] = pp->boolvars.size();
              pp->boolvars.push_back(varspec((yyvsp[(11) - (13)].sValue),
                                             new BoolVarSpec((yyvsp[(9) - (13)].oSet),!print)));
            }          
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[(12) - (13)].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[(5) - (13)].iValue); i++)
              output->a.push_back(new AST::BoolVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[(11) - (13)].sValue)), a);
          }
          int bva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(11) - (13)].sValue), se_bva(bva)),
            "Duplicate symbol");
        }
        delete (yyvsp[(12) - (13)].argVec); free((yyvsp[(11) - (13)].sValue));
      ;}
    break;

  case 44:
#line 795 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, false, "Floats not supported");
        delete (yyvsp[(12) - (13)].argVec); free((yyvsp[(11) - (13)].sValue));
      ;}
    break;

  case 45:
#line 802 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[(14) - (15)].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[(3) - (15)].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          vector<int> vars((yyvsp[(5) - (15)].iValue));
          if ((yyvsp[(15) - (15)].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[(15) - (15)].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[(5) - (15)].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[(5) - (15)].iValue); i++) {
                SetVarSpec* svsv = static_cast<SetVarSpec*>((*vsv)[i]);
                if (svsv->alias)
                  vars[i] = svsv->i;
                else {
                  vars[i] = pp->setvars.size();
                  pp->setvars.push_back(varspec((yyvsp[(13) - (15)].sValue), (*vsv)[i]));
                }
                if (!pp->hadError && (yyvsp[(11) - (15)].oSet)()) {
                  Option<AST::SetLit*> opt =
                    Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[(11) - (15)].oSet).some()));                    
                  addDomainConstraint(pp, "set_subset",
                                      new AST::SetVar(vars[i]),
                                      opt);
                }
              }
            }
            delete vsv;
          } else {
            SetVarSpec* ispec = new SetVarSpec((yyvsp[(11) - (15)].oSet),!print);
            string arrayname = "["; arrayname += (yyvsp[(13) - (15)].sValue);
            for (int i=0; i<(yyvsp[(5) - (15)].iValue)-1; i++) {
              vars[i] = pp->setvars.size();
              pp->setvars.push_back(varspec(arrayname, ispec));
            }          
            vars[(yyvsp[(5) - (15)].iValue)-1] = pp->setvars.size();
            pp->setvars.push_back(varspec((yyvsp[(13) - (15)].sValue), ispec));
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[(14) - (15)].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[(5) - (15)].iValue); i++)
              output->a.push_back(new AST::SetVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[(13) - (15)].sValue)), a);
          }
          int sva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(13) - (15)].sValue), se_sva(sva)),
            "Duplicate symbol");
        }
        delete (yyvsp[(14) - (15)].argVec); free((yyvsp[(13) - (15)].sValue));
      ;}
    break;

  case 46:
#line 863 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[(3) - (15)].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[(14) - (15)].setValue)->size() == static_cast<unsigned int>((yyvsp[(5) - (15)].iValue)),
                 "Initializer size does not match array dimension");

        if (!pp->hadError) {
          int ia = pp->arrays.size();
          pp->arrays.push_back((yyvsp[(14) - (15)].setValue)->size());
          for (unsigned int i=0; i<(yyvsp[(14) - (15)].setValue)->size(); i++)
            pp->arrays.push_back((*(yyvsp[(14) - (15)].setValue))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(10) - (15)].sValue), se_ia(ia)),
            "Duplicate symbol");
        }
        delete (yyvsp[(14) - (15)].setValue);
        free((yyvsp[(10) - (15)].sValue));
        delete (yyvsp[(11) - (15)].argVec);
      ;}
    break;

  case 47:
#line 884 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[(3) - (15)].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[(14) - (15)].setValue)->size() == static_cast<unsigned int>((yyvsp[(5) - (15)].iValue)),
                 "Initializer size does not match array dimension");
        if (!pp->hadError) {
          int ia = pp->arrays.size();
          pp->arrays.push_back((yyvsp[(14) - (15)].setValue)->size());
          for (unsigned int i=0; i<(yyvsp[(14) - (15)].setValue)->size(); i++)
            pp->arrays.push_back((*(yyvsp[(14) - (15)].setValue))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(10) - (15)].sValue), se_ba(ia)),
            "Duplicate symbol");
        }
        delete (yyvsp[(14) - (15)].setValue);
        free((yyvsp[(10) - (15)].sValue));
        delete (yyvsp[(11) - (15)].argVec);
      ;}
    break;

  case 48:
#line 904 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, false, "Floats not supported");
        delete (yyvsp[(11) - (15)].argVec); free((yyvsp[(10) - (15)].sValue));
      ;}
    break;

  case 49:
#line 911 "gecode/flatzinc/parser.yxx"
    {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[(3) - (17)].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[(16) - (17)].setValueList)->size() == static_cast<unsigned int>((yyvsp[(5) - (17)].iValue)),
                 "Initializer size does not match array dimension");
        if (!pp->hadError) {
          int sa = pp->arrays.size();
          pp->arrays.push_back((yyvsp[(16) - (17)].setValueList)->size());
          pp->arrays.push_back(pp->setvals.size());
          for (unsigned int i=0; i<(yyvsp[(16) - (17)].setValueList)->size(); i++)
            pp->setvals.push_back((*(yyvsp[(16) - (17)].setValueList))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[(12) - (17)].sValue), se_sa(sa)),
            "Duplicate symbol");
        }

        delete (yyvsp[(16) - (17)].setValueList);
        delete (yyvsp[(13) - (17)].argVec); free((yyvsp[(12) - (17)].sValue));
      ;}
    break;

  case 50:
#line 933 "gecode/flatzinc/parser.yxx"
    { 
        (yyval.varSpec) = new IntVarSpec((yyvsp[(1) - (1)].iValue),false);
      ;}
    break;

  case 51:
#line 937 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e) && e.t == ST_INTVAR)
          (yyval.varSpec) = new IntVarSpec(Alias(e.i),false);
        else {
          pp->err << "Error: undefined identifier " << (yyvsp[(1) - (1)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new IntVarSpec(0,false); // keep things consistent
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 52:
#line 952 "gecode/flatzinc/parser.yxx"
    { 
        vector<int> v;
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (4)].sValue), e) && e.t == ST_INTVARARRAY) {
          yyassert(pp,(yyvsp[(3) - (4)].iValue) > 0 && (yyvsp[(3) - (4)].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new IntVarSpec(Alias(pp->arrays[e.i+(yyvsp[(3) - (4)].iValue)]),false);
          else
            (yyval.varSpec) = new IntVarSpec(0,false); // keep things consistent
        } else {
          pp->err << "Error: undefined array identifier " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new IntVarSpec(0,false); // keep things consistent
        }
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 53:
#line 975 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(0); ;}
    break;

  case 54:
#line 977 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (2)].varSpecVec); ;}
    break;

  case 55:
#line 981 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[(1) - (1)].varSpec); ;}
    break;

  case 56:
#line 983 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (3)].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[(3) - (3)].varSpec)); ;}
    break;

  case 59:
#line 988 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(2) - (3)].varSpecVec); ;}
    break;

  case 60:
#line 992 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpec) = new FloatVarSpec((yyvsp[(1) - (1)].dValue),false); ;}
    break;

  case 61:
#line 994 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e) && e.t == ST_FLOATVAR)
          (yyval.varSpec) = new FloatVarSpec(Alias(e.i),false);
        else {
          pp->err << "Error: undefined identifier " << (yyvsp[(1) - (1)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new FloatVarSpec(0.0,false);
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 62:
#line 1009 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (4)].sValue), e) && e.t == ST_FLOATVARARRAY) {
          yyassert(pp,(yyvsp[(3) - (4)].iValue) > 0 && (yyvsp[(3) - (4)].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new FloatVarSpec(Alias(pp->arrays[e.i+(yyvsp[(3) - (4)].iValue)]),false);
          else
            (yyval.varSpec) = new FloatVarSpec(0.0,false);
        } else {
          pp->err << "Error: undefined array identifier " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new FloatVarSpec(0.0,false);
        }
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 63:
#line 1031 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(0); ;}
    break;

  case 64:
#line 1033 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (2)].varSpecVec); ;}
    break;

  case 65:
#line 1037 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[(1) - (1)].varSpec); ;}
    break;

  case 66:
#line 1039 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (3)].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[(3) - (3)].varSpec)); ;}
    break;

  case 67:
#line 1043 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(2) - (3)].varSpecVec); ;}
    break;

  case 68:
#line 1047 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpec) = new BoolVarSpec((yyvsp[(1) - (1)].iValue),false); ;}
    break;

  case 69:
#line 1049 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e) && e.t == ST_BOOLVAR)
          (yyval.varSpec) = new BoolVarSpec(Alias(e.i),false);
        else {
          pp->err << "Error: undefined identifier " << (yyvsp[(1) - (1)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new BoolVarSpec(false,false);
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 70:
#line 1064 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (4)].sValue), e) && e.t == ST_BOOLVARARRAY) {
          yyassert(pp,(yyvsp[(3) - (4)].iValue) > 0 && (yyvsp[(3) - (4)].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new BoolVarSpec(Alias(pp->arrays[e.i+(yyvsp[(3) - (4)].iValue)]),false);
          else
            (yyval.varSpec) = new BoolVarSpec(false,false);
        } else {
          pp->err << "Error: undefined array identifier " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new BoolVarSpec(false,false);
        }
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 71:
#line 1086 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(0); ;}
    break;

  case 72:
#line 1088 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (2)].varSpecVec); ;}
    break;

  case 73:
#line 1092 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[(1) - (1)].varSpec); ;}
    break;

  case 74:
#line 1094 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (3)].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[(3) - (3)].varSpec)); ;}
    break;

  case 75:
#line 1096 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(2) - (3)].varSpecVec); ;}
    break;

  case 76:
#line 1100 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpec) = new SetVarSpec(Option<AST::SetLit*>::some((yyvsp[(1) - (1)].setLit)),false); ;}
    break;

  case 77:
#line 1102 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e) && e.t == ST_SETVAR)
          (yyval.varSpec) = new SetVarSpec(Alias(e.i),false);
        else {
          pp->err << "Error: undefined identifier " << (yyvsp[(1) - (1)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new SetVarSpec(Alias(0),false);
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 78:
#line 1117 "gecode/flatzinc/parser.yxx"
    { 
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[(1) - (4)].sValue), e) && e.t == ST_SETVARARRAY) {
          yyassert(pp,(yyvsp[(3) - (4)].iValue) > 0 && (yyvsp[(3) - (4)].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new SetVarSpec(Alias(pp->arrays[e.i+(yyvsp[(3) - (4)].iValue)]),false);
          else
            (yyval.varSpec) = new SetVarSpec(Alias(0),false);
        } else {
          pp->err << "Error: undefined array identifier " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new SetVarSpec(Alias(0),false);
        }
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 79:
#line 1139 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(0); ;}
    break;

  case 80:
#line 1141 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (2)].varSpecVec); ;}
    break;

  case 81:
#line 1145 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[(1) - (1)].varSpec); ;}
    break;

  case 82:
#line 1147 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(1) - (3)].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[(3) - (3)].varSpec)); ;}
    break;

  case 83:
#line 1150 "gecode/flatzinc/parser.yxx"
    { (yyval.varSpecVec) = (yyvsp[(2) - (3)].varSpecVec); ;}
    break;

  case 84:
#line 1154 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); ;}
    break;

  case 85:
#line 1156 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[(2) - (2)].varSpecVec)); ;}
    break;

  case 86:
#line 1160 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); ;}
    break;

  case 87:
#line 1162 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[(2) - (2)].varSpecVec)); ;}
    break;

  case 88:
#line 1166 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); ;}
    break;

  case 89:
#line 1168 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[(2) - (2)].varSpecVec)); ;}
    break;

  case 90:
#line 1172 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); ;}
    break;

  case 91:
#line 1174 "gecode/flatzinc/parser.yxx"
    { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[(2) - (2)].varSpecVec)); ;}
    break;

  case 92:
#line 1178 "gecode/flatzinc/parser.yxx"
    { 
        ParserState *pp = static_cast<ParserState*>(parm);
        if (!pp->hadError) {
          ConExpr c((yyvsp[(2) - (6)].sValue), (yyvsp[(4) - (6)].argVec));
          try {
            pp->fg->postConstraint(c, (yyvsp[(6) - (6)].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        }
        delete (yyvsp[(6) - (6)].argVec); free((yyvsp[(2) - (6)].sValue));
      ;}
    break;

  case 93:
#line 1192 "gecode/flatzinc/parser.yxx"
    { 
        ParserState *pp = static_cast<ParserState*>(parm);
        if (!pp->hadError) {
          try {
            pp->fg->solve((yyvsp[(2) - (3)].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        } else {
          delete (yyvsp[(2) - (3)].argVec);
        }
      ;}
    break;

  case 94:
#line 1205 "gecode/flatzinc/parser.yxx"
    { 
        ParserState *pp = static_cast<ParserState*>(parm);
        if (!pp->hadError) {
          try {
            if ((yyvsp[(3) - (4)].bValue))
              pp->fg->minimize((yyvsp[(4) - (4)].iValue),(yyvsp[(2) - (4)].argVec));
            else
              pp->fg->maximize((yyvsp[(4) - (4)].iValue),(yyvsp[(2) - (4)].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        } else {
          delete (yyvsp[(2) - (4)].argVec);
        }
      ;}
    break;

  case 95:
#line 1227 "gecode/flatzinc/parser.yxx"
    { (yyval.oSet) = Option<AST::SetLit* >::none(); ;}
    break;

  case 96:
#line 1229 "gecode/flatzinc/parser.yxx"
    { (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit(*(yyvsp[(2) - (3)].setValue))); ;}
    break;

  case 97:
#line 1231 "gecode/flatzinc/parser.yxx"
    { 
        (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit((yyvsp[(1) - (3)].iValue), (yyvsp[(3) - (3)].iValue)));
      ;}
    break;

  case 98:
#line 1237 "gecode/flatzinc/parser.yxx"
    { (yyval.oSet) = Option<AST::SetLit* >::none(); ;}
    break;

  case 99:
#line 1239 "gecode/flatzinc/parser.yxx"
    { bool haveTrue = false;
        bool haveFalse = false;
        for (int i=(yyvsp[(2) - (4)].setValue)->size(); i--;) {
          haveTrue |= ((*(yyvsp[(2) - (4)].setValue))[i] == 1);
          haveFalse |= ((*(yyvsp[(2) - (4)].setValue))[i] == 0);
        }
        delete (yyvsp[(2) - (4)].setValue);
        (yyval.oSet) = Option<AST::SetLit* >::some(
          new AST::SetLit(!haveFalse,haveTrue));
      ;}
    break;

  case 102:
#line 1260 "gecode/flatzinc/parser.yxx"
    { (yyval.setLit) = new AST::SetLit(*(yyvsp[(2) - (3)].setValue)); ;}
    break;

  case 103:
#line 1262 "gecode/flatzinc/parser.yxx"
    { (yyval.setLit) = new AST::SetLit((yyvsp[(1) - (3)].iValue), (yyvsp[(3) - (3)].iValue)); ;}
    break;

  case 104:
#line 1268 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = new vector<int>(0); ;}
    break;

  case 105:
#line 1270 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = (yyvsp[(1) - (2)].setValue); ;}
    break;

  case 106:
#line 1274 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[(1) - (1)].iValue); ;}
    break;

  case 107:
#line 1276 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = (yyvsp[(1) - (3)].setValue); (yyval.setValue)->push_back((yyvsp[(3) - (3)].iValue)); ;}
    break;

  case 108:
#line 1280 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = new vector<int>(0); ;}
    break;

  case 109:
#line 1282 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = (yyvsp[(1) - (2)].setValue); ;}
    break;

  case 110:
#line 1286 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[(1) - (1)].iValue); ;}
    break;

  case 111:
#line 1288 "gecode/flatzinc/parser.yxx"
    { (yyval.setValue) = (yyvsp[(1) - (3)].setValue); (yyval.setValue)->push_back((yyvsp[(3) - (3)].iValue)); ;}
    break;

  case 112:
#line 1292 "gecode/flatzinc/parser.yxx"
    { (yyval.floatSetValue) = new vector<double>(0); ;}
    break;

  case 113:
#line 1294 "gecode/flatzinc/parser.yxx"
    { (yyval.floatSetValue) = (yyvsp[(1) - (2)].floatSetValue); ;}
    break;

  case 114:
#line 1298 "gecode/flatzinc/parser.yxx"
    { (yyval.floatSetValue) = new vector<double>(1); (*(yyval.floatSetValue))[0] = (yyvsp[(1) - (1)].dValue); ;}
    break;

  case 115:
#line 1300 "gecode/flatzinc/parser.yxx"
    { (yyval.floatSetValue) = (yyvsp[(1) - (3)].floatSetValue); (yyval.floatSetValue)->push_back((yyvsp[(3) - (3)].dValue)); ;}
    break;

  case 116:
#line 1304 "gecode/flatzinc/parser.yxx"
    { (yyval.setValueList) = new vector<AST::SetLit>(0); ;}
    break;

  case 117:
#line 1306 "gecode/flatzinc/parser.yxx"
    { (yyval.setValueList) = (yyvsp[(1) - (2)].setValueList); ;}
    break;

  case 118:
#line 1310 "gecode/flatzinc/parser.yxx"
    { (yyval.setValueList) = new vector<AST::SetLit>(1); (*(yyval.setValueList))[0] = *(yyvsp[(1) - (1)].setLit); delete (yyvsp[(1) - (1)].setLit); ;}
    break;

  case 119:
#line 1312 "gecode/flatzinc/parser.yxx"
    { (yyval.setValueList) = (yyvsp[(1) - (3)].setValueList); (yyval.setValueList)->push_back(*(yyvsp[(3) - (3)].setLit)); delete (yyvsp[(3) - (3)].setLit); ;}
    break;

  case 120:
#line 1320 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = new AST::Array((yyvsp[(1) - (1)].arg)); ;}
    break;

  case 121:
#line 1322 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = (yyvsp[(1) - (3)].argVec); (yyval.argVec)->append((yyvsp[(3) - (3)].arg)); ;}
    break;

  case 122:
#line 1326 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (1)].arg); ;}
    break;

  case 123:
#line 1328 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(2) - (3)].argVec); ;}
    break;

  case 124:
#line 1332 "gecode/flatzinc/parser.yxx"
    { (yyval.oArg) = Option<AST::Node*>::none(); ;}
    break;

  case 125:
#line 1334 "gecode/flatzinc/parser.yxx"
    { (yyval.oArg) = Option<AST::Node*>::some((yyvsp[(2) - (2)].arg)); ;}
    break;

  case 126:
#line 1338 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::BoolLit((yyvsp[(1) - (1)].iValue)); ;}
    break;

  case 127:
#line 1340 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::IntLit((yyvsp[(1) - (1)].iValue)); ;}
    break;

  case 128:
#line 1342 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::FloatLit((yyvsp[(1) - (1)].dValue)); ;}
    break;

  case 129:
#line 1344 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (1)].setLit); ;}
    break;

  case 130:
#line 1346 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e)) {
          switch (e.t) {
          case ST_INTVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::IntVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_BOOLVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::BoolVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_SETVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::SetVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_INTVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::IntLit(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_BOOLVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::BoolLit(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_SETVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              int idx = pp->arrays[e.i+1];
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::SetLit(pp->setvals[idx+i]);
              (yyval.arg) = v;
            }
            break;
          case ST_INT:
            (yyval.arg) = new AST::IntLit(e.i);
            break;
          case ST_BOOL:
            (yyval.arg) = new AST::BoolLit(e.i);
            break;
          case ST_SET:
            (yyval.arg) = new AST::SetLit(pp->setvals[e.i]);
            break;
          default:
            (yyval.arg) = getVarRefArg(pp,(yyvsp[(1) - (1)].sValue));
          }
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 131:
#line 1416 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        int i = -1;
        yyassert(pp, (yyvsp[(3) - (4)].arg)->isInt(i), "Non-integer array index");
        if (!pp->hadError)
          (yyval.arg) = getArrayElement(static_cast<ParserState*>(parm),(yyvsp[(1) - (4)].sValue),i);
        else
          (yyval.arg) = new AST::IntLit(0); // keep things consistent
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 132:
#line 1429 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = new AST::Array(0); ;}
    break;

  case 133:
#line 1431 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = (yyvsp[(1) - (2)].argVec); ;}
    break;

  case 134:
#line 1435 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = new AST::Array((yyvsp[(1) - (1)].arg)); ;}
    break;

  case 135:
#line 1437 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = (yyvsp[(1) - (3)].argVec); (yyval.argVec)->append((yyvsp[(3) - (3)].arg)); ;}
    break;

  case 136:
#line 1445 "gecode/flatzinc/parser.yxx"
    { 
        ParserState *pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue),e) && e.t == ST_INTVAR) {
          (yyval.iValue) = e.i;
        } else {
          pp->err << "Error: unknown integer variable " << (yyvsp[(1) - (1)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        }
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 137:
#line 1459 "gecode/flatzinc/parser.yxx"
    {
        SymbolEntry e;
        ParserState *pp = static_cast<ParserState*>(parm);
        if ( (!pp->symbols.get((yyvsp[(1) - (4)].sValue), e)) || e.t != ST_INTVARARRAY) {
          pp->err << "Error: unknown integer variable array " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        }
        if ((yyvsp[(3) - (4)].iValue) == 0 || (yyvsp[(3) - (4)].iValue) > pp->arrays[e.i]) {
          pp->err << "Error: array index out of bounds for array " << (yyvsp[(1) - (4)].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        } else {
          (yyval.iValue) = pp->arrays[e.i+(yyvsp[(3) - (4)].iValue)];
        }
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 140:
#line 1489 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = NULL; ;}
    break;

  case 141:
#line 1491 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = (yyvsp[(1) - (1)].argVec); ;}
    break;

  case 142:
#line 1495 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = new AST::Array((yyvsp[(2) - (2)].arg)); ;}
    break;

  case 143:
#line 1497 "gecode/flatzinc/parser.yxx"
    { (yyval.argVec) = (yyvsp[(1) - (3)].argVec); (yyval.argVec)->append((yyvsp[(3) - (3)].arg)); ;}
    break;

  case 144:
#line 1501 "gecode/flatzinc/parser.yxx"
    { 
        (yyval.arg) = new AST::Call((yyvsp[(1) - (4)].sValue), AST::extractSingleton((yyvsp[(3) - (4)].arg))); free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 145:
#line 1505 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (1)].arg); ;}
    break;

  case 146:
#line 1509 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::Array((yyvsp[(1) - (1)].arg)); ;}
    break;

  case 147:
#line 1511 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (3)].arg); (yyval.arg)->append((yyvsp[(3) - (3)].arg)); ;}
    break;

  case 148:
#line 1515 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (1)].arg); ;}
    break;

  case 149:
#line 1517 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(2) - (3)].arg); ;}
    break;

  case 150:
#line 1521 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::BoolLit((yyvsp[(1) - (1)].iValue)); ;}
    break;

  case 151:
#line 1523 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::IntLit((yyvsp[(1) - (1)].iValue)); ;}
    break;

  case 152:
#line 1525 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = new AST::FloatLit((yyvsp[(1) - (1)].dValue)); ;}
    break;

  case 153:
#line 1527 "gecode/flatzinc/parser.yxx"
    { (yyval.arg) = (yyvsp[(1) - (1)].setLit); ;}
    break;

  case 154:
#line 1529 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        bool gotSymbol = false;
        if (pp->symbols.get((yyvsp[(1) - (1)].sValue), e)) {
          gotSymbol = true;
          switch (e.t) {
          case ST_INTVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::IntVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_BOOLVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::BoolVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_SETVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::SetVar(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_INTVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::IntLit(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_BOOLVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::BoolLit(pp->arrays[e.i+i+1]);
              (yyval.arg) = v;
            }
            break;
          case ST_SETVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              int idx = pp->arrays[e.i+1];
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::SetLit(pp->setvals[idx+i]);
              (yyval.arg) = v;
            }
            break;
          case ST_INT:
            (yyval.arg) = new AST::IntLit(e.i);
            break;
          case ST_BOOL:
            (yyval.arg) = new AST::BoolLit(e.i);
            break;
          case ST_SET:
            (yyval.arg) = new AST::SetLit(pp->setvals[e.i]);
            break;
          default:
            gotSymbol = false;
          }
        }
        if (!gotSymbol)
          (yyval.arg) = getVarRefArg(pp,(yyvsp[(1) - (1)].sValue),true);
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;

  case 155:
#line 1603 "gecode/flatzinc/parser.yxx"
    { 
        ParserState* pp = static_cast<ParserState*>(parm);
        int i = -1;
        yyassert(pp, (yyvsp[(3) - (4)].arg)->isInt(i), "Non-integer array index");
        if (!pp->hadError)
          (yyval.arg) = getArrayElement(static_cast<ParserState*>(parm),(yyvsp[(1) - (4)].sValue),i);
        else
          (yyval.arg) = new AST::IntLit(0); // keep things consistent
        free((yyvsp[(1) - (4)].sValue));
      ;}
    break;

  case 156:
#line 1614 "gecode/flatzinc/parser.yxx"
    {
        (yyval.arg) = new AST::String((yyvsp[(1) - (1)].sValue));
        free((yyvsp[(1) - (1)].sValue));
      ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3373 "gecode/flatzinc/parser.tab.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (parm, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (parm, yymsg);
	  }
	else
	  {
	    yyerror (parm, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, parm);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, parm);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (parm, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, parm);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, parm);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



