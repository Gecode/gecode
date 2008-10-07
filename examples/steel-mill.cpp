/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2008
 *
 *  Last modified:
 *     $Date: 2008-06-02 21:07:11 +0200 (Mon, 02 Jun 2008) $ by $Author: schulte $
 *     $Revision: 7001 $
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


extern int capacities[];
extern int loss[];
extern int ncolors;
extern unsigned int maxOrders;
extern const int order_weight;
extern const int order_color;
extern int orders[][2];



/**
 * \brief %Example: Steel-Mill Slab Design Problem
 *
 * This model solves the Steel Mill Slab Design Problem (Problem 38 in
 * <a href="http://csplib.org">CSPLib</a>). The model used is from
 * Gargani and Refalo, "An efficient model and strategy for the steel
 * mill slab design problem.", CP 2007. The symmetry-breaking search
 * is from Van Hentenryck and Michel, "The Steel Mill Slab Design
 * Problem Revisited", CPAIOR 2008.
 *
 * \ingroup ExProblem
 *
 */
class SteelMill : public MinimizeExample {
protected:
  int norders, nslabs;
  IntVarArray slab, ///< Slab assigned to order i
    slabload, ///< Load of slab j
    slabcost; ///< Cost of slab j
  IntVar total_cost; ///< Total cost

public:
  /// Branching variants
  enum {
    BRANCHING_NAIVE,   ///< Simple branching
    BRANCHING_SYMMETRY ///< Breaking symmetries with branching
  };
  /// Actual model
  SteelMill(const SizeOptions& opt) 
    : norders(opt.size()), nslabs(opt.size()),
      slab(*this, norders, 0,nslabs-1), 
      slabload(*this, nslabs, 0,45),
      slabcost(*this, nslabs, 0, Int::Limits::max),
      total_cost(*this, 0, Int::Limits::max)
  {
    // Boolean variables for slab[o]==s
    BoolVarArgs boolslab(norders*nslabs);
    for (int i = 0; i < norders; ++i) {
      BoolVarArgs tmp(nslabs);
      for (int j = nslabs; j--; ) {
        boolslab[j + i*nslabs] = tmp[j] = BoolVar(*this, 0, 1);
      }
      channel(*this, tmp, slab[i]);
    }
    
    // Packing constraints
    for (int s = 0; s < nslabs; ++s) {
      IntArgs c(norders);
      BoolVarArgs x(norders);
      for (int i = norders; i--; ) {
        c[i] = orders[i][order_weight];
        x[i] = boolslab[s + i*nslabs];
      }
      linear(*this, c, x, IRT_EQ, slabload[s]);
    }

    // Color constraints
    IntArgs nofcolor(ncolors);
    for (int c = ncolors; c--; ) {
      nofcolor[c] = 0;
      for (int o = norders; o--; ) {
        if (orders[o][order_color] == c) nofcolor[c] += 1;
      }
    }
    BoolVar f(*this, 0, 0);
    for (int s = 0; s < nslabs; ++s) {
      BoolVarArgs hascolor(ncolors);
      for (int c = ncolors; c--; ) {
        if (nofcolor[c]) {
          BoolVarArgs hasc(nofcolor[c]);
          int pos = 0;
          for (int o = norders; o--; ) {
            if (orders[o][order_color] == c)
              hasc[pos++] = boolslab[s + o*nslabs];
          }
          assert(pos == nofcolor[c]);
          hascolor[c] = BoolVar(*this, 0, 1);
          rel(*this, BOT_OR, hasc, hascolor[c]);
        } else {
          hascolor[c] = f;
        }
      }
      linear(*this, hascolor, IRT_LQ, 2);
    }

    // Compute slabcost
    IntArgs l(45, loss);
    for (int s = nslabs; s--; ) {
      element(*this, l, slabload[s], slabcost[s]);
    }
    linear(*this, slabcost, IRT_EQ, total_cost);
    
    // Add branching
    if (opt.branching() == BRANCHING_SYMMETRY) {
      // Install custom branching
      SteelMillBranch::post(*this);
    } else { // opt.branching() == BRANCHING_NAIVE
      branch(*this, slab, INT_VAR_MAX_MIN, INT_VAL_MIN);
    }
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "What slab="  << slab << std::endl;
    os << "Slab load="  << slabload << std::endl;
    os << "Slab cost="  << slabcost << std::endl;
    os << "Total cost=" << total_cost << std::endl;
    int nslabsused = 0;
    int nslabscost = 0;
    bool unassigned = false;
    for (int i = nslabs; i--; ) {
      if (!slabload[i].assigned() || !slabcost[i].assigned()) {
        unassigned = true;
        break;
      }
      if (slabload[i].min()>0) ++nslabsused;
      if (slabcost[i].min()>0) ++nslabscost;
    }
    if (!unassigned)
      os << "Number of slabs used=" << nslabsused
         << ", slabs with cost="    << nslabscost
         << std::endl;
    os << std::endl;
  }

