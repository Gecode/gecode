/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include "gecode/int.hh"
#include "gecode/minimodel.hh"
#include "gecode/search.hh"

#include "test/test.hh"

namespace Test { 

  /// Tests for search engines
  namespace Branch {

    using namespace Gecode;

    /// Values for selecting branchings
    enum HowToBranch {
      HTB_NONE,   ///< Do not branch
      HTB_UNARY,  ///< Branch with single alternative
      HTB_BINARY, ///< Branch with two alternatives
      HTB_NARY    ///< Branch with many alternatives
    };

    /// Values for selecting how to constrain
    enum HowToConstrain {
      HTC_NONE,         ///< Do not constrain
      HTC_LEX_SMALLEST, ///< Constrain for lexically smallest
      HTC_LEX_LARGEST,  ///< Constrain for lexically biggest 
      HTC_BAL_SMALLEST, ///< Constrain for smallest balance
      HTC_BAL_LARGEST   ///< Constrain for largest balance
    };

    /// Values for selecting models
    enum WhichModel {
      WM_FAIL_IMMEDIATE, ///< Model that fails immediately
      WM_FAIL_SEARCH,    ///< Model without solutions
      WM_SOLUTIONS       ///< Model with solutions
    };

    /*

    /// %Branching description storing position and values
    class PosValuesDesc : public BranchingDesc {
    protected:
      /// Position of view
      int _pos;
      /// Values to assign to
      int* _values;
    public:
      /// Initialize description for branching \a b, position \a p, and view \a x
      PosValuesDesc(const Branching* b, int p, IntView x)
        : BranchingDesc(b,x.size()), 
          _pos(p), 
          _values(static_cast<int*>(Memory::malloc(sizeof(int)*x.size()))) {
        ViewValues<IntView> v(x);
        int i=0;
        while (v()) {
          _values[i]=v.val(); ++i; ++v;
        }
      }
      /// Return position in array
      int pos(void) const {
        return _pos;
      }
      /// Return value to branch with
      int val(unsigned int a) const {
        return _values[a];
      }
      /// Report size occupied
      virtual size_t size(void) const {
        0;
      }
      /// Destructor
      virtual ~PosValuesDesc(void) {
        Memory::free(_values);
      }
    };


    /// Class for nary branchings
    class NaryBranching : public Branching {
    protected:
      ViewArray<View> x; ///< Views to branch on
      /// Constructor for cloning \a b
      NaryBranching(Space* home, bool share, NaryBranching& b) 
        : Branching(home,share,b) {
        x.update(home,share,b.x);
      }
    public:
      /// Constructor for creation
      NaryBranching(Space* home, ViewArray<View>& x0) 
        : Branching(home), x(x0) {}
      /// Check status of branching, return true if alternatives left
      virtual bool status(const Space* home) const {
        for (int i=0; i<x.size(); i++)
          if (!x[i].assigned())
            return true;
        return false;
      }
      /// Return branching description
      virtual const BranchingDesc* description(const Space* home) const {
        int i=0; 
        while (x[i].assigned()) 
          i++;
        return new PosValuesDesc(this,i,x[i]);
      }
      /// Perform commit for branching description \a d and alternative \a a
      virtual ExecStatus commit(Space* home, const BranchingDesc* d,
                                unsigned int a) {
        const PosValuesDesc* pvd = static_cast<const PosValuesDesc*>(d);
        return me_failed(x[pvd->pos()].eq(home,pvd->val(a)))
          ? ES_FAILED : ES_OK;
      }
      /// Return specification for this branching given a variable map \a m
      virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m) {
        return "ViewValBranching";
      }

      /// Return specification for a branch
      virtual Reflection::BranchingSpec branchingSpec(Space* home, 
                                                      Reflection::VarMap& m,
                                                      const BranchingDesc* d);
      /// Actor type identifier of this branching
      static Support::Symbol ati(void) {
        return "Test::Search::ViewValuesBranching";
      }
      /// Post branching according to specification
      static void post(Space* home, Reflection::VarMap& m,
                       const Reflection::ActorSpec& spec);
      /// Perform cloning
      virtual Actor* copy(Space* home, bool share);
    };


    */

    /// Space that immediately fails
    class FailImmediate : public Space {
    public:
      /// Variables used
      IntVarArray x;
      /// Constructor for space creation
      FailImmediate(void) : x(this,1,0,0) {
        rel(this, x[0], IRT_EQ, 1);
      }
      /// Constructor for cloning \a s
      FailImmediate(bool share, FailImmediate& s) : Space(share,s) {
        x.update(this, share, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new FailImmediate(share,*this);
      }
      /// Add constraint for next better solution
      void constrain(Space*) {
      }
    };

    /// Space that requires propagation and has solutions
    class HasSolutions : public Space {
    public:
      /// Variables used
      IntVarArray x;
      /// How to constrain
      HowToConstrain htc;
      /// Branch on \ x according to \a htb
      void branch(const IntVarArgs& x, HowToBranch htb) {
        switch (htb) {
        case HTB_NONE:
          break;
        case HTB_UNARY:
          assign(this, x, INT_ASSIGN_MIN);
          break;
        case HTB_BINARY:
          Gecode::branch(this, x, INT_VAR_NONE, INT_VAL_MIN);
          break;
        case HTB_NARY:
          assign(this, x, INT_ASSIGN_MIN);
          break;
        }
      }
      /// Constructor for space creation
      HasSolutions(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
                   HowToConstrain htc0) 
        : x(this,6,0,5), htc(htc0) {
        distinct(this,x);
        IntVarArgs x1(2); x1[0]=x[0]; x1[1]=x[1]; branch(x1, htb1);
        IntVarArgs x2(2); x2[0]=x[2]; x2[1]=x[2]; branch(x2, htb2);
        IntVarArgs x3(2); x3[0]=x[4]; x3[1]=x[4]; branch(x3, htb3);
      }
      /// Constructor for cloning \a s
      HasSolutions(bool share, HasSolutions& s) 
        : Space(share,s), htc(s.htc) {
        x.update(this, share, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new HasSolutions(share,*this);
      }
      /// Add constraint for next better solution
      void constrain(Space* _s) {
        HasSolutions* s = static_cast<HasSolutions*>(_s);
        switch (htc) {
        case HTC_NONE:
          break;
        case HTC_LEX_SMALLEST:
        case HTC_LEX_LARGEST:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i].init(this, s->x[i].val(), s->x[i].val());
            lex(this, x, (htc == HTC_LEX_SMALLEST) ? IRT_LE : IRT_GR, y);
            break;
          }
        case HTC_BAL_SMALLEST:
        case HTC_BAL_LARGEST:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i].init(this, s->x[i].val(), s->x[i].val());
            IntVar xs(this, -18, 18);
            IntVar ys(this, -18, 18);
            post(this, x[0]+x[1]+x[2]-x[3]-x[4]-x[5] == xs);
            post(this, y[0]+y[1]+y[2]-y[3]-y[4]-y[5] == ys);
            rel(this, 
                abs(this,xs), 
                (htc == HTC_BAL_SMALLEST) ? IRT_LE : IRT_GR,
                abs(this,ys));
            break;
          }
        }
      }
    };

  }

}

// STATISTICS: test-search
