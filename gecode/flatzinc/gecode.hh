/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2006-12-11 03:27:31 +1100 (Mon, 11 Dec 2006) $ by $Author: schulte $
 *     $Revision: 4024 $
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

#ifndef __FLATZINC_GECODE_HH__
#define __FLATZINC_GECODE_HH__

#include "config.icc"
#include "gecode/kernel.hh"
#include "gecode/int.hh"
#ifdef GECODE_HAS_SET_VARS
#include "gecode/set.hh"
#endif

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

#ifdef GECODEF_GCC_HAS_CLASS_VISIBILITY

#define GECODE_FLATZINC_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_FLATZINC_EXPORT

#endif
#endif

#include "conexpr.hh"
#include "ast.hh"
#include "varspec.hh"

#include <gecode/driver.hh>

class GECODE_FLATZINC_EXPORT FZPrinter {
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
  FZPrinter(void) : _output(NULL) {}
  void init(AST::Array* output);

  void print(std::ostream& out,
             const Gecode::IntVarArray& iv,
             const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
             ,
             const Gecode::SetVarArray& sv
#endif
             ) const;
  
  ~FZPrinter(void);
private:
  FZPrinter(const FZPrinter&);
  FZPrinter& operator=(const FZPrinter&);
};

class FlatZincOptions : public Gecode::BaseOptions {
protected:
    /// \name Search options
    //@{
    Gecode::Driver::UnsignedIntOption _solutions; ///< How many solutions
    Gecode::Driver::BoolOption        _allSolutions; ///< Return all solutions
    Gecode::Driver::DoubleOption      _threads;   ///< How many threads to use
    Gecode::Driver::BoolOption        _parallel; ///< Use all cores
    Gecode::Driver::BoolOption        _free; ///< Use free search
    Gecode::Driver::UnsignedIntOption _c_d;       ///< Copy recomputation distance
    Gecode::Driver::UnsignedIntOption _a_d;       ///< Adaptive recomputation distance
    Gecode::Driver::UnsignedIntOption _node;      ///< Cutoff for number of nodes
    Gecode::Driver::UnsignedIntOption _fail;      ///< Cutoff for number of failures
    Gecode::Driver::UnsignedIntOption _time;      ///< Cutoff for time
    //@}
    
    /// \name Execution options
    //@{
    Gecode::Driver::StringOption      _mode;       ///< Script mode to run
    //@}
public:
  FlatZincOptions(const char* s)
  : Gecode::BaseOptions(s),
    _solutions("-solutions","number of solutions (0 = all)",1),
    _allSolutions("--all", "return all solutions (equal to -solutions 0)"),
    _threads("-threads","number of threads (0 = #processing units)",
             Gecode::Search::Config::threads),
    _parallel("--parallel", "use parallel search (equal to -threads 0)"),
    _free("--free", "no need to follow search-specification"),
    _c_d("-c-d","recomputation commit distance",Gecode::Search::Config::c_d),
    _a_d("-a-d","recomputation adaption distance",Gecode::Search::Config::a_d),
    _node("-node","node cutoff (0 = none, solution mode)"),
    _fail("-fail","failure cutoff (0 = none, solution mode)"),
    _time("-time","time (in ms) cutoff (0 = none, solution mode)"),
    _mode("-mode","how to execute script",Gecode::SM_SOLUTION) {

    _mode.add(Gecode::SM_SOLUTION, "solution");
    _mode.add(Gecode::SM_STAT, "stat");
    _mode.add(Gecode::SM_GIST, "gist");
    add(_solutions); add(_threads); add(_c_d); add(_a_d);
    add(_allSolutions);
    add(_parallel);
    add(_free);
    add(_node); add(_fail); add(_time);
    add(_mode);
  }

  void parse(int& argc, char* argv[]) {
    Gecode::BaseOptions::parse(argc,argv);
    if (_allSolutions.value())
      _solutions.value(0);
    if (_parallel.value())
      _threads.value(0);
  }
  
  unsigned int solutions(void) const { return _solutions.value(); }
  double threads(void) const { return _threads.value(); }
  bool free(void) const { return _free.value(); }
  unsigned int c_d(void) const { return _c_d.value(); }
  unsigned int a_d(void) const { return _a_d.value(); }
  unsigned int node(void) const { return _node.value(); }
  unsigned int fail(void) const { return _fail.value(); }
  unsigned int time(void) const { return _time.value(); }
  Gecode::ScriptMode mode(void) const {
    return static_cast<Gecode::ScriptMode>(_mode.value());
  }
};

class GECODE_FLATZINC_EXPORT FlatZincGecode : public Gecode::Space {
public:
  enum Meth { SAT, MIN, MAX };
protected:
  int intVarCount;
  int boolVarCount;
  int setVarCount;

  int _optVar;
  
  int _noOfSols;
  
  Meth _method;

  void parseSolveAnn(AST::Array* ann);

  FlatZincGecode(bool share, FlatZincGecode&);
private:
  template <template<class> class Engine>
  void
  runEngine(std::ostream& out, const FZPrinter& p, 
            const FlatZincOptions& opt, Gecode::Support::Timer& t_total);
public:
  Gecode::IntVarArray iv;
  std::vector<bool> iv_introduced;
  Gecode::BoolVarArray bv;
  std::vector<bool> bv_introduced;
#ifdef GECODE_HAS_SET_VARS
  Gecode::SetVarArray sv;
  std::vector<bool> sv_introduced;
#endif

  FlatZincGecode(int intVars, int boolVars, int setVars);
  
  void newIntVar(IntVarSpec* vs);
  void newBoolVar(BoolVarSpec* vs);
  void newSetVar(SetVarSpec* vs);
  
  void postConstraint(const ConExpr& ce, AST::Node* annotation);
  
  void solve(AST::Array* annotation);
  void minimize(int var, AST::Array* annotation);
  void maximize(int var, AST::Array* annotation);
  
  void run(std::ostream& out, const FZPrinter& p, 
           const FlatZincOptions& opt, Gecode::Support::Timer& t_total);
  
  void print(std::ostream& out, const FZPrinter& p) const;

  virtual void constrain(const Space& s);

  Meth method(void);

  virtual Gecode::Space* copy(bool share);
};

#endif
