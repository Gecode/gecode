/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2013
 *     Guido Tack, 2004
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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include "test/test.hh"

namespace Test {

  /// Tests for search using no-goods
  namespace NoGoods {

    using namespace Gecode;

    /// A dummy function for branching
    void dummy(Space&) {
    }

    /// Example for testing integer no-goods
    class Queens : public Space {
    public:
      /// Number of queens (must be even)
      const static int n = 18;
      /// Position of queens on boards
      IntVarArray q;
      /// The actual problem
      Queens(IntValBranch ivb, bool assign, bool null)
        : q(*this,n,0,n-1) {
        distinct(*this, IntArgs::create(n,0,1), q, IPL_VAL);
        distinct(*this, IntArgs::create(n,0,-1), q, IPL_VAL);
        distinct(*this, q, IPL_VAL);
        if (assign) {
          IntVar x(*this,0,1); Gecode::assign(*this, x, INT_ASSIGN_MIN());
        }
        {
          IntVarArgs q1(n/2), q2(n/2);
          for (int i=0; i<n/2; i++) {
            q1[i] = q[i]; q2[i] = q[n/2 + i];
          }
          branch(*this, q1, INT_VAR_NONE(), ivb);
          if (null)
            branch(*this, &dummy);
          branch(*this, q2, INT_VAR_NONE(), ivb);
        }
        if (assign) {
          IntVar x(*this,0,1); Gecode::assign(*this, x, INT_ASSIGN_MIN());
        }
      }
      /// Constructor for cloning \a s
      Queens(Queens& s) : Space(s) {
        q.update(*this, s.q);
      }
      /// Perform copying during cloning
      virtual Space* copy(void) {
        return new Queens(*this);
      }
      /// Check whether two solutions are the same
      bool same(const Queens& s) const {
        for (int i=0; i<q.size(); i++)
          if (q[i].val() != s.q[i].val())
            return false;
        return true;
      }
      /// Return increment for node stop
      static unsigned int nodeinc(void) {
        return 500;
      }
      /// Return name
      static std::string name(void) {
        return "Queens";
      }
      /// Return name for branching
      static std::string val(IntValBranch ivb) {
        switch (ivb.select()) {
        case IntValBranch::SEL_MIN: return "INT_VAL_MIN";
        case IntValBranch::SEL_MAX: return "INT_VAL_MAX";
        case IntValBranch::SEL_SPLIT_MIN: return "INT_VAL_SPLIT_MIN";
        case IntValBranch::SEL_SPLIT_MAX: return "INT_VAL_SPLIT_MAX";
        case IntValBranch::SEL_VALUES_MIN: return "INT_VALUES_MIN";
        case IntValBranch::SEL_VALUES_MAX: return "INT_VALUES_MAX";
        default: GECODE_NEVER;
        }
        return "";
      }
    };

#ifdef GECODE_HAS_SET_VARS
    /// Example for testing set no-goods
    class Hamming : public Space {
    private:
      /// Number of symbols (must be even)
      static const int size = 16;
      /// Minimal Hamming-distance
      static const int distance = 4;
      /// Number of bits
      static const int bits = 8;
      /// The hamming code
      SetVarArray x;
    public:
      /// Actual model
      Hamming(SetValBranch svb, bool assign, bool null) :
        x(*this,size,IntSet::empty,1,bits) {
        SetVarArgs cx(x.size());
        for (int i=x.size(); i--;)
          cx[i] = expr(*this, -x[i]);

        for (int i=0; i<x.size(); i++)
          for (int j=i+1; j<x.size(); j++)
            rel(*this,
                cardinality(x[j] & cx[i]) +
                cardinality(x[i] & cx[j]) >= distance);

        if (assign) {
          IntVar x(*this,0,1); Gecode::assign(*this, x, INT_ASSIGN_MIN());
        }
        {
          SetVarArgs x1(size/2), x2(size/2);
          for (int i=0; i<size/2; i++) {
            x1[i] = x[i]; x2[i] = x[size/2 + i];
          }
          branch(*this, x1, SET_VAR_NONE(), svb);
          if (null)
            branch(*this, &dummy);
          branch(*this, x2, SET_VAR_NONE(), svb);
        }
        if (assign) {
          IntVar x(*this,0,1); Gecode::assign(*this, x, INT_ASSIGN_MIN());
        }
      }
      /// Constructor for copying \a s
      Hamming(Hamming& s) : Space(s) {
        x.update(*this, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(void) {
        return new Hamming(*this);
      }
      /// Check whether two solutions are the same
      bool same(const Hamming& s) const {
        for (int i=0; i<x.size(); i++) {
          SetVarGlbRanges a(x[i]), b(s.x[i]);
          if (!Iter::Ranges::equal(a,b))
            return false;
        }
        return true;
      }
      /// Return increment for node stop
      static unsigned int nodeinc(void) {
        return 35;
      }
      /// Return name
      static std::string name(void) {
        return "Hamming";
      }
      /// Return name for branching
      static std::string val(SetValBranch svb) {
        switch (svb.select()) {
        case SetValBranch::SEL_MIN_INC: return "SET_VAL_MIN_INC";
        case SetValBranch::SEL_MAX_INC: return "SET_VAL_MAX_INC";
        case SetValBranch::SEL_MIN_EXC: return "SET_VAL_MIN_EXC";
        case SetValBranch::SEL_MAX_EXC: return "SET_VAL_MAX_EXC";
        default: GECODE_NEVER;
        }
        return "";
      }
    };
#endif

    /// %Base class for no-good tests
    template<class Model, class ValBranch>
    class NoGoods : public Base {
    protected:
      /// How to branch
      ValBranch vb;
      /// Number of threads to use
      unsigned int t;
      /// Whether to also assign some variables
      bool a;
      /// Whether to also create branchers without no-good literals
      bool n;
    public:
      /// Map unsigned integer to string
      static std::string str(unsigned int i) {
        std::stringstream s;
        s << i;
        return s.str();
      }
      /// Initialize test
      NoGoods(ValBranch vb0, unsigned int t0, bool a0, bool n0)
        : Base("NoGoods::"+Model::name()+"::"+Model::val(vb0)+"::"+str(t0)+
               "::"+(a0 ? "+" : "-")+"::"+(n0 ? "+" : "-")),
          vb(vb0), t(t0), a(a0), n(n0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(vb,a,n);
        // Solution without no-goods
        Model* s_plain = dfs(m);
        // Stop and re-start for a while to collect no-goods
        {
          Search::NodeStop ns(Model::nodeinc());
          Search::Options o;
          o.stop = &ns;
          o.threads = t;
          o.nogoods_limit = 256U;
          Search::Engine* e = Search::dfsengine(m,o);
          while (true) {
            Model* s = static_cast<Model*>(e->next());
            delete s;
            if (!e->stopped())
              break;
            // Add no-goods
            e->nogoods().post(*m);
            ns.limit(ns.limit()+Model::nodeinc());
          }
          delete e;
        }
        // Compare whether the a or the same solution is found with no-goods
        Model* s_nogoods = dfs(m);

        bool ok = ((s_nogoods != NULL) &&
                   ((t != 1) || s_plain->same(*s_nogoods)));

        delete m;
        delete s_nogoods;
        delete s_plain;

        return ok;
      }
    };


    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        bool a = false;
        do {
          bool n = false;
          do {
            for (unsigned int t = 1; t<=4; t++) {
              (void) new NoGoods<Queens,IntValBranch>(INT_VAL_MIN(),t,a,n);
              (void) new NoGoods<Queens,IntValBranch>(INT_VAL_MAX(),t,a,n);
              (void) new NoGoods<Queens,IntValBranch>(INT_VAL_SPLIT_MIN(),t,a,n);
              (void) new NoGoods<Queens,IntValBranch>(INT_VAL_SPLIT_MAX(),t,a,n);
              (void) new NoGoods<Queens,IntValBranch>(INT_VALUES_MIN(),t,a,n);
              (void) new NoGoods<Queens,IntValBranch>(INT_VALUES_MAX(),t,a,n);
#ifdef GECODE_HAS_SET_VARS
              (void) new NoGoods<Hamming,SetValBranch>(SET_VAL_MIN_INC(),t,a,n);
              (void) new NoGoods<Hamming,SetValBranch>(SET_VAL_MIN_EXC(),t,a,n);
              (void) new NoGoods<Hamming,SetValBranch>(SET_VAL_MAX_INC(),t,a,n);
              (void) new NoGoods<Hamming,SetValBranch>(SET_VAL_MAX_EXC(),t,a,n);
#endif
            }
            n = !n;
          } while (n);
          a = !a;
        } while (a);
      }
    };

    Create c;
  }

}

// STATISTICS: test-search
