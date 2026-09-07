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
#line 37 "gecode/flatzinc/parser.yxx"

#define YYPARSE_PARAM parm
#define YYLEX_PARAM static_cast<ParserState*>(parm)->yyscanner
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/parser.hh>
#include <gecode/flatzinc/lastval.hh>
#include <gecode/flatzinc/complete.hh>
#include <iostream>
#include <fstream>

#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

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
      case ST_BOOLVALARRAY:
        if(!pp->capture) break;
        if(offset>pp->arrays[e.i]) goto error;
        return new AST::BoolLit(pp->arrays[e.i+offset]);
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
  if (!dom()) {
    if(pp->capture) delete var;
    return;
  }
  if(pp->capture && pp->domainConstraints.size()>=pp->capture->options.max_constraints) {
    pp->capture->fail(*pp,Capture::Status::ResourceLimit,"declaration domain count exceeds capture limit");
    delete var;delete dom.some();dom=Option<AST::SetLit*>::none();return;
  }
  AST::Array* args = new AST::Array(2);
  args->a[0] = var;
  args->a[1] = dom.some();
  pp->domainConstraints.push_back(new ConExpr(id, args, NULL));
  if(pp->capture) dom=Option<AST::SetLit*>::none();
}

void addDomainConstraint(ParserState* pp, AST::Node* var,
                         Option<std::pair<double,double>* > dom) {
  if (!dom()) {
    if(pp->capture) delete var;
    return;
  }
  if(pp->capture && (pp->capture->options.max_constraints<2 ||
      pp->domainConstraints.size()>pp->capture->options.max_constraints-2)) {
    pp->capture->fail(*pp,Capture::Status::ResourceLimit,"declaration domain count exceeds capture limit");
    delete var;delete dom.some();dom=Option<std::pair<double,double>*>::none();return;
  }
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
  if(pp->capture) dom=Option<std::pair<double,double>*>::none();
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
  if (pp->capture) return; // capture never creates native variables/actors
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

AST::Node* arrayOutput(AST::Call* ann, bool capture=false) {
  AST::Node* dimensions=capture ? ann->args->getArray()->a.at(0) : ann->args;
  AST::Array* a = NULL;

  if (dimensions->isArray()) {
    a = dimensions->getArray();
  } else {
    a = new AST::Array(dimensions);
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

  if (!dimensions->isArray()) {
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


#line 669 "gecode/flatzinc/parser.tab.cpp"

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
  YYSYMBOL_60_1 = 60,                      /* $@1  */
  YYSYMBOL_preddecl_items = 61,            /* preddecl_items  */
  YYSYMBOL_preddecl_items_head = 62,       /* preddecl_items_head  */
  YYSYMBOL_vardecl_items = 63,             /* vardecl_items  */
  YYSYMBOL_vardecl_items_head = 64,        /* vardecl_items_head  */
  YYSYMBOL_constraint_items = 65,          /* constraint_items  */
  YYSYMBOL_constraint_items_head = 66,     /* constraint_items_head  */
  YYSYMBOL_preddecl_item = 67,             /* preddecl_item  */
  YYSYMBOL_pred_arg_list = 68,             /* pred_arg_list  */
  YYSYMBOL_pred_arg_list_head = 69,        /* pred_arg_list_head  */
  YYSYMBOL_pred_arg = 70,                  /* pred_arg  */
  YYSYMBOL_pred_arg_type = 71,             /* pred_arg_type  */
  YYSYMBOL_pred_arg_simple_type = 72,      /* pred_arg_simple_type  */
  YYSYMBOL_pred_array_init = 73,           /* pred_array_init  */
  YYSYMBOL_pred_array_init_arg = 74,       /* pred_array_init_arg  */
  YYSYMBOL_var_par_id = 75,                /* var_par_id  */
  YYSYMBOL_vardecl_item = 76,              /* vardecl_item  */
  YYSYMBOL_int_init = 77,                  /* int_init  */
  YYSYMBOL_int_init_list = 78,             /* int_init_list  */
  YYSYMBOL_int_init_list_head = 79,        /* int_init_list_head  */
  YYSYMBOL_list_tail = 80,                 /* list_tail  */
  YYSYMBOL_int_var_array_literal = 81,     /* int_var_array_literal  */
  YYSYMBOL_float_init = 82,                /* float_init  */
  YYSYMBOL_float_init_list = 83,           /* float_init_list  */
  YYSYMBOL_float_init_list_head = 84,      /* float_init_list_head  */
  YYSYMBOL_float_var_array_literal = 85,   /* float_var_array_literal  */
  YYSYMBOL_bool_init = 86,                 /* bool_init  */
  YYSYMBOL_bool_init_list = 87,            /* bool_init_list  */
  YYSYMBOL_bool_init_list_head = 88,       /* bool_init_list_head  */
  YYSYMBOL_bool_var_array_literal = 89,    /* bool_var_array_literal  */
  YYSYMBOL_set_init = 90,                  /* set_init  */
  YYSYMBOL_set_init_list = 91,             /* set_init_list  */
  YYSYMBOL_set_init_list_head = 92,        /* set_init_list_head  */
  YYSYMBOL_set_var_array_literal = 93,     /* set_var_array_literal  */
  YYSYMBOL_vardecl_int_var_array_init = 94, /* vardecl_int_var_array_init  */
  YYSYMBOL_vardecl_bool_var_array_init = 95, /* vardecl_bool_var_array_init  */
  YYSYMBOL_vardecl_float_var_array_init = 96, /* vardecl_float_var_array_init  */
  YYSYMBOL_vardecl_set_var_array_init = 97, /* vardecl_set_var_array_init  */
  YYSYMBOL_constraint_item = 98,           /* constraint_item  */
  YYSYMBOL_solve_item = 99,                /* solve_item  */
  YYSYMBOL_int_ti_expr_tail = 100,         /* int_ti_expr_tail  */
  YYSYMBOL_bool_ti_expr_tail = 101,        /* bool_ti_expr_tail  */
  YYSYMBOL_float_ti_expr_tail = 102,       /* float_ti_expr_tail  */
  YYSYMBOL_set_literal = 103,              /* set_literal  */
  YYSYMBOL_int_list = 104,                 /* int_list  */
  YYSYMBOL_int_list_head = 105,            /* int_list_head  */
  YYSYMBOL_bool_list = 106,                /* bool_list  */
  YYSYMBOL_bool_list_head = 107,           /* bool_list_head  */
  YYSYMBOL_float_list = 108,               /* float_list  */
  YYSYMBOL_float_list_head = 109,          /* float_list_head  */
  YYSYMBOL_set_literal_list = 110,         /* set_literal_list  */
  YYSYMBOL_set_literal_list_head = 111,    /* set_literal_list_head  */
  YYSYMBOL_flat_expr_list = 112,           /* flat_expr_list  */
  YYSYMBOL_flat_expr = 113,                /* flat_expr  */
  YYSYMBOL_non_array_expr_opt = 114,       /* non_array_expr_opt  */
  YYSYMBOL_non_array_expr = 115,           /* non_array_expr  */
  YYSYMBOL_non_array_expr_list = 116,      /* non_array_expr_list  */
  YYSYMBOL_non_array_expr_list_head = 117, /* non_array_expr_list_head  */
  YYSYMBOL_solve_expr = 118,               /* solve_expr  */
  YYSYMBOL_minmax = 119,                   /* minmax  */
  YYSYMBOL_annotations = 120,              /* annotations  */
  YYSYMBOL_annotations_head = 121,         /* annotations_head  */
  YYSYMBOL_annotation = 122,               /* annotation  */
  YYSYMBOL_annotation_list = 123,          /* annotation_list  */
  YYSYMBOL_annotation_expr = 124,          /* annotation_expr  */
  YYSYMBOL_annotation_list_tail = 125,     /* annotation_list_tail  */
  YYSYMBOL_ann_non_array_expr = 126        /* ann_non_array_expr  */
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
#define YYLAST   367

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  163
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  348

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
       0,   745,   745,   744,   749,   751,   754,   755,   757,   759,
     762,   764,   767,   769,   772,   774,   782,   785,   787,   790,
     791,   794,   798,   799,   800,   801,   804,   806,   808,   809,
     812,   813,   816,   817,   823,   823,   826,   861,   896,   939,
     975,   988,  1002,  1015,  1030,  1112,  1190,  1273,  1353,  1376,
    1398,  1420,  1445,  1449,  1466,  1490,  1491,  1495,  1497,  1500,
    1500,  1502,  1506,  1508,  1525,  1548,  1549,  1553,  1555,  1559,
    1563,  1565,  1582,  1605,  1606,  1610,  1612,  1615,  1618,  1620,
    1637,  1660,  1661,  1665,  1667,  1670,  1675,  1676,  1681,  1682,
    1687,  1688,  1693,  1694,  1698,  1867,  1884,  1912,  1914,  1916,
    1922,  1924,  1937,  1939,  1948,  1950,  1957,  1958,  1962,  1964,
    1969,  1970,  1974,  1976,  1981,  1982,  1986,  1988,  1993,  1994,
    1998,  2000,  2008,  2010,  2014,  2016,  2021,  2022,  2026,  2028,
    2030,  2032,  2034,  2130,  2145,  2146,  2150,  2152,  2160,  2201,
    2208,  2215,  2250,  2251,  2259,  2260,  2264,  2266,  2270,  2274,
    2278,  2280,  2284,  2286,  2288,  2291,  2291,  2294,  2296,  2298,
    2300,  2302,  2408,  2420
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
  "model", "$@1", "preddecl_items", "preddecl_items_head", "vardecl_items",
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

#define YYPACT_NINF (-118)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -10,    39,    27,   215,   -10,   -12,    -2,  -118,   104,    -4,
       7,    18,    20,    44,  -118,   215,    54,    60,  -118,   102,
      59,    91,  -118,  -118,  -118,    81,    28,    65,    73,    75,
     126,    85,    85,    85,   112,   128,   100,  -118,  -118,   207,
      97,  -118,  -118,   139,   171,   140,   138,  -118,   143,  -118,
    -118,   193,   198,     9,  -118,  -118,   147,   155,   156,    85,
      85,    85,   189,  -118,  -118,   195,   195,   195,   157,   206,
     173,   128,   166,  -118,  -118,    23,     9,  -118,   102,  -118,
     216,  -118,  -118,   174,  -118,   222,  -118,   226,   177,   195,
     195,   195,   230,    15,   190,   231,   196,   199,    85,   201,
     195,   200,   209,  -118,   236,  -118,   -11,  -118,  -118,  -118,
    -118,    85,  -118,  -118,  -118,   203,   203,   203,   205,   242,
    -118,  -118,   213,  -118,    50,   171,   211,  -118,  -118,  -118,
    -118,   163,    15,   163,   163,   195,    78,    30,  -118,  -118,
     262,    23,   234,   195,   163,  -118,  -118,  -118,   235,   267,
      15,  -118,  -118,   220,   217,   149,   242,  -118,  -118,   223,
    -118,  -118,  -118,  -118,  -118,   224,   163,   111,  -118,  -118,
    -118,  -118,  -118,    83,  -118,  -118,   135,   203,  -118,   232,
    -118,   137,    15,   221,  -118,   228,   163,   163,  -118,   229,
     227,   195,    78,  -118,  -118,   233,  -118,   207,  -118,  -118,
     109,   237,   238,   239,   248,  -118,    15,  -118,  -118,  -118,
     240,  -118,  -118,   163,  -118,  -118,  -118,   281,  -118,   255,
     241,   243,   244,    85,    85,    85,   260,  -118,  -118,   245,
       9,    85,    85,    85,   195,   195,   195,   246,  -118,   249,
     195,   195,   195,   247,   250,   251,    85,    85,   252,   254,
     256,   257,   259,   261,   195,   195,   263,  -118,   264,  -118,
     265,  -118,   288,   292,   171,   258,   266,   170,  -118,    93,
    -118,   176,  -118,   268,   156,  -118,   269,   253,   270,   272,
     273,  -118,  -118,   274,  -118,   275,   277,  -118,   278,  -118,
     276,   282,  -118,   279,  -118,   280,   284,  -118,  -118,  -118,
     295,  -118,  -118,    21,    10,  -118,   300,  -118,   170,  -118,
     312,  -118,    93,  -118,   316,  -118,   176,  -118,  -118,   242,
    -118,   283,   285,   286,  -118,   287,   289,  -118,   290,  -118,
     291,  -118,   293,  -118,  -118,    21,  -118,   317,  -118,    10,
    -118,  -118,  -118,  -118,  -118,   294,  -118,  -118
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     8,     5,     0,     0,     1,     0,     0,
       0,     0,     0,     0,     2,     9,     0,     0,     6,    17,
       0,     0,   100,   102,    97,     0,   106,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,    10,     7,     0,
       0,    28,    29,     0,   106,     0,    59,    19,     0,    25,
      26,     0,     0,     0,   108,   112,     0,    59,    59,     0,
       0,     0,     0,    34,    35,   144,   144,   144,     0,     0,
       0,    13,     0,    11,    24,     0,     0,    16,    60,    18,
       0,    99,   103,     0,    98,    60,   107,    60,     0,   144,
     144,   144,     0,     0,     0,   145,     0,     0,     0,     0,
     144,     0,     0,    14,     0,    32,     0,    30,    27,    20,
      21,     0,   109,   113,   101,   126,   126,   126,     0,   158,
     157,   159,    34,   163,     0,   106,   161,   160,   146,   149,
     152,     0,     0,     0,     0,   144,     0,     0,     3,    15,
       0,     0,     0,   144,     0,    36,    37,    38,     0,     0,
       0,   153,   150,   155,     0,     0,   129,   128,   130,   132,
     131,    42,   147,    41,    40,     0,   134,     0,   122,   124,
     143,   142,    95,     0,    33,    31,     0,   126,   127,     0,
     105,     0,   156,     0,   104,     0,     0,     0,   136,     0,
      59,   144,     0,   139,   140,   138,    96,     0,    22,    39,
       0,     0,     0,     0,     0,   148,     0,   151,   154,   162,
       0,    43,   125,    60,   135,    94,   123,     0,    23,     0,
       0,     0,     0,     0,     0,     0,     0,   133,   137,     0,
       0,     0,     0,     0,   144,   144,   144,     0,   141,     0,
     144,   144,   144,     0,     0,     0,     0,     0,    86,    88,
      90,     0,     0,     0,   144,   144,     0,    44,     0,    45,
       0,    46,   110,   114,   106,     0,    92,    55,    87,    73,
      89,    65,    91,     0,    59,   116,     0,    59,     0,     0,
       0,    47,    52,    53,    57,     0,    59,    70,    71,    75,
       0,    59,    62,    63,    67,     0,    59,    49,   111,    50,
      60,   115,    48,   118,    81,    93,     0,    61,    60,    56,
       0,    77,    60,    74,     0,    69,    60,    66,   117,     0,
     120,     0,    59,    79,    83,     0,    59,    78,     0,    58,
       0,    76,     0,    68,    51,    60,   119,     0,    85,    60,
      82,    54,    72,    64,   121,     0,    84,    80
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,   334,
    -118,  -118,   271,  -118,   -33,  -118,   202,   -31,   327,    38,
    -118,  -118,   -54,  -118,    34,  -118,  -118,  -118,    40,  -118,
    -118,  -118,    12,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
     296,  -118,    -3,   153,   154,   -86,  -117,  -118,  -118,    94,
    -118,  -118,  -118,  -118,  -118,   165,  -102,   -92,  -118,  -118,
    -118,  -118,   -56,  -118,   -84,   208,  -118,  -118,   204
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,    35,     3,     4,    14,    15,    70,    71,     5,
      45,    46,    47,    48,    49,   106,   107,   159,    16,   284,
     285,   286,    79,   268,   294,   295,   296,   272,   289,   290,
     291,   270,   324,   325,   326,   305,   257,   259,   261,   281,
      72,   101,    50,    28,    29,   160,    56,    57,   273,    58,
     276,   277,   321,   322,   167,   168,   145,   169,   189,   190,
     196,   173,    94,    95,   152,   153,   129,   183,   130
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,    66,    67,    86,    88,    27,    74,   127,   154,   128,
      96,    97,    20,   319,   146,   147,    63,    64,   119,   120,
     121,   122,    64,   123,   319,     1,   104,     7,    89,    90,
      91,    54,    55,   115,   116,   117,    18,    24,   127,   161,
     141,   163,   164,   142,   137,     6,   127,    19,   162,    30,
      83,   105,   178,   119,   120,   121,   122,    64,   123,    31,
     170,   171,   126,   172,   127,    44,   125,   135,   124,   127,
      32,   125,    33,   108,   188,   199,    34,   125,    51,   165,
     143,   156,   157,   158,    63,    64,   193,   177,   194,    63,
      64,    63,    64,   126,   210,   211,   127,   287,   207,    63,
      64,   126,    37,   124,   151,    20,   125,    20,    38,    21,
      52,    39,    20,    53,    21,    40,    41,    59,    22,   126,
     127,   228,   207,    22,   126,    60,    42,    61,    23,    62,
      24,   166,    24,    23,   125,   215,   214,    24,    20,    43,
      68,    25,   195,   198,   197,    69,   219,   278,    73,    41,
      75,   126,   119,   120,   121,    63,    64,   123,    44,    42,
      26,   191,   192,    24,   218,    26,   156,   157,   158,    63,
      64,    76,    43,   282,    54,   126,    63,    64,   243,   244,
     245,   292,    63,    64,   248,   249,   250,   205,   206,    78,
      77,    44,   234,   235,   236,    80,    81,   220,   265,   266,
     240,   241,   242,    82,    84,   125,    85,    87,    92,    98,
      20,    93,    99,   100,   103,   254,   255,   320,   327,   125,
     298,    41,   110,   301,     8,   112,   111,   239,     9,    10,
     113,    42,   309,   118,   114,    24,   283,   313,   288,    11,
     293,   200,   317,    12,    43,   131,   201,   132,   138,   344,
     136,   133,    13,   327,   134,   140,   202,   139,   144,   148,
     203,   149,   150,    44,   155,   174,   176,   179,   336,   204,
     180,   182,   340,   323,   184,   208,   186,   283,   213,   187,
     226,   288,   209,   212,   229,   293,   217,   230,   237,   223,
     224,   225,    55,   231,   227,   232,   233,   275,   246,   238,
     318,   247,   251,   328,   300,   252,   253,   256,   323,   258,
     262,   260,   263,   279,   264,   330,   267,   269,   271,   332,
     345,   280,   297,   299,   302,   303,   304,   306,   308,   307,
     311,   310,   314,   312,   315,   316,   335,   334,    17,   337,
     339,   338,    36,   175,   341,   342,   329,   343,   347,   109,
     333,   346,   331,   221,   222,     0,   274,   216,   181,   185,
       0,     0,     0,     0,     0,     0,     0,   102
};

static const yytype_int16 yycheck[] =
{
      31,    32,    33,    57,    58,     8,    39,    93,   125,    93,
      66,    67,     3,     3,   116,   117,     6,     7,     3,     4,
       5,     6,     7,     8,     3,    35,     3,     0,    59,    60,
      61,     3,     4,    89,    90,    91,    48,    28,   124,   131,
      51,   133,   134,    54,   100,     6,   132,    49,   132,    53,
      53,    28,   144,     3,     4,     5,     6,     7,     8,    52,
      30,    31,    93,    33,   150,    56,    56,    98,    53,   155,
      52,    56,    52,    76,   166,   177,    32,    56,    19,   135,
     111,     3,     4,     5,     6,     7,     3,   143,     5,     6,
       7,     6,     7,   124,   186,   187,   182,     4,   182,     6,
       7,   132,    48,    53,    54,     3,    56,     3,    48,     5,
      19,     9,     3,    32,     5,    13,    14,    52,    14,   150,
     206,   213,   206,    14,   155,    52,    24,    52,    24,     3,
      28,    53,    28,    24,    56,   191,   190,    28,     3,    37,
      28,    37,   173,   176,     9,    17,    37,   264,    48,    14,
      53,   182,     3,     4,     5,     6,     7,     8,    56,    24,
      56,    50,    51,    28,   197,    56,     3,     4,     5,     6,
       7,    32,    37,     3,     3,   206,     6,     7,   234,   235,
     236,     5,     6,     7,   240,   241,   242,    50,    51,    51,
      50,    56,   223,   224,   225,    52,     3,   200,   254,   255,
     231,   232,   233,     5,    57,    56,    51,    51,    19,    52,
       3,    16,     6,    40,    48,   246,   247,   303,   304,    56,
     274,    14,     6,   277,     9,     3,    52,   230,    13,    14,
       4,    24,   286,     3,    57,    28,   267,   291,   269,    24,
     271,     9,   296,    28,    37,    55,    14,    16,    48,   335,
      49,    55,    37,   339,    55,    19,    24,    48,    55,    54,
      28,    19,    49,    56,    53,     3,    32,    32,   322,    37,
       3,    51,   326,   304,    57,    54,    53,   308,    51,    55,
      32,   312,    54,    54,     3,   316,    53,    32,    28,    52,
      52,    52,     4,    52,    54,    52,    52,     5,    52,    54,
       5,    52,    55,     3,    51,    55,    55,    55,   339,    55,
      53,    55,    53,    55,    53,     3,    53,    53,    53,     3,
       3,    55,    54,    54,    54,    53,    53,    53,    51,    54,
      54,    53,    53,    51,    54,    51,    51,    54,     4,    53,
      51,    54,    15,   141,    54,    54,   308,    54,    54,    78,
     316,   339,   312,   200,   200,    -1,   262,   192,   150,   155,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    35,    59,    61,    62,    67,     6,     0,     9,    13,
      14,    24,    28,    37,    63,    64,    76,    67,    48,    49,
       3,     5,    14,    24,    28,    37,    56,   100,   101,   102,
      53,    52,    52,    52,    32,    60,    76,    48,    48,     9,
      13,    14,    24,    37,    56,    68,    69,    70,    71,    72,
     100,    19,    19,    32,     3,     4,   104,   105,   107,    52,
      52,    52,     3,     6,     7,    75,    75,    75,    28,    17,
      65,    66,    98,    48,    72,    53,    32,    50,    51,    80,
      52,     3,     5,   100,    57,    51,    80,    51,    80,    75,
      75,    75,    19,    16,   120,   121,   120,   120,    52,     6,
      40,    99,    98,    48,     3,    28,    73,    74,   100,    70,
       6,    52,     3,     4,    57,   120,   120,   120,     3,     3,
       4,     5,     6,     8,    53,    56,    75,   103,   122,   124,
     126,    55,    16,    55,    55,    75,    49,   120,    48,    48,
      19,    51,    54,    75,    55,   114,   114,   114,    54,    19,
      49,    54,   122,   123,   104,    53,     3,     4,     5,    75,
     103,   115,   122,   115,   115,   120,    53,   112,   113,   115,
      30,    31,    33,   119,     3,    74,    32,   120,   115,    32,
       3,   123,    51,   125,    57,   126,    53,    55,   115,   116,
     117,    50,    51,     3,     5,    75,   118,     9,    72,   114,
       9,    14,    24,    28,    37,    50,    51,   122,    54,    54,
     115,   115,    54,    51,    80,   120,   113,    53,    72,    37,
     100,   101,   102,    52,    52,    52,    32,    54,   115,     3,
      32,    52,    52,    52,    75,    75,    75,    28,    54,   100,
      75,    75,    75,   120,   120,   120,    52,    52,   120,   120,
     120,    55,    55,    55,    75,    75,    55,    94,    55,    95,
      55,    96,    53,    53,    53,   120,   120,    53,    81,    53,
      89,    53,    85,   106,   107,     5,   108,   109,   104,    55,
      55,    97,     3,    75,    77,    78,    79,     4,    75,    86,
      87,    88,     5,    75,    82,    83,    84,    54,    80,    54,
      51,    80,    54,    53,    53,    93,    53,    54,    51,    80,
      53,    54,    51,    80,    53,    54,    51,    80,     5,     3,
     103,   110,   111,    75,    90,    91,    92,   103,     3,    77,
       3,    86,     3,    82,    54,    51,    80,    53,    54,    51,
      80,    54,    54,    54,   103,     3,    90,    54
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    58,    60,    59,    61,    61,    62,    62,    63,    63,
      64,    64,    65,    65,    66,    66,    67,    68,    68,    69,
      69,    70,    71,    71,    71,    71,    72,    72,    72,    72,
      73,    73,    74,    74,    75,    75,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    77,    77,    77,    78,    78,    79,    79,    80,
      80,    81,    82,    82,    82,    83,    83,    84,    84,    85,
      86,    86,    86,    87,    87,    88,    88,    89,    90,    90,
      90,    91,    91,    92,    92,    93,    94,    94,    95,    95,
      96,    96,    97,    97,    98,    99,    99,   100,   100,   100,
     101,   101,   102,   102,   103,   103,   104,   104,   105,   105,
     106,   106,   107,   107,   108,   108,   109,   109,   110,   110,
     111,   111,   112,   112,   113,   113,   114,   114,   115,   115,
     115,   115,   115,   115,   116,   116,   117,   117,   118,   118,
     118,   118,   119,   119,   120,   120,   121,   121,   122,   122,
     123,   123,   124,   124,   124,   125,   125,   126,   126,   126,
     126,   126,   126,   126
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     6,     0,     1,     2,     3,     0,     1,
       2,     3,     0,     1,     2,     3,     5,     0,     2,     1,
       3,     3,     6,     7,     2,     1,     1,     3,     1,     1,
       1,     3,     1,     3,     1,     1,     6,     6,     6,     8,
       6,     6,     6,     8,    13,    13,    13,    15,    15,    15,
      15,    17,     1,     1,     4,     0,     2,     1,     3,     0,
       1,     3,     1,     1,     4,     0,     2,     1,     3,     3,
       1,     1,     4,     0,     2,     1,     3,     3,     1,     1,
       4,     0,     2,     1,     3,     3,     0,     2,     0,     2,
       0,     2,     0,     2,     6,     3,     4,     1,     3,     3,
       1,     4,     1,     3,     3,     3,     0,     2,     1,     3,
       0,     2,     1,     3,     0,     2,     1,     3,     0,     2,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     1,
       1,     1,     1,     4,     0,     2,     1,     3,     1,     1,
       1,     4,     1,     1,     0,     1,     2,     3,     4,     1,
       1,     3,     1,     2,     4,     0,     1,     1,     1,     1,
       1,     1,     4,     1
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
  switch (yykind)
    {
    case YYSYMBOL_FZ_ID: /* FZ_ID  */
#line 732 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) free(((*yyvaluep).sValue)); }
#line 2045 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_FZ_U_ID: /* FZ_U_ID  */
#line 732 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) free(((*yyvaluep).sValue)); }
#line 2051 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_FZ_STRING_LIT: /* FZ_STRING_LIT  */
#line 732 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) free(((*yyvaluep).sValue)); }
#line 2057 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_var_par_id: /* var_par_id  */
#line 732 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) free(((*yyvaluep).sValue)); }
#line 2063 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_init: /* int_init  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).varSpec); }
#line 2069 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_init_list: /* int_init_list  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2075 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_init_list_head: /* int_init_list_head  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2081 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_var_array_literal: /* int_var_array_literal  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2087 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_init: /* float_init  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).varSpec); }
#line 2093 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_init_list: /* float_init_list  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2099 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_init_list_head: /* float_init_list_head  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2105 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_var_array_literal: /* float_var_array_literal  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2111 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_init: /* bool_init  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).varSpec); }
#line 2117 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_init_list: /* bool_init_list  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2123 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_init_list_head: /* bool_init_list_head  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2129 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_var_array_literal: /* bool_var_array_literal  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2135 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_init: /* set_init  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).varSpec); }
#line 2141 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_init_list: /* set_init_list  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2147 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_init_list_head: /* set_init_list_head  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2153 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_var_array_literal: /* set_var_array_literal  */
#line 735 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) { for(auto* p:*((*yyvaluep).varSpecVec)) delete p; delete ((*yyvaluep).varSpecVec); } }
#line 2159 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_vardecl_int_var_array_init: /* vardecl_int_var_array_init  */
#line 736 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oVarSpecVec)()) { for(auto* p:*((*yyvaluep).oVarSpecVec).some()) delete p; delete ((*yyvaluep).oVarSpecVec).some(); } }
#line 2165 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_vardecl_bool_var_array_init: /* vardecl_bool_var_array_init  */
#line 736 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oVarSpecVec)()) { for(auto* p:*((*yyvaluep).oVarSpecVec).some()) delete p; delete ((*yyvaluep).oVarSpecVec).some(); } }
#line 2171 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_vardecl_float_var_array_init: /* vardecl_float_var_array_init  */
#line 736 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oVarSpecVec)()) { for(auto* p:*((*yyvaluep).oVarSpecVec).some()) delete p; delete ((*yyvaluep).oVarSpecVec).some(); } }
#line 2177 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_vardecl_set_var_array_init: /* vardecl_set_var_array_init  */
#line 736 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oVarSpecVec)()) { for(auto* p:*((*yyvaluep).oVarSpecVec).some()) delete p; delete ((*yyvaluep).oVarSpecVec).some(); } }
#line 2183 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_ti_expr_tail: /* int_ti_expr_tail  */
#line 734 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oSet)()) delete ((*yyvaluep).oSet).some(); }
#line 2189 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_ti_expr_tail: /* bool_ti_expr_tail  */
#line 734 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oSet)()) delete ((*yyvaluep).oSet).some(); }
#line 2195 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_ti_expr_tail: /* float_ti_expr_tail  */
#line 734 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oPFloat)()) delete ((*yyvaluep).oPFloat).some(); }
#line 2201 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_literal: /* set_literal  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setLit); }
#line 2207 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_list: /* int_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValue); }
#line 2213 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_int_list_head: /* int_list_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValue); }
#line 2219 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_list: /* bool_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValue); }
#line 2225 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_bool_list_head: /* bool_list_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValue); }
#line 2231 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_list: /* float_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).floatSetValue); }
#line 2237 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_float_list_head: /* float_list_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).floatSetValue); }
#line 2243 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_literal_list: /* set_literal_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValueList); }
#line 2249 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_set_literal_list_head: /* set_literal_list_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).setValueList); }
#line 2255 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_flat_expr_list: /* flat_expr_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).argVec); }
#line 2261 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_flat_expr: /* flat_expr  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2267 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_non_array_expr_opt: /* non_array_expr_opt  */
#line 734 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture && ((*yyvaluep).oArg)()) delete ((*yyvaluep).oArg).some(); }
#line 2273 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_non_array_expr: /* non_array_expr  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2279 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_non_array_expr_list: /* non_array_expr_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).argVec); }
#line 2285 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_non_array_expr_list_head: /* non_array_expr_list_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).argVec); }
#line 2291 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_annotations: /* annotations  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).argVec); }
#line 2297 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_annotations_head: /* annotations_head  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).argVec); }
#line 2303 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_annotation: /* annotation  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2309 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_annotation_list: /* annotation_list  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2315 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_annotation_expr: /* annotation_expr  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2321 "gecode/flatzinc/parser.tab.cpp"
        break;

    case YYSYMBOL_ann_non_array_expr: /* ann_non_array_expr  */
#line 733 "gecode/flatzinc/parser.yxx"
            { if(static_cast<ParserState*>(parm)->capture) delete ((*yyvaluep).arg); }
#line 2327 "gecode/flatzinc/parser.tab.cpp"
        break;

      default:
        break;
    }
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
  case 2: /* $@1: %empty  */
#line 745 "gecode/flatzinc/parser.yxx"
      { ParserState* pp=static_cast<ParserState*>(parm);
        if(pp->capture && !pp->hadError) pp->capture->begin(*pp); }
#line 2607 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 10: /* vardecl_items_head: vardecl_item ';'  */
#line 763 "gecode/flatzinc/parser.yxx"
      { if(static_cast<ParserState*>(parm)->capture && static_cast<ParserState*>(parm)->hadError) YYABORT; }
#line 2613 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 11: /* vardecl_items_head: vardecl_items_head vardecl_item ';'  */
#line 765 "gecode/flatzinc/parser.yxx"
      { if(static_cast<ParserState*>(parm)->capture && static_cast<ParserState*>(parm)->hadError) YYABORT; }
#line 2619 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 14: /* constraint_items_head: constraint_item ';'  */
#line 773 "gecode/flatzinc/parser.yxx"
      { if(static_cast<ParserState*>(parm)->capture && static_cast<ParserState*>(parm)->hadError) YYABORT; }
#line 2625 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 15: /* constraint_items_head: constraint_items_head constraint_item ';'  */
#line 775 "gecode/flatzinc/parser.yxx"
      { if(static_cast<ParserState*>(parm)->capture && static_cast<ParserState*>(parm)->hadError) YYABORT; }
#line 2631 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 16: /* preddecl_item: FZ_PREDICATE FZ_ID '(' pred_arg_list ')'  */
#line 783 "gecode/flatzinc/parser.yxx"
      { free((yyvsp[-3].sValue)); }
#line 2637 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 21: /* pred_arg: pred_arg_type ':' FZ_ID  */
#line 795 "gecode/flatzinc/parser.yxx"
      { free((yyvsp[0].sValue)); }
#line 2643 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 26: /* pred_arg_simple_type: int_ti_expr_tail  */
#line 805 "gecode/flatzinc/parser.yxx"
      { if ((yyvsp[0].oSet)()) delete (yyvsp[0].oSet).some(); }
#line 2649 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 27: /* pred_arg_simple_type: FZ_SET FZ_OF int_ti_expr_tail  */
#line 807 "gecode/flatzinc/parser.yxx"
      { if ((yyvsp[0].oSet)()) delete (yyvsp[0].oSet).some(); }
#line 2655 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 36: /* vardecl_item: FZ_VAR int_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 827 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec));
        if(pp->capture) pp->capture->variable_count(*pp,1);
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
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
        if(pp->capture && pp->hadError && (yyvsp[0].oArg)() && (yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2694 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 37: /* vardecl_item: FZ_VAR bool_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 862 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec));
        if(pp->capture) pp->capture->variable_count(*pp,1);
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
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
        if(pp->capture && pp->hadError && (yyvsp[0].oArg)() && (yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2733 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 38: /* vardecl_item: FZ_VAR float_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 897 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec));
        if(pp->capture) pp->capture->variable_count(*pp,1);
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
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
            if(pp->capture) (yyvsp[-4].oPFloat)=Option<std::pair<double,double>*>::none();
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
        if(pp->capture && pp->hadError && (yyvsp[0].oArg)() && (yyvsp[-4].oPFloat)()) delete (yyvsp[-4].oPFloat).some();
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2780 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 39: /* vardecl_item: FZ_VAR FZ_SET FZ_OF int_ti_expr_tail ':' var_par_id annotations non_array_expr_opt  */
#line 940 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec));
        if(pp->capture) pp->capture->variable_count(*pp,1);
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasAtom("output_var");
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
        if(pp->capture && pp->hadError && (yyvsp[0].oArg)() && (yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2820 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 40: /* vardecl_item: FZ_INT ':' var_par_id annotations '=' non_array_expr  */
#line 976 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-2].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[0].arg)->isInt(), "Invalid int initializer");
        if (!pp->capture || !pp->hadError) {
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_i((yyvsp[0].arg)->getInt())),
          "Duplicate symbol");
        }
        if(pp->capture) delete (yyvsp[0].arg);
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2837 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 41: /* vardecl_item: FZ_FLOAT ':' var_par_id annotations '=' non_array_expr  */
#line 989 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-2].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[0].arg)->isFloat(), "Invalid float initializer");
        if (!pp->capture || !pp->hadError) {
        pp->floatvals.push_back((yyvsp[0].arg)->getFloat());
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_f(pp->floatvals.size()-1)),
          "Duplicate symbol");
        }
        if(pp->capture) delete (yyvsp[0].arg);
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2855 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 42: /* vardecl_item: FZ_BOOL ':' var_par_id annotations '=' non_array_expr  */
#line 1003 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-2].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[0].arg)->isBool(), "Invalid bool initializer");
        if (!pp->capture || !pp->hadError) {
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_b((yyvsp[0].arg)->getBool())),
          "Duplicate symbol");
        }
        if(pp->capture) delete (yyvsp[0].arg);
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2872 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 43: /* vardecl_item: FZ_SET FZ_OF FZ_INT ':' var_par_id annotations '=' non_array_expr  */
#line 1016 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-2].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[0].arg)->isSet(), "Invalid set initializer");
        if (!pp->capture || !pp->hadError) {
        AST::SetLit* set = (yyvsp[0].arg)->getSet();
        pp->setvals.push_back(*set);
        yyassert(pp,
          pp->symbols.put((yyvsp[-3].sValue), se_s(pp->setvals.size()-1)),
          "Duplicate symbol");
        delete set;
        } else delete (yyvsp[0].arg);
        delete (yyvsp[-2].argVec); free((yyvsp[-3].sValue));
      }
#line 2891 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 44: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR int_ti_expr_tail ':' var_par_id annotations vardecl_int_var_array_init  */
#line 1032 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if(pp->capture) { pp->capture->array_size(*pp,(yyvsp[-8].iValue));
          pp->capture->variable_count(*pp,(yyvsp[-8].iValue),(yyvsp[0].oVarSpecVec)());
        }
        if (!pp->hadError) {
          if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec),(yyvsp[-8].iValue));
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
          vector<int> vars((yyvsp[-8].iValue));
          if (!pp->hadError) {
            if ((yyvsp[0].oVarSpecVec)()) {
              vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
              yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                       "Initializer size does not match array dimension");
              if (!pp->hadError) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
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
                    if(pp->capture) (*vsv)[i]=nullptr;
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
              if(pp->capture) { for(auto* child:*vsv) delete child; (yyvsp[0].oVarSpecVec)=Option<vector<VarSpec*>*>::none(); }
              delete vsv;
            } else {
              if ((yyvsp[-8].iValue)>0) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
                  Option<AST::SetLit*> dom =
                    (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                         : Option<AST::SetLit*>::none();
                  IntVarSpec* ispec = new IntVarSpec(dom,!print,false);
                  vars[i] = pp->intvars.size();
                  pp->intvars.push_back(varspec((yyvsp[-2].sValue), ispec));
                }
              }
              if ((yyvsp[-4].oSet)()) { delete (yyvsp[-4].oSet).some(); if(pp->capture) (yyvsp[-4].oSet)=Option<AST::SetLit*>::none(); }
            }
          }
          if (print && (!pp->capture || !pp->hadError)) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array"),pp->capture!=nullptr));
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
        if(pp->capture) {
          if((yyvsp[0].oVarSpecVec)()) { for(auto* child:*(yyvsp[0].oVarSpecVec).some()) delete child; delete (yyvsp[0].oVarSpecVec).some(); }
          if((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 2976 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 45: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR bool_ti_expr_tail ':' var_par_id annotations vardecl_bool_var_array_init  */
#line 1114 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec),(yyvsp[-8].iValue));
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if(pp->capture) { pp->capture->array_size(*pp,(yyvsp[-8].iValue));
          pp->capture->variable_count(*pp,(yyvsp[-8].iValue),(yyvsp[0].oVarSpecVec)());
        }
        if (!pp->hadError) {
          vector<int> vars((yyvsp[-8].iValue));
          if ((yyvsp[0].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
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
                    if(pp->capture) (*vsv)[i]=nullptr;
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
            if(pp->capture) { for(auto* child:*vsv) delete child; (yyvsp[0].oVarSpecVec)=Option<vector<VarSpec*>*>::none(); }
              delete vsv;
          } else {
            for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
              Option<AST::SetLit*> dom =
                (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                     : Option<AST::SetLit*>::none();
              vars[i] = pp->boolvars.size();
              pp->boolvars.push_back(varspec((yyvsp[-2].sValue),
                                       new BoolVarSpec(dom,!print,false)));
            }
            if ((yyvsp[-4].oSet)()) { delete (yyvsp[-4].oSet).some(); if(pp->capture) (yyvsp[-4].oSet)=Option<AST::SetLit*>::none(); }
          }
          if (print && (!pp->capture || !pp->hadError)) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array"),pp->capture!=nullptr));
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
        if(pp->capture) {
          if((yyvsp[0].oVarSpecVec)()) { for(auto* child:*(yyvsp[0].oVarSpecVec).some()) delete child; delete (yyvsp[0].oVarSpecVec).some(); }
          if((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 3057 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 46: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR float_ti_expr_tail ':' var_par_id annotations vardecl_float_var_array_init  */
#line 1193 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        yyassert(pp, (yyvsp[-10].iValue)==1, "Arrays must start at 1");
        if(pp->capture) { pp->capture->array_size(*pp,(yyvsp[-8].iValue));
          pp->capture->variable_count(*pp,(yyvsp[-8].iValue),(yyvsp[0].oVarSpecVec)());
        }
        if (!pp->hadError) {
          if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec),(yyvsp[-8].iValue));
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
          vector<int> vars((yyvsp[-8].iValue));
          if (!pp->hadError) {
            if ((yyvsp[0].oVarSpecVec)()) {
              vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
              yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-8].iValue)),
                       "Initializer size does not match array dimension");
              if (!pp->hadError) {
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
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
                    if(pp->capture) (*vsv)[i]=nullptr;
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
              if(pp->capture) { for(auto* child:*vsv) delete child; (yyvsp[0].oVarSpecVec)=Option<vector<VarSpec*>*>::none(); }
              delete vsv;
            } else {
              if ((yyvsp[-8].iValue)>0) {
                Option<std::pair<double,double> > dom =
                  (yyvsp[-4].oPFloat)() ? Option<std::pair<double,double> >::some(*(yyvsp[-4].oPFloat).some())
                       : Option<std::pair<double,double> >::none();
                for (int i=0; i<(yyvsp[-8].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
                  FloatVarSpec* ispec = new FloatVarSpec(dom,!print,false);
                  vars[i] = pp->floatvars.size();
                  pp->floatvars.push_back(varspec((yyvsp[-2].sValue), ispec));
                }
              }
            }
          }
          if (print && (!pp->capture || !pp->hadError)) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array"),pp->capture!=nullptr));
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
        if ((yyvsp[-4].oPFloat)()) { delete (yyvsp[-4].oPFloat).some(); if(pp->capture) (yyvsp[-4].oPFloat)=Option<std::pair<double,double>*>::none(); }
        if(pp->capture) {
          if((yyvsp[0].oVarSpecVec)()) { for(auto* child:*(yyvsp[0].oVarSpecVec).some()) delete child; delete (yyvsp[0].oVarSpecVec).some(); }
          if((yyvsp[-4].oPFloat)()) delete (yyvsp[-4].oPFloat).some();
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 3142 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 47: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_VAR FZ_SET FZ_OF int_ti_expr_tail ':' var_par_id annotations vardecl_set_var_array_init  */
#line 1275 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture) pp->capture->declaration(*pp,(yyvsp[-2].sValue),(yyvsp[-1].argVec),(yyvsp[-10].iValue));
        bool print = (!pp->capture || !pp->hadError) && (yyvsp[-1].argVec) != NULL && (yyvsp[-1].argVec)->hasCall("output_array");
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        if(pp->capture) { pp->capture->array_size(*pp,(yyvsp[-10].iValue));
          pp->capture->variable_count(*pp,(yyvsp[-10].iValue),(yyvsp[0].oVarSpecVec)());
        }
        if (!pp->hadError) {
          vector<int> vars((yyvsp[-10].iValue));
          if ((yyvsp[0].oVarSpecVec)()) {
            vector<VarSpec*>* vsv = (yyvsp[0].oVarSpecVec).some();
            yyassert(pp, vsv->size() == static_cast<unsigned int>((yyvsp[-10].iValue)),
                     "Initializer size does not match array dimension");
            if (!pp->hadError) {
              for (int i=0; i<(yyvsp[-10].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
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
                    if(pp->capture) (*vsv)[i]=nullptr;
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
            if(pp->capture) { for(auto* child:*vsv) delete child; (yyvsp[0].oVarSpecVec)=Option<vector<VarSpec*>*>::none(); }
              delete vsv;
          } else {
            if ((yyvsp[-10].iValue)>0) {
              for (int i=0; i<(yyvsp[-10].iValue); i++) {
                  if(pp->capture && pp->hadError) break;
                Option<AST::SetLit*> dom =
                  (yyvsp[-4].oSet)() ? Option<AST::SetLit*>::some(new AST::SetLit((yyvsp[-4].oSet).some()))
                        : Option<AST::SetLit*>::none();
                SetVarSpec* ispec = new SetVarSpec(dom,!print,false);
                vars[i] = pp->setvars.size();
                pp->setvars.push_back(varspec((yyvsp[-2].sValue), ispec));
              }
              if ((yyvsp[-4].oSet)()) { delete (yyvsp[-4].oSet).some(); if(pp->capture) (yyvsp[-4].oSet)=Option<AST::SetLit*>::none(); }
            }
          }
          if (print && (!pp->capture || !pp->hadError)) {
            AST::Array* a = new AST::Array();
            a->a.push_back(arrayOutput((yyvsp[-1].argVec)->getCall("output_array"),pp->capture!=nullptr));
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
        if(pp->capture) {
          if((yyvsp[0].oVarSpecVec)()) { for(auto* child:*(yyvsp[0].oVarSpecVec).some()) delete child; delete (yyvsp[0].oVarSpecVec).some(); }
          if((yyvsp[-4].oSet)()) delete (yyvsp[-4].oSet).some();
        }
        delete (yyvsp[-1].argVec); free((yyvsp[-2].sValue));
      }
#line 3225 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 48: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_INT ':' var_par_id annotations '=' '[' int_list ']'  */
#line 1355 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-4].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        if(pp->capture) pp->capture->array_size(*pp,(yyvsp[-10].iValue));
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
#line 3251 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 49: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_BOOL ':' var_par_id annotations '=' '[' bool_list ']'  */
#line 1378 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-4].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        if(pp->capture) pp->capture->array_size(*pp,(yyvsp[-10].iValue));
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
#line 3276 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 50: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_FLOAT ':' var_par_id annotations '=' '[' float_list ']'  */
#line 1400 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-4].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[-12].iValue)==1, "Arrays must start at 1");
        if(pp->capture) pp->capture->array_size(*pp,(yyvsp[-10].iValue));
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
#line 3301 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 51: /* vardecl_item: FZ_ARRAY '[' FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT ']' FZ_OF FZ_SET FZ_OF FZ_INT ':' var_par_id annotations '=' '[' set_literal_list ']'  */
#line 1422 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        if(pp->capture && (yyvsp[-4].argVec)) pp->capture->fail(*pp,Capture::Status::InvalidInput,"parameter declarations cannot have annotations");
        yyassert(pp, (yyvsp[-14].iValue)==1, "Arrays must start at 1");
        if(pp->capture) pp->capture->array_size(*pp,(yyvsp[-12].iValue));
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
#line 3327 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 52: /* int_init: FZ_INT_LIT  */
#line 1446 "gecode/flatzinc/parser.yxx"
      {
        (yyval.varSpec) = new IntVarSpec((yyvsp[0].iValue),false,false);
      }
#line 3335 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 53: /* int_init: var_par_id  */
#line 1450 "gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_INTVAR || e.t == ST_INT)) {
          if(pp->capture && e.t == ST_INT) (yyval.varSpec) = new IntVarSpec(e.i,false,false);
          else (yyval.varSpec) = new IntVarSpec(Alias(e.i),false,false);
        }
        else {
          pp->err << "Error: undefined identifier for type int " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new IntVarSpec(0,false,false); // keep things consistent
        }
        free((yyvsp[0].sValue));
      }
#line 3356 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 54: /* int_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1467 "gecode/flatzinc/parser.yxx"
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
#line 3381 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 55: /* int_init_list: %empty  */
#line 1490 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3387 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 56: /* int_init_list: int_init_list_head list_tail  */
#line 1492 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3393 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 57: /* int_init_list_head: int_init  */
#line 1496 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3399 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 58: /* int_init_list_head: int_init_list_head ',' int_init  */
#line 1498 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3405 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 61: /* int_var_array_literal: '[' int_init_list ']'  */
#line 1503 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3411 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 62: /* float_init: FZ_FLOAT_LIT  */
#line 1507 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new FloatVarSpec((yyvsp[0].dValue),false,false); }
#line 3417 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 63: /* float_init: var_par_id  */
#line 1509 "gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_FLOATVAR || e.t == ST_FLOAT)) {
          if(pp->capture && e.t == ST_FLOAT) (yyval.varSpec) = new FloatVarSpec(pp->floatvals.at(e.i),false,false);
          else (yyval.varSpec) = new FloatVarSpec(Alias(e.i),false,false);
        }
        else {
          pp->err << "Error: undefined identifier for type float " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new FloatVarSpec(0.0,false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3438 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 64: /* float_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1526 "gecode/flatzinc/parser.yxx"
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
#line 3462 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 65: /* float_init_list: %empty  */
#line 1548 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3468 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 66: /* float_init_list: float_init_list_head list_tail  */
#line 1550 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3474 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 67: /* float_init_list_head: float_init  */
#line 1554 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3480 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 68: /* float_init_list_head: float_init_list_head ',' float_init  */
#line 1556 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3486 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 69: /* float_var_array_literal: '[' float_init_list ']'  */
#line 1560 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3492 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 70: /* bool_init: FZ_BOOL_LIT  */
#line 1564 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new BoolVarSpec((yyvsp[0].iValue),false,false); }
#line 3498 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 71: /* bool_init: var_par_id  */
#line 1566 "gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState* pp = static_cast<ParserState*>(parm);
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_BOOLVAR || e.t == ST_BOOL)) {
          if(pp->capture && e.t == ST_BOOL) (yyval.varSpec) = new BoolVarSpec(e.i != 0,false,false);
          else (yyval.varSpec) = new BoolVarSpec(Alias(e.i),false,false);
        }
        else {
          pp->err << "Error: undefined identifier for type bool " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new BoolVarSpec(false,false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3519 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 72: /* bool_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1583 "gecode/flatzinc/parser.yxx"
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
#line 3543 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 73: /* bool_init_list: %empty  */
#line 1605 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3549 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 74: /* bool_init_list: bool_init_list_head list_tail  */
#line 1607 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3555 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 75: /* bool_init_list_head: bool_init  */
#line 1611 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3561 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 76: /* bool_init_list_head: bool_init_list_head ',' bool_init  */
#line 1613 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3567 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 77: /* bool_var_array_literal: '[' bool_init_list ']'  */
#line 1615 "gecode/flatzinc/parser.yxx"
                                                { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3573 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 78: /* set_init: set_literal  */
#line 1619 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpec) = new SetVarSpec((yyvsp[0].setLit),false,false); }
#line 3579 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 79: /* set_init: var_par_id  */
#line 1621 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        if (pp->symbols.get((yyvsp[0].sValue), e) && (e.t == ST_SETVAR || e.t == ST_SET)) {
          if(pp->capture && e.t == ST_SET) (yyval.varSpec) = new SetVarSpec(new AST::SetLit(pp->setvals.at(e.i)),false,false);
          else (yyval.varSpec) = new SetVarSpec(Alias(e.i),false,false);
        }
        else {
          pp->err << "Error: undefined identifier for type set " << (yyvsp[0].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
          (yyval.varSpec) = new SetVarSpec(Alias(0),false,false);
        }
        free((yyvsp[0].sValue));
      }
#line 3600 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 80: /* set_init: var_par_id '[' FZ_INT_LIT ']'  */
#line 1638 "gecode/flatzinc/parser.yxx"
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
#line 3624 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 81: /* set_init_list: %empty  */
#line 1660 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(0); }
#line 3630 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 82: /* set_init_list: set_init_list_head list_tail  */
#line 1662 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3636 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 83: /* set_init_list_head: set_init  */
#line 1666 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = new vector<VarSpec*>(1); (*(yyval.varSpecVec))[0] = (yyvsp[0].varSpec); }
#line 3642 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 84: /* set_init_list_head: set_init_list_head ',' set_init  */
#line 1668 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-2].varSpecVec); (yyval.varSpecVec)->push_back((yyvsp[0].varSpec)); }
#line 3648 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 85: /* set_var_array_literal: '[' set_init_list ']'  */
#line 1671 "gecode/flatzinc/parser.yxx"
      { (yyval.varSpecVec) = (yyvsp[-1].varSpecVec); }
#line 3654 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 86: /* vardecl_int_var_array_init: %empty  */
#line 1675 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3660 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 87: /* vardecl_int_var_array_init: '=' int_var_array_literal  */
#line 1677 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3666 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 88: /* vardecl_bool_var_array_init: %empty  */
#line 1681 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3672 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 89: /* vardecl_bool_var_array_init: '=' bool_var_array_literal  */
#line 1683 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3678 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 90: /* vardecl_float_var_array_init: %empty  */
#line 1687 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3684 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 91: /* vardecl_float_var_array_init: '=' float_var_array_literal  */
#line 1689 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3690 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 92: /* vardecl_set_var_array_init: %empty  */
#line 1693 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::none(); }
#line 3696 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 93: /* vardecl_set_var_array_init: '=' set_var_array_literal  */
#line 1695 "gecode/flatzinc/parser.yxx"
      { (yyval.oVarSpecVec) = Option<vector<VarSpec*>* >::some((yyvsp[0].varSpecVec)); }
#line 3702 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 94: /* constraint_item: FZ_CONSTRAINT FZ_ID '(' flat_expr_list ')' annotations  */
#line 1699 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        if (pp->capture) {
          if (!pp->hadError) pp->capture->constraint(*pp,(yyvsp[-4].sValue),(yyvsp[-2].argVec),(yyvsp[0].argVec));
          delete (yyvsp[-2].argVec); delete (yyvsp[0].argVec);
        } else if (!pp->hadError) {
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
            const int low = (yyvsp[-2].argVec)->a[0]->getInt();
            const int high = (yyvsp[-2].argVec)->a[1]->getInt();
            if (low > high) {
              std::ostringstream oss;
              oss << "gecode_on_restart_uniform_int requires low (" << low
                  << ") <= high (" << high << ")";
              yyerror(pp, oss.str().c_str());
            } else {
              int base = getBaseIntVar(pp,(yyvsp[-2].argVec)->a[2]->getIntVar());
              pp->uniform_int.emplace_back(std::array<int, 3>{ low, high, base });
            }
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
#line 3874 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 95: /* solve_item: FZ_SOLVE annotations FZ_SATISFY  */
#line 1868 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        initfg(pp);
        if (pp->capture) {
          if (!pp->hadError) pp->capture->solve(*pp,Capture::Method::Satisfy,(yyvsp[-1].argVec));
          delete (yyvsp[-1].argVec);
        } else if (!pp->hadError) {
          try {
            pp->fg->solve((yyvsp[-1].argVec));
          } catch (Gecode::FlatZinc::Error& e) {
            yyerror(pp, e.toString().c_str());
          }
        } else {
          delete (yyvsp[-1].argVec);
        }
      }
#line 3895 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 96: /* solve_item: FZ_SOLVE annotations minmax solve_expr  */
#line 1885 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        initfg(pp);
        if (pp->capture) {
          if (!pp->hadError) pp->capture->solve(*pp,(yyvsp[-1].bValue)?Capture::Method::Minimize:Capture::Method::Maximize,(yyvsp[-2].argVec));
          delete (yyvsp[-2].argVec);
        } else if (!pp->hadError) {
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
#line 3921 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 97: /* int_ti_expr_tail: FZ_INT  */
#line 1913 "gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::none(); }
#line 3927 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 98: /* int_ti_expr_tail: '{' int_list '}'  */
#line 1915 "gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit(*(yyvsp[-1].setValue))); if(static_cast<ParserState*>(parm)->capture) delete (yyvsp[-1].setValue); }
#line 3933 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 99: /* int_ti_expr_tail: FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT  */
#line 1917 "gecode/flatzinc/parser.yxx"
      {
        (yyval.oSet) = Option<AST::SetLit* >::some(new AST::SetLit((yyvsp[-2].iValue), (yyvsp[0].iValue)));
      }
#line 3941 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 100: /* bool_ti_expr_tail: FZ_BOOL  */
#line 1923 "gecode/flatzinc/parser.yxx"
      { (yyval.oSet) = Option<AST::SetLit* >::none(); }
#line 3947 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 101: /* bool_ti_expr_tail: '{' bool_list_head list_tail '}'  */
#line 1925 "gecode/flatzinc/parser.yxx"
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
#line 3962 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 102: /* float_ti_expr_tail: FZ_FLOAT  */
#line 1938 "gecode/flatzinc/parser.yxx"
      { (yyval.oPFloat) = Option<std::pair<double,double>* >::none(); }
#line 3968 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 103: /* float_ti_expr_tail: FZ_FLOAT_LIT FZ_DOTDOT FZ_FLOAT_LIT  */
#line 1940 "gecode/flatzinc/parser.yxx"
      { std::pair<double,double>* dom = new std::pair<double,double>((yyvsp[-2].dValue),(yyvsp[0].dValue));
        (yyval.oPFloat) = Option<std::pair<double,double>* >::some(dom); }
#line 3975 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 104: /* set_literal: '{' int_list '}'  */
#line 1949 "gecode/flatzinc/parser.yxx"
      { (yyval.setLit) = new AST::SetLit(*(yyvsp[-1].setValue)); if(static_cast<ParserState*>(parm)->capture) delete (yyvsp[-1].setValue); }
#line 3981 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 105: /* set_literal: FZ_INT_LIT FZ_DOTDOT FZ_INT_LIT  */
#line 1951 "gecode/flatzinc/parser.yxx"
      { (yyval.setLit) = new AST::SetLit((yyvsp[-2].iValue), (yyvsp[0].iValue)); }
#line 3987 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 106: /* int_list: %empty  */
#line 1957 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(0); }
#line 3993 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 107: /* int_list: int_list_head list_tail  */
#line 1959 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-1].setValue); }
#line 3999 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 108: /* int_list_head: FZ_INT_LIT  */
#line 1963 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[0].iValue); }
#line 4005 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 109: /* int_list_head: int_list_head ',' FZ_INT_LIT  */
#line 1965 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-2].setValue); (yyval.setValue)->push_back((yyvsp[0].iValue)); }
#line 4011 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 110: /* bool_list: %empty  */
#line 1969 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(0); }
#line 4017 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 111: /* bool_list: bool_list_head list_tail  */
#line 1971 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-1].setValue); }
#line 4023 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 112: /* bool_list_head: FZ_BOOL_LIT  */
#line 1975 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = new vector<int>(1); (*(yyval.setValue))[0] = (yyvsp[0].iValue); }
#line 4029 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 113: /* bool_list_head: bool_list_head ',' FZ_BOOL_LIT  */
#line 1977 "gecode/flatzinc/parser.yxx"
      { (yyval.setValue) = (yyvsp[-2].setValue); (yyval.setValue)->push_back((yyvsp[0].iValue)); }
#line 4035 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 114: /* float_list: %empty  */
#line 1981 "gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = new vector<double>(0); }
#line 4041 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 115: /* float_list: float_list_head list_tail  */
#line 1983 "gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = (yyvsp[-1].floatSetValue); }
#line 4047 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 116: /* float_list_head: FZ_FLOAT_LIT  */
#line 1987 "gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = new vector<double>(1); (*(yyval.floatSetValue))[0] = (yyvsp[0].dValue); }
#line 4053 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 117: /* float_list_head: float_list_head ',' FZ_FLOAT_LIT  */
#line 1989 "gecode/flatzinc/parser.yxx"
      { (yyval.floatSetValue) = (yyvsp[-2].floatSetValue); (yyval.floatSetValue)->push_back((yyvsp[0].dValue)); }
#line 4059 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 118: /* set_literal_list: %empty  */
#line 1993 "gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = new vector<AST::SetLit>(0); }
#line 4065 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 119: /* set_literal_list: set_literal_list_head list_tail  */
#line 1995 "gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = (yyvsp[-1].setValueList); }
#line 4071 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 120: /* set_literal_list_head: set_literal  */
#line 1999 "gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = new vector<AST::SetLit>(1); (*(yyval.setValueList))[0] = *(yyvsp[0].setLit); delete (yyvsp[0].setLit); }
#line 4077 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 121: /* set_literal_list_head: set_literal_list_head ',' set_literal  */
#line 2001 "gecode/flatzinc/parser.yxx"
      { (yyval.setValueList) = (yyvsp[-2].setValueList); (yyval.setValueList)->push_back(*(yyvsp[0].setLit)); delete (yyvsp[0].setLit); }
#line 4083 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 122: /* flat_expr_list: flat_expr  */
#line 2009 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 4089 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 123: /* flat_expr_list: flat_expr_list ',' flat_expr  */
#line 2011 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 4095 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 124: /* flat_expr: non_array_expr  */
#line 2015 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 4101 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 125: /* flat_expr: '[' non_array_expr_list ']'  */
#line 2017 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-1].argVec); }
#line 4107 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 126: /* non_array_expr_opt: %empty  */
#line 2021 "gecode/flatzinc/parser.yxx"
      { (yyval.oArg) = Option<AST::Node*>::none(); }
#line 4113 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 127: /* non_array_expr_opt: '=' non_array_expr  */
#line 2023 "gecode/flatzinc/parser.yxx"
      { (yyval.oArg) = Option<AST::Node*>::some((yyvsp[0].arg)); }
#line 4119 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 128: /* non_array_expr: FZ_BOOL_LIT  */
#line 2027 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::BoolLit((yyvsp[0].iValue)); }
#line 4125 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 129: /* non_array_expr: FZ_INT_LIT  */
#line 2029 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::IntLit((yyvsp[0].iValue)); }
#line 4131 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 130: /* non_array_expr: FZ_FLOAT_LIT  */
#line 2031 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::FloatLit((yyvsp[0].dValue)); }
#line 4137 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 131: /* non_array_expr: set_literal  */
#line 2033 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].setLit); }
#line 4143 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 132: /* non_array_expr: var_par_id  */
#line 2035 "gecode/flatzinc/parser.yxx"
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
          (yyval.arg) = pp->capture ? new AST::IntLit(0) : NULL;
        }
        free((yyvsp[0].sValue));
      }
#line 4243 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 133: /* non_array_expr: var_par_id '[' non_array_expr ']'  */
#line 2131 "gecode/flatzinc/parser.yxx"
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
#line 4259 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 134: /* non_array_expr_list: %empty  */
#line 2145 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array(0); }
#line 4265 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 135: /* non_array_expr_list: non_array_expr_list_head list_tail  */
#line 2147 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-1].argVec); }
#line 4271 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 136: /* non_array_expr_list_head: non_array_expr  */
#line 2151 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 4277 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 137: /* non_array_expr_list_head: non_array_expr_list_head ',' non_array_expr  */
#line 2153 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 4283 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 138: /* solve_expr: var_par_id  */
#line 2161 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        SymbolEntry e;
        bool haveSym = pp->symbols.get((yyvsp[0].sValue),e);
        if (haveSym) {
          switch (e.t) {
          case ST_INTVAR:
            if(pp->capture){AST::IntVar value(e.i);pp->capture->objective(*pp,&value);}
            (yyval.iValue) = e.i;
            break;
          case ST_FLOATVAR:
            if(pp->capture){AST::FloatVar value(e.i);pp->capture->objective(*pp,&value);}
            (yyval.iValue) = -e.i-1;
            break;
          case ST_INT:
          case ST_FLOAT:
            if(pp->capture){
              if(e.t==ST_INT){AST::IntLit value(e.i);pp->capture->objective(*pp,&value);}
              else {AST::FloatLit value(pp->floatvals.at(e.i));pp->capture->objective(*pp,&value);}
              (yyval.iValue)=0;break;
            }
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
#line 4328 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 139: /* solve_expr: FZ_INT_LIT  */
#line 2202 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        if(pp->capture){AST::IntLit value((yyvsp[0].iValue));pp->capture->objective(*pp,&value);(yyval.iValue)=0;}
        else {pp->intvars.push_back(varspec("OBJ_CONST_INTRODUCED",
          new IntVarSpec(0,true,false)));(yyval.iValue) = pp->intvars.size()-1;}
      }
#line 4339 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 140: /* solve_expr: FZ_FLOAT_LIT  */
#line 2209 "gecode/flatzinc/parser.yxx"
      {
        ParserState *pp = static_cast<ParserState*>(parm);
        if(pp->capture){AST::FloatLit value((yyvsp[0].dValue));pp->capture->objective(*pp,&value);(yyval.iValue)=0;}
        else {pp->intvars.push_back(varspec("OBJ_CONST_INTRODUCED",
          new IntVarSpec(0,true,false)));(yyval.iValue) = pp->intvars.size()-1;}
      }
#line 4350 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 141: /* solve_expr: var_par_id '[' FZ_INT_LIT ']'  */
#line 2216 "gecode/flatzinc/parser.yxx"
      {
        SymbolEntry e;
        ParserState *pp = static_cast<ParserState*>(parm);
        if(pp->capture){
          if(!pp->symbols.get((yyvsp[-3].sValue),e)||(e.t!=ST_INTVARARRAY&&e.t!=ST_FLOATVARARRAY)||
             e.i<0||static_cast<size_t>(e.i)>=pp->arrays.size()||(yyvsp[-1].iValue)<1||(yyvsp[-1].iValue)>pp->arrays[e.i]) {
            pp->capture->fail(*pp,Capture::Status::InvalidInput,"invalid objective array reference");(yyval.iValue)=0;
          } else {
            const int slot=pp->arrays.at(static_cast<size_t>(e.i)+(yyvsp[-1].iValue));
            if(e.t==ST_INTVARARRAY){AST::IntVar value(slot);pp->capture->objective(*pp,&value);(yyval.iValue)=slot;}
            else{AST::FloatVar value(slot);pp->capture->objective(*pp,&value);(yyval.iValue)=-slot-1;}
          }
        } else if ( (!pp->symbols.get((yyvsp[-3].sValue), e)) ||
             (e.t != ST_INTVARARRAY && e.t != ST_FLOATVARARRAY)) {
          pp->err << "Error: unknown int or float variable array " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        }
        if (!pp->capture && ((yyvsp[-1].iValue) == 0 || (yyvsp[-1].iValue) > pp->arrays[e.i])) {
          pp->err << "Error: array index out of bounds for array " << (yyvsp[-3].sValue)
                  << " in line no. "
                  << yyget_lineno(pp->yyscanner) << std::endl;
          pp->hadError = true;
        } else if (!pp->capture) {
          if (e.t == ST_INTVARARRAY)
            (yyval.iValue) = pp->arrays[e.i+(yyvsp[-1].iValue)];
          else
            (yyval.iValue) = -pp->arrays[e.i+(yyvsp[-1].iValue)]-1;
        }
        free((yyvsp[-3].sValue));
      }
#line 4387 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 144: /* annotations: %empty  */
#line 2259 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = NULL; }
#line 4393 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 145: /* annotations: annotations_head  */
#line 2261 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[0].argVec); }
#line 4399 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 146: /* annotations_head: FZ_COLONCOLON annotation  */
#line 2265 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = new AST::Array((yyvsp[0].arg)); }
#line 4405 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 147: /* annotations_head: annotations_head FZ_COLONCOLON annotation  */
#line 2267 "gecode/flatzinc/parser.yxx"
      { (yyval.argVec) = (yyvsp[-2].argVec); (yyval.argVec)->append((yyvsp[0].arg)); }
#line 4411 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 148: /* annotation: FZ_ID '(' annotation_list ')'  */
#line 2271 "gecode/flatzinc/parser.yxx"
      {
        (yyval.arg) = new AST::Call((yyvsp[-3].sValue), static_cast<ParserState*>(parm)->capture ? (yyvsp[-1].arg) : AST::extractSingleton((yyvsp[-1].arg))); free((yyvsp[-3].sValue));
      }
#line 4419 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 149: /* annotation: annotation_expr  */
#line 2275 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 4425 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 150: /* annotation_list: annotation  */
#line 2279 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::Array((yyvsp[0].arg)); }
#line 4431 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 151: /* annotation_list: annotation_list ',' annotation  */
#line 2281 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-2].arg); (yyval.arg)->append((yyvsp[0].arg)); }
#line 4437 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 152: /* annotation_expr: ann_non_array_expr  */
#line 2285 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].arg); }
#line 4443 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 153: /* annotation_expr: '[' ']'  */
#line 2287 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::Array(); }
#line 4449 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 154: /* annotation_expr: '[' annotation_list annotation_list_tail ']'  */
#line 2289 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[-2].arg); }
#line 4455 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 157: /* ann_non_array_expr: FZ_BOOL_LIT  */
#line 2295 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::BoolLit((yyvsp[0].iValue)); }
#line 4461 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 158: /* ann_non_array_expr: FZ_INT_LIT  */
#line 2297 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::IntLit((yyvsp[0].iValue)); }
#line 4467 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 159: /* ann_non_array_expr: FZ_FLOAT_LIT  */
#line 2299 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = new AST::FloatLit((yyvsp[0].dValue)); }
#line 4473 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 160: /* ann_non_array_expr: set_literal  */
#line 2301 "gecode/flatzinc/parser.yxx"
      { (yyval.arg) = (yyvsp[0].setLit); }
#line 4479 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 161: /* ann_non_array_expr: var_par_id  */
#line 2303 "gecode/flatzinc/parser.yxx"
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
#line 4589 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 162: /* ann_non_array_expr: var_par_id '[' ann_non_array_expr ']'  */
#line 2409 "gecode/flatzinc/parser.yxx"
      {
        ParserState* pp = static_cast<ParserState*>(parm);
        int i = -1;
        yyassert(pp, (yyvsp[-1].arg)->isInt(i), "Non-integer array index");
        if (!pp->hadError)
          (yyval.arg) = getArrayElement(static_cast<ParserState*>(parm),(yyvsp[-3].sValue),i,true);
        else
          (yyval.arg) = new AST::IntLit(0); // keep things consistent
        if(pp->capture) delete (yyvsp[-1].arg);
        free((yyvsp[-3].sValue));
      }
#line 4605 "gecode/flatzinc/parser.tab.cpp"
    break;

  case 163: /* ann_non_array_expr: FZ_STRING_LIT  */
#line 2421 "gecode/flatzinc/parser.yxx"
      {
        (yyval.arg) = new AST::String((yyvsp[0].sValue));
        free((yyvsp[0].sValue));
      }
#line 4614 "gecode/flatzinc/parser.tab.cpp"
    break;


#line 4618 "gecode/flatzinc/parser.tab.cpp"

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
