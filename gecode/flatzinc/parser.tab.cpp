/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 37 "./gecode/flatzinc/parser.yxx"

#define YYPARSE_PARAM parm
#define YYLEX_PARAM static_cast<ParserState*>(parm)->yyscanner
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/parser.hh>
#include <gecode/flatzinc/lastval.hh>
#include <gecode/flatzinc/complete.hh>
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

AST::Node* getArrayElement(ParserState* pp, string id, int offset,
                           bool annotation) {
  if (offset > 0) {
    SymbolEntry e;
    if (pp->symbols.get(id,e)) {
      switch (e.t) {
      case ST_INTVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        {
          std::string n;
          if (annotation) {
            std::ostringstream oss;
            oss << id << "[" << offset << "]";
            n = oss.str();
          }
          return new AST::IntVar(pp->arrays[e.i+offset],n);
        }
      case ST_BOOLVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        {
          std::string n;
          if (annotation) {
            std::ostringstream oss;
            oss << id << "[" << offset << "]";
            n = oss.str();
          }
          return new AST::BoolVar(pp->arrays[e.i+offset],n);
        }
      case ST_SETVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        {
          std::string n;
          if (annotation) {
            std::ostringstream oss;
            oss << id << "[" << offset << "]";
            n = oss.str();
          }
          return new AST::SetVar(pp->arrays[e.i+offset],n);
        }
      case ST_FLOATVARARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        {
          std::string n;
          if (annotation) {
            std::ostringstream oss;
            oss << id << "[" << offset << "]";
            n = oss.str();
          }
          return new AST::FloatVar(pp->arrays[e.i+offset],n);
        }
      case ST_INTVALARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::IntLit(pp->arrays[e.i+offset]);
      case ST_SETVALARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::SetLit(pp->setvals[pp->arrays[e.i+1]+offset-1]);
      case ST_FLOATVALARRAY:
        if (offset > pp->arrays[e.i])
          goto error;
        return new AST::FloatLit(pp->floatvals[pp->arrays[e.i+1]+offset-1]);
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
  string n;
  if (annotation)
    n = id;
  if (pp->symbols.get(id, e)) {
    switch (e.t) {
    case ST_INTVAR: return new AST::IntVar(e.i,n);
    case ST_BOOLVAR: return new AST::BoolVar(e.i,n);
    case ST_SETVAR: return new AST::SetVar(e.i,n);
    case ST_FLOATVAR: return new AST::FloatVar(e.i,n);
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
  pp->domainConstraints.push_back(new ConExpr(id, args, NULL));
}

void addDomainConstraint(ParserState* pp, AST::Node* var,
                         Option<std::pair<double,double>* > dom) {
  if (!dom())
    return;
  {
    AST::Array* args = new AST::Array(2);
    args->a[0] = new AST::FloatLit(dom.some()->first);
    args->a[1] = var;
    pp->domainConstraints.push_back(new ConExpr("float_le", args, NULL));
  }
  {
    AST::Array* args = new AST::Array(2);
    AST::FloatVar* fv = static_cast<AST::FloatVar*>(var);
    args->a[0] = new AST::FloatVar(fv->i,fv->n);
    args->a[1] = new AST::FloatLit(dom.some()->second);
    pp->domainConstraints.push_back(new ConExpr("float_le", args, NULL));
  }
  delete dom.some();
}

int getBaseIntVar(ParserState* pp, int i) {
  int base = i;
  IntVarSpec* ivs = static_cast<IntVarSpec*>(pp->intvars[base].second);
  while (ivs->alias) {
    base = ivs->i;
    ivs = static_cast<IntVarSpec*>(pp->intvars[base].second);
  }
  return base;
}
int getBaseBoolVar(ParserState* pp, int i) {
  int base = i;
  BoolVarSpec* ivs = static_cast<BoolVarSpec*>(pp->boolvars[base].second);
  while (ivs->alias) {
    base = ivs->i;
    ivs = static_cast<BoolVarSpec*>(pp->boolvars[base].second);
  }
  return base;
}
int getBaseFloatVar(ParserState* pp, int i) {
  int base = i;
  FloatVarSpec* ivs = static_cast<FloatVarSpec*>(pp->floatvars[base].second);
  while (ivs->alias) {
    base = ivs->i;
    ivs = static_cast<FloatVarSpec*>(pp->floatvars[base].second);
  }
  return base;
}
int getBaseSetVar(ParserState* pp, int i) {
  int base = i;
  SetVarSpec* ivs = static_cast<SetVarSpec*>(pp->setvars[base].second);
  while (ivs->alias) {
    base = ivs->i;
    ivs = static_cast<SetVarSpec*>(pp->setvars[base].second);
  }
  return base;
}

/*
 * Initialize the root gecode space
 *
 */

void initfg(ParserState* pp) {
  if (!pp->hadError)
    pp->fg->init(pp->intvars.size(),
                 pp->boolvars.size(),
                 pp->setvars.size(),
                 pp->floatvars.size());

  for (unsigned int i=0; i<pp->intvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newIntVar(static_cast<IntVarSpec*>(pp->intvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }
    delete pp->intvars[i].second;
    pp->intvars[i].second = NULL;
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
    delete pp->boolvars[i].second;
    pp->boolvars[i].second = NULL;
  }
  for (unsigned int i=0; i<pp->setvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newSetVar(static_cast<SetVarSpec*>(pp->setvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }
    delete pp->setvars[i].second;
    pp->setvars[i].second = NULL;
  }
  for (unsigned int i=0; i<pp->floatvars.size(); i++) {
    if (!pp->hadError) {
      try {
        pp->fg->newFloatVar(
          static_cast<FloatVarSpec*>(pp->floatvars[i].second));
      } catch (Gecode::FlatZinc::Error& e) {
        yyerror(pp, e.toString().c_str());
      }
    }
    delete pp->floatvars[i].second;
    pp->floatvars[i].second = NULL;
  }
  if (!pp->hadError) {
    int iv_size = pp->sol_int.size() * 2;
    iv_size += pp->last_val_int.size();
    iv_size += pp->uniform_int.size();
    iv_size += (pp->status_idx >= 0) ? 1 : 0;

    if (iv_size > 0) {
      pp->fg->restart_data.init();
      pp->fg->on_restart_iv = IntVarArray(*(pp->fg), iv_size);
      int base = 0;

      pp->fg->restart_data().on_restart_iv_sol = pp->sol_int.size();
      for (size_t i = 0; i < pp->sol_int.size(); ++i) {
        pp->fg->on_restart_iv[base + i] = pp->fg->iv[(pp->sol_int[i][0])];
        pp->fg->on_restart_iv[base + pp->sol_int.size() + i] = pp->fg->iv[(pp->sol_int[i][1])];
      }
      base += pp->sol_int.size() * 2;

      pp->fg->restart_data().last_val_int = std::vector<int>(pp->last_val_int.size());
      IntVarArgs last_val_args;
      for (size_t i = 0; i < pp->last_val_int.size(); ++i) {
        pp->fg->on_restart_iv[base + i] = pp->fg->iv[(pp->last_val_int[i][1])];
        last_val_args << pp->fg->iv[(pp->last_val_int[i][0])];
      }
      LastValInt::post(*(pp->fg), last_val_args);
      base += pp->last_val_int.size();

      pp->fg->restart_data().uniform_range_int = std::vector<std::pair<int, int>>(pp->uniform_int.size());
      for (size_t i = 0; i < pp->uniform_int.size(); ++i) {
        pp->fg->restart_data().uniform_range_int[i] = std::pair<int, int>(pp->uniform_int[i][0], pp->uniform_int[i][1]);
        pp->fg->on_restart_iv[base + i] = pp->fg->iv[pp->uniform_int[i][2]];
      }
      base += pp->uniform_int.size();

      if (pp->status_idx >= 0) {
        pp->fg->restart_data().on_restart_status = true;
        pp->fg->on_restart_iv[base] = pp->fg->iv[pp->status_idx];
        base += 1;
      }
      assert(base == iv_size);
    }
  }
  if (!pp->hadError) {
    int bv_size = pp->sol_bool.size() * 2;
    bv_size += pp->last_val_bool.size();

    if (bv_size > 0) {
      pp->fg->restart_data.init();
      pp->fg->on_restart_bv = BoolVarArray(*(pp->fg), bv_size);
      int base = 0;

      pp->fg->restart_data().on_restart_bv_sol = pp->sol_bool.size();
      for (size_t i = 0; i < pp->sol_bool.size(); ++i) {
        pp->fg->on_restart_bv[base + i] = pp->fg->bv[(pp->sol_bool[i][0])];
        pp->fg->on_restart_bv[base + pp->sol_bool.size() + i] = pp->fg->bv[(pp->sol_bool[i][1])];
      }
      base += pp->sol_bool.size() * 2;

      pp->fg->restart_data().last_val_bool = std::vector<bool>(pp->last_val_bool.size());
      BoolVarArgs last_val_args;
      for (size_t i = 0; i < pp->last_val_bool.size(); ++i) {
        pp->fg->on_restart_bv[base + i] = pp->fg->bv[(pp->last_val_bool[i][1])];
        last_val_args << pp->fg->bv[(pp->last_val_bool[i][0])];
      }
      LastValBool::post(*(pp->fg), last_val_args);
      base += pp->last_val_bool.size();
      assert(base == bv_size);
    }
    if (pp->complete_idx >= 0) {
      Complete::post(*(pp->fg), pp->fg->bv[pp->complete_idx]);
    }
  }
#ifdef GECODE_HAS_SET_VARS
  if (!pp->hadError) {
    int sv_size = pp->sol_set.size() * 2;
    sv_size += pp->last_val_set.size();

    if (sv_size > 0) {
      pp->fg->restart_data.init();
      pp->fg->on_restart_sv = SetVarArray(*(pp->fg), sv_size);
      int base = 0;
      
      pp->fg->restart_data().on_restart_sv_sol = pp->sol_set.size();
      for (size_t i = 0; i < pp->sol_set.size(); ++i) {
        pp->fg->on_restart_sv[base + i] = pp->fg->sv[(pp->sol_set[i][0])];
        pp->fg->on_restart_sv[base + pp->sol_set.size() + i] = pp->fg->sv[(pp->sol_set[i][1])];
      }
      base += pp->sol_set.size() * 2;

      pp->fg->restart_data().last_val_set = std::vector<IntSet>(pp->last_val_set.size());
      SetVarArgs last_val_args;
      for (size_t i = 0; i < pp->last_val_set.size(); ++i) {
        pp->fg->on_restart_sv[base + i] = pp->fg->sv[(pp->last_val_set[i][1])];
        last_val_args << pp->fg->sv[(pp->last_val_set[i][0])];
      }
      LastValSet::post(*(pp->fg), last_val_args);
      base += pp->last_val_set.size();
      assert(base == sv_size);
    }
  }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
  if (!pp->hadError) {
    int fv_size = pp->sol_float.size() * 2;
    fv_size += pp->last_val_float.size();
    fv_size += pp->uniform_float.size();

    if (fv_size > 0) {
      pp->fg->restart_data.init();
      pp->fg->on_restart_fv = FloatVarArray(*(pp->fg), fv_size);
      int base = 0;

      pp->fg->restart_data().on_restart_fv_sol =  pp->sol_float.size();
      for (size_t i = 0; i < pp->sol_float.size(); ++i) {
        pp->fg->on_restart_fv[base + i] = pp->fg->fv[(pp->sol_float[i][0])];
        pp->fg->on_restart_fv[base + pp->sol_float.size() + i] = pp->fg->fv[(pp->sol_float[i][1])];
      }
      base += pp->sol_float.size() * 2;
      
      pp->fg->restart_data().last_val_float = std::vector<FloatVal>(pp->last_val_float.size());
      FloatVarArgs last_val_args;
      for (size_t i = 0; i < pp->last_val_float.size(); ++i) {
        pp->fg->on_restart_fv[base + i] = pp->fg->fv[(pp->last_val_float[i][1])];
        last_val_args << pp->fg->fv[(pp->last_val_float[i][0])];
      }
      LastValFloat::post(*(pp->fg), last_val_args);
      base += pp->last_val_float.size();

      pp->fg->restart_data().uniform_range_float = std::vector<std::pair<FloatVal, FloatVal>>(pp->uniform_float.size());
      for (size_t i = 0; i < pp->uniform_float.size(); ++i) {
        pp->fg->restart_data().uniform_range_float[i] = std::pair<FloatVal, FloatVal>(std::get<0>(pp->uniform_float[i]), std::get<1>(pp->uniform_float[i]));
        pp->fg->on_restart_fv[base + i] = pp->fg->fv[std::get<2>(pp->uniform_float[i])];
      }
      base += pp->uniform_float.size();
      assert(base == fv_size);
    }
  }
#endif
  if (!pp->hadError) {
    pp->fg->postConstraints(pp->domainConstraints);
    pp->fg->postConstraints(pp->constraints);
  }
}

void fillPrinter(ParserState& pp, Gecode::FlatZinc::Printer& p) {
  p.init(pp.getOutput());
  for (unsigned int i=0; i<pp.intvars.size(); i++) {
    if (!pp.hadError) {
      p.addIntVarName(pp.intvars[i].first);
    }
  }
  for (unsigned int i=0; i<pp.boolvars.size(); i++) {
    if (!pp.hadError) {
      p.addBoolVarName(pp.boolvars[i].first);
    }
  }
#ifdef GECODE_HAS_FLOAT_VARS
  for (unsigned int i=0; i<pp.floatvars.size(); i++) {
    if (!pp.hadError) {
      p.addFloatVarName(pp.floatvars[i].first);
    }
  }
#endif
#ifdef GECODE_HAS_SET_VARS
  for (unsigned int i=0; i<pp.setvars.size(); i++) {
    if (!pp.hadError) {
      p.addSetVarName(pp.setvars[i].first);
    }
  }
#endif
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
                       FlatZincSpace* fzs, Rnd& rnd) {
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
      fzs = new FlatZincSpace(rnd);
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
      fzs = new FlatZincSpace(rnd);
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
                       FlatZincSpace* fzs, Rnd& rnd) {
    std::string s = string(istreambuf_iterator<char>(is),
                           istreambuf_iterator<char>());

    if (fzs == NULL) {
      fzs = new FlatZincSpace(rnd);
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


#line 644 "gecode/flatzinc/parser.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_FZ_INT_LIT = 3,                 /* FZ_INT_LIT  */
  YYSYMBOL_FZ_BOOL_LIT = 4,                /* FZ_BOOL_LIT  */
  YYSYMBOL_FZ_FLOAT_LIT = 5,               /* FZ_FLOAT_LIT  */
  YYSYMBOL_FZ_ID = 6,                      /* FZ_ID  */
  YYSYMBOL_FZ_U_ID = 7,                    /* FZ_U_ID  */
  YYSYMBOL_FZ_STRING_LIT = 8,              /* FZ_STRING_LIT  */
  YYSYMBOL_FZ_VAR = 9,                     /* FZ_VAR  */
  YYSYMBOL_FZ_PAR = 10,                    /* FZ_PAR  */
  YYSYMBOL_FZ_ANNOTATION = 11,             /* FZ_ANNOTATION  */
  YYSYMBOL_FZ_ANY = 12,                    /* FZ_ANY  */
  YYSYMBOL_FZ_ARRAY = 13,                  /* FZ_ARRAY  */
  YYSYMBOL_FZ_BOOL = 14,                   /* FZ_BOOL  */
  YYSYMBOL_FZ_CASE = 15,                   /* FZ_CASE  */
  YYSYMBOL_FZ_COLONCOLON = 16,             /* FZ_COLONCOLON  */
  YYSYMBOL_FZ_CONSTRAINT = 17,             /* FZ_CONSTRAINT  */
  YYSYMBOL_FZ_DEFAULT = 18,                /* FZ_DEFAULT  */
  YYSYMBOL_FZ_DOTDOT = 19,                 /* FZ_DOTDOT  */
  YYSYMBOL_FZ_ELSE = 20,                   /* FZ_ELSE  */
  YYSYMBOL_FZ_ELSEIF = 21,                 /* FZ_ELSEIF  */
  YYSYMBOL_FZ_ENDIF = 22,                  /* FZ_ENDIF  */
  YYSYMBOL_FZ_ENUM = 23,                   /* FZ_ENUM  */
  YYSYMBOL_FZ_FLOAT = 24,                  /* FZ_FLOAT  */
  YYSYMBOL_FZ_FUNCTION = 25,               /* FZ_FUNCTION  */
  YYSYMBOL_FZ_IF = 26,                     /* FZ_IF  */
  YYSYMBOL_FZ_INCLUDE = 27,                /* FZ_INCLUDE  */
  YYSYMBOL_FZ_INT = 28,                    /* FZ_INT  */
  YYSYMBOL_FZ_LET = 29,                    /* FZ_LET  */
  YYSYMBOL_FZ_MAXIMIZE = 30,               /* FZ_MAXIMIZE  */
  YYSYMBOL_FZ_MINIMIZE = 31,               /* FZ_MINIMIZE  */
  YYSYMBOL_FZ_OF = 32,                     /* FZ_OF  */
  YYSYMBOL_FZ_SATISFY = 33,                /* FZ_SATISFY  */
  YYSYMBOL_FZ_OUTPUT = 34,                 /* FZ_OUTPUT  */
  YYSYMBOL_FZ_PREDICATE = 35,              /* FZ_PREDICATE  */
  YYSYMBOL_FZ_RECORD = 36,                 /* FZ_RECORD  */
  YYSYMBOL_FZ_SET = 37,                    /* FZ_SET  */
  YYSYMBOL_FZ_SHOW = 38,                   /* FZ_SHOW  */
  YYSYMBOL_FZ_SHOWCOND = 39,               /* FZ_SHOWCOND  */
  YYSYMBOL_FZ_SOLVE = 40,                  /* FZ_SOLVE  */
  YYSYMBOL_FZ_STRING = 41,                 /* FZ_STRING  */
  YYSYMBOL_FZ_TEST = 42,                   /* FZ_TEST  */
  YYSYMBOL_FZ_THEN = 43,                   /* FZ_THEN  */
  YYSYMBOL_FZ_TUPLE = 44,                  /* FZ_TUPLE  */
  YYSYMBOL_FZ_TYPE = 45,                   /* FZ_TYPE  */
  YYSYMBOL_FZ_VARIANT_RECORD = 46,         /* FZ_VARIANT_RECORD  */
  YYSYMBOL_FZ_WHERE = 47,                  /* FZ_WHERE  */
  YYSYMBOL_48_ = 48,                       /* ';'  */
  YYSYMBOL_49_ = 49,                       /* '('  */
  YYSYMBOL_50_ = 50,                       /* ')'  */
  YYSYMBOL_51_ = 51,                       /* ','  */
  YYSYMBOL_52_ = 52,                       /* ':'  */
  YYSYMBOL_53_ = 53,                       /* '['  */
  YYSYMBOL_54_ = 54,                       /* ']'  */
  YYSYMBOL_55_ = 55,                       /* '='  */
  YYSYMBOL_56_ = 56,                       /* '{'  */
  YYSYMBOL_57_ = 57,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 58,                  /* $accept  */
  YYSYMBOL_model = 59,                     /* model  */
  YYSYMBOL_preddecl_items = 60,            /* preddecl_items  */
  YYSYMBOL_preddecl_items_head = 61,       /* preddecl_items_head  */
  YYSYMBOL_vardecl_items = 62,             /* vardecl_items  */
  YYSYMBOL_vardecl_items_head = 63,        /* vardecl_items_head  */
  YYSYMBOL_constraint_items = 64,          /* constraint_items  */
  YYSYMBOL_constraint_items_head = 65,     /* constraint_items_head  */
  YYSYMBOL_preddecl_item = 66,             /* preddecl_item  */
  YYSYMBOL_pred_arg_list = 67,             /* pred_arg_list  */
  YYSYMBOL_pred_arg_list_head = 68,        /* pred_arg_list_head  */
  YYSYMBOL_pred_arg = 69,                  /* pred_arg  */
  YYSYMBOL_pred_arg_type = 70,             /* pred_arg_type  */
  YYSYMBOL_pred_arg_simple_type = 71,      /* pred_arg_simple_type  */
  YYSYMBOL_pred_array_init = 72,           /* pred_array_init  */
  YYSYMBOL_pred_array_init_arg = 73,       /* pred_array_init_arg  */
  YYSYMBOL_var_par_id = 74,                /* var_par_id  */
  YYSYMBOL_vardecl_item = 75,              /* vardecl_item  */
  YYSYMBOL_int_init = 76,                  /* int_init  */
  YYSYMBOL_int_init_list = 77,             /* int_init_list  */
  YYSYMBOL_int_init_list_head = 78,        /* int_init_list_head  */
  YYSYMBOL_list_tail = 79,                 /* list_tail  */
  YYSYMBOL_int_var_array_literal = 80,     /* int_var_array_literal  */
  YYSYMBOL_float_init = 81,                /* float_init  */
  YYSYMBOL_float_init_list = 82,           /* float_init_list  */
  YYSYMBOL_float_init_list_head = 83,      /* float_init_list_head  */
  YYSYMBOL_float_var_array_literal = 84,   /* float_var_array_literal  */
  YYSYMBOL_bool_init = 85,                 /* bool_init  */
  YYSYMBOL_bool_init_list = 86,            /* bool_init_list  */
  YYSYMBOL_bool_init_list_head = 87,       /* bool_init_list_head  */
  YYSYMBOL_bool_var_array_literal = 88,    /* bool_var_array_literal  */
  YYSYMBOL_set_init = 89,                  /* set_init  */
  YYSYMBOL_set_init_list = 90,             /* set_init_list  */
  YYSYMBOL_set_init_list_head = 91,        /* set_init_list_head  */
  YYSYMBOL_set_var_array_literal = 92,     /* set_var_array_literal  */
  YYSYMBOL_vardecl_int_var_array_init = 93, /* vardecl_int_var_array_init  */
  YYSYMBOL_vardecl_bool_var_array_init = 94, /* vardecl_bool_var_array_init  */
  YYSYMBOL_vardecl_float_var_array_init = 95, /* vardecl_float_var_array_init  */
  YYSYMBOL_vardecl_set_var_array_init = 96, /* vardecl_set_var_array_init  */
  YYSYMBOL_constraint_item = 97,           /* constraint_item  */
  YYSYMBOL_solve_item = 98,                /* solve_item  */
  YYSYMBOL_int_ti_expr_tail = 99,          /* int_ti_expr_tail  */
  YYSYMBOL_bool_ti_expr_tail = 100,        /* bool_ti_expr_tail  */
  YYSYMBOL_float_ti_expr_tail = 101,       /* float_ti_expr_tail  */
  YYSYMBOL_set_literal = 102,              /* set_literal  */
  YYSYMBOL_int_list = 103,                 /* int_list  */
  YYSYMBOL_int_list_head = 104,            /* int_list_head  */
  YYSYMBOL_bool_list = 105,                /* bool_list  */
  YYSYMBOL_bool_list_head = 106,           /* bool_list_head  */
  YYSYMBOL_float_list = 107,               /* float_list  */
  YYSYMBOL_float_list_head = 108,          /* float_list_head  */
  YYSYMBOL_set_literal_list = 109,         /* set_literal_list  */
  YYSYMBOL_set_literal_list_head = 110,    /* set_literal_list_head  */
  YYSYMBOL_flat_expr_list = 111,           /* flat_expr_list  */
  YYSYMBOL_flat_expr = 112,                /* flat_expr  */
  YYSYMBOL_non_array_expr_opt = 113,       /* non_array_expr_opt  */
  YYSYMBOL_non_array_expr = 114,           /* non_array_expr  */
  YYSYMBOL_non_array_expr_list = 115,      /* non_array_expr_list  */
  YYSYMBOL_non_array_expr_list_head = 116, /* non_array_expr_list_head  */
  YYSYMBOL_solve_expr = 117,               /* solve_expr  */
  YYSYMBOL_minmax = 118,                   /* minmax  */
  YYSYMBOL_annotations = 119,              /* annotations  */
  YYSYMBOL_annotations_head = 120,         /* annotations_head  */
  YYSYMBOL_annotation = 121,               /* annotation  */
  YYSYMBOL_annotation_list = 122,          /* annotation_list  */
  YYSYMBOL_annotation_expr = 123,          /* annotation_expr  */
  YYSYMBOL_annotation_list_tail = 124,     /* annotation_list_tail  */
  YYSYMBOL_ann_non_array_expr = 125        /* ann_non_array_expr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   360

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  162
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  347

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   302


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   712,   712,   714,   716,   719,   720,   722,   724,   727,
     728,   730,   732,   735,   736,   743,   746,   748,   751,   752,
     755,   759,   760,   761,   762,   765,   767,   769,   770,   773,
     774,   777,   778,   784,   784,   787,   819,   851,   890,   923,
     932,   942,   951,   963,  1033,  1099,  1170,  1238,  1259,  1279,
    1299,  1322,  1326,  1341,  1365,  1366,  1370,  1372,  1375,  1375,
    1377,  1381,  1383,  1398,  1421,  1422,  1426,  1428,  1432,  1436,
    1438,  1453,  1476,  1477,  1481,  1483,  1486,  1489,  1491,  1506,
    1529,  1530,  1534,  1536,  1539,  1544,  1545,  1550,  1551,  1556,
    1557,  1562,  1563,  1567,  1724,  1738,  1763,  1765,  1767,  1773,
    1775,  1788,  1790,  1799,  1801,  1808,  1809,  1813,  1815,  1820,
    1821,  1825,  1827,  1832,  1833,  1837,  1839,  1844,  1845,  1849,
    1851,  1859,  1861,  1865,  1867,  1872,  1873,  1877,  1879,  1881,
    1883,  1885,  1981,  1996,  1997,  2001,  2003,  2011,  2045,  2052,
    2059,  2085,  2086,  2094,  2095,  2099,  2101,  2105,  2109,  2113,
    2115,  2119,  2121,  2123,  2126,  2126,  2129,  2131,  2133,  2135,
    2137,  2243,  2254
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "FZ_INT_LIT",
  "FZ_BOOL_LIT", "FZ_FLOAT_LIT", "FZ_ID", "FZ_U_ID", "FZ_STRING_LIT",
  "FZ_VAR", "FZ_PAR", "FZ_ANNOTATION", "FZ_ANY", "FZ_ARRAY", "FZ_BOOL",
  "FZ_CASE", "FZ_COLONCOLON", "FZ_CONSTRAINT", "FZ_DEFAULT", "FZ_DOTDOT",
  "FZ_ELSE", "FZ_ELSEIF", "FZ_ENDIF", "FZ_ENUM", "FZ_FLOAT", "FZ_FUNCTION",
  "FZ_IF", "FZ_INCLUDE", "FZ_INT", "FZ_LET", "FZ_MAXIMIZE", "FZ_MINIMIZE",
  "FZ_OF", "FZ_SATISFY", "FZ_OUTPUT", "FZ_PREDICATE", "FZ_RECORD",
  "FZ_SET", "FZ_SHOW", "FZ_SHOWCOND", "FZ_SOLVE", "FZ_STRING", "FZ_TEST",
  "FZ_THEN", "FZ_TUPLE", "FZ_TYPE", "FZ_VARIANT_RECORD", "FZ_WHERE", "';'",
  "'('", "')'", "','", "':'", "'['", "']'", "'='", "'{'", "'}'", "$accept",
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
  "annotation_list_tail", "ann_non_array_expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-123)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -25,    13,    30,   253,   -25,   -20,   -13,  -123,   102,    -7,
       6,    18,    38,    87,   108,   253,    79,    81,  -123,    84,
     116,   118,  -123,  -123,  -123,   113,   111,    91,    95,   101,
     161,   126,   126,   126,   149,   173,   140,   108,   137,   138,
    -123,  -123,   217,   134,  -123,  -123,   157,   185,   142,   139,
    -123,   146,  -123,  -123,   188,   194,    78,  -123,  -123,   147,
     152,   154,   126,   126,   126,   189,  -123,  -123,   191,   191,
     191,   158,   160,   191,   163,   170,  -123,  -123,  -123,    56,
      78,  -123,    84,  -123,   211,  -123,  -123,   171,  -123,   218,
    -123,   220,   172,   191,   191,   191,   223,    35,   179,   216,
     181,   187,   126,   169,   119,  -123,  -123,   208,  -123,    28,
    -123,  -123,  -123,  -123,   126,  -123,  -123,  -123,   192,   192,
     192,   186,   224,  -123,  -123,   197,  -123,    46,   185,   196,
    -123,  -123,  -123,  -123,    57,    35,    57,    57,   191,   224,
    -123,  -123,    57,   200,  -123,   106,  -123,  -123,  -123,  -123,
    -123,   156,   255,    56,   227,   191,    57,  -123,  -123,  -123,
     228,   258,    35,  -123,  -123,   212,   214,    19,  -123,  -123,
    -123,  -123,   210,  -123,   221,   231,    57,   191,   169,  -123,
    -123,   225,  -123,  -123,  -123,   114,   192,  -123,    20,  -123,
     117,    35,   229,  -123,   232,    57,  -123,    57,  -123,   233,
    -123,  -123,   271,   217,  -123,  -123,   141,   236,   237,   239,
     247,  -123,    35,  -123,  -123,  -123,  -123,  -123,  -123,   238,
    -123,   261,   242,   243,   244,   126,   126,   126,   257,  -123,
      78,   126,   126,   126,   191,   191,   191,   245,   246,   191,
     191,   191,   248,   249,   250,   126,   126,   251,   254,   256,
     259,   260,   262,   191,   191,   263,  -123,   264,  -123,   265,
    -123,   295,   296,   185,   266,   267,    62,  -123,    88,  -123,
     177,  -123,   269,   154,  -123,   270,   268,   272,   274,   275,
    -123,  -123,   276,  -123,   277,   279,  -123,   280,  -123,   278,
     283,  -123,   282,  -123,   284,   285,  -123,  -123,  -123,   297,
    -123,  -123,    17,    11,  -123,   305,  -123,    62,  -123,   307,
    -123,    88,  -123,   311,  -123,   177,  -123,  -123,   224,  -123,
     286,   288,   289,  -123,   287,   292,  -123,   290,  -123,   291,
    -123,   293,  -123,  -123,    17,  -123,   317,  -123,    11,  -123,
    -123,  -123,  -123,  -123,   294,  -123,  -123
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     7,     4,     0,     0,     1,     0,     0,
       0,     0,     0,     0,    11,     8,     0,     0,     5,    16,
       0,     0,    99,   101,    96,     0,   105,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       9,     6,     0,     0,    27,    28,     0,   105,     0,    58,
      18,     0,    24,    25,     0,     0,     0,   107,   111,     0,
      58,    58,     0,     0,     0,     0,    33,    34,   143,   143,
     143,     0,     0,   143,     0,     0,    13,    10,    23,     0,
       0,    15,    59,    17,     0,    98,   102,     0,    97,    59,
     106,    59,     0,   143,   143,   143,     0,     0,     0,   144,
       0,     0,     0,     0,     0,     2,    14,     0,    31,     0,
      29,    26,    19,    20,     0,   108,   112,   100,   125,   125,
     125,     0,   157,   156,   158,    33,   162,     0,   105,   160,
     159,   145,   148,   151,     0,     0,     0,     0,   143,   128,
     127,   129,   133,   131,   130,     0,   121,   123,   142,   141,
      94,     0,     0,     0,     0,   143,     0,    35,    36,    37,
       0,     0,     0,   152,   149,   154,     0,     0,    41,   146,
      40,    39,     0,   135,     0,    58,     0,   143,     0,   138,
     139,   137,    95,    32,    30,     0,   125,   126,     0,   104,
       0,   155,     0,   103,     0,     0,   124,    59,   134,     0,
      93,   122,     0,     0,    21,    38,     0,     0,     0,     0,
       0,   147,     0,   150,   153,   161,    42,   136,   132,     0,
      22,     0,     0,     0,     0,     0,     0,     0,     0,   140,
       0,     0,     0,     0,   143,   143,   143,     0,     0,   143,
     143,   143,     0,     0,     0,     0,     0,    85,    87,    89,
       0,     0,     0,   143,   143,     0,    43,     0,    44,     0,
      45,   109,   113,   105,     0,    91,    54,    86,    72,    88,
      64,    90,     0,    58,   115,     0,    58,     0,     0,     0,
      46,    51,    52,    56,     0,    58,    69,    70,    74,     0,
      58,    61,    62,    66,     0,    58,    48,   110,    49,    59,
     114,    47,   117,    80,    92,     0,    60,    59,    55,     0,
      76,    59,    73,     0,    68,    59,    65,   116,     0,   119,
       0,    58,    78,    82,     0,    58,    77,     0,    57,     0,
      75,     0,    67,    50,    59,   118,     0,    84,    59,    81,
      53,    71,    63,   120,     0,    83,    79
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,   321,  -123,
    -123,   273,  -123,   -37,  -123,   184,   -31,   331,    42,  -123,
    -123,   -57,  -123,   -15,  -123,  -123,  -123,    39,  -123,  -123,
    -123,    14,  -123,  -123,  -123,  -123,  -123,  -123,  -123,   314,
    -123,     0,   148,   150,   -90,  -122,  -123,  -123,    92,  -123,
    -123,  -123,  -123,  -123,   180,  -108,  -121,  -123,  -123,  -123,
    -123,    16,  -123,   -88,   195,  -123,  -123,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     3,     4,    14,    15,    36,    37,     5,    48,
      49,    50,    51,    52,   109,   110,   143,    16,   283,   284,
     285,    83,   267,   293,   294,   295,   271,   288,   289,   290,
     269,   323,   324,   325,   304,   256,   258,   260,   280,    38,
      74,    53,    28,    29,   144,    59,    60,   272,    61,   275,
     276,   320,   321,   145,   146,   157,   147,   174,   175,   182,
     151,    98,    99,   164,   165,   132,   192,   133
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      68,    69,    70,    90,    92,    78,   166,   130,    27,   131,
       1,   158,   159,   168,   318,   170,   171,    66,    67,     6,
     318,   173,   122,   123,   124,    66,    67,   126,    18,   206,
       7,    93,    94,    95,   207,   187,    19,   130,   122,   123,
     124,   125,    67,   126,   208,   130,    30,   169,   209,   122,
     123,   124,   125,    67,   126,   199,    87,   210,    31,   107,
     139,   140,   141,    66,    67,   281,   129,   128,    66,    67,
      32,   138,   130,   128,   216,   128,   217,   130,   205,   153,
     111,    20,   154,   155,   108,   100,   101,    20,   127,   104,
      33,   128,   286,    42,    66,    67,   129,    43,    44,   127,
     163,   130,   128,   213,   129,    20,    24,    21,    45,   118,
     119,   120,    24,   128,    57,    58,    22,    20,   198,    34,
     181,    46,   130,   203,   213,    35,    23,    40,    44,    41,
      24,   129,    66,    67,    47,    54,   129,    55,    45,    25,
      47,   277,    24,    62,    20,    56,    21,    63,   204,   148,
     149,    46,   150,    64,   172,    22,   177,   178,    26,   179,
     129,   180,    66,    67,    65,    23,   220,   211,   212,    24,
      47,   186,   139,   140,   141,    66,    67,    71,   221,    72,
      73,   129,   291,    66,    67,    76,    77,    79,    57,    80,
      82,    85,    81,   200,   234,   235,   236,    26,    84,    86,
     239,   240,   241,    89,    88,    91,   222,    97,    96,   103,
     102,   105,   319,   326,   253,   254,   297,   113,   106,   300,
      20,   115,   142,   114,   116,   128,   121,   152,   308,   117,
     238,    44,   135,   312,   134,   282,   136,   287,   316,   292,
     160,    45,   137,   161,   343,    24,   162,   156,   326,   167,
     242,   243,   244,   176,    46,   247,   248,   249,   183,   185,
     188,   189,     8,   191,   335,   195,     9,    10,   339,   264,
     265,   193,   322,    47,   219,   196,   282,    11,   202,   228,
     287,    12,   197,   214,   292,   237,   215,   218,   225,   226,
      13,   227,   229,   230,   231,   232,   233,   245,   246,    58,
     332,   274,   317,   250,   251,   252,   255,   322,   327,   257,
     329,   259,   261,   262,   331,   263,   266,   268,   270,   299,
     344,   278,   279,   296,   298,    17,   301,   302,   303,   305,
     307,   306,   310,   309,   311,   313,   315,   184,   314,   334,
     333,   337,   336,   338,   340,   341,    39,   342,   346,   328,
     330,    75,   345,   273,   223,   112,   224,   190,   201,     0,
     194
};

static const yytype_int16 yycheck[] =
{
      31,    32,    33,    60,    61,    42,   128,    97,     8,    97,
      35,   119,   120,   134,     3,   136,   137,     6,     7,     6,
       3,   142,     3,     4,     5,     6,     7,     8,    48,     9,
       0,    62,    63,    64,    14,   156,    49,   127,     3,     4,
       5,     6,     7,     8,    24,   135,    53,   135,    28,     3,
       4,     5,     6,     7,     8,   176,    56,    37,    52,     3,
       3,     4,     5,     6,     7,     3,    97,    56,     6,     7,
      52,   102,   162,    56,   195,    56,   197,   167,   186,    51,
      80,     3,    54,   114,    28,    69,    70,     3,    53,    73,
      52,    56,     4,     9,     6,     7,   127,    13,    14,    53,
      54,   191,    56,   191,   135,     3,    28,     5,    24,    93,
      94,    95,    28,    56,     3,     4,    14,     3,   175,    32,
     151,    37,   212,     9,   212,    17,    24,    48,    14,    48,
      28,   162,     6,     7,    56,    19,   167,    19,    24,    37,
      56,   263,    28,    52,     3,    32,     5,    52,   185,    30,
      31,    37,    33,    52,   138,    14,    50,    51,    56,     3,
     191,     5,     6,     7,     3,    24,   203,    50,    51,    28,
      56,   155,     3,     4,     5,     6,     7,    28,    37,     6,
      40,   212,     5,     6,     7,    48,    48,    53,     3,    32,
      51,     3,    50,   177,   225,   226,   227,    56,    52,     5,
     231,   232,   233,    51,    57,    51,   206,    16,    19,    49,
      52,    48,   302,   303,   245,   246,   273,     6,    48,   276,
       3,     3,    53,    52,     4,    56,     3,    19,   285,    57,
     230,    14,    16,   290,    55,   266,    55,   268,   295,   270,
      54,    24,    55,    19,   334,    28,    49,    55,   338,    53,
     234,   235,   236,    53,    37,   239,   240,   241,     3,    32,
      32,     3,     9,    51,   321,    55,    13,    14,   325,   253,
     254,    57,   303,    56,     3,    54,   307,    24,    53,    32,
     311,    28,    51,    54,   315,    28,    54,    54,    52,    52,
      37,    52,    54,    32,    52,    52,    52,    52,    52,     4,
     315,     5,     5,    55,    55,    55,    55,   338,     3,    55,
       3,    55,    53,    53,     3,    53,    53,    53,    53,    51,
       3,    55,    55,    54,    54,     4,    54,    53,    53,    53,
      51,    54,    54,    53,    51,    53,    51,   153,    54,    51,
      54,    54,    53,    51,    54,    54,    15,    54,    54,   307,
     311,    37,   338,   261,   206,    82,   206,   162,   178,    -1,
     167
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    35,    59,    60,    61,    66,     6,     0,     9,    13,
      14,    24,    28,    37,    62,    63,    75,    66,    48,    49,
       3,     5,    14,    24,    28,    37,    56,    99,   100,   101,
      53,    52,    52,    52,    32,    17,    64,    65,    97,    75,
      48,    48,     9,    13,    14,    24,    37,    56,    67,    68,
      69,    70,    71,    99,    19,    19,    32,     3,     4,   103,
     104,   106,    52,    52,    52,     3,     6,     7,    74,    74,
      74,    28,     6,    40,    98,    97,    48,    48,    71,    53,
      32,    50,    51,    79,    52,     3,     5,    99,    57,    51,
      79,    51,    79,    74,    74,    74,    19,    16,   119,   120,
     119,   119,    52,    49,   119,    48,    48,     3,    28,    72,
      73,    99,    69,     6,    52,     3,     4,    57,   119,   119,
     119,     3,     3,     4,     5,     6,     8,    53,    56,    74,
     102,   121,   123,   125,    55,    16,    55,    55,    74,     3,
       4,     5,    53,    74,   102,   111,   112,   114,    30,    31,
      33,   118,    19,    51,    54,    74,    55,   113,   113,   113,
      54,    19,    49,    54,   121,   122,   103,    53,   114,   121,
     114,   114,   119,   114,   115,   116,    53,    50,    51,     3,
       5,    74,   117,     3,    73,    32,   119,   114,    32,     3,
     122,    51,   124,    57,   125,    55,    54,    51,    79,   114,
     119,   112,    53,     9,    71,   113,     9,    14,    24,    28,
      37,    50,    51,   121,    54,    54,   114,   114,    54,     3,
      71,    37,    99,   100,   101,    52,    52,    52,    32,    54,
      32,    52,    52,    52,    74,    74,    74,    28,    99,    74,
      74,    74,   119,   119,   119,    52,    52,   119,   119,   119,
      55,    55,    55,    74,    74,    55,    93,    55,    94,    55,
      95,    53,    53,    53,   119,   119,    53,    80,    53,    88,
      53,    84,   105,   106,     5,   107,   108,   103,    55,    55,
      96,     3,    74,    76,    77,    78,     4,    74,    85,    86,
      87,     5,    74,    81,    82,    83,    54,    79,    54,    51,
      79,    54,    53,    53,    92,    53,    54,    51,    79,    53,
      54,    51,    79,    53,    54,    51,    79,     5,     3,   102,
     109,   110,    74,    89,    90,    91,   102,     3,    76,     3,
      85,     3,    81,    54,    51,    79,    53,    54,    51,    79,
      54,    54,    54,   102,     3,    89,    54
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    58,    59,    60,    60,    61,    61,    62,    62,    63,
      63,    64,    64,    65,    65,    66,    67,    67,    68,    68,
      69,    70,    70,    70,    70,    71,    71,    71,    71,    72,
      72,    73,    73,    74,    74,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    76,    76,    76,    77,    77,    78,    78,    79,    79,
      80,    81,    81,    81,    82,    82,    83,    83,    84,    85,
      85,    85,    86,    86,    87,    87,    88,    89,    89,    89,
      90,    90,    91,    91,    92,    93,    93,    94,    94,    95,
      95,    96,    96,    97,    98,    98,    99,    99,    99,   100,
     100,   101,   101,   102,   102,   103,   103,   104,   104,   105,
     105,   106,   106,   107,   107,   108,   108,   109,   109,   110,
     110,   111,   111,   112,   112,   113,   113,   114,   114,   114,
     114,   114,   114,   115,   115,   116,   116,   117,   117,   117,
     117,   118,   118,   119,   119,   120,   120,   121,   121,   122,
     122,   123,   123,   123,   124,   124,   125,   125,   125,   125,
     125,   125,   125
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     5,     0,     1,     2,     3,     0,     1,     2,
       3,     0,     1,     2,     3,     5,     0,     2,     1,     3,
       3,     6,     7,     2,     1,     1,     3,     1,     1,     1,
       3,     1,     3,     1,     1,     6,     6,     6,     8,     6,
       6,     6,     8,    13,    13,    13,    15,    15,    15,    15,
      17,     1,     1,     4,     0,     2,     1,     3,     0,     1,
       3,     1,     1,     4,     0,     2,     1,     3,     3,     1,
       1,     4,     0,     2,     1,     3,     3,     1,     1,     4,
       0,     2,     1,     3,     3,     0,     2,     0,     2,     0,
       2,     0,     2,     6,     3,     4,     1,     3,     3,     1,
       4,     1,     3,     3,     3,     0,     2,     1,     3,     0,
       2,     1,     3,     0,     2,     1,     3,     0,     2,     1,
       3,     1,     3,     1,     3,     0,     2,     1,     1,     1,
       1,     1,     4,     0,     2,     1,     3,     1,     1,     1,
       4,     1,     1,     0,     1,     2,     3,     4,     1,     1,
       3,     1,     2,     4,     0,     1,     1,     1,     1,     1,
       1,     4,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (parm, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, parm); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *parm)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (parm);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *parm)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, parm);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *parm)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], parm);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, parm); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *parm)
{
  YY_USE (yyvaluep);
  YY_USE (parm);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *parm)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, YYLEX_PARAM);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 15: /* preddecl_item: FZ_PREDICATE FZ_ID '(' pred_arg_list ')'  */
#line 744 "./gecode/flatzinc/parser.yxx"
      { free((yyvsp[-3].sValue)); }
#line 2288 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 20: /* pred_arg: pred_arg_type ':' FZ_ID  */
#line 756 "./gecode/flatzinc/parser.yxx"
      { free((yyvsp[0].sValue)); }
#line 2294 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 25: /* pred_arg_simple_type: int_ti_expr_tail  */
#line 766 "./gecode/flatzinc/parser.yxx"
      { if ((yyvsp[0].oSet)()) delete (yyvsp[0].oSet).some(); }
#line 2300 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 26: /* pred_arg_simple_type: FZ_SET FZ_OF int_ti_expr_tail  */
#line 768 "./gecode/flatzinc/parser.yxx"
      { if ((yyvsp[0].oSet)()) delete (yyvsp[0].oSet).some(); }
#line 2306 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 35: /* vardecl_item: FZ_VAR int_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 788 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
        bool funcDep = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("is_defined_var");
        yyassert(pp,
          pp->symbols.put((yyvsp[-2].sValue), se_iv(pp->intvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[-2].sValue)), new AST::IntVar(pp->intvars.size()));
        }
        if ((yyvsp[0].oArg)()) {
          AST::Node* arg = (yyvsp[0].oArg).some();
          if (arg->isInt()) {
            pp->intvars.push_back(varspec((yyvsp[-2].sValue),
              new IntVarSpec(arg->getInt(),!print,funcDep)));
          } else if (arg->isIntVar()) {
            pp->intvars.push_back(varspec((yyvsp[-2].sValue),
              new IntVarSpec(Alias(arg->getIntVar()),!print,funcDep)));
          } else {
            yyassert(pp, false, "Invalid var int initializer");
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "int_in",
                                new AST::IntVar(pp->intvars.size()-1), (yyvsp[-4].oSet));
          delete arg;
        } else {
          pp->intvars.push_back(varspec((yyvsp[-2].sValue),
            new IntVarSpec((yyvsp[-4].oSet),!print,funcDep)));
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2342 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 36: /* vardecl_item: FZ_VAR bool_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 820 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
        bool funcDep = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("is_defined_var");
        yyassert(pp,
          pp->symbols.put((yyvsp[-2].sValue), se_bv(pp->boolvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[-2].sValue)), new AST::BoolVar(pp->boolvars.size()));
        }
        if ((yyvsp[0].oArg)()) {
          AST::Node* arg = (yyvsp[0].oArg).some();
          if (arg->isBool()) {
            pp->boolvars.push_back(varspec((yyvsp[-2].sValue),
              new BoolVarSpec(arg->getBool(),!print,funcDep)));
          } else if (arg->isBoolVar()) {
            pp->boolvars.push_back(varspec((yyvsp[-2].sValue),
              new BoolVarSpec(Alias(arg->getBoolVar()),!print,funcDep)));
          } else {
            yyassert(pp, false, "Invalid var bool initializer");
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "int_in",
                                new AST::BoolVar(pp->boolvars.size()-1), (yyvsp[-4].oSet));
          delete arg;
        } else {
          pp->boolvars.push_back(varspec((yyvsp[-2].sValue),
            new BoolVarSpec((yyvsp[-4].oSet),!print,funcDep)));
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2378 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 37: /* vardecl_item: FZ_VAR float_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 852 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
        bool funcDep = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("is_defined_var");
        yyassert(pp,
          pp->symbols.put((yyvsp[-2].sValue), se_fv(pp->floatvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[-2].sValue)),
                     new AST::FloatVar(pp->floatvars.size()));
        }
        if ((yyvsp[0].oArg)()) {
          AST::Node* arg = (yyvsp[0].oArg).some();
          if (arg->isFloat()) {
            pp->floatvars.push_back(varspec((yyvsp[-2].sValue),
              new FloatVarSpec(arg->getFloat(),!print,funcDep)));
          } else if (arg->isFloatVar()) {
            pp->floatvars.push_back(varspec((yyvsp[-2].sValue),
              new FloatVarSpec(
                Alias(arg->getFloatVar()),!print,funcDep)));
          } else {
            yyassert(pp, false, "Invalid var float initializer");
          }
          if (!pp->hadError && (yyvsp[-4].oPFloat)()) {
            AST::FloatVar* fv = new AST::FloatVar(pp->floatvars.size()-1);
            addDomainConstraint(pp, fv, (yyvsp[-4].oPFloat));
          }
          delete arg;
        } else {
          Option<std::pair<double,double> > dom =
            (yyvsp[-4].oPFloat)() ? Option<std::pair<double,double> >::some(*(yyvsp[-4].oPFloat).some())
                 : Option<std::pair<double,double> >::none();
          if ((yyvsp[-4].oPFloat)()) delete (yyvsp[-4].oPFloat).some();
          pp->floatvars.push_back(varspec((yyvsp[-2].sValue),
            new FloatVarSpec(dom,!print,funcDep)));
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2421 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 38: /* vardecl_item: FZ_VAR FZ_SET FZ_OF int_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 891 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
        bool funcDep = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("is_defined_var");
        yyassert(pp,
          pp->symbols.put((yyvsp[-2].sValue), se_sv(pp->setvars.size())),
          "Duplicate symbol");
        if (print) {
          pp->output(std::string((yyvsp[-2].sValue)), new AST::SetVar(pp->setvars.size()));
        }
        if ((yyvsp[0].oArg)()) {
          AST::Node* arg = (yyvsp[0].oArg).some();
          if (arg->isSet()) {
            pp->setvars.push_back(varspec((yyvsp[-2].sValue),
              new SetVarSpec(arg->getSet(),!print,funcDep)));
          } else if (arg->isSetVar()) {
            pp->setvars.push_back(varspec((yyvsp[-2].sValue),
              new SetVarSpec(Alias(arg->getSetVar()),!print,funcDep)));
            delete arg;
          } else {
            yyassert(pp, false, "Invalid var set initializer");
            delete arg;
          }
          if (!pp->hadError)
            addDomainConstraint(pp, "set_subset",
                                new AST::SetVar(pp->setvars.size()-1), (yyvsp[-4].oSet));
        } else {
          pp->setvars.push_back(varspec((yyvsp[-2].sValue),
            new SetVarSpec((yyvsp[-4].oSet),!print,funcDep)));
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2458 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 39: /* vardecl_item: FZ_INT ':' var_par_id annotations '=' non_array_expr  */
#line 924 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[0].arg)->isInt(), "Invalid int initializer");
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_i((yyvsp[0].arg)->getInt())),
          "Duplicate symbol");
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2471 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 40: /* vardecl_item: FZ_FLOAT ':' var_par_id annotations '=' non_array_expr  */
#line 933 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[0].arg)->isFloat(), "Invalid float initializer");
        pp->floatvals.push_back((yyvsp[0].arg)->getFloat());
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_f(pp->floatvals.size()-1)),
          "Duplicate symbol");
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2485 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 41: /* vardecl_item: FZ_BOOL ':' var_par_id annotations '=' non_array_expr  */
#line 943 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[0].arg)->isBool(), "Invalid bool initializer");
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_b((yyvsp[0].arg)->getBool())),
          "Duplicate symbol");
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2498 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 42: /* vardecl_item: FZ_SET FZ_OF FZ_INT ':' var_par_id annotations '=' non_array_expr  */
#line 952 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[0].arg)->isSet(), "Invalid set initializer");
        AST::SetLit* set = (yyvsp[0].arg)->getSet();
        pp->setvals.push_back(*set);
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_s(pp->setvals.size()-1)),
          "Duplicate symbol");
        delete set;
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2514 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 43: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR int_ti_expr_tail ':' var_par_id annotations vardecl_int_var_array_init  */
#line 965 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
          vector<int> vars((yyvsp[-8].iValue));
          if (!pp->hadError) {
            if ((yyvsp[0].oVarSpecVec)()) {
              vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
              yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                       "Initializer size does not match array dimension");
              if (!pp->hadError) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  IntVarSpec* ivsv = static_cast<IntVarSpec*>((*vsv)[i]);
                  if (ivsv->alias) {
                    if (print)
                      static_cast<IntVarSpec*>(pp->intvars[ivsv->i].second)->introduced = false;
                    vars[i] = ivsv->i;
                  } else {
                    if (print)
                      ivsv->introduced = false;
                    vars[i] = pp->intvars.size();
                    pp->intvars.push_back(varspec((yyvsp[-2].sValue), ivsv));
                  }
                  if (!pp->hadError && (yyvsp[-4].oSet)()) {
                    Option<AST::SetLit*> opt =
                      Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[-4].oSet).some()));
                    addDomainConstraint(pp, "int_in",
                                        new AST::IntVar(vars[i]),
                                        opt);
                  }
                }
              }
              delete vsv;
            } else {
              if ((yyvsp[-8].iValue)>0) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  Option<AST::SetLit*> dom =
                    (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                         : Option<AST::SetLit*>::none();
                  IntVarSpec* ispec = new IntVarSpec(dom,!print,false);
                  vars[i] = pp->intvars.size();
                  pp->intvars.push_back(varspec((yyvsp[-2].sValue), ispec));
                }
              }
              if ((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
            }
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[-8].iValue); i++)
              output->a.push_back(new AST::IntVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[-2].sValue)), a);
          }
          int iva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-2].sValue), se_iva(iva)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2587 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 44: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR bool_ti_expr_tail ':' var_par_id annotations vardecl_bool_var_array_init  */
