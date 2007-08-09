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
#include "gecode/minimodel.hh"

/// Number of warehouses
static const int n_suppliers = 5;
/// Number of stores
static const int n_stores = 10;

/// Cost of building one warehouse
static const int building_cost = 30;

/// Capacity of a single warehouse
static const int capacity[n_suppliers] = {
  1, 4, 2, 1, 3
};

/// Cost for store to warehouse
static const int cost_matrix[n_stores][n_suppliers] = {
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
 * \ingroup ExProblem
 *
 */
class Warehouses : public Example {
protected:
  /// Map store to the supplier
  IntVarArray supplier;
  /// Is a supplier open (warehouse needed)
  BoolVarArray open;
  /// Cost of a store
  IntVarArray cost;
  /// Total cost
  IntVar total;
public:
  /// Actual model
  Warehouses(const Options& opt)
    : supplier(this, n_stores, 0, n_suppliers-1),
      open(this, n_suppliers, 0, 1),
      cost(this, n_stores, 0, Limits::Int::int_max),
      total(this, 0, Limits::Int::int_max) {
    // Compute total cost
    {
      // Opening cost
      IntArgs c(n_suppliers);
      for (int i=0; i<n_suppliers; i++)
        c[i]=building_cost;
      IntVar oc(this, 0, Limits::Int::int_max);
      linear(this, c, open, IRT_EQ, oc);
      // Total cost of stores
      IntVarArgs tc(n_stores+1);
      for (int i=0; i<n_stores; i++)
        tc[i]=cost[i];
      tc[n_stores] = oc;
      linear(this, tc, IRT_EQ, total);
    }

    // Compute cost for store
    for (int i=0; i<n_stores; i++) {
      IntArgs c(n_suppliers);
      for (int j=0; j<n_suppliers; j++)
        c[j] = cost_matrix[i][j];
      element(this, c, supplier[i], cost[i]);
    }

    // Do not exceed capacity
    for (int i=0; i<n_suppliers; i++)
      count(this, supplier, i, IRT_LQ, capacity[i]);

    // A warehouse is open, if it supplies to a shop
    for (int i=0; i<n_suppliers; i++) {
      BoolVarArgs store_by_supplier(n_stores);
      for (int j=0; j<n_stores; j++)
        store_by_supplier[j] = post(this, ~(supplier[j] == i));
      rel(this, store_by_supplier, BOT_OR, open[i]);
    }

    branch(this, cost, BVAR_REGRET_MIN_MAX, BVAL_MIN);
  }

  /// Add constraint for next better solution
  void
  constrain(Space* s) {
    rel(this, total, IRT_LE, static_cast<Warehouses*>(s)->total.val());
  }
  /// Constructor for cloning \a s
  Warehouses(bool share, Warehouses& s) : Example(share,s) {
    supplier.update(this, share, s.supplier);
    open.update(this, share, s.open);
    cost.update(this, share, s.cost);
    total.update(this, share, s.total);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Warehouses(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tSupplier: {";
    for (int i=0; i<n_stores; i++) {
      std::cout << supplier[i] << ((i<n_stores-1)?", ":"};\n");
    }
    std::cout << "\tCost: {";
    for (int i=0; i<n_stores; i++) {
      std::cout << cost[i] << ((i<n_stores-1)?", ":"};\n");
    }
    std::cout << "\tTotal cost: " << total << std::endl;
    std::cout << std::endl;
  }
};

/** \brief Main-function
 *  \relates Warehouses
 */
int
main(int argc, char** argv) {
  Options opt("Warehouses");
  opt.solutions  = 0;
  opt.iterations = 10;
  opt.naive      = true;
  opt.parse(argc,argv);
  Example::run<Warehouses,BAB>(opt);
  return 0;
}

// STATISTICS: example-any

