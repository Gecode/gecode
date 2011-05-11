/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2008
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

#include <fstream>

using namespace Gecode;

/** \brief Order-specifications
 *
 * Used in the \ref SteelMill example.
 *
 */
//@{
typedef int (*order_t)[2];     ///< Type of the order-specification
extern const int order_weight; ///< Weight-position in order-array elements
extern const int order_color;  ///< Color-position in order-array elements
//@}

/** \brief Constants for CSPLib instance of the Steel Mill Slab Design Problem.
 *
 * Used in the \ref SteelMill example.
 */
//@{
extern int csplib_capacities[];         ///< Capacities
extern unsigned int csplib_ncapacities; ///< Number of capacities
extern unsigned int csplib_maxcapacity; ///< Maximum capacity
extern int csplib_loss[];               ///< Loss for all sizes
extern int csplib_orders[][2];          ///< Orders
extern unsigned int csplib_ncolors;     ///< Number of colors
extern unsigned int csplib_norders;     ///< Number of orders
//@}


/** \brief %SteelMillOptions for examples with size option and an additional
 * optional file name parameter.
 *
 * Used in the \ref SteelMill example.
 */
class SteelMillOptions : public Options {
private:
  unsigned int _size;    ///< Size value
  int* _capacities;      ///< Capacities
  int  _ncapacities;     ///< Number of capacities
  int  _maxcapacity;     ///< Maximum capacity
  int* _loss;            ///< Loss for all sizes
  order_t _orders;       ///< Orders
  int  _ncolors;         ///< Number of colors
  unsigned int _norders; ///< Number of orders
public:
  /// Initialize options for example with name \a n
  SteelMillOptions(const char* n)
    : Options(n), _size(csplib_norders),
      _capacities(csplib_capacities), _ncapacities(csplib_ncapacities),
      _maxcapacity(csplib_maxcapacity),
      _loss(csplib_loss), _orders(&(csplib_orders[0])), _ncolors(csplib_ncolors),
      _norders(csplib_norders) {}
  /// Print help text
  virtual void help(void);
  /// Parse options from arguments \a argv (number is \a argc)
  bool parse(int& argc, char* argv[]);

  /// Return size
  unsigned int size(void) const { return _size;        }
  /// Return capacities
  int* capacities(void) const   { return _capacities;  }
  /// Return number of capacities
  int ncapacities(void) const   { return _ncapacities; }
  /// Return maximum of capacities
  int maxcapacity(void) const   { return _maxcapacity; }
  /// Return loss values
  int* loss(void) const         { return _loss;        }
  /// Return orders
  order_t orders(void) const    { return _orders;      }
  /// Return number of colors
  int ncolors(void) const       { return _ncolors;     }
  /// Return number of orders
  int norders(void) const       { return _norders;     }
};


/**
 * \brief %Example: Steel-mill slab design problem
 *
 * This model solves the Steel Mill Slab Design Problem (Problem 38 in
 * <a href="http://csplib.org">CSPLib</a>). The model is from Gargani
 * and Refalo, "An efficient model and strategy for the steel mill
 * slab design problem.", CP 2007, except that a decomposition of the
 * packing constraint is used. The symmetry-breaking search is from
 * Van Hentenryck and Michel, "The Steel Mill Slab Design Problem
 * Revisited", CPAIOR 2008.
 *
 * The program accepts an optional argument for a data-file containing
 * an instance of the problem. The format for the data-file is the following:
 * <pre>
 * "number of slab capacities" "sequence of capacities in increasing order"
 * "number of colors"
 * "number of orders"
 * "size order 1" "color of order 1"
 * "size order 2" "color of order 2"
 * ...
 * </pre>
 * Hard instances are available from <a href=
 * "http://becool.info.ucl.ac.be/steelmillslab">
 * http://becool.info.ucl.ac.be/steelmillslab</a>.
 *
 * \ingroup Example
 *
 */
class SteelMill : public MinimizeScript {
protected:
  /** \name Instance specification
   */
  //@{
  int* capacities;      ///< Capacities
  int  ncapacities;     ///< Number of capacities
  int  maxcapacity;     ///< Maximum capacity
  int* loss;            ///< Loss for all sizes
  int  ncolors;         ///< Number of colors
  order_t orders;       ///< Orders
  unsigned int norders; ///< Number of orders
  unsigned int nslabs;  ///< Number of slabs
  //@}

  /** \name Problem variables
   */
  //@{
  IntVarArray slab, ///< Slab assigned to order i
    slabload, ///< Load of slab j
    slabcost; ///< Cost of slab j
  IntVar total_cost; ///< Total cost
  //@}

public:
  /// Branching variants
  enum {
    SYMMETRY_NONE,   ///< Simple symmetry
    SYMMETRY_BRANCHING ///< Breaking symmetries with symmetry
  };

