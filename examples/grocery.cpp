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
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: %Grocery puzzle
 *
 * A kid goes into a grocery store and buys four items. The cashier
 * charges $7.11, the kid pays and is about to leave when the cashier
 * calls the kid back, and says ''Hold on, I multiplied the four items
 * instead of adding them; I'll try again; Hah, with adding them the
 * price still comes to $7.11''. What were the prices of the four items?
 *
 * The model is taken from: Christian Schulte, Gert Smolka, Finite Domain
 * Constraint Programming in Oz. A Tutorial. 2001.
 * Available from: http://www.mozart-oz.org/documentation/fdt/
 *
 * \ingroup Example
 *
 */
class Grocery : public Script {
protected:
  /// The price of each item
  IntVarArray abcd;
  /// Sum and product of prices
  static const int s = 711;
  /// Decimal product of prices
  static const int p = 711 * 100 * 100 * 100;
public:
  /// The actual model
  Grocery(const Options& opt)
    : Script(opt), abcd(*this,4,0,s) {
    IntVar a(abcd[0]), b(abcd[1]), c(abcd[2]), d(abcd[3]);

    // The sum of all variables is s
    rel(*this, a+b+c+d == s, opt.ipl());

    // The product of all variables is s (corrected by scale factor)
    rel(*this, (a*b)*(c*d) == p, opt.ipl());

    // Break symmetries: order the variables
    rel(*this, abcd, IRT_LQ);

    branch(*this, abcd, INT_VAR_NONE(), INT_VAL_SPLIT_MAX());
  }

  /// Constructor for cloning \a s
  Grocery(Grocery& s) : Script(s) {
    abcd.update(*this, s.abcd);
  }

  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Grocery(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << abcd << std::endl;
  }
};

/** \brief Main-function
 *  \relates Grocery
 */
int
main(int argc, char* argv[]) {
  Options opt("Grocery");
  opt.iterations(20);
  opt.parse(argc,argv);
  Script::run<Grocery,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