#line 1035 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          vector<int> vars((yyvsp[-8].iValue));
          if ((yyvsp[0].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[-8].iValue); i++) {
                BoolVarSpec* bvsv = static_cast<BoolVarSpec*>((*vsv)[i]);
                if (bvsv->alias) {
                  if (print)
                    static_cast<BoolVarSpec*>(pp->boolvars[bvsv->i].second)->introduced = false;
                  vars[i] = bvsv->i;
                } else {
                  if (print)
                    bvsv->introduced = false;
                  vars[i] = pp->boolvars.size();
                  pp->boolvars.push_back(varspec((yyvsp[-2].sValue), (*vsv)[i]));
                }
                if (!pp->hadError && (yyvsp[-4].oSet)()) {
                  Option<AST::SetLit*> opt =
                    Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[-4].oSet).some()));
                  addDomainConstraint(pp, "int_in",
                                      new AST::BoolVar(vars[i]),
                                      opt);
                }
              }
            }
            delete vsv;
          } else {
            for (int i=0; i<(yyvsp[-8].iValue); i++) {
              Option<AST::SetLit*> dom =
                (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                     : Option<AST::SetLit*>::none();
              vars[i] = pp->boolvars.size();
              pp->boolvars.push_back(varspec((yyvsp[-2].sValue),
                                       new BoolVarSpec(dom,!print,false)));
            }
            if ((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[-8].iValue); i++)
              output->a.push_back(new AST::BoolVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[-2].sValue)), a);
          }
          int bva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-2].sValue), se_bva(bva)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2656 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 45: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR float_ti_expr_tail ':' var_par_id annotations vardecl_float_var_array_init  */
