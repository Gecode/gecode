/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

namespace {

  class SimpleA : public IntTest {
  public:
    SimpleA(void)
      : IntTest("Regular::Simple::A",4,2,2) {}
    virtual bool solution(const Assignment& x) const {
      return (((x[0] == 0) || (x[0] == 2)) &&
              ((x[1] == -1) || (x[1] == 1)) &&
              ((x[2] == 0) || (x[2] == 1)) &&
              ((x[3] == 0) || (x[3] == 1)));
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      regular(home, x,
              (REG(0) | REG(2)) +
              (REG(-1) | REG(1)) +
              (REG(7) | REG(0) | REG(1)) +
              (REG(0) | REG(1)));
    }
  };
  



  class SimpleB : public IntTest {
  public:
    SimpleB(void)
      : IntTest("Regular::Simple::B",4,2,2) {}
    virtual bool solution(const Assignment& x) const {
      return (x[0]<x[1]) && (x[1]<x[2]) && (x[2]<x[3]);
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      regular(home, x,
              (REG(-2) + REG(-1) + REG(0) + REG(1)) |
              (REG(-2) + REG(-1) + REG(0) + REG(2)) |
              (REG(-2) + REG(-1) + REG(1) + REG(2)) |
              (REG(-2) + REG(0) + REG(1) + REG(2)) |
              (REG(-1) + REG(0) + REG(1) + REG(2)));
      }
  };
  
  class Distinct : public IntTest {
  public:
    Distinct(void)
      : IntTest("Regular::Distinct",4,-1,4) {}
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++) {
        if ((x[i] < 0) || (x[i] > 3))
          return false;
        for (int j=i+1; j<x.size(); j++)
          if (x[i]==x[j])
            return false;
      }
      return true;
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      regular(home, x,
              (REG(0)+REG(1)+REG(2)+REG(3)) |
              (REG(0)+REG(1)+REG(3)+REG(2)) |
              (REG(0)+REG(2)+REG(1)+REG(3)) |
              (REG(0)+REG(2)+REG(3)+REG(1)) |
              (REG(0)+REG(3)+REG(1)+REG(2)) |
              (REG(0)+REG(3)+REG(2)+REG(1)) |
              (REG(1)+REG(0)+REG(2)+REG(3)) |
              (REG(1)+REG(0)+REG(3)+REG(2)) |
              (REG(1)+REG(2)+REG(0)+REG(3)) |
              (REG(1)+REG(2)+REG(3)+REG(0)) |
              (REG(1)+REG(3)+REG(0)+REG(2)) |
              (REG(1)+REG(3)+REG(2)+REG(0)) |
              (REG(2)+REG(0)+REG(1)+REG(3)) |
              (REG(2)+REG(0)+REG(3)+REG(1)) |
              (REG(2)+REG(1)+REG(0)+REG(3)) |
              (REG(2)+REG(1)+REG(3)+REG(0)) |
              (REG(2)+REG(3)+REG(0)+REG(1)) |
              (REG(2)+REG(3)+REG(1)+REG(0)) |
              (REG(3)+REG(0)+REG(1)+REG(2)) |
              (REG(3)+REG(0)+REG(2)+REG(1)) |
              (REG(3)+REG(1)+REG(0)+REG(2)) |
              (REG(3)+REG(1)+REG(2)+REG(0)) |
              (REG(3)+REG(2)+REG(0)+REG(1)) |
              (REG(3)+REG(2)+REG(1)+REG(0)));
    }
  };
  
  class SharedA : public IntTest {
  public:
    SharedA(void)
      : IntTest("Regular::Shared::A",4,2,2) {}
    virtual bool solution(const Assignment& x) const {
      return (((x[0] == 0) || (x[0] == 2)) &&
              ((x[1] == -1) || (x[1] == 1)) &&
              ((x[2] == 0) || (x[2] == 1)) &&
              ((x[3] == 0) || (x[3] == 1)));
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs y(8);
      for (int i=0; i<4; i++)
        y[i]=y[i+4]=x[i];
      unshare(home,y);
      regular(home, y,
              ((REG(0) | REG(2)) +
               (REG(-1) | REG(1)) +
               (REG(7) | REG(0) | REG(1)) +
               (REG(0) | REG(1)))(2,2));
    }
  };

  class SharedB : public IntTest {
  public:
    SharedB(void)
      : IntTest("Regular::Shared::B",4,2,2) {}
    virtual bool solution(const Assignment& x) const {
      return (((x[0] == 0) || (x[0] == 2)) &&
              ((x[1] == -1) || (x[1] == 1)) &&
              ((x[2] == 0) || (x[2] == 1)) &&
              ((x[3] == 0) || (x[3] == 1)));
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs y(12);
      for (int i=0; i<4; i++)
        y[i]=y[i+4]=y[i+8]=x[i];
      unshare(home,y);
      regular(home, y,
              ((REG(0) | REG(2)) +
               (REG(-1) | REG(1)) +
               (REG(7) | REG(0) | REG(1)) +
               (REG(0) | REG(1)))(3,3));
    }
  };

  class SharedC : public IntTest {
  public:
    SharedC(void)
      : IntTest("Regular::Shared::C",4,0,1) {}
    virtual bool solution(const Assignment& x) const {
      return (x[1]==1) && (x[2]==0) && (x[3]==1);
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      Gecode::BoolVarArgs y(8);
      for (int i=0; i<4; i++)
        y[i]=y[i+4]=channel(home,x[i]);
      unshare(home,y);
      regular(home,y,
              ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(2,2));
    }
  };

  class SharedD : public IntTest {
  public:
    SharedD(void)
      : IntTest("Regular::Shared::D",4,0,1) {}
    virtual bool solution(const Assignment& x) const {
      return (x[1]==1) && (x[2]==0) && (x[3]==1);
    }
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      Gecode::BoolVarArgs y(12);
      for (int i=0; i<4; i++)
        y[i]=y[i+4]=y[i+8]=channel(home,x[i]);
      unshare(home, y);
      regular(home, y,
              ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(3,3));
    }
  };

  SimpleA a;
  SimpleB b;

  Distinct d;

  SharedA sa;
  SharedB sb;
  SharedC sc;
  SharedD sd;

}

// STATISTICS: test-int