  /// Constructor for cloning \a s
  SteelMill(bool share, SteelMill& s) : 
    MinimizeExample(share,s), norders(s.norders), nslabs(s.nslabs) {
    slab.update(*this, share, s.slab);
    slabload.update(*this, share, s.slabload);
    slabcost.update(*this, share, s.slabcost);
    total_cost.update(*this, share, s.total_cost);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new SteelMill(share,*this);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return total_cost;
  }


  /** \brief Custom branching for steel mill slab design
   *
   * This class implements a custom branching for SteelMill that
   * considers all slabs with no order assigned to it currently to be
   * symmetric.
   *
   * \relates SteelMill
   */
  class SteelMillBranch : Branching {
    /// Cache of first unassigned value
    mutable int start;
    /// Branching description
    class Description : public BranchingDesc {
    public:
      /// Position of variable
      int pos;
      /// Value of variable
      int val;
      /** Initialize description for branching \a b, number of
       *  alternatives \a a, position \a pos0, and value \a val0.
       */
      Description(const Branching& b, unsigned int a, int pos0, int val0)
        : BranchingDesc(b,a), pos(pos0), val(val0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Description);
      }
    };
    
    /// Construct branching
    SteelMillBranch(Space& home) 
      : Branching(home), start(0) {}
    /// Copy constructor
    SteelMillBranch(Space& home, bool share, SteelMillBranch& b) 
      : Branching(home, share, b), start(b.start) {
    }
    
  public:
    /// Check status of branching, return true if alternatives left. 
    virtual bool status(const Space& home) const {
      const SteelMill& sm = static_cast<const SteelMill&>(home);
      for (int i = start; i < sm.norders; ++i)
        if (!sm.slab[i].assigned()) {
          start = i;
          return true;
        }
      // No non-assigned orders left
      return false;
    }
    /// Return branching description
    virtual BranchingDesc* description(Space& home) {
      SteelMill& sm = static_cast<SteelMill&>(home);
      assert(!sm.slab[start].assigned());
      // Find order with a) minimum size, b) largest weight
      unsigned int size = sm.norders;
      int weight = 45;
      int pos = start;
      for (int i = start; i<sm.norders; ++i) {
        if (!sm.slab[i].assigned()) {
          if (sm.slab[i].size() == size && orders[i][order_weight] > weight) {
            weight = orders[i][order_weight];
            pos = i;
          } else if (sm.slab[i].size() < size) {
            size = sm.slab[i].size() < size;
            weight = orders[i][order_weight];
            pos = i;
          }
        }
      }
      int val = sm.slab[pos].min();
      // Find first still empty slab (all such slabs are symmetric)
      int firstzero = 0;
      while (firstzero < sm.nslabs && sm.slabload[firstzero].min() > 0)
        ++firstzero;
      assert(pos >= 0 && pos < sm.nslabs && 
             val >= 0 && val < sm.norders);
      return new Description(*this, val<firstzero ? 2 : 1, pos, val);
    }
    /// Perform commit for branching description \a d and alternative \a a. 
    virtual ExecStatus commit(Space& home, const BranchingDesc& d, 
                              unsigned int a) {
      SteelMill& sm = static_cast<SteelMill&>(home);
      const Description& desc = 
        static_cast<const Description&>(d);
      if (a)
        return me_failed(Int::IntView(sm.slab[desc.pos]).nq(home, desc.val)) 
          ? ES_FAILED : ES_OK;
      else 
        return me_failed(Int::IntView(sm.slab[desc.pos]).eq(home, desc.val)) 
          ? ES_FAILED : ES_OK;
    }
    /// Copy branching
    virtual Actor* copy(Space& home, bool share) {
      return new (home) SteelMillBranch(home, share, *this);
    }
    /// Reflection name
    virtual const char* name(void) const {
      return "SteelMillBranch";
    }
    /// Post branching
    static void post(Space& home) {
      (void) new (home) SteelMillBranch(home);
    }
  };
};

