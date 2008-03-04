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
 * The first element is the number of squares to be placed, the
 * second the size of the master square, and the remaining the
 * sizes of the squares to be packed.
 *
 * The data is taken from: C. J. Bouwkamp and A. J. W. 
 * Duijvestijn, Catalogue of Simple Perfect Squared Squares of Orders 
 * 21 Through 25, Eindhoven Univ. Technology, Dept. of Math., 
 * Report 92-WSK-03, Nov. 1992. 
 *
 * \relates PerfectSquare
 */
//@
const int s00[] = {
  21, 112,
  50,42,37,35,33,29,27,25,24,19,18,17,16,15,11,9,8,7,6,4,2
};
const int s01[] = {
  22, 110,
  60,50,28,27,26,24,23,22,21,18,17,16,15,14,13,12,8,7,6,4,3,2
};
const int s02[] = {
  22, 192,
  86,71,62,59,57,49,47,41,37,36,35,31,28,26,19,17,14,12,10,9,8,4
};
const int s03[] = {
  23, 110,
  44,41,38,37,32,31,29,28,21,19,16,15,14,13,12,10,8,7,5,4,3,2,1
};
const int s04[] = {
  23, 332,
  129,123,120,112,91,89,83,68,58,56,53,50,49,48,47,38,31,30,26,24,17,15,1
};
const int s05[] = {
  24, 120,
  47,46,41,40,34,33,32,25,23,20,19,17,16,15,14,13,12,10,9,8,6,5,4,3
};
const int s06[] = {
  24, 479,
  175,174,164,160,155,150,140,130,86,77,68,60,52,44,43,35,29,28,26,24,23,17,6,5
};
const int s07[] = {
  25, 147,
  74,73,41,40,34,33,32,27,25,23,20,19,17,16,15,14,13,12,10,9,8,6,5,4,3
};
const int s08[] = {
  25, 661,
  262,248,238,210,203,196,175,161,111,106,102,84,83,77,73,64,41,38,36,31,23,18,17,7,5
};
const int s09[] = {
  26, 212,
  99,85,65,62,57,56,55,48,39,38,32,28,26,24,23,20,19,17,16,12,7,6,5,4,2,1
};
const int s10[] = {
  26, 214,
  86,72,67,64,61,56,55,44,43,39,36,35,34,32,30,29,27,26,23,20,19,10,9,8,6,5
};
const int s11[] = {
  26, 825,
  304,302,288,277,246,235,233,189,157,135,127,117,109,92,90,83,81,76,57,53,49,37,26,25,8,5
};
const int s12[] = {
  27, 180,
  89,56,51,50,48,43,41,40,39,36,34,31,29,25,23,21,19,16,15,13,12,10,9,7,6,4,1
};
const int s13[] = {
  27, 1179,
  484,440,387,379,360,352,316,308,198,194,168,149,145,119,114,108,82,80,69,66,63,50,42,35,29,24,18
};
const int s14[] = {
  28, 201,
  77,70,68,67,64,56,54,39,38,36,34,32,30,24,22,21,18,17,16,13,12,11,10,6,4,3,2,1
};
const int s15[] = {
  28, 1544,
  649,615,510,473,456,439,419,385,260,216,214,208,203,175,147,135,125,116,104,94,81,55,49,17,12,7,6,4
};
const int s16[] = {
  29, 255,
  112,107,84,75,68,64,59,51,49,43,37,36,31,29,28,27,26,25,24,22,17,15,13,11,8,7,6,2,1
};
const int s17[] = {
  29, 2134,
  855,769,761,717,648,604,562,518,338,293,292,286,265,226,224,204,186,179,174,165,161,109,100,91,69,45,43,17,9
};
const int s18[] = {
  30, 237,
  88,82,79,76,73,56,53,46,45,43,40,39,36,34,33,32,29,27,25,24,23,21,20,16,11,10,9,5,3,1
};
const int s19[] = {
  30, 2710,
  992,981,948,936,826,782,781,737,465,440,418,289,272,264,260,242,227,210,208,154,140,124,122,108,92,64,29,16,15,4
};

