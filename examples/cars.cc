/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include "examples/support.hh"

/**
 * \name Machine capabilities
 * \relates Cars
 */
//@{
IntArgs l1(6, 1,0,0,0,1,1);
IntArgs l2(6, 0,0,1,1,0,1);
IntArgs l3(6, 1,0,0,0,1,0);
IntArgs l4(6, 1,1,0,1,0,0);
IntArgs l5(6, 0,0,1,0,0,0);
//@}

/**
 * \brief %Example: Car sequencing
 *
 * Schedule a sequence of cars to be produced. This is problem 001 from
 * csplib.
 *
 * \ingroup ExProblem
 *
 */
class Cars : public Example {
private:
  IntVarArray x; BoolVarArray y;
public:

  BoolVar& o(int i, int j) {
    return y[(i-1)*5+j-1];
  }

  void
  atmost(int n, int m) {
    count(this, x, m, IRT_LQ, n);
  }

  void ele(const IntArgs& l, int i, int j, int k) {
    element(this, l, x[i-1], o(j,k));
  }

  void sum(int n, BoolVar& b0, BoolVar& b1) {
    BoolVarArgs b(2);
    b[0]=b0; b[1]=b1;
    linear(this, b,IRT_LQ,n);
  }

  void sum(int n, BoolVar& b0, BoolVar& b1, BoolVar& b2) {
    BoolVarArgs b(3);
    b[0]=b0; b[1]=b1; b[2]=b2;
    linear(this, b,IRT_LQ,n);
  }

  void sum(int n,
           BoolVar& b0, BoolVar& b1, BoolVar& b2, BoolVar& b3, BoolVar& b4) {
    BoolVarArgs b(5);
    b[0]=b0; b[1]=b1; b[2]=b2; b[3]=b3; b[4]=b4;
    linear(this, b,IRT_LQ,n);
  }

  Cars(const Options&)
    : x(this,10,0,5), y(this,50,0,1) {
    atmost(1,0);
    atmost(1,1);
    atmost(2,2);
    atmost(2,3);
    atmost(2,4);
    atmost(2,5);

    for (int i = 1; i < 11; i++) {
      ele(l1,i,i,1);
      ele(l2,i,i,2);
      ele(l3,i,i,3);
      ele(l4,i,i,4);
      ele(l5,i,i,5);
    }

    for (int i = 1; i <= 9; i++)
      sum(1,o(i,1),o(i+1,1));
    for (int i = 1; i <= 8; i++)
      sum(2,o(i,2),o(i+1,2),o(i+2,2));
    for (int i = 1; i <= 8; i++)
      sum(1,o(i,3),o(i+1,3),o(i+2,3));
    for (int i = 1; i <= 6; i++)
      sum(2,o(i,4),o(i+1,4),o(i+2,4),o(i+3,4),o(i+4,4));
    for (int i = 1; i <= 6; i++)
      sum(1,o(i,5),o(i+1,5),o(i+2,5),o(i+3,5),o(i+4,5));

    // redundant constraints

    /*
      O11+O21+O31+O41+O51+O61+O71+O81 #>= 4,
      O11+O21+O31+O41+O51+O61         #>= 3,
      O11+O21+O31+O41                 #>= 2,
      O11+O21                         #>= 1,

      O12+O22+O32+O42+O52+O62+O72     #>= 4,
      O12+O22+O32+O42                 #>= 2,
      O12                             #>= 0,

      O13+O23+O33+O43+O53+O63+O73     #>= 2,
      O13+O23+O33+O43                 #>= 1,
      O13                             #>= 0,

      O14+O24+O34+O44+O54             #>= 2,

      O15+O25+O35+O45+O55             #>= 1,
    */

    branch(this, x, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }

  Cars(bool share, Cars& s)
    : Example(share,s) {
    x.update(this,share,s.x);
    y.update(this,share,s.y);
  }

  virtual Space*
  copy(bool share) {
    return new Cars(share,*this);
  }

  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < x.size(); i++)
      std::cout << x[i] << ", ";
    std::cout << std::endl;
  }
};

int
main(int argc, char* argv[]) {
  Options o("Cars");
  o.solutions(0);
  o.iterations(100);
  o.parse(argc,argv);
  Example::run<Cars,DFS,Options>(o);
  return 0;
}

// STATISTICS: example-any