#line 1102 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
          vector<int> vars((yyvsp[-8].iValue));
          if (!pp->hadError) {
            if ((yyvsp[0].oVarSpecVec)()) {
              vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
              yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                       "Initializer size does not match array dimension");
              if (!pp->hadError) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  FloatVarSpec* ivsv = static_cast<FloatVarSpec*>((*vsv)[i]);
                  if (ivsv->alias) {
                    if (print)
                      static_cast<FloatVarSpec*>(pp->floatvars[ivsv->i].second)->introduced = false;
                    vars[i] = ivsv->i;
                  } else {
                    if (print)
                      ivsv->introduced = false;
                    vars[i] = pp->floatvars.size();
                    pp->floatvars.push_back(varspec((yyvsp[-2].sValue), ivsv));
                  }
                  if (!pp->hadError && (yyvsp[-4].oPFloat)()) {
                    Option<std::pair<double,double>*> opt =
                      Option<std::pair<double,double>*>::some(
                        new std::pair<double,double>(*(yyvsp[-4].oPFloat).some()));
                    addDomainConstraint(pp, new AST::FloatVar(vars[i]),
                                        opt);
                  }
                }
              }
              delete vsv;
            } else {
              if ((yyvsp[-8].iValue)>0) {
                Option<std::pair<double,double> > dom =
                  (yyvsp[-4].oPFloat)() ? Option<std::pair<double,double> >::some(*(yyvsp[-4].oPFloat).some())
                       : Option<std::pair<double,double> >::none();
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  FloatVarSpec* ispec = new FloatVarSpec(dom,!print,false);
                  vars[i] = pp->floatvars.size();
                  pp->floatvars.push_back(varspec((yyvsp[-2].sValue), ispec));
                }
              }
            }
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[-8].iValue); i++)
              output->a.push_back(new AST::FloatVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[-2].sValue)), a);
          }
          int fva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-2].sValue), se_fva(fva)),
            "Duplicate symbol");
        }
        if ((yyvsp[-4].oPFloat)()) delete (yyvsp[-4].oPFloat).some();
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2729 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 46: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR FZ_SET FZ_OF int_ti_expr_tail ':' var_par_id annotations vardecl_set_var_array_init  */
#line 1172 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        bool print = (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        if (!pp->hadError) {
          vector<int> vars((yyvsp[-10].iValue));
          if ((yyvsp[0].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-10].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[-10].iValue); i++) {
                SetVarSpec* svsv = static_cast<SetVarSpec*>((*vsv)[i]);
                if (svsv->alias) {
                  if (print)
                    static_cast<SetVarSpec*>(pp->setvars[svsv->i].second)->introduced = false;
                  vars[i] = svsv->i;
                } else {
                  if (print)
                    svsv->introduced = false;
                  vars[i] = pp->setvars.size();
                  pp->setvars.push_back(varspec((yyvsp[-2].sValue), (*vsv)[i]));
                }
                if (!pp->hadError && (yyvsp[-4].oSet)()) {
                  Option<AST::SetLit*> opt =
                    Option<AST::SetLit*>::some(new AST::SetLit(*(yyvsp[-4].oSet).some()));
                  addDomainConstraint(pp, "set_subset",
                                      new AST::SetVar(vars[i]),
                                      opt);
                }
              }
            }
            delete vsv;
          } else {
            if ((yyvsp[-10].iValue)>0) {
              for (int i=0; i<(yyvsp[-10].iValue); i++) {
                Option<AST::SetLit*> dom =
                  (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                        : Option<AST::SetLit*>::none();
                SetVarSpec* ispec = new SetVarSpec(dom,!print,false);
                vars[i] = pp->setvars.size();
                pp->setvars.push_back(varspec((yyvsp[-2].sValue), ispec));
              }
              if ((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
            }
          }
          if (print) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array")));
            AST::Array* output = new AST::Array();
            for (int i=0; i<(yyvsp[-10].iValue); i++)
              output->a.push_back(new AST::SetVar(vars[i]));
            a->a.push_back(output);
            a->a.push_back(new AST::String(")"));
            pp->output(std::string((yyvsp[-2].sValue)), a);
          }
          int sva = pp->arrays.size();
          pp->arrays.push_back(vars.size());
          for (unsigned int i=0; i<vars.size(); i++)
            pp->arrays.push_back(vars[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-2].sValue), se_sva(sva)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2800 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 47: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_INT ':' var_par_id annotations '=' '[' int_list ']'  */
#line 1240 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[-1].setValue)->size() == static_cast<unsigned int>((yyvsp[-10].iValue)),
                 "Initializer size does not match array dimension");

        if (!pp->hadError) {
          int ia = pp->arrays.size();
          pp->arrays.push_back((yyvsp[-1].setValue)->size());
          for (unsigned int i=0; i<(yyvsp[-1].setValue)->size(); i++)
            pp->arrays.push_back((*(yyvsp[-1].setValue))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-5].sValue), se_ia(ia)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].setValue);
        free((yyvsp[-5].sValue));
        delete (yyvsp[-4].argVec);
      }