const int* specs[] = {
  &s00[0],&s01[0],&s02[0],&s03[0],&s04[0],
  &s05[0],&s06[0],&s07[0],&s08[0],&s09[0],
  &s10[0],&s11[0],&s12[0],&s13[0],&s14[0],
  &s15[0],&s16[0],&s17[0],&s18[0],&s19[0]
};
const unsigned int n_specs = sizeof(specs) / sizeof(int*);
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
  PerfectSquare(const SizeOptions& opt)
    : x(this,specs[opt.size()][0],0,specs[opt.size()][1]-1), 
      y(this,specs[opt.size()][0],0,specs[opt.size()][1]-1) {

    const int* s = specs[opt.size()];
    int n = *s++;
    int w = *s++;

    // Restrict position according to square size
    for (int i=n; i--; ) {
      rel(this, x[i], IRT_LQ, w-s[i]);
      rel(this, y[i], IRT_LQ, w-s[i]);
    }

    // Squares do not overlap
    for (int i=0; i<n; i++)
      for (int j=i+1; j<n; j++)
        post(this, tt(~(x[j]-x[i] >= s[i]) || ~(x[i]-x[j] >= s[j]) ||
                      ~(y[j]-y[i] >= s[i]) || ~(y[i]-y[j] >= s[j])));

    /*
     * Symmetry breaking (assumes that square size are sorted)
     *
     */
    for (int i=n; i--; )
      if (s[i] == s[i+1]) 
        rel(this, x[i], IRT_LQ, x[i+1]);

    /*
     * Capacity constraints
     *
     */
    if (opt.propagation() == PROP_REIFIED) {
      IntArgs sa(n,s);
      BoolVarArgs b(n);
      for (int cx=0; cx<w; cx++) {
        for (int i=0; i<n; i++) {
          b[i].init(this,0,1);
          dom(this, x[i], cx-s[i]+1, cx, b[i]);
        }
        linear(this, sa, b, IRT_EQ, w);
      }
      for (int cy=0; cy<w; cy++) {
        for (int i=0; i<n; i++) {
          b[i].init(this,0,1);
          dom(this, y[i], cy-s[i]+1, cy, b[i]);
        }
        linear(this, sa, b, IRT_EQ, w);
      }
    } else {
      IntArgs m(n), dh(n);
      for (int i = n; i--; ) {
        m[i]=0; dh[i]=s[i];
      }
      IntVarArgs e(n);
      IntArgs limit(1);
      {
        // x-direction
        for (int i = n; i--; )
          e[i].init(this, 0, w);
        limit[0] = w;
        cumulatives(this, m, x, dh, e, dh, limit, true);
        cumulatives(this, m, x, dh, e, dh, limit, false);
      }
      {
        // y-direction
        for (int i = n; i--; )
          e[i].init(this, 0, w);
        limit[0] = w;
        cumulatives(this, m, y, dh, e, dh, limit, true);
        cumulatives(this, m, y, dh, e, dh, limit, false);
      }
    }

    branch(this, x, INT_VAR_MIN_MIN, INT_VAL_MIN);
    branch(this, y, INT_VAR_MIN_MIN, INT_VAL_MIN);
  }

  /// Constructor for cloning \a s
  PerfectSquare(bool share, PerfectSquare& s) : Example(share,s) {
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
  print(std::ostream& os) {
    os << "\t";
    for (int i=0; i<x.size(); i++)
      os << "(" << x[i] << "," << y[i] << ") ";
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates PerfectSquare
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("PerfectSquare");
  opt.propagation(PerfectSquare::PROP_REIFIED);
  opt.propagation(PerfectSquare::PROP_REIFIED,     "reified");
  opt.propagation(PerfectSquare::PROP_CUMULATIVES, "cumulatives");
  opt.parse(argc,argv);
  if (opt.size() >= n_specs) {
    std::cerr << "Error: size must be between 0 and " << n_specs - 1
              << std::endl;
    return 1;
  }
  Example::run<PerfectSquare,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

