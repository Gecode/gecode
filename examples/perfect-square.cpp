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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

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
const int s20[] = {
  40, 510,
  219,173,156,135,134,128,124,118,114,95,81,79,71,65,63,59,58,55,54,51,49,46,34,33,32,31,28,24,21,20,19,18,17,16,14,10,8,4,3,1
};
const int s21[] = {
  40, 1121,
  409,408,396,345,317,316,242,238,221,198,166,159,157,143,130,123,120,117,109,102,101,93,87,79,76,67,64,55,53,49,46,44,39,33,21,19,14,13,5,3
};
const int s22[] = {
  50, 788,
  301,300,246,242,187,182,177,168,145,139,135,128,114,110,103,93,87,84,82,81,79,73,69,63,58,57,52,51,49,47,41,40,34,33,26,23,22,21,20,19,18,15,13,11,10,9,8,7,4,2
};
const int s23[] = {
  50, 1034,
  588,446,305,283,175,163,160,138,132,130,128,124,120,116,110,107,106,103,101,100,94,86,85,82,80,77,74,64,63,62,61,60,57,54,47,46,45,43,40,39,32,30,28,27,26,25,22,7,6,1
};
const int s24[] = {
  60, 1097,
  645,452,268,264,204,188,184,176,172,165,161,143,132,127,116,114,108,104,100,94,92,90,88,84,75,74,72,71,69,68,67,64,62,61,56,51,46,36,34,30,29,28,26,25,21,20,19,18,17,16,15,14,12,10,9,7,5,4,2,1
};
const int s25[] = {
  60, 1192,
  638,554,335,303,285,271,219,180,174,159,149,148,136,125,110,98,94,85,77,76,75,74,72,71,69,65,63,62,61,60,59,57,55,51,50,49,48,47,46,45,44,43,40,39,37,35,32,31,25,16,15,14,12,10,9,8,6,4,2,1
};
const int s26[] = {
  75, 1412,
  793,619,473,320,287,207,188,181,179,170,167,153,151,149,142,140,132,127,121,117,116,106,105,103,97,93,92,91,90,87,84,83,82,76,74,73,72,71,70,69,67,66,65,64,63,61,54,53,49,45,39,38,35,34,33,32,30,29,28,27,26,24,21,20,19,18,15,14,13,11,10,9,6,5,3
};


const int* specs[] = {
  &s00[0],&s01[0],&s02[0],&s03[0],&s04[0],
  &s05[0],&s06[0],&s07[0],&s08[0],&s09[0],
  &s10[0],&s11[0],&s12[0],&s13[0],&s14[0],
  &s15[0],&s16[0],&s17[0],&s18[0],&s19[0],
  &s20[0],&s21[0],&s22[0],&s23[0],&s24[0],
  &s25[0],&s26[0]
};
const unsigned int n_specs = sizeof(specs) / sizeof(int*);
//@}

/**
 * \brief %Example: Packing squares into a rectangle
 *
 * See problem 9 at http://www.csplib.org/.
 *
 * \ingroup Example
 */
class PerfectSquare : public Script {
protected:
  /// Array of x-coordinates of squares
  IntVarArray x;
  /// Array of y-coordinates of squares
  IntVarArray y;
public:
  /// Propagation to use for model
  enum {
    PROP_REIFIED,    ///< Use reified constraints
    PROP_CUMULATIVES ///< Use cumulatives constraint
  };
  /// Actual model
  PerfectSquare(const SizeOptions& opt)
    : x(*this,specs[opt.size()][0],0,specs[opt.size()][1]-1),
      y(*this,specs[opt.size()][0],0,specs[opt.size()][1]-1) {

    const int* s = specs[opt.size()];
    int n = *s++;
    int w = *s++;

    // Restrict position according to square size
    for (int i=n; i--; ) {
      rel(*this, x[i], IRT_LQ, w-s[i]);
      rel(*this, y[i], IRT_LQ, w-s[i]);
    }

    IntArgs sa(n,s);

    // Squares do not overlap
    nooverlap(*this, x, sa, y, sa);

    /*
     * Capacity constraints
     *
     */
    switch (opt.propagation()) {
    case PROP_REIFIED:
      {
        for (int cx=0; cx<w; cx++) {
          BoolVarArgs bx(*this,n,0,1);
          for (int i=0; i<n; i++)
            dom(*this, x[i], cx-s[i]+1, cx, bx[i]);
          linear(*this, sa, bx, IRT_EQ, w);
        }
        for (int cy=0; cy<w; cy++) {
          BoolVarArgs by(*this,n,0,1);
          for (int i=0; i<n; i++)
            dom(*this, y[i], cy-s[i]+1, cy, by[i]);
          linear(*this, sa, by, IRT_EQ, w);
        }
      }
      break;
    case PROP_CUMULATIVES:
      {
        IntArgs m(n), dh(n);
        for (int i = n; i--; ) {
          m[i]=0; dh[i]=s[i];
        }
        IntArgs limit(1, w);
        {
          // x-direction
          IntVarArgs e(n);
          for (int i=n; i--;)
            e[i]=expr(*this, x[i]+dh[i]);
          cumulatives(*this, m, x, dh, e, dh, limit, true);
          cumulatives(*this, m, x, dh, e, dh, limit, false);
        }
        {
          // y-direction
          IntVarArgs e(n);
          for (int i=n; i--;)
            e[i]=expr(*this, y[i]+dh[i]);
          cumulatives(*this, m, y, dh, e, dh, limit, true);
          cumulatives(*this, m, y, dh, e, dh, limit, false);
        }
      }
      break;
    default:
      GECODE_NEVER;
    }

    branch(*this, x, INT_VAR_MIN_MIN, INT_VAL_MIN);
    branch(*this, y, INT_VAR_MIN_MIN, INT_VAL_MIN);
  }

  /// Constructor for cloning \a s
  PerfectSquare(bool share, PerfectSquare& s) : Script(share,s) {
    x.update(*this, share, s.x);
    y.update(*this, share, s.y);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new PerfectSquare(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
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
  opt.a_d(5);
  opt.c_d(20);
  opt.parse(argc,argv);
  if (opt.size() >= n_specs) {
    std::cerr << "Error: size must be between 0 and " << n_specs - 1
              << std::endl;
    return 1;
  }
  Script::run<PerfectSquare,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