#line 2824 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 48: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_BOOL ':' var_par_id annotations '=' '[' bool_list ']'  */
#line 1261 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[-1].setValue)->size() == static_cast<unsigned int>((yyvsp[-10].iValue)),
                 "Initializer size does not match array dimension");
        if (!pp->hadError) {
          int ia = pp->arrays.size();
          pp->arrays.push_back((yyvsp[-1].setValue)->size());
          for (unsigned int i=0; i<(yyvsp[-1].setValue)->size(); i++)
            pp->arrays.push_back((*(yyvsp[-1].setValue))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-5].sValue), se_ba(ia)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].setValue);
        free((yyvsp[-5].sValue));
        delete (yyvsp[-4].argVec);
      }
#line 2847 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 49: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_FLOAT ':' var_par_id annotations '=' '[' float_list ']'  */
#line 1281 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[-1].floatSetValue)->size() == static_cast<unsigned int>((yyvsp[-10].iValue)),
                 "Initializer size does not match array dimension");
        if (!pp->hadError) {
          int fa = pp->arrays.size();
          pp->arrays.push_back((yyvsp[-1].floatSetValue)->size());
          pp->arrays.push_back(pp->floatvals.size());
          for (unsigned int i=0; i<(yyvsp[-1].floatSetValue)->size(); i++)
            pp->floatvals.push_back((*(yyvsp[-1].floatSetValue))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-5].sValue), se_fa(fa)),
            "Duplicate symbol");
        }
        delete (yyvsp[-1].floatSetValue);
        delete (yyvsp[-4].argVec); free((yyvsp[-5].sValue));
      }
