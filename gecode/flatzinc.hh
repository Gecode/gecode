/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#ifndef __GECODE_FLATZINC_HH__
#define __GECODE_FLATZINC_HH__

#include <iostream>

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

#include <map>

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_FLATZINC
#define GECODE_FLATZINC_EXPORT __declspec( dllexport )
#else
#define GECODE_FLATZINC_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define GECODE_FLATZINC_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_FLATZINC_EXPORT

#endif
#endif

// Configure auto-linking
#ifndef GECODE_BUILD_FLATZINC
#define GECODE_LIBRARY_NAME "FlatZinc"
#include <gecode/support/auto-link.hpp>
#endif

#include <gecode/driver.hh>

#include <gecode/flatzinc/conexpr.hh>
#include <gecode/flatzinc/ast.hh>
#include <gecode/flatzinc/varspec.hh>

/**
 * \namespace Gecode::FlatZinc
 * \brief Interpreter for the %FlatZinc language
 *
 * The Gecode::FlatZinc namespace contains all functionality required
 * to parse and solve constraint models written in the %FlatZinc language.
 *
 */

namespace Gecode { namespace FlatZinc {

  /**
   * \brief Output support class for %FlatZinc interpreter
   *
   */
  class GECODE_FLATZINC_EXPORT Printer {
  private:
    AST::Array* _output;
    void printElem(std::ostream& out,
                   AST::Node* ai,
                   const Gecode::IntVarArray& iv,
                   const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                   ,
                   const Gecode::SetVarArray& sv
#endif
                   ) const;
  public:
    Printer(void) : _output(NULL) {}
    void init(AST::Array* output);

    void print(std::ostream& out,
               const Gecode::IntVarArray& iv,
               const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
               ,
               const Gecode::SetVarArray& sv
#endif
               ) const;
  
    ~Printer(void);
    
    void shrinkElement(AST::Node* node,
                       std::map<int,int>& iv, std::map<int,int>& bv, 
                       std::map<int,int>& sv);

    void shrinkArrays(Space& home,
                      int& optVar,
                      Gecode::IntVarArray& iv,
                      Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                      ,
                      Gecode::SetVarArray& sv
#endif
                     );
    
  private:
    Printer(const Printer&);
    Printer& operator=(const Printer&);
  };

  /**
   * \brief %Options for running %FlatZinc models
   *
   */
  class FlatZincOptions : public Gecode::BaseOptions {
  protected:
      /// \name Search options
      //@{
      Gecode::Driver::UnsignedIntOption _solutions; ///< How many solutions
      Gecode::Driver::BoolOption        _allSolutions; ///< Return all solutions
      Gecode::Driver::DoubleOption      _threads;   ///< How many threads to use
      Gecode::Driver::BoolOption        _free; ///< Use free search
      Gecode::Driver::StringOption      _search; ///< Search engine variant
      Gecode::Driver::UnsignedIntOption _c_d;       ///< Copy recomputation distance
      Gecode::Driver::UnsignedIntOption _a_d;       ///< Adaptive recomputation distance
      Gecode::Driver::UnsignedIntOption _node;      ///< Cutoff for number of nodes
      Gecode::Driver::UnsignedIntOption _fail;      ///< Cutoff for number of failures
      Gecode::Driver::UnsignedIntOption _time;      ///< Cutoff for time
      Gecode::Driver::IntOption         _seed;      ///< Random seed
      //@}
    
