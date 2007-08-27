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
#include "test/log.hh"
#include "gecode/support/sort.hh"

static Gecode::IntSet ds_12(1,2);
static Gecode::IntSet ds_13(1,3);
static Gecode::IntSet ds_14(1,4);
static Gecode::IntSet ds_03(0,3);

/*

class IntSortMin {
public:
  bool operator()(const int& x, const int& y) {
    return x < y;
  }
};

class SortPermAssignment : public Assignment{
  int problow;
  int probup;
  int permlow;
  int permup;
  int xsize;
public:
  SortPermAssignment(int xlow, int xup,
                     int plow, int pup,
                     int xs,
                     int n0, const Gecode::IntSet& d0)
    : Assignment(n0, d0),
      problow(xlow), probup(xup),
      permlow(plow), permup(pup),
      xsize(xs) {
    reset();
  }
  void reset(void) {
    done = false;
    Gecode::IntSet perm_dom(permlow, permup);
    Gecode::IntSet var_dom(problow, probup);
    for (int i = 2*xsize; i < n; i++) {
      dsv[i].init(perm_dom);
    }
    for (int i=0; i < 2*xsize; i++)
      dsv[i].init(var_dom);
  }
  void operator++(void) {
    Gecode::IntSet perm_dom(permlow, permup);
    Gecode::IntSet var_dom(problow, probup);
    int i = n-1;
    while (true) {
      ++dsv[i];
      if (dsv[i]())
        return;
      dsv[i].init(d);
      if (i >= 2*xsize && i < n) {
        dsv[i].init(perm_dom);
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

class Sortedness_NoVar : public IntTest {
private:
  Gecode::IntConLevel icl;
  static const int xs = 6;
  static const int ve = xs /2;

public:
  Sortedness_NoVar(const char* t, Gecode::IntConLevel icl0)
    : IntTest(t, xs, ds_13), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    int sortx[ve];
    for (int i = 0; i < ve; i++) {
      sortx[i] = x[i];
    }
    IntSortMin min_inc;
    Support::quicksort<int, IntSortMin> (&sortx[0], ve, min_inc);

//     std::cout << "defined sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       if (i < ve) {
//         std::cout << sortx[i]<<" ";
//       } else {
//         std::cout <<x[i] << " ";
//       }
//     }
//     std::cout <<"...";


    for (int i = ve; i < xs - 1; i++) {
      if ( !(x[i] <= x[i + 1]) ) {
        // std::cout << "y not sorted\n";
        return false;
      }
    }

    for (int i = ve; i < xs; i++) {
      if (sortx[i - ve] != x[i]) {
        // std::cout << "no sorting poss\n";
        return false;
      }
    }

    // std::cout << "valid\n";
    return true;
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs z(ve);
    for (int i = 0; i < ve; i++) {
      z[i] = x[i];
    }

    IntVarArgs y(ve);
    for (int i = ve; i < xs; i++) {
      y[i - ve] = x[i];
    }

    sortedness(home, z, y, icl);
  }
};


class Sortedness_PermVar : public IntTest {
private:
  Gecode::IntConLevel icl;
  static const int xs = 9;
  static const int ve1 = xs / 3;
  static const int ve2 = 2 * ve1;

  static const int xlow  = 1;
  static const int xup   = 3;
  static const int plow  = 0;
  static const int pup   = 2;
  static const int xsize = 3;

public:
  Sortedness_PermVar(const char* t, Gecode::IntConLevel icl0)
    : IntTest(t, xs, ds_03), icl(icl0) {}
  virtual Assignment* assignment(void) const {
    return new SortPermAssignment(1,3,0,2,3,9,dom);
  }

  virtual bool solution(const Assignment& x) const {


    int sortx[ve1];
    for (int i = 0; i < ve1; i++) {
      sortx[i] = x[i];
    }

    IntSortMin imin;
    Support::quicksort<int, IntSortMin> (&sortx[0], ve1, imin);

//     std::cout << "perm issol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve1 || i == ve2) std::cout << "||";
//       std::cout <<x[i] << " ";
//         }
//     std::cout <<"...";

    // ys have to be sorted
    for (int i = ve1; i < ve2 - 1; i++) {
      if ( !(x[i] <= x[i + 1]) ) {
//         std::cout << "y not sorted\n";
        return false;
      }
    }


    // assert correct domains
    for (int i = 0; i < ve2; i++) {
      if (!(x[i] > 0)) {
//         std::cout << "false domain\n";
        return false;
      }
    }

    for (int i = ve2; i < xs; i++) {
      if (! (x[i] < 3)) {
//         std::cout << "false domain 2\n";
        return false;
      }
    }

    // perm index
    for (int i = 0; i < ve1; i++) {
      if (x[i] != x[ve1 + x[i + ve2]]) {
//         std::cout << "wrong index\n";
        return false;
      }
    }

    // perm distinct
    for (int i = ve2; i < xs - 1; i++) {
      for (int j = i + 1; j < xs; j++) {
        if (x[i] == x[j]) {
//           std::cout << "no perm\n";
          return false;
        }
      }
    }


    for (int i = ve1; i < ve2; i++) {
      if (sortx[i - ve1] != x[i]) {
//         std::cout << "no sorting poss\n";
        return false;
      }
    }

//     std::cout << "valid\n";
    return true;
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs z(ve1);
    for (int i = 0; i < ve1; i++) {
      z[i] = x[i];
    }

    IntVarArgs y(ve1);
    for (int i = ve1; i < ve2; i++) {
      y[i - ve1] = x[i];
    }

    IntVarArgs p(ve1);
    for (int i = ve2; i < xs; i++) {
      p[i - ve2] = x[i];
    }

    for (int i = 0; i < ve1; i++) {
      rel(home, z[i], Gecode::IRT_GQ, 1);
      rel(home, y[i], Gecode::IRT_GQ, 1);
      rel(home, p[i], Gecode::IRT_LQ, 2);
    }
    sortedness(home, z, y, p, icl);
  }
};


Sortedness_NoVar  _sort_novar("Sortedness::NoPermutationVariables::Bnd",Gecode::ICL_BND);
Sortedness_PermVar  _sort_permvar("Sortedness::WithPermutationVariables::Bnd",Gecode::ICL_BND);

*/

// STATISTICS: test-int

