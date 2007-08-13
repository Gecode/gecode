/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#include "gecode/kernel.hh"
#include "gecode/int.hh"
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

class Options;

/**
 * \brief Base class for options
 *
 */
class BaseOption {
  friend class Options;
protected:
  const char* opt;  ///< String for option (including hyphen)
  const char* exp;  ///< Short explanation
  BaseOption* next; ///< Next option
public:
  /// Initialize for option \a o and explanation \a e
  BaseOption(const char* o, const char* e);
  /// Parse option at first position and possibly delete
  virtual bool parse(int& argc, char* argv[]) = 0;
  /// Print help text
  virtual void help(void) = 0;
  /// Destructor
  virtual ~BaseOption(void);
};

/**
 * \brief String-valued option
 *
 */
class StringOption : public BaseOption {
protected:
  /// Option value
  class Value {
  public:
    int         val;  ///< Value for an option value
    const char* opt;  ///< String for option value
    const char* help; ///< Optional help text
    Value*      next; ///< Next option value
  };
  int    cur; ///< Current value
  Value* fst; ///< First option value
  Value* lst; ///< Last option value
public:
  /// Initialize for option \a o and explanation \a e and default value \a v
  StringOption(const char* o, const char* e, int v=0);
  /// Set default value to \a v
  void value(int v);
  /// Return current option value
  int value(void) const;
  /// Add option value for value \a v, string \a o, and help text \a h
  void add(int v, const char* o, const char* h = NULL);
  /// Parse option at first position
  virtual bool parse(int& argc, char* argv[]);
  /// Print help text
  virtual void help(void);
  /// Destructor
  virtual ~StringOption(void);
};


/**
 * \brief Unsigned integer option
 *
 */
class UnsignedIntOption : public BaseOption {
protected:
  unsigned int cur; ///< Current value
public:
  /// Initialize for option \a o and explanation \a e and default value \a v
  UnsignedIntOption(const char* o, const char* e, unsigned int v=0);
  /// Set default value to \a v
  void value(unsigned int v);
  /// Return current option value
  unsigned int value(void) const;
  /// Parse option at first position
  virtual bool parse(int& argc, char* argv[]);
  /// Print help text
  virtual void help(void);
};


/**
 * \brief Options for examples
 *
 */
class Options {
protected:
  BaseOption* fst;   ///< First registered option
  BaseOption* lst;   ///< Last registered option
  const char* _name; ///< Example name

  /// \name Model options
  //@{
  StringOption _model;       ///< General model options
  StringOption _propagation; ///< Propagation options
  StringOption _icl;         ///< Integer consistency level
  StringOption _branching;   ///< Branching options
  //@}

  /// \name Search options
  //@{
  StringOption      _search;    ///< Search options
  UnsignedIntOption _solutions; ///< How many solutions
  UnsignedIntOption _c_d;       ///< Copy recomputation distance
  UnsignedIntOption _a_d;       ///< Adaptive recomputation distance
  UnsignedIntOption _fail;      ///< Cutoff for number of failures
  UnsignedIntOption _time;      ///< Cutoff for time
  //@}

  /// \name Execution options
  //@{
  StringOption      _mode;       ///< Example mode to run
  UnsignedIntOption _samples;    ///< How many samples
  UnsignedIntOption _iterations; ///< How many iterations per sample
  //@}

public:
  /// Initialize options for example with name \a s
  Options(const char* s);
  /// Add new option \a o
  void add(BaseOption& o);
  /// Print help text
  virtual void help(void);
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]);

  /// Return name of example
  const char* name(void) const;

  /// \name Model options
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

  /// Set default integer consistency level
  void icl(IntConLevel i);
  /// Return integer consistency level
  IntConLevel icl(void) const;

  /// Set default branching value
  void branching(int v);
  /// Add branching option value for value \a v, string \a o, and help \a h
  void branching(int v, const char* o, const char* h = NULL);
  /// Return branching value
  int branching(void) const;
  //@}

  /// \name Search options
  //@{
  /// Set default search value
  void search(int v);
  /// Add search option value for value \a v, string \a o, and help \a h
  void search(int v, const char* o, const char* h = NULL);
  /// Return search value
  int search(void) const;

  /// Set default number of solutions to search for
  void solutions(unsigned int n);
  /// Return number of solutions to search for
  unsigned int solutions(void) const;

  /// Set default copy recompution distance
  void c_d(unsigned int d);
  /// Return copy recomputation distance
  unsigned int c_d(void) const;

  /// Set default adaptive recompution distance
  void a_d(unsigned int d);
  /// Return adaptive recomputation distance
  unsigned int a_d(void) const;

  /// Set default failure cutoff
  void fail(unsigned int n);
  /// Return failure cutoff
  unsigned int fail(void) const;

  /// Set default time cutoff
  void time(unsigned int t);
  /// Return time cutoff
  unsigned int time(void) const;
  //@}

  /// \name Execution options
  //@{
  /// Set default mode
  void mode(ExampleMode em);
  /// Return mode
  ExampleMode mode(void) const;  

  /// Set default number of iterations
  void iterations(unsigned int i);
  /// Return number of iterations
  unsigned int iterations(void) const;

  /// Set default number of samples
  void samples(unsigned int s);
  /// Return number of samples
  unsigned int samples(void) const;
  //@}

  /// Destructor
  virtual ~Options(void);
};

/**
 * \brief Options for examples with additional size parameter
 *
 */
class SizeOptions : public Options {
protected:
  unsigned int _size; ///< Size value
public:
  /// Initialize options for example with name \a s
  SizeOptions(const char* s);
  /// Print help text
  virtual void help(void);
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]);

  /// Set default size
  void size(unsigned int s);
  /// Return size
  unsigned int size(void) const;
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
  template <class Script, template<class> class Engine, class Options>
  static void run(const Options& opt);
private:
  /// Catch wrong definitions of copy constructor
  explicit Example(Example& e);
};

#include "examples/support/example.icc"

#endif

// STATISTICS: example-any
