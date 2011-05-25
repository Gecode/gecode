/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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
#include <gecode/search.hh>

#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

namespace Test { namespace Int {

   /// %Tests for scheduling constraints
   namespace Cumulatives {

     /**
      * \defgroup TaskTestIntCumulatives Cumnulatives scheduling constraint
      * \ingroup TaskTestInt
      */
     //@{
     /**
      * \brief Script for generating assignments.
      *
      * We are only interested in assignments that represent tasks (s_i,
      * d_i, e_i, h_i) such that the following hold:
      *   - The task starts at a positive time and has some extension.
      *   - The equation s_i + d_i = e_i holds.
      *   - The tasks are ordered to remove some symmetries, i.e.,
      *     s_i <= s_{i+1}
      */
     class Ass : public Gecode::Space {
     public:
       /// Store task information
       Gecode::IntVarArray x;
       /// Initialize model for assignments
       Ass(int n, const Gecode::IntSet& d) : x(*this, n, d) {
         using namespace Gecode;
         for (int i = 0; i < n; i += 4) {
           rel(*this, x[i+0] >= 0);
           rel(*this, x[i+1] >= 0);
           rel(*this, x[i+2] >= 0);
           rel(*this, x[i] + x[i+1] == x[i+2]);
           branch(*this, x, INT_VAR_NONE, INT_VAL_MIN);
         }
       }
       /// Constructor for cloning \a s
       Ass(bool share, Ass& s) : Gecode::Space(share,s) {
         x.update(*this, share, s.x);
       }
       /// Create copy during cloning
       virtual Gecode::Space* copy(bool share) {
         return new Ass(share,*this);
       }
     };

     /// Class for generating reasonable assignments
     class CumulativeAssignment : public Assignment {
       /// Current assignment
       Ass* cur;
       /// Next assignment
       Ass* nxt;
       /// Search engine to find assignments
       Gecode::DFS<Ass>* e;
     public:
       /// Initialize assignments for \a n0 variables and values \a d0
       CumulativeAssignment(int n, const Gecode::IntSet& d) : Assignment(n,d) {
         Ass* a = new Ass(n, d);
         e = new Gecode::DFS<Ass>(a);
         delete a;
         nxt = cur = e->next();
         if (cur != NULL)
           nxt = e->next();
       }
       /// %Test whether all assignments have been iterated
       virtual bool operator()(void) const {
         return nxt != NULL;
       }
       /// Move to next assignment
       virtual void operator++(void) {
         delete cur;
         cur = nxt;
         if (cur != NULL) nxt = e->next();
       }
       /// Return value for variable \a i
       virtual int  operator[](int i) const {
         assert((i>=0) && (i<n) && (cur != NULL));
         return cur->x[i].val();
       }
       /// Destructor
       virtual ~CumulativeAssignment(void) {
         delete cur; delete nxt; delete e;
       }
     };

     /// %Event to be scheduled
     class Event {
     public:
       int p, h; ///< Position and height of event
       bool start; ///< Whether event has just started
       /// Initialize event
       Event(int pos, int height, bool s) : p(pos), h(height), start(s) {}
       /// %Test whether this event is before event \a e
       bool operator<(const Event& e) const {
         return p<e.p;
       }
     };

     /// Describe that event is below a certain limit
     class Below {
     public:
       int limit; ///< limit
       /// Initialize
       Below(int l) : limit(l) {}
       /// %Test whether \a val is below limit
       bool operator()(int val) {
         return val <= limit;
       }
     };
     /// Describe that event is above a certain limit
     class Above {
     public:
       int limit; ///< limit
       /// Initialize
       Above(int l) : limit(l) {}
       /// %Test whether \a val is above limit
       bool operator()(int val) {
         return val >= limit;
       }
     };

