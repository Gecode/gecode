/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
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

namespace Test { namespace Int { namespace GCC {

  /**
   * \defgroup TaskTestIntGCC Counting constraints (global cardinality)
   * \ingroup TaskTestInt
   */
  //@{
  /// Test for integer cardinality with lower and upper bound for all variables
  class IntAllLbUb : public Test {
  public:
    /// Create and register test
    IntAllLbUb(Gecode::IntConLevel icl)
      : Test("GCC::Int::All::(lb,ub)::"+str(icl),4,1,4,false,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n[4];
      for (int i=4; i--; )
        n[i]=0;
      for (int i=x.size(); i--; )
        n[x[i]-1]++;
      for (int i=4; i--;)
        if (n[i]>2)
          return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntArgs values(4);
      IntSet fixed(0,2);
      IntSetArgs cards(4);
      for (int i=0; i<4; i++) {
        values[i] = i+1; cards[i] = fixed;
      }
      count(home, x, cards, values, icl);
    }
  };
  
  /// Test for integer cardinality with upper bound for all variables
  class IntAllEqUb : public Test {
  public:
    /// Create and register test
    IntAllEqUb(Gecode::IntConLevel icl)
      : Test("GCC::Int::All::ub::"+str(icl), 4, 1,2, false, icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n[2];
      for (int i=2; i--; )
        n[i] = 0;
      for (int i=x.size(); i--; )
        n[x[i] - 1]++;
      if (n[0] != 2 || n[1] != 2)
        return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::IntArgs values(2, 1,2);
      Gecode::count(home, x, Gecode::IntSet(2,2), values, icl);
    }
  };
  
  
  /// Test for integer cardinality with for some variables
  class IntSome : public Test {
  public:
    /// Create and register test
    IntSome(Gecode::IntConLevel icl)
      : Test("GCC::Int::Some::(v,lb,ub)::"+str(icl),4,1,4,false,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n[4];
      for (int i=4; i--; )
        n[i]=0;
      for (int i=x.size(); i--; )
        n[x[i]-1]++;
      if ((n[0] < 2) || (n[1] < 2) || (n[2] > 0) || (n[3] > 0))
        return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntArgs values(2, 1,2);
      Gecode::IntSet fixed(0,2);
      Gecode::IntSetArgs cards(2);
      cards[0]=fixed; cards[1]=fixed;
      count(home, x, cards, values, icl);
    }
  };
  
  IntAllLbUb val_all(Gecode::ICL_VAL);
  IntAllLbUb bnd_all(Gecode::ICL_BND);
  IntAllLbUb dom_all(Gecode::ICL_DOM);
  
  IntAllEqUb bnd_alleq(Gecode::ICL_BND);
  IntAllEqUb dom_alleq(Gecode::ICL_DOM);
  IntAllEqUb val_alleq(Gecode::ICL_VAL);

