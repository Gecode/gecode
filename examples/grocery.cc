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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
 * \ingroup Example
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
    {
      IntVar t0(this,0,Limits::Int::int_max);
      IntVar t1(this,0,Limits::Int::int_max);
      IntVar t2(this,0,Limits::Int::int_max);
      rel(this, t2, IRT_EQ, s*100*100*100);
      mult(this, x[0], x[1], t0);
      mult(this, x[2], x[3], t1);
      mult(this, t0, t1, t2);
    }

    // Break symmetries: order the variables
    rel(this, x[0], IRT_LQ, x[1]);
    rel(this, x[1], IRT_LQ, x[2]);
    rel(this, x[2], IRT_LQ, x[3]);

    branch(this, x, BVAR_SIZE_MIN, BVAL_SPLIT_MAX);
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
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < x.size(); i++) 
      std::cout << x[i] << ", ";
    std::cout << std::endl;
  }
};

/** \brief Main-function
 *  \relates Grocery
 */
int
main(int argc, char** argv) {
  Options opt("Grocery");
  opt.iterations = 20;
  opt.parse(argc,argv);
  Example::run<Grocery,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