      /// \name Execution options
      //@{
      Gecode::Driver::StringOption      _mode;       ///< Script mode to run
      Gecode::Driver::BoolOption        _stat;       ///< Emit statistics
      Gecode::Driver::StringValueOption _output;     ///< Output file
      //@}
  public:
    enum SearchOptions {
      FZ_SEARCH_BAB,    //< Branch-and-bound search
      FZ_SEARCH_RESTART //< Restart search
    };
    /// Constructor
    FlatZincOptions(const char* s)
    : Gecode::BaseOptions(s),
      _solutions("-n","number of solutions (0 = all)",1),
      _allSolutions("-a", "return all solutions (equal to -solutions 0)"),
      _threads("-p","number of threads (0 = #processing units)",
               Gecode::Search::Config::threads),
      _free("--free", "no need to follow search-specification"),
      _search("-search","search engine variant", FZ_SEARCH_BAB),
      _c_d("-c-d","recomputation commit distance",Gecode::Search::Config::c_d),
      _a_d("-a-d","recomputation adaption distance",Gecode::Search::Config::a_d),
      _node("-node","node cutoff (0 = none, solution mode)"),
      _fail("-fail","failure cutoff (0 = none, solution mode)"),
      _time("-time","time (in ms) cutoff (0 = none, solution mode)"),
      _seed("-r","random seed",0),
      _mode("-mode","how to execute script",Gecode::SM_SOLUTION),
      _stat("-s","emit statistics"),
      _output("-o","file to send output to") {

      _search.add(FZ_SEARCH_BAB, "bab");
      _search.add(FZ_SEARCH_RESTART, "restart");
      _mode.add(Gecode::SM_SOLUTION, "solution");
      _mode.add(Gecode::SM_STAT, "stat");
      _mode.add(Gecode::SM_GIST, "gist");
      add(_solutions); add(_threads); add(_c_d); add(_a_d);
      add(_allSolutions);
      add(_free);
      add(_search);
      add(_node); add(_fail); add(_time);
      add(_seed);
      add(_mode); add(_stat);
      add(_output);
    }

    void parse(int& argc, char* argv[]) {
      Gecode::BaseOptions::parse(argc,argv);
      if (_allSolutions.value()) {
        _solutions.value(0);
      }
      if (_stat.value())
        _mode.value(Gecode::SM_STAT);
    }
  
    virtual void help(void) {
      std::cerr << "Gecode FlatZinc interpreter" << std::endl
                << " - Supported FlatZinc version: " << GECODE_FLATZINC_VERSION
                << std::endl << std::endl;
      Gecode::BaseOptions::help();
    }
  
    unsigned int solutions(void) const { return _solutions.value(); }
    bool allSolutions(void) const { return _allSolutions.value(); }
    double threads(void) const { return _threads.value(); }
    bool free(void) const { return _free.value(); }
    SearchOptions search(void) const {
      return static_cast<SearchOptions>(_search.value());
    }
    unsigned int c_d(void) const { return _c_d.value(); }
    unsigned int a_d(void) const { return _a_d.value(); }
    unsigned int node(void) const { return _node.value(); }
    unsigned int fail(void) const { return _fail.value(); }
    unsigned int time(void) const { return _time.value(); }
    int seed(void) const { return _seed.value(); }
    const char* output(void) const { return _output.value(); }
    Gecode::ScriptMode mode(void) const {
      return static_cast<Gecode::ScriptMode>(_mode.value());
    }
  };

  /**
   * \brief A space that can be initialized with a %FlatZinc model
   *
   */
  class GECODE_FLATZINC_EXPORT FlatZincSpace : public Space {
  public:
    enum Meth {
      SAT, //< Solve as satisfaction problem
      MIN, //< Solve as minimization problem
      MAX  //< Solve as maximization problem
    };
  protected:
    /// Number of integer variables
    int intVarCount;
    /// Number of Boolean variables
    int boolVarCount;
    /// Number of set variables
    int setVarCount;

    /// Index of the integer variable to optimize
    int _optVar;
  
    /// Whether to solve as satisfaction or optimization problem
    Meth _method;
    
    /// Annotations on the solve item
    AST::Array* _solveAnnotations;