  IntSome bnd_some(Gecode::ICL_BND);
  IntSome dom_some(Gecode::ICL_DOM);
  IntSome val_some(Gecode::ICL_VAL);


/*

using namespace Gecode;

static Gecode::IntSet ds_02(0,2);

class GCCAssignment : public Assignment {
  int problow;
  int probup;
  int cardlow;
  int cardup;
  int xsize;
public:
  /// Create and register test
  GCCAssignment(int xlow, int xup,
                int clow, int cup,
                int xs,
                int n0, const Gecode::IntSet& d0)
    : Assignment(n0, d0),
      problow(xlow), probup(xup),
      cardlow(clow), cardup(cup),
      xsize(xs) {
    reset();
  }

  void reset(void) {
    done = false;
    Gecode::IntSet card_dom(cardlow, cardup);
    Gecode::IntSet var_dom(problow, probup);
    for (int i = xsize; i < n; i++) {
      dsv[i].init(card_dom);
    }
    for (int i = 0; i < xsize; i++ )
      dsv[i].init(var_dom);
  }
  void operator++(void) {
    Gecode::IntSet card_dom(cardlow, cardup);
    Gecode::IntSet var_dom(problow, probup);
    int i = n-1;
    while (true) {
      ++dsv[i];
      if (dsv[i]())
        return;
      if (i >= xsize && i < n) {
        dsv[i].init(card_dom);
      } else {
        dsv[i].init(var_dom);
      }
      --i;
      if (i<0) {
        done = true;
        return;
      }
    }
  }
};


*/


/*
class VC_AllLbUb : public Test {
private:
  static const int lb = 0;
  static const int rb = 2;

  static const int xs = 7;
  static const int ve = 4;

  static const int minocc = 0;
  static const int maxocc = 2;



public:
  virtual Assignment* assignment(void) const {
    return new GCCAssignment(lb, rb, minocc, maxocc, ve, xs, dom);
  }

  /// Create and register test
  VC_AllLbUb(const char* t, Gecode::IntConLevel icl)
    : Test(t, xs, ds_02, false,icl) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
//     std::cout << "GCC-Sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       std::cout << x[i] << " ";
//     }
//     std::cout << "...";

    for (int i = 0; i < ve; i++) {
      if ( x[i] < lb || x[i] > rb) {
//         std::cout << "wrong bounds\n";
        return false;
      }
    }

    GECODE_AUTOARRAY(int, count, xs - ve);
    for (int i = ve; i < xs; i++) {
      count[i - ve] = 0;
      if (x[i] < minocc || x[i] > maxocc) {
//         std::cout << "min-max-occ\n";
        return false;
      }
    }

    for (int i = 0; i < ve; i++) {
      count[x[i]]++;
    }

    for (int i = 0; i < xs - ve; i++) {
      if (count[i] != x[i + ve]) {
//         std::cout << "counting failed\n";
        return false;
      }
    }
//     std::cout << "valid\n";
    return true;
  }

  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    // std::cout << "test_post\n";

    // get the number of used values
    GECODE_AUTOARRAY(bool, done, xs - ve);
    for (int i = 0; i < xs - ve; i++) {
      done[i] = false;
    }

//     int nov = 0;
//     for (int i = 0; i < xs - ve; i++) {
//       for (int j = 0; j < ve; j++) {
//         if (x[j].in(i) && !done[i]) {
//           nov++;
//           done[i] = true;
//         }
//       }
//     }

//     std::cout << "nov = "<<nov<<"\n";
//     for (int i = ve; i < ve + nov; i++) {
    IntVarArgs cards(xs - ve);
    for (int i = ve; i < xs; i++) {
      cards[i - ve] = x[i];
//       rel(home, y[i - ve], Gecode::IRT_LQ, maxocc);
//       rel(home, y[i - ve], Gecode::IRT_GQ, minocc);
    }

    IntVarArgs vars(ve);
    for (int i = 0; i < ve; i++) {
//       std::cout << x[i] << " ";
      vars[i] = x[i];
    }
//     std::cout <<"\n";
//     gcc(home, z, c, 12, 2, icl);
    
    count(home, vars, cards, icl);
  }
};


class VC_AllTriple : public Test {
private:
  static const int lb = 0;
  static const int rb = 2;

  static const int xs = 7;
  static const int ve = 4;

  static const int minocc = 0;
  static const int maxocc = 2;

public:
  virtual Assignment* assignment(void) const {
    return new GCCAssignment(lb, rb, minocc, maxocc, ve, xs, dom);
  }


  /// Create and register test
  VC_AllTriple(const char* t, Gecode::IntConLevel icl)
    : Test(t, xs, ds_02, false,icl) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
//     std::cout << "GCC-Sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       std::cout << x[i] << " ";
//     }
//     std::cout << "\n";

    for (int i = 0; i < ve; i++) {
      if ( x[i] < lb || x[i] > rb) {
        // std::cout << "wrong bounds\n";
        return false;
      }
    }

    GECODE_AUTOARRAY(int, count, xs - ve);
    for (int i = ve; i < xs; i++) {
      count[i - ve] = 0;
      if (x[i] < minocc || x[i] > maxocc) {
        // std::cout << "min-max-occ\n";
        return false;
      }
    }

    for (int i = 0; i < ve; i++) {
      count[x[i]]++;
    }

//     std::cout << "count: ";
//     for (int i = 0; i < xs - ve; i++) {
//       std::cout << count[i] << " ";
//     }
//     std::cout << "\n";

    for (int i = 0; i < xs - ve; i++) {
      // std::cout << "comp: "<< count[i] <<" & "<<x[i + ve] << "\n";
      if (count[i] != x[i + ve]) {
        // std::cout << "count not met\n";
        return false;
      }
    }

    // std::cout << "valid\n";
    return true;
  }

  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    // std::cout << "test_post\n";

    // get the number of used values
    GECODE_AUTOARRAY(bool, done, xs - ve);
    for (int i = 0; i < xs - ve; i++) {
      done[i] = false;
    }

    IntVarArgs cards(xs - ve);
    for (int i = ve; i < xs; i++) {
      cards[i - ve] = x[i];
      rel(home, cards[i - ve], Gecode::IRT_LQ, maxocc);
      rel(home, cards[i - ve], Gecode::IRT_GQ, minocc);
//       if (i - ve == 0)
//         rel(home, cards[i - ve], Gecode::IRT_GQ, 1);
    }

    IntVarArgs vars(ve);
    for (int i = 0; i < ve; i++) {
      vars[i] = x[i];
    }
    
    // gcc(home, z, value, y, 3, 2, true, 0,2,  icl);
    count(home, vars, cards, icl);
    
  }
};


class VC_SomeTriple : public Test {
private:
  static const int lb = 0;
  static const int rb = 2;

  static const int xs = 6;
  static const int ve = 4;

  static const int minocc = 0;
  static const int maxocc = 2;

  virtual Assignment* assignment(void) const {
    return new GCCAssignment(lb, rb, minocc, maxocc, ve, xs, dom);
  }

public:
  /// Create and register test
  VC_SomeTriple(const char* t, Gecode::IntConLevel icl)
    : Test(t, xs, ds_02, false,icl) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
//     std::cout << "GCC-Sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       std::cout << x[i] << " ";
//     }
//     std::cout << "\n";

    for (int i = 0; i < ve; i++) {
      if ( x[i] < lb || x[i] > rb) {
//         std::cout << "wrong bounds\n";
        return false;
      }
    }

    GECODE_AUTOARRAY(int, count, xs - ve);
    for (int i = ve; i < xs; i++) {
      count[i - ve] = 0;
      if (x[i] < minocc || x[i] > maxocc) {
//         std::cout << "min-max-occ\n";
        return false;
      }
    }

    for (int i = 0; i < ve; i++) {
      if (x[i] == 0) {
        count[0]++;
      }
      if (x[i] == 1) {
        count[1]++;
      }
    }

    for (int i = 0; i < ve; i++) {
      if (x[i] == 2) {
//         std::cout << "2 not allowed!\n";
        return false;
      }
    }
//     std::cout << "\n";

    for (int i = 0; i < xs - ve; i++) {
//       std::cout << "comp: "<< count[i] <<" & "<<x[i + ve] << "\n";
      if (count[i] != x[i + ve]) {
//         std::cout << "count not met\n";
        return false;
      }
    }

//     std::cout << "valid\n";
    return true;
  }

  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {

    IntVarArgs cards(xs - ve);
    for (int i = ve; i < xs; i++) {
      cards[i - ve] = x[i];
//       rel(home, cards[i - ve], Gecode::IRT_LQ, maxocc);
//       rel(home, cards[i - ve], Gecode::IRT_GQ, minocc);
    }

    IntVarArgs vars(ve);
    for (int i = 0; i < ve; i++) {
      vars[i] = x[i];
    }

    IntArgs values(2);
    for (int i = 0; i < 2; i++)
      values[i] = i;

    // gcc(home, z, value, y, 2, 0, false, 0,2,  icl);
    count(home, vars, cards, values, icl);
  }
};

*/



// Testing with Cardinality Variables

/*
VC_AllLbUb bnd_all_var("GCC::VarCard::Bnd::All::(lb,ub)",Gecode::ICL_BND);
VC_AllLbUb dom_all_var("GCC::VarCard::Dom::All::(lb,ub)",Gecode::ICL_DOM);
VC_AllLbUb val_all_var("GCC::VarCard::Val::All::(lb,ub)",Gecode::ICL_VAL);

VC_AllTriple bnd_alltrip_var("GCC::VarCard::Bnd::All::(v,lb,ub)",Gecode::ICL_BND);
VC_AllTriple dom_alltrip_var("GCC::VarCard::Dom::All::(v,lb,ub)",Gecode::ICL_DOM);
VC_AllTriple val_alltrip_var("GCC::VarCard::Val::All::(v,lb,ub)",Gecode::ICL_VAL);

VC_SomeTriple bnd_sometrip__var("GCC::VarCard::Bnd::Some::(v,lb,ub)",Gecode::ICL_BND);
VC_SomeTriple dom_sometrip__var("GCC::VarCard::Dom::Some::(v,lb,ub)",Gecode::ICL_DOM);
VC_SomeTriple val_sometrip__var("GCC::VarCard::Val::Some::(v,lb,ub)",Gecode::ICL_VAL);
*/

