/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

using namespace Gecode;

/**
 * \brief %Example: Solving 20 linear equations
 *
 * Standard benchmark for solving linear equations.
 *
 * \ingroup Example
 *
 */
class Eq20 : public Script {
private:
  /// Number of variables
  static const int x_n = 7;
  /// Number of equations
  static const int e_n = 20;
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  Eq20(const Options& opt)
    : Script(opt), x(*this,x_n,0,10) {
    // Coefficients and constants for the equations
    int eqs[e_n][x_n+1] = {
      {876370, -16105, 62397, -6704, 43340, 95100, -68610, 58301},
      {533909, 51637, 67761, 95951, 3834, -96722, 59190, 15280},
      {915683, 1671, -34121, 10763, 80609, 42532, 93520, -33488},
      {129768, 71202, -11119, 73017, -38875, -14413, -29234, 72370},
      {752447, 8874, -58412, 73947, 17147, 62335, 16005, 8632},
      {90614, 85268, 54180, -18810, -48219, 6013, 78169, -79785},
      {1198280, -45086, 51830, -4578, 96120, 21231, 97919, 65651},
      {18465, -64919, 80460, 90840, -59624, -75542, 25145, -47935},
      {1503588, -43277, 43525, 92298, 58630, 92590, -9372, -60227},
      {1244857, -16835, 47385, 97715, -12640, 69028, 76212, -81102},
      {1410723, -60301, 31227, 93951, 73889, 81526, -72702, 68026},
      {25334, 94016, -82071, 35961, 66597, -30705, -44404, -38304},
      {277271, -67456, 84750, -51553, 21239, 81675, -99395, -4254},
      {249912, -85698, 29958, 57308, 48789, -78219, 4657, 34539},
      {373854, 85176, -95332, -1268, 57898, 15883, 50547, 83287},
      {740061, -10343, 87758, -11782, 19346, 70072, -36991, 44529},
      {146074, 49149, 52871, -7132, 56728, -33576, -49530, -62089},
      {251591, -60113, 29475, 34421, -76870, 62646, 29278, -15212},
      {22167, 87059, -29101, -5513, -21219, 22128, 7276, 57308},
      {821228, -76706, 98205, 23445, 67921, 24111, -48614, -41906}
    };
    // Post equation constraints
    for (int i = e_n; i--; ) {
      IntArgs c(x_n, &eqs[i][1]);
      linear(*this, c, x, IRT_EQ, eqs[i][0], opt.ipl());
    }
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Eq20(Eq20& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new Eq20(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tx[] = " << x << std::endl;
  }

};

/** \brief Main-function
 *  \relates Eq20
 */
int
main(int argc, char* argv[]) {
  Options opt("Eq20");
  opt.solutions(0);
  opt.iterations(1000);
  opt.parse(argc,argv);
  Script::run<Eq20,DFS,Options>(opt);
  return 0;
}



// STATISTICS: example-any