    /// Copy constructor
    FlatZincSpace(bool share, FlatZincSpace&);
  private:
    /// Run the search engine
    template<template<class> class Engine>
    void
    runEngine(std::ostream& out, const Printer& p, 
              const FlatZincOptions& opt, Gecode::Support::Timer& t_total);
    void
    branchWithPlugin(AST::Node* ann);
  public:
    /// The integer variables
    Gecode::IntVarArray iv;
    /// Indicates whether an integer variable is introduced by mzn2fzn
    std::vector<bool> iv_introduced;
    /// Indicates whether an integer variable aliases a Boolean variable
    int* iv_boolalias;
    /// The Boolean variables
    Gecode::BoolVarArray bv;
    /// Indicates whether a Boolean variable is introduced by mzn2fzn
    std::vector<bool> bv_introduced;
#ifdef GECODE_HAS_SET_VARS
    /// The set variables
    Gecode::SetVarArray sv;
    /// Indicates whether a set variable is introduced by mzn2fzn
    std::vector<bool> sv_introduced;
#endif
    /// Construct empty space
    FlatZincSpace(void);
  
    /// Destructor
    ~FlatZincSpace(void);
  
    /// Initialize space with given number of variables
    void init(int intVars, int boolVars, int setVars);

    /// Create new integer variable from specification
    void newIntVar(IntVarSpec* vs);
    /// Link integer variable \a iv to Boolean variable \a bv
    void aliasBool2Int(int iv, int bv);
    /// Return linked Boolean variable for integer variable \a iv
    int aliasBool2Int(int iv);
    /// Create new Boolean variable from specification
    void newBoolVar(BoolVarSpec* vs);
    /// Create new set variable from specification
    void newSetVar(SetVarSpec* vs);
  
    /// Post a constraint specified by \a ce
    void postConstraint(const ConExpr& ce, AST::Node* annotation);
  
    /// Post the solve item
    void solve(AST::Array* annotation);
    /// Post that integer variable \a var should be minimized
    void minimize(int var, AST::Array* annotation);
    /// Post that integer variable \a var should be maximized
    void maximize(int var, AST::Array* annotation);

    /// Run the search
    void run(std::ostream& out, const Printer& p, 
             const FlatZincOptions& opt, Gecode::Support::Timer& t_total);
  
    /// Produce output on \a out using \a p
    void print(std::ostream& out, const Printer& p) const;

    /**
     * \brief Remove all variables not needed for output
     *
     * After calling this function, no new constraints can be posted through
     * FlatZinc variable references, and the createBranchers method must
     * not be called again.
     *
     */
    void shrinkArrays(Printer& p);

    /// Return whether to solve a satisfaction or optimization problem
    Meth method(void) const;

    /// Return index of variable used for optimization
    int optVar(void) const;

    /**
     * \brief Create branchers corresponding to the solve item annotations
     *
     * If \a ignoreUnknown is true, unknown solve item annotations will be
     * ignored, otherwise a warning is written to \a err.
     *
     * The seed for random branchers is given by the \a seed parameter.
     *
     */
    void createBranchers(AST::Node* ann,
                         int seed,
                         bool ignoreUnknown,
                         std::ostream& err = std::cerr);

    /// Return the solve item annotations
    AST::Array* solveAnnotations(void) const;

    /// Implement optimization
    virtual void constrain(const Space& s);
    /// Copy function
    virtual Gecode::Space* copy(bool share);
  };

  /// %Exception class for %FlatZinc errors
  class Error {
  private:
    const std::string msg;
  public:
    Error(const std::string& where, const std::string& what)
    : msg(where+": "+what) {}
    const std::string& toString(void) const { return msg; }
  };

  /**
   * \brief Parse FlatZinc file \a fileName into \a fzs and return it.
   *
   * Creates a new empty FlatZincSpace if \a fzs is NULL.
   */
  GECODE_FLATZINC_EXPORT
  FlatZincSpace* parse(const std::string& fileName,
                       Printer& p, std::ostream& err = std::cerr,
                       FlatZincSpace* fzs=NULL);

  /**
   * \brief Parse FlatZinc from \a is into \a fzs and return it.
   *
   * Creates a new empty FlatZincSpace if \a fzs is NULL.
   */
  GECODE_FLATZINC_EXPORT
  FlatZincSpace* parse(std::istream& is,
                       Printer& p, std::ostream& err = std::cerr,
                       FlatZincSpace* fzs=NULL);

}}

#endif

// STATISTICS: flatzinc-any
