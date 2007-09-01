/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "gecode/minimodel.hh"

namespace Test { namespace Int { namespace Channel {

  /**
   * \defgroup TaskTestIntChannel Channel constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Simple test for channel (testing all variables)
  class ChannelFull : public IntTest {
  public:
    /// Construct and register test
    ChannelFull(Gecode::IntConLevel icl)
      : IntTest("Channel::Full::"+str(icl),8,0,3,false,icl) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<4; i++)
        if (x[4+x[i]] != i)
          return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs xa(4); IntVarArgs ya(4);
      for (int i=4; i--; ) {
        xa[i] = x[i]; ya[i] = x[4+i];
      }
      channel(home, xa, ya, icl);
    }
  };
  
  /// Simple test for channel (testing single set of variables)
  class ChannelHalf : public IntTest {
  public:
    /// Construct and register test
    ChannelHalf(Gecode::IntConLevel icl)
      : IntTest("Channel::Half::"+str(icl),6,0,5,false,icl) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<6; i++)
        for (int j=i+1; j<6; j++)
          if (x[i] == x[j])
            return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      Gecode::IntVarArgs y(6);
      for (int i=0; i<6; i++)
        y[i].init(home,dom);
      for (int i=0; i<6; i++)
        for (int j=0; j<6; j++) {
          Gecode::BoolVar b(home,0,1);
          rel(home, x[i], Gecode::IRT_EQ, j, b);
          rel(home, y[j], Gecode::IRT_EQ, i, b);
        }
      channel(home, x, y, icl);
    }
  };
  
  /// Test channel with shared variables
  class ChannelShared : public IntTest {
  public:
    /// Construct and register test
    ChannelShared(Gecode::IntConLevel icl)
      : IntTest("Channel::Shared::"+str(icl),6,0,5,false,icl) {
      testdomcon = false;
    }
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<6; i++)
        if (x[x[i]] != i)
          return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      channel(home, x, x, icl);
    }
  };
  
  /// Test channel between integer and Boolean variable
  class ChannelLinkSingle : public IntTest {
  public:
    /// Construct and register test
    ChannelLinkSingle(void)
      : IntTest("Channel::Bool::Single",2,-1,2) {
      testdomcon = false;
    }
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return ((x[0]==0) || (x[0]==1)) && (x[0]==x[1]);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      Gecode::BoolVar b(home,0,1);
      channel(home, x[0], b);
      channel(home, x[1], b);
    }
  };
  
  /// Test channel between integer variable and array of Boolean variables
  class ChannelLinkMulti : public IntTest {
  private:
    int o;
  public:
    /// Construct and register test
    ChannelLinkMulti(const std::string& s, int min, int max, int o0)
      : IntTest("Channel::Bool::Multi::"+s,7,min,max), o(o0) {
      testdomcon = false;
    }
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n = x.size()-1;
      for (int i=n; i--; )
        if ((x[i] != 0) && (x[i] != 1))
          return false;
      int k=x[n]-o;
      if ((k<0) || (k>=n))
        return false;
      for (int i=0; i<k; i++)
        if (x[i] != 0)
          return false;
      for (int i=k+1; i<n; i++)
        if (x[i] != 0)
          return false;
      return x[k] == 1;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      int n=x.size()-1;
      Gecode::BoolVarArgs b(n);
      for (int i=n; i--; )
        b[i]=channel(home,x[i]);
      channel(home, b, x[n], o);
    }
  };
  


  ChannelFull cfd(Gecode::ICL_DOM);
  ChannelFull cfv(Gecode::ICL_VAL);

  ChannelHalf chd(Gecode::ICL_DOM);
  ChannelHalf chv(Gecode::ICL_VAL);

  ChannelShared csd(Gecode::ICL_DOM);
  ChannelShared csv(Gecode::ICL_VAL);

  ChannelLinkSingle cls;

  ChannelLinkMulti clma("A", 0, 5, 0);
  ChannelLinkMulti clmb("B", 1, 6, 1);
  ChannelLinkMulti clmc("C",-1, 4,-1);
  //@}

}}}

// STATISTICS: test-int