/** \brief Main-function
 *  \relates SteelMill
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Steel Mill Slab design");
  opt.branching(SteelMill::BRANCHING_SYMMETRY);
  opt.branching(SteelMill::BRANCHING_NAIVE,"naive");
  opt.branching(SteelMill::BRANCHING_SYMMETRY,"symmetry");
  opt.size(1);
  opt.solutions(0);
  opt.parse(argc,argv);
  if (opt.size() <= 0 || opt.size() > maxOrders) {
    std::cerr << "Size must be between 1 and " << maxOrders << std::endl;
    return 1;
  }
  // Fill in the loss-values
  loss[0] = 0;
  int currcap = 0;
  for (int c = 1; c < 45; ++c) {
    if (c > capacities[currcap]) ++currcap;
    loss[c] = capacities[currcap] - c;
  }
  Example::run<SteelMill,BAB,SizeOptions>(opt);
  return 0;
}


int capacities[] = {12, 14, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 32, 35, 39, 42, 43, 44};
int loss[45];
int ncolors = 89;
unsigned int maxOrders = 111;
const int order_weight = 0;
const int order_color = 1;
int orders[][2] = {
  {4, 1},
  {22, 2},
  {9, 3},
  {5, 4},
  {8, 5},
  {3, 6},
  {3, 4},
  {4, 7},
  {7, 4},
  {7, 8},
  {3, 6},
  {2, 6},
  {2, 4},
  {8, 9},
  {5, 10},
  {7, 11},
  {4, 7},
  {7, 11},
  {5, 10},
  {7, 11},
  {8, 9},
  {3, 1},
  {25, 12},
  {14, 13},
  {3, 6},
  {22, 14},
  {19, 15},
  {19, 15},
  {22, 16},
  {22, 17},
  {22, 18},
  {20, 19},
  {22, 20},
  {5, 21},
  {4, 22},
  {10, 23},
  {26, 24},
  {17, 25},
  {20, 26},
  {16, 27},
  {10, 28},
  {19, 29},
  {10, 30},
  {10, 31},
  {23, 32},
  {22, 33},
  {26, 34},
  {27, 35},
  {22, 36},
  {27, 37},
  {22, 38},
  {22, 39},
  {13, 40},
  {14, 41},
  {16, 27},
  {26, 34},
  {26, 42},
  {27, 35},
  {22, 36},
  {20, 43},
  {26, 24},
  {22, 44},
  {13, 45},
  {19, 46},
  {20, 47},
  {16, 48},
  {15, 49},
  {17, 50},
  {10, 28},
  {20, 51},
  {5, 52},
  {26, 24},
  {19, 53},
  {15, 54},
  {10, 55},
  {10, 56},
  {13, 57},
  {13, 58},
  {13, 59},
  {12, 60},
  {12, 61},
  {18, 62},
  {10, 63},
  {18, 64},
  {16, 65},
  {20, 66},
  {12, 67},
  {6, 68},
  {6, 68},
  {15, 69},
  {15, 70},
  {15, 70},
  {21, 71},
  {30, 72},
  {30, 73},
  {30, 74},
  {30, 75},
  {23, 76},
  {15, 77},
  {15, 78},
  {27, 79},
  {27, 80},
  {27, 81},
  {27, 82},
  {27, 83},
  {27, 84},
  {27, 79},
  {27, 85},
  {27, 86},
  {10, 87},
  {3, 88}
};

// STATISTICS: example-any
