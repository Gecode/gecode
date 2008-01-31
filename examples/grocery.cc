/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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
 * \brief %Example: %Grocery puzzle
 *
 * A kid goes into a grocery store and buys four items. The cashier
 * charges $7.11, the kid pays and is about to leave when the cashier
 * calls the kid back, and says "Hold on, I multiplied the four items
 * instead of adding them; I'll try again; Hah, with adding them the
 * price still comes to $7.11''. What were the prices of the four items?
 *
 * The model is taken from: Christian Schulte, Gert Smolka, Finite Domain
 * Constraint Programming in Oz. A Tutorial. 2001.
 * Available from: http://www.mozart-oz.org/documentation/fdt/
 *
 * \ingroup ExProblem
 *
 */
class Grocery : public Example {
protected:
  /// The price of each item
  IntVarArray x;
  /// Sum and product of prices
  static const int s = 711;
public:
  /// The actual model
  Grocery(const Options&) : x(this,4,0,s) {

    // The sum of all variables is s
    post(this, x[0]+x[1]+x[2]+x[3] == s);

    // The product of all variables is s (corrected by scale factor)
    rel(this, mult(this, mult(this, x[0], x[1]), mult(this, x[2], x[3])), 
        IRT_EQ, s*100*100*100);

    // Break symmetries: order the variables
    rel(this, x[0], IRT_LQ, x[1]);
    rel(this, x[1], IRT_LQ, x[2]);
    rel(this, x[2], IRT_LQ, x[3]);

    branch(this, x, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MAX);
  }

  /// Constructor for cloning \a s
  Grocery(bool share, Grocery& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Grocery(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\t" << x << std::endl;
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
  Example::run<Grocery,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

