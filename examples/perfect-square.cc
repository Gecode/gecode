/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include "examples/support.hh"
#include "gecode/minimodel.hh"

/**
 * \name Specifications for perfect square problems
 *
 * \relates PerfectSquare
 */
//@
/// PerfectSquare problem specification
class SquareSpec {
public:
  int x; int y;
  int n; const int* s;
  SquareSpec(int x0, int y0, const int s0[]) :
    x(x0), y(y0), s(s0) {
    int i = 0;
    while (s[i]) i++;
    n = i;
  }
};

static const int s0_s[] = {2,2,2,2,0};
static const SquareSpec s0(4,4,s0_s);

static const int s1_s[] = {3,2,2,1,1,1,0};
static const SquareSpec s1(5,4,s1_s);

static const int s2_s[] = {6,4,4,4,2,2,2,2,0};
static SquareSpec s2(10,10,s2_s);

static const int s3_s[] = {9,8,8,7,5,4,4,4,4,4,3,3,3,2,2,1,1,0};
static const SquareSpec s3(20,20,s3_s);

static const int s4_s[] = {18,15,14,10,9,8,7,4,1,0};
static const SquareSpec s4(32,33,s4_s);

static const int s5_s[] = {25,24,23,22,19,17,11,6,5,3,0};
static const SquareSpec s5(65,47,s5_s);

static const int s6_s[] = {50,42,37,35,33,29,27,25,24,19,18,
                           17,16,15,11,9,8,7,6,4,2,0};
static const SquareSpec s6(112,112,s6_s);

static const int s7_s[] = {81,64,56,55,51,43,39,38,35,33,31,30,29,20,
                           18,16,14,9,8,5,4,3,2,1,0};
static const SquareSpec s7(175,175,s7_s);

static const SquareSpec* specs[] = {&s0,&s1,&s2,&s3,&s4,&s5,&s6,&s7};
static const unsigned int n_examples = sizeof(specs) / sizeof(SquareSpec*);
//@}

/**
 * \brief %Example: packing squares into a rectangle
 *
 * See problem 9 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 */
class PerfectSquare : public Example {
protected:
  /// Specification used
  const SquareSpec& s;
  /// Array of x-coordinates of squares
  IntVarArray x;
  /// Array of y-coordinates of squares
  IntVarArray y;
public:
  /// Propagation to use for model
  enum {
    PROP_REIFIED,     ///< Use reified constraints
    PROP_CUMULATIVES, ///< Use cumulatives constraint
  };
  /// Actual model
  PerfectSquare(const Options& opt)
    : s(*specs[opt.size]),
      x(this,s.n,0,s.x-1), y(this,s.n,0,s.y-1) {

    // Restrict position according to square size
    for (int i=s.n; i--; ) {
      rel(this, x[i], IRT_LQ, s.x-s.s[i]);
      rel(this, y[i], IRT_LQ, s.y-s.s[i]);
    }

    // Squares do not overlap
    for (int i=0; i<s.n; i++)
      for (int j=i+1; j<s.n; j++)
        post(this, tt(~(x[j]-x[i] >= s.s[i]) || ~(x[i]-x[j] >= s.s[j]) ||
                      ~(y[j]-y[i] >= s.s[i]) || ~(y[i]-y[j] >= s.s[j])));

    /*
     * Symmetry breaking
     *
     */
    for (int i=s.n-1; i--; )
      if (s.s[i] == s.s[i+1])
        rel(this, x[i], IRT_LQ, x[i+1]);

    /*
     * Capacity constraints
     *
     */
    if (opt.propagation() == PROP_REIFIED) {
      IntArgs sa(s.n,s.s);
      BoolVarArgs b(s.n);
      for (int cx=0; cx<s.x; cx++) {
        for (int i=0; i<s.n; i++) {
          BoolVar b_cx(this,0,1);
          dom(this, x[i], cx-s.s[i]+1, cx, b_cx);
          b[i] = b_cx;
        }
        linear(this, sa, b, IRT_EQ, s.y);
      }
      for (int cy=0; cy<s.y; cy++) {
        for (int i=0; i<s.n; i++) {
          BoolVar b_cy(this,0,1);
          dom(this, y[i], cy-s.s[i]+1, cy, b_cy);
          b[i] = b_cy;
        }
        linear(this, sa, b, IRT_EQ, s.x);
      }
    } else {
      IntArgs m(s.n), dh(s.n);
      for (int i = s.n; i--; ) {
        m[i]  = 0;
        dh[i] = s.s[i];
      }
      IntVarArgs e(s.n);
      IntArgs limit(1);
      {
        // x-direction
        for (int i = s.n; i--; ) {
          IntVar ei(this, 0, s.x);
          e[i] = ei;
        }
        limit[0] = s.y;
        cumulatives(this, m, x, dh, e, dh, limit,  true);
        cumulatives(this, m, x, dh, e, dh, limit, false);
      }
      {
        // y-direction
        for (int i = s.n; i--; ) {
          IntVar ei(this, 0, s.y);
          e[i] = ei;
        }
        limit[0] = s.x;
        cumulatives(this, m, y, dh, e, dh, limit,  true);
        cumulatives(this, m, y, dh, e, dh, limit, false);
      }
    }

    branch(this, x, BVAR_MIN_MIN, BVAL_MIN);
    branch(this, y, BVAR_MIN_MIN, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  PerfectSquare(bool share, PerfectSquare& s) : Example(share,s), s(s.s) {
    x.update(this, share, s.x);
    y.update(this, share, s.y);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new PerfectSquare(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i=0; i<s.n; i++)
      std::cout << "(" << x[i] << "," << y[i] << ") ";
    std::cout << std::endl;
  }
};

/** \brief Main-function
 *  \relates PerfectSquare
 */
int
main(int argc, char** argv) {
  Options opt("PerfectSquare");
  opt.size  = 7;
  opt.propagation(PerfectSquare::PROP_REIFIED);
  opt.propagation(PerfectSquare::PROP_REIFIED,     "reified");
  opt.propagation(PerfectSquare::PROP_CUMULATIVES, "cumulatives");
  opt.parse(argc,argv);
  if (opt.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and " << n_examples - 1
              << std::endl;
    return 1;
  }
  Example::run<PerfectSquare,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

