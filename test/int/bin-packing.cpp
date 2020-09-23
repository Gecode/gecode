/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

#include "test/int.hh"

#include <gecode/minimodel.hh>
#include <climits>

namespace Test { namespace Int {

  /// %Tests for bin-packing constraint
  namespace BinPacking {

    /**
     * \defgroup TaskTestIntBinPacking Bin-packing constraints
     * \ingroup TaskTestInt
     */
    //@{
    /// Generate load and bin assignments
    class LoadBinAssignment : public Assignment {
    protected:
      /// Number of bins
      int n_bins;
      /// Number of items
      int n_items;
      /// Domain for load variables
      Gecode::IntSet d_load;
      /// Domain for bin variables
      Gecode::IntSet d_bin;
      /// Load to generate (unless -1)
      int load;
      /// Iterator for each variable
      Gecode::IntSetValues* dsv;
    public:
      /// Initialize assignments for load and bin variables
      LoadBinAssignment(int m, const Gecode::IntSet& d_m,
                        int n, const Gecode::IntSet& d_n,
                        int l)
        : Assignment(m+n,d_m),
          n_bins(m), n_items(n), d_load(d_m), d_bin(d_n), load(l),
          dsv(new Gecode::IntSetValues[static_cast<unsigned int>(m+n)]) {
        for (int i=n_bins; i--; )
          dsv[i].init(d_load);
        for (int i=n_items; i--; )
          dsv[n_bins+i].init(d_bin);
      }
      /// Test whether all assignments have been iterated
      virtual bool has_more(void) const {
        return dsv[0]();
      }
      /// Move to next assignment
      virtual void next(Gecode::Support::RandomGenerator&) {
        // Try to generate next bin assignment
        {
          int i = n_items-1;
          while (i >= 0) {
            ++dsv[n_bins+i];
            if (dsv[n_bins+i]())
              return;
            dsv[n_bins+(i--)].init(d_bin);
          }
        }
        // Try to generate next load assignment
        {
        retry:
          int i = n_bins-1;
          while (true) {
            ++dsv[i];
            if (dsv[i]() || (i == 0)) {
              if (dsv[i]() && (load >= 0)) {
                int l = 0;
                for (int k=0;k<n_bins; k++)
                  l += dsv[k].val();
                if (load != l)
                  goto retry;
              }
              return;
            }
            dsv[i--].init(d_load);
          }
        }
      }
      /// Return value for variable \a i
      virtual int operator[](int i) const {
        assert((i>=0) && (i<n_bins+n_items));
        return dsv[i].val();
      }
      /// Destructor
      virtual ~LoadBinAssignment(void) {
        delete [] dsv;
      }
    };

    /// %Test with different bin loads and items
    class BPT : public Test {
    protected:
      /// Number of bins
      int m;
      /// Item sizes
      Gecode::IntArgs s;
      /// Whether to generate only valid loads
      bool valid;
      /// Total item sizes
      int t;
      /// Array of sufficient size for computing item loads
      mutable int il[8];
      /// Compute total size
      static int total(const Gecode::IntArgs& s) {
        int t = 0;
        for (int i=s.size(); i--; )
          t += s[i];
        return t;
      }
    public:
      /// Create and register test for \a m bins and item sizes \a s
      BPT(int m0, const Gecode::IntArgs& s0, bool v=true)
        : Test("BinPacking::"+str(m0)+"::"+str(s0)+"::"+(v ? "+" : "-"),
               m0+s0.size(), 0, 100),
          m(m0), s(s0), valid(v), t(total(s)) {
        testsearch = false;
      }
      /// Create assignment
      virtual Assignment* assignment(void) const {
        // Compute plausible bin sizes
        int a = t / m;
        return new LoadBinAssignment(m,Gecode::IntSet(a-1,a+2),
                                     s.size(),Gecode::IntSet(0,m-1),
                                     valid ? t : -1);
      }
      /// %Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        // Loads are from 0 to m-1, after that are items
        // Check whether loads sum up to total size
        int l=0;
        for (int j=m; j--; )
          l += x[j];
        if (l != t)
          return false;
        // Check whether items are at possible bins
        for (int j=m; j--; )
          if ((x[m+j] < 0) || (x[m+j] >= m))
            return false;
        // Compute whether items add up
        for (int j=m; j--; )
          il[j] = 0;
        for (int i=s.size(); i--; )
          il[x[m+i]] += s[i];
        for (int j=m; j--; )
          if (il[j] != x[j])
            return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        using namespace Gecode;
        IntVarArgs l(m);
        IntVarArgs b(s.size());
        for (int j=m; j--; )
          l[j]=x[j];
        for (int i=s.size(); i--; )
          b[i]=x[m+i];
        binpacking(home, l, b, s);
      }
    };

