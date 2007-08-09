/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __GECODE_EXAMPLES_SUPPORT_HH__
#define __GECODE_EXAMPLES_SUPPORT_HH__

#include <iostream>
#include <iomanip>

#include <cstdlib>
#include <cstring>

#include "gecode/kernel.hh"
#include "gecode/int.hh"

#ifdef GECODE_HAVE_CPLTSET_VARS
#include "gecode/cpltset.hh"
#endif

#include "gecode/search.hh"

#include "examples/timer.hh"

/*
 * Options for running the examples
 *
 */

using namespace Gecode;

/// Different modes for executing examples
enum ExampleMode {
  EM_SOLUTION, ///< Print solution and some statistics
  EM_TIME,     ///< Measure average runtime
  EM_STAT      ///< Print statistics for example
};

class OptValue {
private:
  class Value {
  public:
    int         val;
    const char* opt;
    const char* help;
    Value*      next;
  };
  int    cur;
  Value* fst;
  Value* lst;
public:
  OptValue(void) : cur(0), fst(NULL), lst(NULL) {}
  void value(int v) {
    cur = v;
  }
  int value(void) const {
    return cur;
  }
  void add(int v, const char* o, const char* h = NULL) {
    Value* n = new Value;
    n->val  = v;
    n->opt  = o;
    n->help = h;
    n->next = NULL;
    if (fst == NULL) {
      fst = lst = n;
    } else {
      lst->next = n; lst = n;
    }
  }
  bool parse(const char* o) {
    if (fst == NULL)
      return false;
    for (Value* v = fst; v != NULL; v = v->next)
      if (!strcmp(o,v->opt)) {
        cur = v->val;
        return true;
      }
    return false;
  }
  void help(const char* o, const char* t) {
    if (fst == NULL)
      return;
    std::cerr << '\t' << o << " (";
    const char* d = NULL;
    for (Value* v = fst; v != NULL; v = v->next) {
      std::cerr << v->opt << ((v->next != NULL) ? ", " : "");
      if (v->val == cur)
        d = v->opt;
    }
    std::cerr << ")";
    if (d != NULL) 
      std::cerr << " default: " << d;
    std::cerr << std::endl
              << "\t\t" << t << std::endl;
    for (Value* v = fst; v != NULL; v = v->next)
      if (v->help != NULL)
        std::cerr << "\t\t  " << v->opt << ": " << v->help << std::endl;
  }
};

/// Class for options for examples
class Options {
public:
  IntConLevel  icl;        ///< integer consistency level
  unsigned int c_d;        ///< recomputation copy distance
  unsigned int a_d;        ///< recomputation adaption distance
  ExampleMode  mode;       ///< in which mode to run
  bool         quiet;      ///< quiet mode
  unsigned int samples;    ///< how many samples
  unsigned int iterations; ///< how many iterations per sample
  unsigned int solutions;  ///< how many solutions (0 == all)
  int          fails;      ///< number of fails before stopping search
  int          time;       ///< allowed time before stopping search
  bool         naive;      ///< use naive version
  unsigned int size;       ///< problem size/variant
  const char*  name;       ///< name of problem

  OptValue     model;
  OptValue     propagation;
  OptValue     branching;

#ifdef GECODE_HAVE_CPLTSET_VARS
  unsigned int initvarnum; ///< initial number of bdd nodes in the table
  unsigned int initcache;  ///< initial cachesize for bdd operations
  SetConLevel  scl;        ///< bdd consistency level
#endif 
  
  /// Initialize options for example with name \a s
  Options(const char* s);
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int argc, char** argv);
};


/**
 * \brief Base-class for %Gecode examples
 *
 * All examples must inherit from this class
 *  - adds printing to examples
 *  - run allows to execute examples
 *
 */
class Example : public Space {
public:
  Example(void) {}
  Example(bool share, Example& e) : Space(share,e) {}
  virtual void print(void) {}
  template <class Script, template<class> class Engine>
  static void run(const Options&);
private:
  explicit Example(Example& e);
};

#include "examples/support.icc"

#endif

// STATISTICS: example-any
