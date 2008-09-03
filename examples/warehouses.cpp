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

#include "examples/support.hh"
#include <gecode/minimodel.hh>

/// Number of warehouses
const int n_suppliers = 5;
/// Number of stores
const int n_stores = 10;

/// Cost of building one warehouse
const int building_cost = 30;

/// Capacity of a single warehouse
const int capacity[n_suppliers] = {
  1, 4, 2, 1, 3
};

/// Cost for store to warehouse
const int cost_matrix[n_stores][n_suppliers] = {
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
 * \ingroup ExProblem
 *
 */
class Warehouses : public MinimizeExample {
protected:
  /// Map store to the supplier
  IntVarArray supplier;
  /// Is a supplier open (warehouse needed)
  BoolVarArray open;
  /// Cost of a store
  IntVarArray scost;
  /// Total cost
  IntVar total;
public:
  /// Actual model
  Warehouses(const Options&)
    : supplier(*this, n_stores, 0, n_suppliers-1),
      open(*this, n_suppliers, 0, 1),
      scost(*this, n_stores, 0, Int::Limits::max),
      total(*this, 0, Int::Limits::max) {
    // Compute total cost
    {
      // Opening cost
      IntArgs c(n_suppliers);
      for (int i=0; i<n_suppliers; i++)
        c[i]=building_cost;
      IntVar oc(*this, 0, Int::Limits::max);
      linear(*this, c, open, IRT_EQ, oc);
      // Total cost of stores
      IntVarArgs tc(n_stores+1);
      for (int i=0; i<n_stores; i++)
        tc[i]=scost[i];
      tc[n_stores] = oc;
      linear(*this, tc, IRT_EQ, total);
    }

    // Compute cost for store
    for (int i=0; i<n_stores; i++) {
      IntArgs c(n_suppliers);
      for (int j=0; j<n_suppliers; j++)
        c[j] = cost_matrix[i][j];
      element(*this, c, supplier[i], scost[i]);
    }

    // Do not exceed capacity
    for (int i=0; i<n_suppliers; i++)
      count(*this, supplier, i, IRT_LQ, capacity[i]);

    // A warehouse is open, if it supplies to a shop
    for (int i=0; i<n_suppliers; i++) {
      BoolVarArgs store_by_supplier(n_stores);
      for (int j=0; j<n_stores; j++)
        store_by_supplier[j] = post(*this, ~(supplier[j] == i));
      rel(*this, BOT_OR, store_by_supplier, open[i]);
    }

    branch(*this, scost, INT_VAR_REGRET_MIN_MAX, INT_VAL_MIN);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return total;
  }
  /// Constructor for cloning \a s
  Warehouses(bool share, Warehouses& s) : MinimizeExample(share,s) {
    supplier.update(*this, share, s.supplier);
    open.update(*this, share, s.open);
    scost.update(*this, share, s.scost);
    total.update(*this, share, s.total);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Warehouses(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\tSupplier: " << supplier << std::endl
       << "\tCost: " << scost << std::endl
       << "\tTotal cost: " << total << std::endl
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
  MinimizeExample::run<Warehouses,BAB,Options>(opt);
  return 0;
}

// STATISTICS: example-any