#line 2870 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 50: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_SET FZ_OF FZ_INT ':' var_par_id annotations '=' '[' set_literal_list ']'  */
#line 1301 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-14].iValue)==1, "Arrays must start at 1");
        yyassert(pp, (yyvsp[-1].setValueList)->size() == static_cast<unsigned int>((yyvsp[-12].iValue)),
                 "Initializer size does not match array dimension");
        if (!pp->hadError) {
          int sa = pp->arrays.size();
          pp->arrays.push_back((yyvsp[-1].setValueList)->size());
          pp->arrays.push_back(pp->setvals.size());
          for (unsigned int i=0; i<(yyvsp[-1].setValueList)->size(); i++)
            pp->setvals.push_back((*(yyvsp[-1].setValueList))[i]);
          yyassert(pp,
            pp->symbols.put((yyvsp[-5].sValue), se_sa(sa)),
            "Duplicate symbol");
        }

        delete (yyvsp[-1].setValueList);
        delete (yyvsp[-4].argVec); free((yyvsp[-5].sValue));
      }
#line 2894 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 51: /* int_init: FZ_INT_LIT  */
#line 1323 "./gecode/flatzinc/parser.yxx"
      {
        (yyval.varSpec) = new IntVarSpec((yyvsp[0].iValue),false,false);
      }
