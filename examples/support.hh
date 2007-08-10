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
#include <ctime>
#include <cmath>

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#ifdef GECODE_HAVE_CPLTSET_VARS
#include "gecode/cpltset.hh"
#endif
#include "gecode/search.hh"

using namespace Gecode;

/*
 * Options for running the examples
 *
 */

/// Different modes for executing examples
enum ExampleMode {
  EM_SOLUTION, ///< Print solution and some statistics
  EM_TIME,     ///< Measure average runtime
  EM_STAT      ///< Print statistics for example
};

/**
 * \brief Configurable option values
 *
 */
class StringOption {
private:
  /// Option value
  class Value {
  public:
    int         val;  ///< Value for an option value
    const char* opt;  ///< String for option value
    const char* help; ///< Optional help text
    Value*      next; ///< Next option value
  };
  const char* opt; ///< String for option (including hyphen)
  const char* exp; ///< Short explanation
  int         cur; ///< Current value
  Value*      fst; ///< First option value
  Value*      lst; ///< Last option value
public:
  /// Initialize for option \a o and explanation \a e and default value \a v
  StringOption(const char* o, const char* e, int v=0);
  /// Set default value to \a v
  void value(int v);
  /// Return current option value
  int value(void) const;
  /// Add option value for value \a v, string \a o, and help text \a h
  void add(int v, const char* o, const char* h = NULL);
  /// Parse option at position \a i
  bool parse(int argc, char* argv[], int& i);
  /// Print help text
  void help(void);
};

/**
 * \brief Options for examples
 *
 */
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

#ifdef GECODE_HAVE_CPLTSET_VARS
  unsigned int initvarnum; ///< initial number of bdd nodes in the table
  unsigned int initcache;  ///< initial cachesize for bdd operations
  SetConLevel  scl;        ///< bdd consistency level
#endif 

private:  
  StringOption _model;       ///< Model options
  StringOption _propagation; ///< Propagation options
  StringOption _branching;   ///< Branching options
  StringOption _search;      ///< Search options

public:
  /// Initialize options for example with name \a s
  Options(const char* s);
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int argc, char* argv[]);

  /// \name User-definable options
  //@{
  /// Set default model value
  void model(int v);
  /// Add model option value for value \a v, string \a o, and help \a h
  void model(int v, const char* o, const char* h = NULL);
  /// Return model value
  int model(void) const;

  /// Set default propagation value
  void propagation(int v);
  /// Add propagation option value for value \a v, string \a o, and help \a h
  void propagation(int v, const char* o, const char* h = NULL);
  /// Return propagation value
  int propagation(void) const;

  /// Set default branching value
  void branching(int v);
  /// Add branching option value for value \a v, string \a o, and help \a h
  void branching(int v, const char* o, const char* h = NULL);
  /// Return branching value
  int branching(void) const;

  /// Set default search value
  void search(int v);
  /// Add search option value for value \a v, string \a o, and help \a h
  void search(int v, const char* o, const char* h = NULL);
  /// Return search value
  int search(void) const;
  //@}
};

#include "examples/support/options.icc"


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
  /// Default constructor
  Example(void) {}
  /// Constructor used for cloning
  Example(bool share, Example& e) : Space(share,e) {}
  /// Print a solution
  virtual void print(void) {}
  /// Run example with search engine \a Engine and options \a opt
  template <class Script, template<class> class Engine>
  static void run(const Options& opt);
private:
  /// Catch wrong definitions of copy constructor
  explicit Example(Example& e);
};

#include "examples/support/example.icc"

#endif

// STATISTICS: example-any
