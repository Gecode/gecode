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
  namespace Search {

    using namespace Gecode;
    using namespace Gecode::Int;

    /// Values for selecting branchings
    enum HowToBranch {
      HTB_NONE,   ///< Do not branch
      HTB_UNARY,  ///< Branch with single alternative
      HTB_BINARY, ///< Branch with two alternatives
      HTB_NARY    ///< Branch with many alternatives
    };

    /// Values for selecting how to constrain
    enum HowToConstrain {
      HTC_NONE,   ///< Do not constrain
      HTC_LEX_LE, ///< Constrain for lexically smallest
      HTC_LEX_GR, ///< Constrain for lexically biggest 
      HTC_BAL_LE, ///< Constrain for smallest balance
      HTC_BAL_GR  ///< Constrain for largest balance
    };

    /// Values for selecting models
    enum WhichModel {
      WM_FAIL_IMMEDIATE, ///< Model that fails immediately
      WM_FAIL_SEARCH,    ///< Model without solutions
      WM_SOLUTIONS       ///< Model with solutions
    };

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
      /// Return value to branch with for alternative \a a
      int val(unsigned int a) const {
        return _values[a];
      }
      /// Report size occupied
      virtual size_t size(void) const {
        return 0;
      }
      /// Destructor
      virtual ~PosValuesDesc(void) {
        Memory::free(_values);
      }
    };


    /// Class for nary branchings
    class NaryBranching : public Branching {
    protected:
      /// Views to branch on
      ViewArray<IntView> x; 
      /// Constructor for cloning \a b
      NaryBranching(Space* home, bool share, NaryBranching& b) 
        : Branching(home,share,b) {
        x.update(home,share,b.x);
      }
    public:
      /// Constructor for creation
      NaryBranching(Space* home, ViewArray<IntView>& x0) 
        : Branching(home), x(x0) {}
      /// Perform cloning
      virtual Actor* copy(Space* home, bool share) {
        return new (home) NaryBranching(home,share,*this);
      }
      /// Check status of branching, return true if alternatives left
      virtual bool status(const Space*) const {
        for (int i=0; i<x.size(); i++)
          if (!x[i].assigned())
            return true;
        return false;
      }
      /// Return branching description
      virtual const BranchingDesc* description(const Space*) const {
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
      /// Actor type identifier of this branching
      static Support::Symbol ati(void) {
        return "Test::Search::NaryBranching";
      }
      /// Return specification for this branching given a variable map \a m
      virtual Reflection::ActorSpec spec(Space* home, Reflection::VarMap& m) {
        Reflection::ActorSpec s(ati());
        return s << x.spec(home, m);
      }

      /// Return specification for a branch
      virtual Reflection::BranchingSpec branchingSpec(Space*, 
                                                      Reflection::VarMap&,
                                                      const BranchingDesc* d) {
        Reflection::BranchingSpec bs(d);
        return bs;
      }
      /// Post branching according to specification
      static void post(Space*, Reflection::VarMap&,
                       const Reflection::ActorSpec&) {
      }
    };

    /// Space with information
    class TestSpace : public Space {
    public:
      /// Constructor for space creation
      TestSpace(void) {}
      /// Constructor for cloning \a s
      TestSpace(bool share, TestSpace& s) : Space(share,s) {}
      /// Return number of solutions
      virtual int solutions(void) const = 0;
      /// Verify that this is best solution
      virtual bool best(void) const = 0;
    };

    /// Space that immediately fails
    class FailImmediate : public TestSpace {
    public:
      /// Variables used
      IntVarArray x;
      /// Constructor for space creation
      FailImmediate(HowToBranch, HowToBranch, HowToBranch, 
                    HowToConstrain=HTC_NONE) 
        : x(this,1,0,0) {
        rel(this, x[0], IRT_EQ, 1);
      }
      /// Constructor for cloning \a s
      FailImmediate(bool share, FailImmediate& s) : TestSpace(share,s) {
        x.update(this, share, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new FailImmediate(share,*this);
      }
      /// Add constraint for next better solution
      void constrain(Space*) {
      }
      /// Return number of solutions
      virtual int solutions(void) const {
        return 0;
      }
      /// Verify that this is best solution
      virtual bool best(void) const {
        return false;
      }
      /// Return name
      static std::string name(void) {
        return "Fail";
      }
    };

    /// Space that requires propagation and has solutions
    class HasSolutions : public TestSpace {
    public:
      /// Variables used
      IntVarArray x;
      /// How to branch
      HowToBranch htb1, htb2, htb3;
      /// How to constrain
      HowToConstrain htc;
      /// Branch on \a x according to \a htb
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
          {
            ViewArray<IntView> xv(this,x);
            new (this) NaryBranching(this, xv);
            break;
          }
        }
      }
      /// Constructor for space creation
      HasSolutions(HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
                   HowToConstrain _htc=HTC_NONE) 
        : x(this,6,0,5), htb1(_htb1), htb2(_htb2), htb3(_htb3), htc(_htc) {
        distinct(this, x);
        rel(this, x[2], IRT_LQ, 3); rel(this, x[3], IRT_LQ, 3);
        rel(this, x[4], IRT_LQ, 1); rel(this, x[5], IRT_LQ, 1);
        IntVarArgs x1(2); x1[0]=x[0]; x1[1]=x[1]; branch(x1, htb1);
        IntVarArgs x2(2); x2[0]=x[2]; x2[1]=x[3]; branch(x2, htb2);
        IntVarArgs x3(2); x3[0]=x[4]; x3[1]=x[5]; branch(x3, htb3);
      }
      /// Constructor for cloning \a s
      HasSolutions(bool share, HasSolutions& s) 
        : TestSpace(share,s), 
          htb1(s.htb1), htb2(s.htb2), htb3(s.htb3), htc(s.htc) {
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
        case HTC_LEX_LE:
        case HTC_LEX_GR:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i].init(this, s->x[i].val(), s->x[i].val());
            lex(this, x, (htc == HTC_LEX_LE) ? IRT_LE : IRT_GR, y);
            break;
          }
        case HTC_BAL_LE:
        case HTC_BAL_GR:
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
                (htc == HTC_BAL_LE) ? IRT_LE : IRT_GR,
                abs(this,ys));
            break;
          }
        }
      }
      /// Return number of solutions
      virtual int solutions(void) const {
        if (htb1 == HTB_NONE) {
          assert((htb2 == HTB_NONE) && (htb3 == HTB_NONE));
          return 1;
        }
        if ((htb1 == HTB_UNARY) || (htb2 == HTB_UNARY))
          return 0;
        if (htb3 == HTB_UNARY)
          return 4;
        return 8;
      }
      /// Verify that this is best solution
      virtual bool best(void) const {
        if ((htb1 == HTB_NONE) || (htb2 == HTB_NONE) || (htb3 == HTB_NONE) ||
            (htb1 == HTB_UNARY) || (htb2 == HTB_UNARY) || (htb3 == HTB_UNARY))
          return true;
        switch (htc) {
        case HTC_NONE: 
          return true;
        case HTC_LEX_LE:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==2) && (x[3].val()==3) && 
                  (x[4].val()==0) && (x[5].val()==1));
        case HTC_LEX_GR:
          return ((x[0].val()==5) && (x[1].val()==4) &&
                  (x[2].val()==3) && (x[3].val()==2) &&
                  (x[4].val()==1) && (x[5].val()==0));
        case HTC_BAL_LE:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==2) && (x[3].val()==3) && 
                  (x[4].val()==0) && (x[5].val()==1));
        case HTC_BAL_GR:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==3) && (x[3].val()==2) &&
                  (x[4].val()==0) && (x[5].val()==1));
        default: GECODE_NEVER;
        }
        return false;
      }
      /// Return name
      static std::string name(void) {
        return "Sol";
      }
    };

    /// Base class for search tests
    class Test : public Base {
    public:
      /// How to branch
      HowToBranch htb1, htb2, htb3;
      /// How to constrain
      HowToConstrain htc;
      /// Map unsigned integer to string
      static std::string str(unsigned int i) {
        std::stringstream s;
        s << i;
        return s.str();
      }
      /// Map branching to string
      static std::string str(HowToBranch htb) {
        switch (htb) {
        case HTB_NONE:   return "None";
        case HTB_UNARY:  return "Unary";
        case HTB_BINARY: return "Binary";
        case HTB_NARY:   return "Nary";
        default: GECODE_NEVER;
        }
        GECODE_NEVER;
        return "";
      }
      /// Map constrain to string
      static std::string str(HowToConstrain htc) {
        switch (htc) {
        case HTC_NONE:   return "None";
        case HTC_LEX_LE: return "LexLe";
        case HTC_LEX_GR: return "LexGr";
        case HTC_BAL_LE: return "BalLe";
        case HTC_BAL_GR: return "BalGr";
        default: GECODE_NEVER;
        }
        GECODE_NEVER;
        return "";
      }
      /// Initialize test
      Test(const std::string& s,
           HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
           HowToConstrain _htc=HTC_NONE) 
        : Base("Search::"+s), 
          htb1(_htb1), htb2(_htb2), htb3(_htb3), htc(_htc) {}
    };

    /// Test for depth-first search
    template <class Model>
    class DFS : public Test {
    private:
      /// Minimal recomputation distance
      unsigned int c_d;
      /// Adaptive recomputation distance
      unsigned int a_d;
    public:
      /// Initialize test
      DFS(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int c_d0, unsigned int a_d0) 
        : Test("DFS::"+Model::name()+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3)+"::"+
               str(c_d0)+"::"+str(a_d0),
               htb1,htb2,htb3), c_d(c_d0), a_d(a_d0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.c_d  = c_d;
        o.a_d  = a_d;
        o.stop = &f;
        Gecode::DFS<Model> dfs(m,o);
        int n = m->solutions();
        delete m;
        while (true) {
          Model* s = dfs.next();
          if (s != NULL) {
            n--; delete s;
          }
          if ((s == NULL) && !dfs.stopped())
            break;
          f.limit(f.limit()+2);
        }
        return n == 0;
      }
    };

    /// Test for limited discrepancy search
    template <class Model>
    class LDS : public Test {
    public:
      /// Initialize test
      LDS(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3)
        : Test("LDS::"+Model::name()+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3),
               htb1,htb2,htb3) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.stop = &f;
        Gecode::LDS<Model> lds(m,50,o);
        int n = m->solutions();
        delete m;
        while (true) {
          Model* s = lds.next();
          if (s != NULL) {
            n--; delete s;
          }
          if ((s == NULL) && !lds.stopped())
            break;
          f.limit(f.limit()+2);
        }
        return n == 0;
      }
    };

    /// Test for best solution search
    template <class Model, template<class> class Engine>
    class Best : public Test {
    private:
      /// Minimal recomputation distance
      unsigned int c_d;
      /// Adaptive recomputation distance
      unsigned int a_d;
    public:
      /// Initialize test
      Best(const std::string& b, HowToConstrain htc,
           HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
           unsigned int c_d0, unsigned int a_d0) 
        : Test(b+"::"+Model::name()+"::"+str(htc)+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3)+"::"+
               str(c_d0)+"::"+str(a_d0),
               htb1,htb2,htb3,htc), c_d(c_d0), a_d(a_d0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3,htc);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.c_d  = c_d;
        o.a_d  = a_d;
        o.stop = &f;
        Engine<Model> best(m,o);
        delete m;
        Model* b = NULL;
        while (true) {
          Model* s = best.next();
          if (s != NULL) {
            delete b; b=s;
          }
          if ((s == NULL) && !best.stopped())
            break;
          f.limit(f.limit()+2);
        }
        bool ok = (b == NULL) || b->best();
        delete b;
        return ok;
      }
    };

    /// Iterator for branching types
    class BranchTypes {
    private:
      /// Array of branching types
      static const HowToBranch htbs[3];
      /// Current position in branching type array
      int i; 
    public:
      /// Initialize iterator
      BranchTypes(void) : i(0) {}
      /// Test whether iterator is done
      bool operator()(void) const {
        return i<3;
      } 
      /// Increment to next branching type
      void operator++(void) {
        i++;
      }
      /// Return current branching type
      HowToBranch htb(void) const {
        return htbs[i];
      }
    };

    const HowToBranch BranchTypes::htbs[3] = {HTB_UNARY, HTB_BINARY, HTB_NARY};
    
    /// Iterator for constrain types
    class ConstrainTypes {
    private:
      /// Array of constrain types
      static const HowToConstrain htcs[4];
      /// Current position in constrain type array
      int i; 
    public:
      /// Initialize iterator
      ConstrainTypes(void) : i(0) {}
      /// Test whether iterator is done
      bool operator()(void) const {
        return i<4;
      } 
      /// Increment to next constrain type
      void operator++(void) {
        i++;
      }
      /// Return current constrain type
      HowToConstrain htc(void) const {
        return htcs[i];
      }
    };

    const HowToConstrain ConstrainTypes::htcs[4] = 
      {HTC_LEX_LE, HTC_LEX_GR, HTC_BAL_LE, HTC_BAL_GR};
    

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        // Depth-first search
        for (unsigned int c_d = 1; c_d<10; c_d++)
          for (unsigned int a_d = 1; a_d<=c_d; a_d++) {
            for (BranchTypes htb1; htb1(); ++htb1)
              for (BranchTypes htb2; htb2(); ++htb2)
                for (BranchTypes htb3; htb3(); ++htb3)
                  (void) new DFS<HasSolutions>(htb1.htb(),htb2.htb(),htb3.htb(),
                                               c_d, a_d);
            new DFS<FailImmediate>(HTB_NONE, HTB_NONE, HTB_NONE, c_d, a_d);
            new DFS<HasSolutions>(HTB_NONE, HTB_NONE, HTB_NONE, c_d, a_d);
          }

        // Limited discrepancy search
        for (BranchTypes htb1; htb1(); ++htb1)
          for (BranchTypes htb2; htb2(); ++htb2)
            for (BranchTypes htb3; htb3(); ++htb3)
              (void) new LDS<HasSolutions>(htb1.htb(),htb2.htb(),htb3.htb());
        new LDS<FailImmediate>(HTB_NONE, HTB_NONE, HTB_NONE);
        new LDS<HasSolutions>(HTB_NONE, HTB_NONE, HTB_NONE);

        // Best solution search
        for (unsigned int c_d = 1; c_d<10; c_d++)
          for (unsigned int a_d = 1; a_d<=c_d; a_d++) {
            for (ConstrainTypes htc; htc(); ++htc)
              for (BranchTypes htb1; htb1(); ++htb1)
                for (BranchTypes htb2; htb2(); ++htb2)
                  for (BranchTypes htb3; htb3(); ++htb3) {
                    (void) new Best<HasSolutions,BAB>
                      ("BAB",htc.htc(),htb1.htb(),htb2.htb(),htb3.htb(),c_d,a_d);
                    (void) new Best<HasSolutions,Restart>
                      ("Restart",htc.htc(),htb1.htb(),htb2.htb(),htb3.htb(),c_d,a_d);
                  }
            (void) new Best<FailImmediate,BAB>
              ("BAB",HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,c_d,a_d);
            (void) new Best<HasSolutions,BAB>
              ("BAB",HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,c_d,a_d);
          }

      }
    };

    Create c;
  }

}

// STATISTICS: test-search