    /// %Test with different bin loads and items
    class MBPT : public Test {
    protected:
      /// Dimension
      int d;
      /// Number of bins
      int m;
      /// Item sizes
      Gecode::IntArgs s;
      /// Bin capacities
      Gecode::IntArgs c;
      /// Array of sufficient size for computing item loads
      mutable int il[4][8];
    public:
      /// Create and register test for \a d0 dimensions, \a m0 bins, item sizes \a s0, and capacities \a c0
      MBPT(int d0, int m0,
           const Gecode::IntArgs& s0, const Gecode::IntArgs& c0)
        : Test("MultiBinPacking::"+str(d0)+"::"+str(m0)+"::"+
               str(s0)+"::"+str(c0), s0.size() / d0, 0, m0-1),
          d(d0), m(m0), s(s0), c(c0) {
        testsearch = false;
        testfix = false;
      }
      /// %Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        // x are the bin variables
        for (int k=d; k--; )
          for (int j=m; j--; )
            il[k][j] = 0;
        for (int k=d; k--; )
          for (int i=x.size(); i--; )
            il[k][x[i]] += s[i*d+k];
        for (int k=d; k--; )
          for (int j=m; j--; )
            if (il[k][j] > c[k])
              return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        using namespace Gecode;
        IntVarArgs l(d*m);
        for (int j=m*d; j--; )
          l[j]=IntVar(home, 0, Gecode::Int::Limits::max);
        binpacking(home, d, l, x, s, c);
      }
    };

    /// Test for testing the max-clique finding for multi bin-packing
    class CliqueMBPT : public Base {
    protected:
      /// Number of items
      int n_items;
      /// Expected clique
      Gecode::IntArgs clique;
      /// Simple test space class
      class TestSpace : public Gecode::Space {
      public:
        // Constructor
        TestSpace(void) {}
        // Copy function
        virtual Gecode::Space* copy(void) {
          return nullptr;
        }
      };
    public:
      /// Test for number of items \a n expected clique \a c
      CliqueMBPT(const Gecode::IntArgs& c)
        : Base("Int::MultiBinPacking::Clique::"+Test::str(c)), clique(c) {}
      /// Run the actual test
      virtual bool run(void) {
        using namespace Gecode;
        TestSpace* home = new TestSpace;
        /*
         * Set up a multi-dimensional bin packing problems of dimension 2
         * where the item sizes in one dimension are all one but for some
         * random items and two in the other dimension if the item is
         * included in the clique  and where the capacity in both dimensions
         * is 3.
         */
        // Number of items
        int n_items = clique[clique.size()-1] + 1;
        // Capacity
        IntArgs c({3,3});
        // Item sizes
        IntArgs s(2*n_items);
        for (int i=2*n_items; i--; )
          s[i]=1;
        // Create some random conflicts
        for (int i=clique.size()-1; i--; )
          s[_rand(n_items)*2+0]=2;
        // Create conflicts corresponding to the clique
        for (int i=clique.size(); i--; )
          s[clique[i]*2+1]=2;
        // Load and bin variables
        IntVarArgs b(*home, n_items, 0, n_items-1);
        IntVarArgs l(*home, 2*n_items, 0, 3);
        IntSet mc = binpacking(*home, 2, l, b, s, c);
        if (home->status() == SS_FAILED) {
          delete home;
          return false;
        }
        if (static_cast<unsigned int>(clique.size()) != mc.size()) {
          delete home;
          return false;
        }
        for (int i=clique.size(); i--; )
          if (!mc.in(clique[i])) {
            delete home;
            return false;
          }
        delete home;
        return true;
      }
    };

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;

