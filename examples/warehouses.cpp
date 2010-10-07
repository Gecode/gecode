/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

/// Number of warehouses
const int n_warehouses = 5;
/// Number of stores
const int n_stores = 10;

/// Fixed cost for one warehouse
const int c_fixed = 30;

/// Capacity of a single warehouse
const int capacity[n_warehouses] = {
  1, 4, 2, 1, 3
};

/// Cost for supply a store by a warehouse
const int c_supply[n_stores][n_warehouses] = {
  {20, 24, 11, 25, 30},
  {28, 27, 82, 83, 74},
  {74, 97, 71, 96, 70},
  { 2, 55, 73, 69, 61},
  {46, 96, 59, 83,  4},
  {42, 22, 29, 67, 59},
  { 1,  5, 73, 59, 56},
  {10, 73, 13, 43, 96},
  {93, 35, 63, 85, 46},
  {47, 65, 55, 71, 95}
};



/**
 * \brief %Example: Locating warehouses
 *
 * A company needs to construct warehouses to supply stores with
 * goods. Each warehouse possibly to be constructed has a certain
 * capacity defining how many stores it can supply. Constructing a
 * warehouse incurs a fixed cost. Costs for transportation from
 * warehouses to stores depend on the locations of warehouses and
 * stores.
 *
 * Determine which warehouses should be constructed and which
 * warehouse should supply which store such that overall cost
 * (transportation cost plus construction cost) is smallest.
 *
 * Taken from:
 *   Pascal Van Hentenryck, The OPL Optmization Programming Language,
 *   The MIT Press, 1999.
 *
 * See also problem 34 at http://www.csplib.org/.
 *
 * Note that "Modeling and Programming with Gecode" uses this example
 * as a case study.
 *
 * \ingroup Example
 *
 */
class Warehouses : public MinimizeScript {
protected:
  /// Which warehouse supplies a store
  IntVarArray supplier;
  /// Is a warehouse open (warehouse needed)
  BoolVarArray open;
  /// Cost of a store
  IntVarArray c_store;
  /// Total cost
  IntVar c_total;
public:
  /// Actual model
  Warehouses(const Options&)
    : supplier(*this, n_stores, 0, n_warehouses-1),
      open(*this, n_warehouses, 0, 1),
      c_store(*this, n_stores) {

    // A warehouse is open, if it supplies to a store
    for (int s=0; s<n_stores; s++)
      element(*this, open, supplier[s], 1);

    // Compute cost for each warehouse
    for (int s=0; s<n_stores; s++) {
      IntArgs c(n_warehouses, c_supply[s]);
      c_store[s] = expr(*this, element(c, supplier[s]));
    }

    // Do not exceed capacity
    {
      IntSetArgs c(n_warehouses);
      for (int w=0; w<n_warehouses; w++)
        c[w] = IntSet(0,capacity[w]);
      count(*this, supplier, c, ICL_DOM);
    }

    // Compute total cost
    c_total = expr(*this, c_fixed*sum(open) + sum(c_store));

    // Branch with largest minimum regret on store cost
    branch(*this, c_store, INT_VAR_REGRET_MIN_MAX, INT_VAL_MIN);

    // Branch by assigning a supplier to each store
    branch(*this, supplier, INT_VAR_NONE, INT_VAL_MIN);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return c_total;
  }
  /// Constructor for cloning \a s
  Warehouses(bool share, Warehouses& s) : MinimizeScript(share,s) {
    supplier.update(*this, share, s.supplier);
    open.update(*this, share, s.open);
    c_store.update(*this, share, s.c_store);
    c_total.update(*this, share, s.c_total);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Warehouses(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tSupplier:        " << supplier << std::endl
       << "\tOpen warehouses: " << open << std::endl
       << "\tStore cost:      " << c_store << std::endl
       << "\tTotal cost:      " << c_total << std::endl
       << std::endl;
  }
};

/** \brief Main-function
 *  \relates Warehouses
 */
int
main(int argc, char* argv[]) {
  Options opt("Warehouses");
  opt.solutions(0);
  opt.iterations(10);
  opt.parse(argc,argv);
  MinimizeScript::run<Warehouses,BAB,Options>(opt);
  return 0;
}

// STATISTICS: example-any