  /// Actual model
  SteelMill(const SteelMillOptions& opt)
    : // Initialize instance data
      capacities(opt.capacities()), ncapacities(opt.ncapacities()),
      maxcapacity(opt.maxcapacity()), loss(opt.loss()),
      ncolors(opt.ncolors()), orders(opt.orders()),
      norders(opt.size()), nslabs(opt.size()),
      // Initialize problem variables
      slab(*this, norders, 0,nslabs-1),
      slabload(*this, nslabs, 0,45),
      slabcost(*this, nslabs, 0, Int::Limits::max),
      total_cost(*this, 0, Int::Limits::max)
  {
    // Boolean variables for slab[o]==s
    BoolVarArgs boolslab(norders*nslabs);
    for (unsigned int i = 0; i < norders; ++i) {
      BoolVarArgs tmp(nslabs);
      for (int j = nslabs; j--; ) {
        boolslab[j + i*nslabs] = tmp[j] = BoolVar(*this, 0, 1);
      }
      channel(*this, tmp, slab[i]);
    }

    // Packing constraints
    for (unsigned int s = 0; s < nslabs; ++s) {
      IntArgs c(norders);
      BoolVarArgs x(norders);
      for (int i = norders; i--; ) {
        c[i] = orders[i][order_weight];
        x[i] = boolslab[s + i*nslabs];
      }
      linear(*this, c, x, IRT_EQ, slabload[s]);
    }
    // Redundant packing constraint
    int totalweight = 0;
    for (unsigned int i = norders; i-- ; ) 
       totalweight += orders[i][order_weight] ;
    linear(*this, slabload, IRT_EQ, totalweight);


    // Color constraints
    IntArgs nofcolor(ncolors);
    for (int c = ncolors; c--; ) {
      nofcolor[c] = 0;
      for (int o = norders; o--; ) {
        if (orders[o][order_color] == c) nofcolor[c] += 1;
      }
    }
    BoolVar f(*this, 0, 0);
    for (unsigned int s = 0; s < nslabs; ++s) {
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
    IntArgs l(maxcapacity, loss);
    for (int s = nslabs; s--; ) {
      element(*this, l, slabload[s], slabcost[s]);
    }
    linear(*this, slabcost, IRT_EQ, total_cost);

    // Add branching
    if (opt.symmetry() == SYMMETRY_BRANCHING) {
      // Symmetry breaking branching
      SteelMillBranch::post(*this);
    } else { // opt.symmetry() == SYMMETRY_NONE
      branch(*this, slab, INT_VAR_MAX_MIN, INT_VAL_MIN);
    }
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
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
  SteelMill(bool share, SteelMill& s)
    : MinimizeScript(share,s),
      capacities(s.capacities), ncapacities(s.ncapacities),
      maxcapacity(s.maxcapacity), loss(s.loss),
      ncolors(s.ncolors), orders(s.orders),
      norders(s.norders), nslabs(s.nslabs) {
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


  /** \brief Custom brancher for steel mill slab design
   *
   * This class implements a custom brancher for SteelMill that
   * considers all slabs with no order assigned to it currently to be
   * symmetric.
   *
   * \relates SteelMill
   */
  class SteelMillBranch : Brancher {
  protected:
    /// Cache of first unassigned value
    mutable int start;
    /// %Choice
    class Choice : public Gecode::Choice {
    public:
      /// Position of variable
      int pos;
      /// Value of variable
      int val;
      /** Initialize choice for brancher \a b, number of
       *  alternatives \a a, position \a pos0, and value \a val0.
       */
      Choice(const Brancher& b, unsigned int a, int pos0, int val0)
        : Gecode::Choice(b,a), pos(pos0), val(val0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Choice);
      }
      /// Archive into \a e
      virtual void archive(Archive& e) const {
        Gecode::Choice::archive(e);
        e << alternatives() << pos << val;
      }
    };

    /// Construct brancher
    SteelMillBranch(Home home)
      : Brancher(home), start(0) {}
    /// Copy constructor
    SteelMillBranch(Space& home, bool share, SteelMillBranch& b)
      : Brancher(home, share, b), start(b.start) {
    }

  public:
    /// Check status of brancher, return true if alternatives left.
    virtual bool status(const Space& home) const {
      const SteelMill& sm = static_cast<const SteelMill&>(home);
      for (unsigned int i = start; i < sm.norders; ++i)
        if (!sm.slab[i].assigned()) {
          start = i;
          return true;
        }
      // No non-assigned orders left
      return false;
    }
    /// Return choice
    virtual Gecode::Choice* choice(Space& home) {
      SteelMill& sm = static_cast<SteelMill&>(home);
      assert(!sm.slab[start].assigned());
      // Find order with a) minimum size, b) largest weight
      unsigned int size = sm.norders;
      int weight = 0;
      unsigned int pos = start;
      for (unsigned int i = start; i<sm.norders; ++i) {
        if (!sm.slab[i].assigned()) {
          if (sm.slab[i].size() == size && 
              sm.orders[i][order_weight] > weight) {
            weight = sm.orders[i][order_weight];
            pos = i;
          } else if (sm.slab[i].size() < size) {
            size = sm.slab[i].size();
            weight = sm.orders[i][order_weight];
            pos = i;
          }
        }
      }
      unsigned int val = sm.slab[pos].min();
      // Find first still empty slab (all such slabs are symmetric)
      unsigned int firstzero = 0;
      while (firstzero < sm.nslabs && sm.slabload[firstzero].min() > 0)
        ++firstzero;
      assert(pos < sm.nslabs &&
             val < sm.norders);
      return new Choice(*this, (val<firstzero) ? 2 : 1, pos, val);
    }
    virtual Choice* choice(const Space&, Archive& e) {
      unsigned int alt; int pos, val;
      e >> alt >> pos >> val;
      return new Choice(*this, alt, pos, val);
    }
    /// Perform commit for choice \a _c and alternative \a a
    virtual ExecStatus commit(Space& home, const Gecode::Choice& _c,
                              unsigned int a) {
      SteelMill& sm = static_cast<SteelMill&>(home);
      const Choice& c = static_cast<const Choice&>(_c);
      if (a)
        return me_failed(Int::IntView(sm.slab[c.pos]).nq(home, c.val))
          ? ES_FAILED : ES_OK;
      else
        return me_failed(Int::IntView(sm.slab[c.pos]).eq(home, c.val))
          ? ES_FAILED : ES_OK;
    }
    /// Copy brancher
    virtual Actor* copy(Space& home, bool share) {
      return new (home) SteelMillBranch(home, share, *this);
    }
    /// Post brancher
    static void post(Home home) {
      (void) new (home) SteelMillBranch(home);
    }
    /// Delete brancher and return its size
    virtual size_t dispose(Space&) {
      return sizeof(*this);
    }
  };
};

/** \brief Main-function
 *  \relates SteelMill
 */
int
main(int argc, char* argv[]) {
  SteelMillOptions opt("Steel Mill Slab design");
  opt.symmetry(SteelMill::SYMMETRY_BRANCHING);
  opt.symmetry(SteelMill::SYMMETRY_NONE,"none");
  opt.symmetry(SteelMill::SYMMETRY_BRANCHING,"branching");
  opt.solutions(0);
  if (!opt.parse(argc,argv))
    return 1;
  Script::run<SteelMill,BAB,SteelMillOptions>(opt);
  return 0;
}


void
SteelMillOptions::help(void) {
  Options::help();
  std::cerr << "\t(string), optional" << std::endl
            << "\t\tBenchmark to load." << std::endl
            << "\t\tIf none is given, the standard CSPLib instance is used."
            << std::endl;
  std::cerr << "\t(unsigned int), optional" << std::endl
            << "\t\tNumber of orders to use, in the interval [0..norders]."
            << std::endl
            << "\t\tIf none is given, all orders are used." << std::endl;
}

bool
SteelMillOptions::parse(int& argc, char* argv[]) {
  Options::parse(argc,argv);
  // Check number of arguments
  if (argc >= 4) {
    std::cerr << "Too many arguments given, max two allowed (given={";
    for (int i = 1; i < argc; ++i) {
      std::cerr << "\"" << argv[i] << "\"";
      if (i < argc-1) std::cerr << ",";
    }
    std::cerr << "})." << std::endl;
    return false;
  }
  // Parse options
  while (argc >= 2) {
    bool issize = true;
    for (int i = strlen(argv[argc-1]); i-- && issize; )
      issize &= (isdigit(argv[argc-1][i]) != 0);
    if (issize) {
      _size = atoi(argv[argc-1]);
    } else {
      std::ifstream instance(argv[argc-1]);
      if (instance.fail()) {
        std::cerr << "Argument \"" << argv[argc-1]
                  << "\" is neither an integer nor a readable file"
                  << std::endl;
        return false;
      }
      // Read file instance
      instance >> _ncapacities;
      _capacities = new int[_ncapacities];
      _maxcapacity = -1;
      for (int i = 0; i < _ncapacities; ++i) {
        instance >> _capacities[i];
        _maxcapacity = std::max(_maxcapacity, _capacities[i]);
      }
      instance >> _ncolors >> _norders;
      _orders = new int[_norders][2];
      for (unsigned int i = 0; i < _norders; ++i) {
        instance >> _orders[i][order_weight] >> _orders[i][order_color];
      }
    }

    --argc;
  }
  // Compute loss
  {
    _loss = new int[_maxcapacity+1];
    _loss[0] = 0;
    int currcap = 0;
    for (int c = 1; c < _maxcapacity; ++c) {
      if (c > _capacities[currcap]) ++currcap;
      _loss[c] = _capacities[currcap] - c;
    }
  }
  // Set size, if none given
  if (_size == 0) {
    _size = _norders;
  }
  // Check size reasonability
  if (_size == 0 || _size > _norders) {
    std::cerr << "Size must be between 1 and " << _norders << std::endl;
    return false;
  }
  return true;
}

// Positions in order array
const int order_weight = 0;
const int order_color = 1;

// CSPLib instance
int csplib_capacities[] =
  {12, 14, 17, 18, 19,
   20, 23, 24, 25, 26,
   27, 28, 29, 30, 32,
   35, 39, 42, 43, 44};
unsigned int csplib_ncapacities = 20;
unsigned int csplib_maxcapacity = 44;
int csplib_loss[45];
unsigned int csplib_ncolors = 89;
unsigned int csplib_norders = 111;
int csplib_orders[][2] = {
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