  //@}

}}}

// STATISTICS: test-int

/*****

class GCC_FC_Shared_AllLbUb : public Test {
public:
  /// Create and register test
  GCC_FC_Shared_AllLbUb(const char* t, Gecode::IntConLevel icl)
    : Test(t,2,ds_14,false, icl) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    if (x[0] != x[1]) {
      return true;
    } else {
      return false;
    }
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(6);
    for (int i = 0; i < 6; i++) {
      if (i < 3) {
        y[i] = x[0];
      } else {
        y[i] = x[1];
      }
    }
    IntArgs values(4);
    Gecode::IntSet fixed(0,3);
    Gecode::IntSetArgs cards(4);
    for (int i = 1; i < 5; i++) {
      values[i - 1] = i;
      cards[i - 1] = fixed;
    }
    count(home, x, cards, values, icl);

  }
};


// GCC_FC_Shared_AllLbUb _gccbnd_shared_all("GCC::FixCard::Bnd::Shared::All::(lb,ub)",Gecode::ICL_BND);
// GCC_FC_Shared_AllLbUb _gccdom_shared_all("GCC::FixCard::Dom::Shared::All::(lb,ub)",Gecode::ICL_DOM);
// GCC_FC_Shared_AllLbUb _gccval_shared_all("GCC::FixCard::Val::Shared::All::(lb,ub)",Gecode::ICL_VAL);

*/