#line 2902 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 52: /* int_init: var_par_id  */
#line 1327 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_INTVAR || e.t == ST_INT))
          (yyval.varSpec) = new IntVarSpec(Alias(e.i),false,false);
        else {
          pp->err << "Error: undefined identifier for type int " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new IntVarSpec(0,false,false); // keep things consistent
        }
        free((yyvsp[0].sValue));
      }
#line 2921 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 53: /* int_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1342 "./gecode/flatzinc/parser.yxx"
      {
        vector<int> v;
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[-3].sValue), e) && e.t == ST_INTVARARRAY) {
          yyassert(pp,(yyvsp[-1].iValue) > 0 && (yyvsp[-1].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new IntVarSpec(Alias(pp->arrays[e.i+(yyvsp[-1].iValue)]),false,false);
          else
            (yyval.varSpec) = new IntVarSpec(0,false,false); // keep things consistent
        } else {
          pp->err << "Error: undefined array identifier for type int " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new IntVarSpec(0,false,false); // keep things consistent
        }
        free((yyvsp[-3].sValue));
      }
#line 2946 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 54: /* int_init_list: %empty  */
#line 1365 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 2952 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 55: /* int_init_list: int_init_list_head list_tail  */
#line 1367 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 2958 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 56: /* int_init_list_head: int_init  */
#line 1371 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 2964 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 57: /* int_init_list_head: int_init_list_head ',' int_init  */
#line 1373 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 2970 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 60: /* int_var_array_literal: '[' int_init_list ']'  */
#line 1378 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 2976 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 61: /* float_init: FZ_FLOAT_LIT  */
#line 1382 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new FloatVarSpec((yyvsp[0].dValue),false,false); }
#line 2982 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 62: /* float_init: var_par_id  */
#line 1384 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_FLOATVAR || e.t == ST_FLOAT))
          (yyval.varSpec) = new FloatVarSpec(Alias(e.i),false,false);
        else {
          pp->err << "Error: undefined identifier for type float " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new FloatVarSpec(0.0,false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3001 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 63: /* float_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1399 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[-3].sValue), e) && e.t == ST_FLOATVARARRAY) {
          yyassert(pp,(yyvsp[-1].iValue) > 0 && (yyvsp[-1].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new FloatVarSpec(Alias(pp->arrays[e.i+(yyvsp[-1].iValue)]),false,false);
          else
            (yyval.varSpec) = new FloatVarSpec(0.0,false,false);
        } else {
          pp->err << "Error: undefined array identifier for type float " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new FloatVarSpec(0.0,false,false);
        }
        free((yyvsp[-3].sValue));
      }
#line 3025 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 64: /* float_init_list: %empty  */
#line 1421 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3031 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 65: /* float_init_list: float_init_list_head list_tail  */
#line 1423 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3037 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 66: /* float_init_list_head: float_init  */
#line 1427 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3043 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 67: /* float_init_list_head: float_init_list_head ',' float_init  */
#line 1429 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3049 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 68: /* float_var_array_literal: '[' float_init_list ']'  */
#line 1433 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3055 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 69: /* bool_init: FZ_BOOL_LIT  */
#line 1437 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new BoolVarSpec((yyvsp[0].iValue),false,false); }
#line 3061 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 70: /* bool_init: var_par_id  */
#line 1439 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_BOOLVAR || e.t == ST_BOOL))
          (yyval.varSpec) = new BoolVarSpec(Alias(e.i),false,false);
        else {
          pp->err << "Error: undefined identifier for type bool " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new BoolVarSpec(false,false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3080 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 71: /* bool_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1454 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[-3].sValue), e) && e.t == ST_BOOLVARARRAY) {
          yyassert(pp,(yyvsp[-1].iValue) > 0 && (yyvsp[-1].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new BoolVarSpec(Alias(pp->arrays[e.i+(yyvsp[-1].iValue)]),false,false);
          else
            (yyval.varSpec) = new BoolVarSpec(false,false,false);
        } else {
          pp->err << "Error: undefined array identifier for type bool " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new BoolVarSpec(false,false,false);
        }
        free((yyvsp[-3].sValue));
      }
#line 3104 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 72: /* bool_init_list: %empty  */
#line 1476 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3110 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 73: /* bool_init_list: bool_init_list_head list_tail  */
#line 1478 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3116 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 74: /* bool_init_list_head: bool_init  */
#line 1482 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3122 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 75: /* bool_init_list_head: bool_init_list_head ',' bool_init  */
#line 1484 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3128 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 76: /* bool_var_array_literal: '[' bool_init_list ']'  */
#line 1486 "./gecode/flatzinc/parser.yxx"
                                                { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3134 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 77: /* set_init: set_literal  */
#line 1490 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new SetVarSpec((yyvsp[0].setLit),false,false); }
#line 3140 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 78: /* set_init: var_par_id  */
#line 1492 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_SETVAR || e.t == ST_SET))
          (yyval.varSpec) = new SetVarSpec(Alias(e.i),false,false);
        else {
          pp->err << "Error: undefined identifier for type set " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new SetVarSpec(Alias(0),false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3159 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 79: /* set_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1507 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[-3].sValue), e) && e.t == ST_SETVARARRAY) {
          yyassert(pp,(yyvsp[-1].iValue) > 0 && (yyvsp[-1].iValue) <= pp->arrays[e.i],
                   "array access out of bounds");
          if (!pp->hadError)
            (yyval.varSpec) = new SetVarSpec(Alias(pp->arrays[e.i+(yyvsp[-1].iValue)]),false,false);
          else
            (yyval.varSpec) = new SetVarSpec(Alias(0),false,false);
        } else {
          pp->err << "Error: undefined array identifier for type set " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new SetVarSpec(Alias(0),false,false);
        }
        free((yyvsp[-3].sValue));
      }
#line 3183 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 80: /* set_init_list: %empty  */
#line 1529 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3189 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 81: /* set_init_list: set_init_list_head list_tail  */
#line 1531 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3195 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 82: /* set_init_list_head: set_init  */
#line 1535 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3201 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 83: /* set_init_list_head: set_init_list_head ',' set_init  */
#line 1537 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3207 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 84: /* set_var_array_literal: '[' set_init_list ']'  */
#line 1540 "./gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3213 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 85: /* vardecl_int_var_array_init: %empty  */
#line 1544 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3219 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 86: /* vardecl_int_var_array_init: '=' int_var_array_literal  */
#line 1546 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3225 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 87: /* vardecl_bool_var_array_init: %empty  */
#line 1550 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3231 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 88: /* vardecl_bool_var_array_init: '=' bool_var_array_literal  */
#line 1552 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3237 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 89: /* vardecl_float_var_array_init: %empty  */
#line 1556 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3243 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 90: /* vardecl_float_var_array_init: '=' float_var_array_literal  */
#line 1558 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3249 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 91: /* vardecl_set_var_array_init: %empty  */
#line 1562 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3255 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 92: /* vardecl_set_var_array_init: '=' set_var_array_literal  */
#line 1564 "./gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3261 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 93: /* constraint_item: FZ_CONSTRAINT FZ_ID '(' flat_expr_list ')' annotations  */
#line 1568 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        if (!pp->hadError) {
          std::string cid((yyvsp[-4].sValue));
          if (cid=="gecode_on_restart_status" && (yyvsp[-2].argVec)->a[0]->isIntVar()) {
            pp->status_idx = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[0]->getIntVar());
          } else if (cid=="gecode_on_restart_complete" && (yyvsp[-2].argVec)->a[0]->isBoolVar()) {
            pp->complete_idx = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[0]->getBoolVar());
          } else if (cid=="gecode_on_restart_last_val_int" && (yyvsp[-2].argVec)->a[0]->isIntVar() && (yyvsp[-2].argVec)->a[1]->isIntVar()) {
            int base0 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[0]->getIntVar());
            int base1 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[1]->getIntVar());
            pp->last_val_int.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_sol_int" && (yyvsp[-2].argVec)->a[0]->isIntVar() && (yyvsp[-2].argVec)->a[1]->isIntVar()) {
            int base0 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[0]->getIntVar());
            int base1 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[1]->getIntVar());
            pp->sol_int.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_uniform_int" && (yyvsp[-2].argVec)->a[0]->isInt() && (yyvsp[-2].argVec)->a[1]->isInt() && (yyvsp[-2].argVec)->a[2]->isIntVar()) {
            int base = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[2]->getIntVar());
            pp->uniform_int.emplace_back(std::array<int, 3>{ (yyvsp[-2].argVec)->a[0]->getInt(), (yyvsp[-2].argVec)->a[1]->getInt(), base });
          } else if (cid=="gecode_on_restart_last_val_bool" && (yyvsp[-2].argVec)->a[0]->isBoolVar() && (yyvsp[-2].argVec)->a[1]->isBoolVar()) {
            int base0 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[0]->getBoolVar());
            int base1 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[1]->getBoolVar());
            pp->last_val_bool.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_sol_bool" && (yyvsp[-2].argVec)->a[0]->isBoolVar() && (yyvsp[-2].argVec)->a[1]->isBoolVar()) {
            int base0 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[0]->getBoolVar());
            int base1 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[1]->getBoolVar());
            pp->sol_bool.emplace_back(std::array<int, 2>{ base0, base1 });