        {
          IntArgs s0({0,0,0,0});
          IntArgs s1({2,1,1});
          IntArgs s2({1,2,3,4});
          IntArgs s3({4,3,2,1});
          IntArgs s4({1,2,4,8});
          IntArgs s5({1,1,1,1});
          IntArgs s6({1,1,2,2});
          IntArgs s7({1,3,3,4});
          IntArgs s8({1,3,3,0,4,0});
          IntArgs s9({1,2,4,8,16,32});

          for (int m=1; m<4; m++) {
            (void) new BPT(m, s0);
            (void) new BPT(m, s1);
            (void) new BPT(m, s2);
            (void) new BPT(m, s3);
            (void) new BPT(m, s4);
            (void) new BPT(m, s5);
            (void) new BPT(m, s6);
            (void) new BPT(m, s7);
            (void) new BPT(m, s8);
            (void) new BPT(m, s9);
            (void) new BPT(m, s1, false);
          }
        }

        {
          IntArgs s1({1,2, 2,1, 1,2, 2,1});
          IntArgs c1({3,3});
          (void) new MBPT(2, 4, s1, c1);
          (void) new MBPT(2, 6, s1, c1);
          IntArgs s2({1,1, 1,1, 1,1});
          IntArgs c21({1,1});
          IntArgs c22({2,2});
          (void) new MBPT(2, 6, s2, c21);
          (void) new MBPT(2, 6, s2, c22);
          IntArgs s3({1,2,3, 3,2,1, 2,1,3, 1,3,2});
          IntArgs c31({3,3,3});
          IntArgs c32({4,4,4});
          IntArgs c33({6,6,6});
          (void) new MBPT(3, 4, s3, c31);
          (void) new MBPT(3, 4, s3, c32);
          (void) new MBPT(3, 4, s3, c33);
          (void) new MBPT(3, 5, s3, c31);
          (void) new MBPT(3, 5, s3, c32);
          (void) new MBPT(3, 5, s3, c33);
        }

        {
          IntArgs c1({0,2,4,6});
          IntArgs c2({1,2,3,4,5,6,7,8});
          IntArgs c3({1,3,7,10,15,22,27,97});
          IntArgs c41({1,2,3,4,5,6,7,14});
          IntArgs c42({1,2,3,4,5,6,7,15});
          IntArgs c43({1,2,3,4,5,6,7,16});
          IntArgs c44({1,2,3,4,5,6,7,30});
          IntArgs c45({1,2,3,4,5,6,7,31});
          IntArgs c46({1,2,3,4,5,6,7,32});
          IntArgs c47({1,2,3,4,5,6,7,62});
          IntArgs c48({1,2,3,4,5,6,7,63});
          IntArgs c49({1,2,3,4,5,6,7,64});

          (void) new CliqueMBPT(c1);
          (void) new CliqueMBPT(c2);
          (void) new CliqueMBPT(c3);
          (void) new CliqueMBPT(c41);
          (void) new CliqueMBPT(c42);
          (void) new CliqueMBPT(c43);
          (void) new CliqueMBPT(c44);
          (void) new CliqueMBPT(c45);
          (void) new CliqueMBPT(c46);
          (void) new CliqueMBPT(c47);
          (void) new CliqueMBPT(c48);
          (void) new CliqueMBPT(c49);
        }
      }
    };

    Create c;

    //@}

  }

}}


// STATISTICS: test-int

