/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <Chris.Mears@monash.edu>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christopher Mears, 2011
 *     Christian Schulte, 2011
 *     Guido Tack, 2011
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

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {

   /// %Tests for value precedence constraints
   namespace Precede {
            
     static IntSet ds(-1,3);
    
     /// %Test for single value precedence constraint
     class Single : public SetTest {
     private:
       /// The values for precedence
       int s, t;
       
       /// Check if \a i is a member of set \a x
       bool in(int i, int x) const {
         CountableSetRanges xr(ds,x);
         Iter::Ranges::Singleton ir(i,i);
         return Iter::Ranges::subset(ir,xr);
       }
       
     public:
       /// Create and register test
       Single(int s0, int t0)
         : SetTest("Precede::Single::"+str(s0)+"<"+str(t0),4,ds,false),
           s(s0), t(t0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const SetAssignment& x) const {
         int n = x.size();
         for (int i = 0 ; i < n ; i++) {
           if (!in(s,x[i]) && in(t,x[i]))
             return false;
           if (in(s,x[i]) && !in(t,x[i]))
             return true;
         }
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::SetVarArray& x,
                         Gecode::IntVarArray&) {
         Gecode::precede(home, x, s, t);
       }
     };
     
     /// %Test for multiple value precedence constraint
     class Multi : public SetTest {
     private:
       /// The values for precedence
       Gecode::IntArgs c;

       /// Check if \a i is a member of set \a x
       bool in(int i, int x) const {
         CountableSetRanges xr(ds,x);
         Iter::Ranges::Singleton ir(i,i);
         return Iter::Ranges::subset(ir,xr);
       }
     public:
       /// Create and register test
       Multi(const Gecode::IntArgs& c0)
         : SetTest("Precede::Multi::"+str(c0),4,ds,false), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const SetAssignment& x) const {
         for (int j=0; j<c.size()-1; j++)
           for (int i=0; i<x.size(); i++) {
             if (!in(c[j],x[i]) && in(c[j+1],x[i]))
               return false;
             if (in(c[j],x[i]) && !in(c[j+1],x[i]))
               break;
           }
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::SetVarArray& x,
                         Gecode::IntVarArray&) {
         Gecode::precede(home, x, c);
       }
     };
     
     Single _a(2, 3);
     Single _b(0, 3);

     Multi _c(Gecode::IntArgs(3, 1,2,3));
     Multi _d(Gecode::IntArgs(3, 3,2,1));
     Multi _e(Gecode::IntArgs(4, 4,2,3,1));

   }

}}

// STATISTICS: test-set