#ifdef GECODE_HAS_SET_VARS
          } else if (cid=="gecode_on_restart_last_val_set" && (yyvsp[-2].argVec)->a[0]->isSetVar() && (yyvsp[-2].argVec)->a[1]->isSetVar()) {
            int base0 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[0]->getSetVar());
            int base1 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[1]->getSetVar());
            pp->last_val_set.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_sol_set" && (yyvsp[-2].argVec)->a[0]->isSetVar() && (yyvsp[-2].argVec)->a[1]->isSetVar()) {
            int base0 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[0]->getSetVar());
            int base1 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[1]->getSetVar());
            pp->sol_set.emplace_back(std::array<int, 2>{ base0, base1 });
#endif
#ifdef GECODE_HAS_FLOAT_VARS
          } else if (cid=="gecode_on_restart_last_val_float" && (yyvsp[-2].argVec)->a[0]->isFloatVar() && (yyvsp[-2].argVec)->a[1]->isFloatVar()) {
            int base0 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[0]->getFloatVar());
            int base1 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[1]->getFloatVar());
            pp->last_val_float.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_sol_float" && (yyvsp[-2].argVec)->a[0]->isFloatVar() && (yyvsp[-2].argVec)->a[1]->isFloatVar()) {
            int base0 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[0]->getFloatVar());
            int base1 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[1]->getFloatVar());
            pp->sol_float.emplace_back(std::array<int, 2>{ base0, base1 });
          } else if (cid=="gecode_on_restart_uniform_float" && (yyvsp[-2].argVec)->a[0]->isFloat() && (yyvsp[-2].argVec)->a[1]->isFloat() && (yyvsp[-2].argVec)->a[2]->isFloatVar()) {
            int base = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[2]->getFloatVar());
            pp->uniform_float.emplace_back( (yyvsp[-2].argVec)->a[0]->getFloat(), (yyvsp[-2].argVec)->a[1]->getFloat(), base );
#endif
          } else if (cid=="int_eq" && (yyvsp[-2].argVec)->a[0]->isIntVar() && (yyvsp[-2].argVec)->a[1]->isIntVar()) {
            int base0 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[0]->getIntVar());
            int base1 = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[1]->getIntVar());
            if (base0 > base1) {
              std::swap(base0, base1);
            }
            if (base0==base1) {
              // do nothing, already aliased
            } else {
              IntVarSpec* ivs1 = static_cast<IntVarSpec*>(pp->intvars[base1].second);
              AST::SetLit* sl = NULL;
              if (ivs1->assigned) {
                sl = new AST::SetLit(ivs1->i,ivs1->i);
              } else if (ivs1->domain()) {
                sl = new AST::SetLit(ivs1->domain.some()->getSet());
              }
              if (sl) {
                Option<AST::SetLit*> newdom = Option<AST::SetLit*>::some(sl);
                addDomainConstraint(pp, "int_in",
                                    new AST::IntVar(base0), newdom);
                ivs1->domain = Option<AST::SetLit*>::none();
              }
              ivs1->alias = true;
              ivs1->i = base0;
            }
          } else if (cid=="bool_eq" && (yyvsp[-2].argVec)->a[0]->isBoolVar() && (yyvsp[-2].argVec)->a[1]->isBoolVar()) {
            int base0 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[0]->getBoolVar());
            int base1 = getBaseBoolVar(pp,(yyvsp[-2].argVec)->a[1]->getBoolVar());
            if (base0 > base1) {
              std::swap(base0, base1);
            }
            if (base0==base1) {
              // do nothing, already aliased
            } else {
              BoolVarSpec* ivs1 = static_cast<BoolVarSpec*>(pp->boolvars[base1].second);
              AST::SetLit* sl = NULL;
              if (ivs1->assigned) {
                sl = new AST::SetLit(ivs1->i,ivs1->i);
              } else if (ivs1->domain()) {
                sl = new AST::SetLit(ivs1->domain.some()->getSet());
              }
              if (sl) {
                Option<AST::SetLit*> newdom = Option<AST::SetLit*>::some(sl);
                addDomainConstraint(pp, "int_in",
                                    new AST::BoolVar(base0), newdom);
                ivs1->domain = Option<AST::SetLit*>::none();
              }
              ivs1->alias = true;
              ivs1->i = base0;
            }
          } else if (cid=="float_eq" && (yyvsp[-2].argVec)->a[0]->isFloatVar() && (yyvsp[-2].argVec)->a[1]->isFloatVar()) {
            int base0 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[0]->getFloatVar());
            int base1 = getBaseFloatVar(pp,(yyvsp[-2].argVec)->a[1]->getFloatVar());
            if (base0 > base1) {
              std::swap(base0, base1);
            }
            if (base0==base1) {
              // do nothing, already aliased
            } else {
              FloatVarSpec* ivs1 = static_cast<FloatVarSpec*>(pp->floatvars[base1].second);
              ivs1->alias = true;
              ivs1->i = base0;
              if (ivs1->domain()) {
                std::pair<double,double>* dom = new std::pair<double,double>(ivs1->domain.some());
                addDomainConstraint(pp, new AST::FloatVar(base0), Option<std::pair<double,double>* >::some(dom));
                ivs1->domain = Option<std::pair<double,double> >::none();
              }
            }
          } else if (cid=="set_eq" && (yyvsp[-2].argVec)->a[0]->isSetVar() && (yyvsp[-2].argVec)->a[1]->isSetVar()) {
            int base0 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[0]->getSetVar());
            int base1 = getBaseSetVar(pp,(yyvsp[-2].argVec)->a[1]->getSetVar());
            if (base0 > base1) {
              std::swap(base0, base1);
            }
            if (base0==base1) {
              // do nothing, already aliased
            } else {
              SetVarSpec* ivs1 = static_cast<SetVarSpec*>(pp->setvars[base1].second);
              ivs1->alias = true;
              ivs1->i = base0;
              if (ivs1->upperBound()) {
                AST::SetLit* sl = new AST::SetLit(ivs1->upperBound.some()->getSet());
                Option<AST::SetLit*> newdom = Option<AST::SetLit*>::some(sl);
                if (ivs1->assigned) {
                  addDomainConstraint(pp, "set_eq",
                                      new AST::SetVar(base0), newdom);
                } else {
                  addDomainConstraint(pp, "set_subset",
                                      new AST::SetVar(base0), newdom);
                }
                ivs1->upperBound = Option<AST::SetLit*>::none();
              }
            }
          } else if ( (cid=="int_le" || cid=="int_lt" || cid=="int_ge" || cid=="int_gt"  ||
                       cid=="int_eq" || cid=="int_ne") &&
                      ((yyvsp[-2].argVec)->a[0]->isInt() || (yyvsp[-2].argVec)->a[1]->isInt()) ) {
            pp->domainConstraints.push_back(new ConExpr((yyvsp[-4].sValue), (yyvsp[-2].argVec), (yyvsp[0].argVec)));
          } else if ( cid=="set_in" && ((yyvsp[-2].argVec)->a[0]->isSet() || (yyvsp[-2].argVec)->a[1]->isSet()) ) {
            pp->domainConstraints.push_back(new ConExpr((yyvsp[-4].sValue), (yyvsp[-2].argVec), (yyvsp[0].argVec)));
          } else {
            pp->constraints.push_back(new ConExpr((yyvsp[-4].sValue), (yyvsp[-2].argVec), (yyvsp[0].argVec)));
          }
        }
        free((yyvsp[-4].sValue));
      }
#line 3421 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 94: /* solve_item: FZ_SOLVE annotations FZ_SATISFY  */
#line 1725 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        initfg(pp);
        if (!pp->hadError) {
          try {
            pp->fg->solve((yyvsp[-1].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        } else {
          delete (yyvsp[-1].argVec);
        }
      }
#line 3439 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 95: /* solve_item: FZ_SOLVE annotations minmax solve_expr  */
#line 1739 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        initfg(pp);
        if (!pp->hadError) {
          try {
            int v = (yyvsp[0].iValue) < 0 ? (-(yyvsp[0].iValue)-1) : (yyvsp[0].iValue);
            bool vi = (yyvsp[0].iValue) >= 0;
            if ((yyvsp[-1].bValue))
              pp->fg->minimize(v,vi,(yyvsp[-2].argVec));
            else
              pp->fg->maximize(v,vi,(yyvsp[-2].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        } else {
          delete (yyvsp[-2].argVec);
        }
      }
#line 3462 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 96: /* int_ti_expr_tail: FZ_INT  */
#line 1764 "./gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::none(); }
#line 3468 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 97: /* int_ti_expr_tail: '{' int_list '}'  */
#line 1766 "./gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit(*(yyvsp[-1].setValue))); }
#line 3474 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 98: /* int_ti_expr_tail: FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT  */
#line 1768 "./gecode/flatzinc/parser.yxx"
      {
        (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit((yyvsp[-2].iValue), (yyvsp[0].iValue)));
      }
#line 3482 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 99: /* bool_ti_expr_tail: FZ_BOOL  */
#line 1774 "./gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::none(); }
#line 3488 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 100: /* bool_ti_expr_tail: '{' bool_list_head list_tail '}'  */
#line 1776 "./gecode/flatzinc/parser.yxx"
      { bool haveTrue = false;
        bool haveFalse = false;
        for (int i=(yyvsp[-2].setValue)->size(); i--;) {
          haveTrue |= ((*(yyvsp[-2].setValue))[i] == 1);
          haveFalse |= ((*(yyvsp[-2].setValue))[i] == 0);
        }
        delete (yyvsp[-2].setValue);
        (yyval.oSet) = Option<AST::SetLit* >::some(
          new AST::SetLit(!haveFalse,haveTrue));
      }
#line 3503 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 101: /* float_ti_expr_tail: FZ_FLOAT  */
#line 1789 "./gecode/flatzinc/parser.yxx"
      { (yyval.oPFloat) = Option<std::pair<double,double>* >::none(); }
#line 3509 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 102: /* float_ti_expr_tail: FZ_FLOAT_LIT FZ_DOTDOT FZ_FLOAT_LIT  */
#line 1791 "./gecode/flatzinc/parser.yxx"
      { std::pair<double,double>* dom = new std::pair<double,double>((yyvsp[-2].dValue),(yyvsp[0].dValue));
        (yyval.oPFloat) = Option<std::pair<double,double>* >::some(dom); }
#line 3516 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 103: /* set_literal: '{' int_list '}'  */
#line 1800 "./gecode/flatzinc/parser.yxx"
      { (yyval.setLit) = new AST::SetLit(*(yyvsp[-1].setValue)); }
#line 3522 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 104: /* set_literal: FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT  */
#line 1802 "./gecode/flatzinc/parser.yxx"
      { (yyval.setLit) = new AST::SetLit((yyvsp[-2].iValue), (yyvsp[0].iValue)); }
#line 3528 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 105: /* int_list: %empty  */
#line 1808 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(0); }
#line 3534 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 106: /* int_list: int_list_head list_tail  */
#line 1810 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-1].setValue); }
#line 3540 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 107: /* int_list_head: FZ_INT_LIT  */
#line 1814 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[0].iValue); }
#line 3546 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 108: /* int_list_head: int_list_head ',' FZ_INT_LIT  */
#line 1816 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-2].setValue); (yyval.setValue)->push_back((yyvsp[0].iValue)); }
#line 3552 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 109: /* bool_list: %empty  */
#line 1820 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(0); }
#line 3558 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 110: /* bool_list: bool_list_head list_tail  */
#line 1822 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-1].setValue); }
#line 3564 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 111: /* bool_list_head: FZ_BOOL_LIT  */
#line 1826 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[0].iValue); }
#line 3570 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 112: /* bool_list_head: bool_list_head ',' FZ_BOOL_LIT  */
#line 1828 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-2].setValue); (yyval.setValue)->push_back((yyvsp[0].iValue)); }
#line 3576 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 113: /* float_list: %empty  */
#line 1832 "./gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = new vector<double>(0); }
#line 3582 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 114: /* float_list: float_list_head list_tail  */
#line 1834 "./gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = (yyvsp[-1].floatSetValue); }
#line 3588 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 115: /* float_list_head: FZ_FLOAT_LIT  */
#line 1838 "./gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = new vector<double>(1); (*(yyval.floatSetValue))[0] = (yyvsp[0].dValue); }
#line 3594 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 116: /* float_list_head: float_list_head ',' FZ_FLOAT_LIT  */
#line 1840 "./gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = (yyvsp[-2].floatSetValue); (yyval.floatSetValue)->push_back((yyvsp[0].dValue)); }
#line 3600 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 117: /* set_literal_list: %empty  */
#line 1844 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = new vector<AST::SetLit>(0); }
#line 3606 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 118: /* set_literal_list: set_literal_list_head list_tail  */
#line 1846 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = (yyvsp[-1].setValueList); }
#line 3612 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 119: /* set_literal_list_head: set_literal  */
#line 1850 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = new vector<AST::SetLit>(1); (*(yyval.setValueList))[0] = *(yyvsp[0].setLit); delete (yyvsp[0].setLit); }
#line 3618 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 120: /* set_literal_list_head: set_literal_list_head ',' set_literal  */
#line 1852 "./gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = (yyvsp[-2].setValueList); (yyval.setValueList)->push_back(*(yyvsp[0].setLit)); delete (yyvsp[0].setLit); }
#line 3624 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 121: /* flat_expr_list: flat_expr  */
#line 1860 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 3630 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 122: /* flat_expr_list: flat_expr_list ',' flat_expr  */
#line 1862 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 3636 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 123: /* flat_expr: non_array_expr  */
#line 1866 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 3642 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 124: /* flat_expr: '[' non_array_expr_list ']'  */
#line 1868 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-1].argVec); }
#line 3648 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 125: /* non_array_expr_opt: %empty  */
#line 1872 "./gecode/flatzinc/parser.yxx"
      { (yyval.oArg) = Option<AST::Node*>::none(); }
#line 3654 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 126: /* non_array_expr_opt: '=' non_array_expr  */
#line 1874 "./gecode/flatzinc/parser.yxx"
      { (yyval.oArg) = Option<AST::Node*>::some((yyvsp[0].arg)); }
#line 3660 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 127: /* non_array_expr: FZ_BOOL_LIT  */
#line 1878 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::BoolLit((yyvsp[0].iValue)); }
#line 3666 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 128: /* non_array_expr: FZ_INT_LIT  */
#line 1880 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::IntLit((yyvsp[0].iValue)); }
#line 3672 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 129: /* non_array_expr: FZ_FLOAT_LIT  */
#line 1882 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::FloatLit((yyvsp[0].dValue)); }
#line 3678 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 130: /* non_array_expr: set_literal  */
#line 1884 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].setLit); }
#line 3684 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 131: /* non_array_expr: var_par_id  */
#line 1886 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[0].sValue), e)) {
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
          case ST_FLOATVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::FloatVar(pp->arrays[e.i+i+1]);
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
          case ST_FLOATVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              int idx = pp->arrays[e.i+1];
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::FloatLit(pp->floatvals[idx+i]);
              (yyval.arg) = v;
            }
            break;
          case ST_INT:
            (yyval.arg) = new AST::IntLit(e.i);
            break;
          case ST_BOOL:
            (yyval.arg) = new AST::BoolLit(e.i);
            break;
          case ST_FLOAT:
            (yyval.arg) = new AST::FloatLit(pp->floatvals[e.i]);
            break;
          case ST_SET:
            (yyval.arg) = new AST::SetLit(pp->setvals[e.i]);
            break;
          default:
            (yyval.arg) = getVarRefArg(pp,(yyvsp[0].sValue));
          }
        } else {
          pp->err << "Error: undefined identifier " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.arg) = NULL;
        }
        free((yyvsp[0].sValue));
      }