     /// Check whether event \a e is valid
     template<class C>
     bool valid(std::vector<Event> e, C comp) {
       std::sort(e.begin(), e.end());
       unsigned int i = 0;
       int p = 0;
       int h = 0;
       int n = 0;
       while (i < e.size()) {
         p = e[i].p;
         while (i < e.size() && e[i].p == p) {
           h += e[i].h;
           n += (e[i].start ? +1 : -1);
           ++i;
         }
         if (n && !comp(h))
           return false;
       }
       return true;
     }

     /// %Test for cumulatives constraint
     class Cumulatives : public Test {
     protected:
       int ntasks;   ///< Number of tasks
       bool at_most; ///< Whether to use atmost reasoning
       int limit;    ///< Limit
     public:
       /// Create and register test
       Cumulatives(const std::string& s, int nt, bool am, int l)
         : Test("Cumulatives::"+s,nt*4,-1,2), ntasks(nt), at_most(am), limit(l) {
         testsearch = false;
       }
       /// Create first assignment
       virtual Assignment* assignment(void) const {
         assert(arity == 4*ntasks);
         return new CumulativeAssignment(arity, dom);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         std::vector<Event> e;
         for (int i = 0; i < ntasks; ++i) {
           int p = i*4;
           // Positive start, duration and end
           if (x[p+0] < 0 || x[p+1] < 1 || x[p+2] < 1) return false;
           // Start + Duration == End
           if (x[p+0] + x[p+1] != x[p+2]) {
             return false;
           }
         }
         for (int i = 0; i < ntasks; ++i) {
           int p = i*4;
           // Up at start, down at end.
           e.push_back(Event(x[p+0], +x[p+3],  true));
           e.push_back(Event(x[p+2], -x[p+3], false));
         }
         if (at_most) {
           return valid(e, Below(limit));
         } else {
           return valid(e, Above(limit));
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntArgs m(ntasks), l(1, limit);
         IntVarArgs s(ntasks), d(ntasks), e(ntasks), h(ntasks);
         for (int i = 0; i < ntasks; ++i) {
           int p = i*4;
           m[i] = 0;
           s[i] = x[p+0]; rel(home, x[p+0], Gecode::IRT_GQ, 0);
           d[i] = x[p+1]; rel(home, x[p+1], Gecode::IRT_GQ, 1);
           e[i] = x[p+2]; rel(home, x[p+2], Gecode::IRT_GQ, 1);
           h[i] = x[p+3];
         }
         cumulatives(home, m, s, d, e, h, l, at_most);
       }
     };

     Cumulatives c1t1("1t1", 1,  true, 1);
     Cumulatives c1f1("1f1", 1, false, 1);
     Cumulatives c1t2("1t2", 1,  true, 2);
     Cumulatives c1f2("1f2", 1, false, 2);
     Cumulatives c1t3("1t3", 1,  true, 3);
     Cumulatives c1f3("1f3", 1, false, 3);
     Cumulatives c2t1("2t1", 2,  true, 1);
     Cumulatives c2f1("2f1", 2, false, 1);
     Cumulatives c2t2("2t2", 2,  true, 2);
     Cumulatives c2f2("2f2", 2, false, 2);
     Cumulatives c2t3("2t3", 2,  true, 3);
     Cumulatives c2f3("2f3", 2, false, 3);
     Cumulatives c3t1("3t1", 3,  true, 1);
     Cumulatives c3f1("3f1", 3, false, 1);
     Cumulatives c3t2("3t2", 3,  true, 2);
     Cumulatives c3f2("3f2", 3, false, 2);
     Cumulatives c3t3("3t3", 3,  true, 3);
     Cumulatives c3f3("3f3", 3, false, 3);
     Cumulatives c3t_1("3t-1", 3,  true, -1);
     Cumulatives c3f_1("3f-1", 3, false, -1);
     Cumulatives c3t_2("3t-2", 3,  true, -2);
     Cumulatives c3f_2("3f-2", 3, false, -2);
     Cumulatives c3t_3("3t-3", 3,  true, -3);
     Cumulatives c3f_3("3f-3", 3, false, -3);
     //@}

   }
}}

// STATISTICS: test-int
