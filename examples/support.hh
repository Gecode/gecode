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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