#line 3784 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 132: /* non_array_expr: var_par_id '[' non_array_expr ']'  */
#line 1982 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        int i = -1;
        yyassert(pp, (yyvsp[-1].arg)->isInt(i), "Non-integer array index");
        if (!pp->hadError)
          (yyval.arg) = getArrayElement(static_cast<ParserState*>(parm),(yyvsp[-3].sValue),i,false);
        else
          (yyval.arg) = new AST::IntLit(0); // keep things consistent
        delete (yyvsp[-1].arg);
        free((yyvsp[-3].sValue));
      }
#line 3800 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 133: /* non_array_expr_list: %empty  */
#line 1996 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array(0); }
#line 3806 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 134: /* non_array_expr_list: non_array_expr_list_head list_tail  */
#line 1998 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-1].argVec); }
#line 3812 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 135: /* non_array_expr_list_head: non_array_expr  */
#line 2002 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 3818 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 136: /* non_array_expr_list_head: non_array_expr_list_head ',' non_array_expr  */
#line 2004 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 3824 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 137: /* solve_expr: var_par_id  */
#line 2012 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        bool haveSym = pp->symbols.get((yyvsp[0].sValue),e);
        if (haveSym) {
          switch (e.t) {
          case ST_INTVAR:
            (yyval.iValue) = e.i;
            break;
          case ST_FLOATVAR:
            (yyval.iValue) = -e.i-1;
            break;
          case ST_INT:
          case ST_FLOAT:
            pp->intvars.push_back(varspec("OBJ_CONST_INTRODUCED",
              new IntVarSpec(0,true,false)));
            (yyval.iValue) = pp->intvars.size()-1;
            break;
          default:
            pp->err << "Error: unknown int or float variable " << (yyvsp[0].sValue)
                    << " in line no. "
                    << yyget_lineno(pp->yyscanner) << std::endl;
            pp->hadError = true;
            break;
          }
        } else {
          pp->err << "Error: unknown int or float variable " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        }
        free((yyvsp[0].sValue));
      }
#line 3862 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 138: /* solve_expr: FZ_INT_LIT  */
#line 2046 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        pp->intvars.push_back(varspec("OBJ_CONST_INTRODUCED",
          new IntVarSpec(0,true,false)));
        (yyval.iValue) = pp->intvars.size()-1;
      }
#line 3873 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 139: /* solve_expr: FZ_FLOAT_LIT  */
#line 2053 "./gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        pp->intvars.push_back(varspec("OBJ_CONST_INTRODUCED",
          new IntVarSpec(0,true,false)));
        (yyval.iValue) = pp->intvars.size()-1;
      }
#line 3884 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 140: /* solve_expr: var_par_id '[' FZ_INT_LIT ']'  */
#line 2060 "./gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState *pp = static_cast<ParserState*>(parm);
        if ( (!pp->symbols.get((yyvsp[-3].sValue), e)) ||
             (e.t != ST_INTVARARRAY && e.t != ST_FLOATVARARRAY)) {
          pp->err << "Error: unknown int or float variable array " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        }
        if ((yyvsp[-1].iValue) == 0 || (yyvsp[-1].iValue) > pp->arrays[e.i]) {
          pp->err << "Error: array index out of bounds for array " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        } else {
          if (e.t == ST_INTVARARRAY)
            (yyval.iValue) = pp->arrays[e.i+(yyvsp[-1].iValue)];
          else
            (yyval.iValue) = -pp->arrays[e.i+(yyvsp[-1].iValue)]-1;
        }
        free((yyvsp[-3].sValue));
      }
#line 3912 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 143: /* annotations: %empty  */
#line 2094 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = NULL; }
#line 3918 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 144: /* annotations: annotations_head  */
#line 2096 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[0].argVec); }
#line 3924 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 145: /* annotations_head: FZ_COLONCOLON annotation  */
#line 2100 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 3930 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 146: /* annotations_head: annotations_head FZ_COLONCOLON annotation  */
#line 2102 "./gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 3936 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 147: /* annotation: FZ_ID '(' annotation_list ')'  */
#line 2106 "./gecode/flatzinc/parser.yxx"
      {
        (yyval.arg) = new AST::Call((yyvsp[-3].sValue), AST::extractSingleton((yyvsp[-1].arg))); free((yyvsp[-3].sValue));
      }
#line 3944 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 148: /* annotation: annotation_expr  */
#line 2110 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 3950 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 149: /* annotation_list: annotation  */
#line 2114 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::Array((yyvsp[0].arg)); }
#line 3956 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 150: /* annotation_list: annotation_list ',' annotation  */
#line 2116 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-2].arg); (yyval.arg)->append((yyvsp[0].arg)); }
#line 3962 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 151: /* annotation_expr: ann_non_array_expr  */
#line 2120 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 3968 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 152: /* annotation_expr: '[' ']'  */
#line 2122 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::Array(); }
#line 3974 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 153: /* annotation_expr: '[' annotation_list annotation_list_tail ']'  */
#line 2124 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-2].arg); }
#line 3980 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 156: /* ann_non_array_expr: FZ_BOOL_LIT  */
#line 2130 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::BoolLit((yyvsp[0].iValue)); }
#line 3986 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 157: /* ann_non_array_expr: FZ_INT_LIT  */
#line 2132 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::IntLit((yyvsp[0].iValue)); }
#line 3992 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 158: /* ann_non_array_expr: FZ_FLOAT_LIT  */
#line 2134 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::FloatLit((yyvsp[0].dValue)); }
#line 3998 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 159: /* ann_non_array_expr: set_literal  */
#line 2136 "./gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].setLit); }
#line 4004 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 160: /* ann_non_array_expr: var_par_id  */
#line 2138 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        bool gotSymbol = false;
        if (pp->symbols.get((yyvsp[0].sValue), e)) {
          gotSymbol = true;
          switch (e.t) {
          case ST_INTVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;) {
                std::ostringstream oss;
                oss << (yyvsp[0].sValue) << "["<<(i+1)<<"]";
                v->a[i] = new AST::IntVar(pp->arrays[e.i+i+1], oss.str());
              }
              (yyval.arg) = v;
            }
            break;
          case ST_BOOLVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;) {
                std::ostringstream oss;
                oss << (yyvsp[0].sValue) << "["<<(i+1)<<"]";
                v->a[i] = new AST::BoolVar(pp->arrays[e.i+i+1], oss.str());
              }
              (yyval.arg) = v;
            }
            break;
          case ST_FLOATVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;) {
                std::ostringstream oss;
                oss << (yyvsp[0].sValue) << "["<<(i+1)<<"]";
                v->a[i] = new AST::FloatVar(pp->arrays[e.i+i+1], oss.str());
              }
              (yyval.arg) = v;
            }
            break;
          case ST_SETVARARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              for (int i=pp->arrays[e.i]; i--;) {
                std::ostringstream oss;
                oss << (yyvsp[0].sValue) << "["<<(i+1)<<"]";
                v->a[i] = new AST::SetVar(pp->arrays[e.i+i+1], oss.str());
              }
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
          case ST_FLOATVALARRAY:
            {
              AST::Array *v = new AST::Array(pp->arrays[e.i]);
              int idx = pp->arrays[e.i+1];
              for (int i=pp->arrays[e.i]; i--;)
                v->a[i] = new AST::FloatLit(pp->floatvals[idx+i]);
              (yyval.arg) = v;
            }
            break;
          case ST_INT:
            (yyval.arg) = new AST::IntLit(e.i);
            break;
          case ST_BOOL:
            (yyval.arg) = new AST::BoolLit(e.i);
            break;
          case ST_FLOAT:
            (yyval.arg) = new AST::FloatLit(pp->floatvals[e.i]);
            break;
          case ST_SET:
            (yyval.arg) = new AST::SetLit(pp->setvals[e.i]);
            break;
          default:
            gotSymbol = false;
          }
        }
        if (!gotSymbol)
          (yyval.arg) = getVarRefArg(pp,(yyvsp[0].sValue),true);
        free((yyvsp[0].sValue));
      }
#line 4114 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 161: /* ann_non_array_expr: var_par_id '[' ann_non_array_expr ']'  */
#line 2244 "./gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        int i = -1;
        yyassert(pp, (yyvsp[-1].arg)->isInt(i), "Non-integer array index");
        if (!pp->hadError)
          (yyval.arg) = getArrayElement(static_cast<ParserState*>(parm),(yyvsp[-3].sValue),i,true);
        else
          (yyval.arg) = new AST::IntLit(0); // keep things consistent
        free((yyvsp[-3].sValue));
      }
#line 4129 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 162: /* ann_non_array_expr: FZ_STRING_LIT  */
#line 2255 "./gecode/flatzinc/parser.yxx"
      {
        (yyval.arg) = new AST::String((yyvsp[0].sValue));
        free((yyvsp[0].sValue));
      }
#line 4138 "gecode/flatzinc/parser.tab.cpp"
    break;


#line 4142 "gecode/flatzinc/parser.tab.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (parm, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, parm);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (parm, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, parm);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, parm);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

