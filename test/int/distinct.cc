/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

namespace Test { namespace Int { namespace Distinct {

  /**
   * \defgroup TaskTestIntDistinct Distinct constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Simple test for distinct constraint
  class Distinct : public IntTest {
  public:
    /// Create and register test
    Distinct(const char* t, const IntSet& d, IntConLevel icl)
      : IntTest(t,6,d,false,icl) {}
    /// Create and register test
    Distinct(const char* t, int min, int max, IntConLevel icl)
      : IntTest(t,6,min,max,false,icl) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (x[i]==x[j])
            return false;
      return true;
    }
    /// Post distinct constraint on \a x
    virtual void post(Space* home, IntVarArray& x) {
      distinct(home, x, icl);
    }
  };
  
  /// Randomized test for distinct constraint
  class DistinctRandom : public IntTest {
  public:
    /// Create and register test
    DistinctRandom(const char* t, int a, int min, int max, IntConLevel icl)
      : IntTest(t,a,min,max,false,icl) {
      testsearch = false;
    }
    /// Create and register initial assignment
    virtual Assignment* assignment(void) const {
      return new RandomAssignment(arity,dom,100);
    }
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (x[i]==x[j])
            return false;
      return true;
    }
    /// Post distinct constraint on \a x
    virtual void post(Space* home, IntVarArray& x) {
      distinct(home, x, icl);
    }
  };
  
  /// Simple test for distinct constraint with offsets
  class DistinctOffset : public IntTest {
  public:
    /// Create and register test
    DistinctOffset(const char* t, const IntSet& d, IntConLevel icl)
      : IntTest(t,6,d,false,icl) {}
    /// Create and register test
    DistinctOffset(const char* t, int min, int max, IntConLevel icl)
      : IntTest(t,6,min,max,false,icl) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (x[i]+i==x[j]+j)
            return false;
      return true;
    }
    /// Post distinct constraint on \a x
    virtual void post(Space* home, IntVarArray& x) {
      IntArgs c(x.size());
      for (int i=0; i<x.size(); i++)
        c[i]=i;
      distinct(home, c, x, icl);
    }
  };


  const int v[7] = {-1001,-1000,-10,0,10,1000,1001};
  Gecode::IntSet d(v,7);

  Distinct dom_d("Distinct::Dom::Dense",-3,3,ICL_DOM);
  Distinct bnd_d("Distinct::Bnd::Dense",-3,3,ICL_BND);
  Distinct val_d("Distinct::Val::Dense",-3,3,ICL_VAL);
  Distinct dom_s("Distinct::Dom::Sparse",d,ICL_DOM);
  Distinct bnd_s("Distinct::Bnd::Sparse",d,ICL_BND);
  Distinct val_s("Distinct::Val::Sparse",d,ICL_VAL);

  DistinctRandom dom_r("Distinct::Dom::Random",20,-50,50,ICL_DOM);
  DistinctRandom bnd_r("Distinct::Bnd::Random",50,-500,500,ICL_BND);
  DistinctRandom val_r("Distinct::Val::Random",50,-500,500,ICL_VAL);

  DistinctOffset dom_od("Distinct::Dom::Offset::Dense",-3,3,ICL_DOM);
  DistinctOffset bnd_od("Distinct::Bnd::Offset::Dense",-3,3,ICL_BND);
  DistinctOffset val_od("Distinct::Val::Offset::Dense",-3,3,ICL_VAL);
  DistinctOffset dom_os("Distinct::Dom::Offset::Sparse",d,ICL_DOM);
  DistinctOffset bnd_os("Distinct::Bnd::Offset::Sparse",d,ICL_BND);
  DistinctOffset val_os("Distinct::Val::Offset::Sparse",d,ICL_VAL);
  //@}

}}}

// STATISTICS: test-int
