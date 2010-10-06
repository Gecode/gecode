/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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
      virtual bool operator()(void) const {
        return dsv[0]();
      }
      /// Move to next assignment
      virtual void operator++(void) {
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
      mutable int il[4];
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
    
    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;

        IntArgs s1(3, 2,1,1);
        IntArgs s2(4, 1,2,3,4);
        IntArgs s3(4, 4,3,2,1);
        IntArgs s4(4, 1,2,4,8);
        IntArgs s5(4, 1,1,1,1);
        IntArgs s6(4, 1,1,2,2);
        IntArgs s7(4, 1,3,3,4);
        IntArgs s8(6, 1,2,4,8,16,32);

        for (int m=1; m<4; m++) {
          (void) new BPT(m, s1);
          (void) new BPT(m, s2);
          (void) new BPT(m, s3);
          (void) new BPT(m, s4);
          (void) new BPT(m, s5);
          (void) new BPT(m, s6);
          (void) new BPT(m, s7);
          (void) new BPT(m, s8);
          (void) new BPT(m, s1, false);
        }

      }
    };
    
    Create c;
    
    //@}
    
  }

}}


// STATISTICS: test-int

