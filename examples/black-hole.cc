/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
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
#include "gecode/support/random.hh"

#include <vector>
#include <algorithm>
#include <sstream>

namespace {
  using std::vector;

  /// Layout of the cards
  vector<vector<int> > layout;
  /// information for locating particular cards in the layout
  vector<int> layer, pile;

  /// Generates the \a layout and intializes \a layer and \a pile from it.
  void generate(int seed) {
    // The layout consists of 17 piles of 3 cards each
    layout = vector<vector<int> >(17, vector<int>(3));
    vector<int> nums(51);
    for (int i = 51; i--; ) nums[i] = i+1;
    Support::RandomGenerator rnd(seed+1);
    std::random_shuffle(nums.begin(), nums.end(), rnd);
    int pos = 0;
    for (int i = 17; i--; )
      for (int j = 3; j--; )
	layout[i][j] = nums[pos++];

    // Location-information for each value
    layer = vector<int>(52);
    pile  = vector<int>(52);
    for (int i = 17; i--; ) {
      for (int j = 3; j--; ) {
	layer[layout[i][j]] = j;
	pile[ layout[i][j]] = i;
      }
    }
  }
}


/** Custom branching that instantiates the varaibles in lexical order,
 *  and chooses the value with the most cards under it.
 */
class BlackHoleBranch : Branching {
  ViewArray<Int::IntView> x;
  mutable int pos, val;
 
  BlackHoleBranch(Space* home, ViewArray<Int::IntView>& xv) 
    : Branching(home), x(xv), pos(-1), val(-1) {}
  BlackHoleBranch(Space* home, bool share, BlackHoleBranch& b) 
    : Branching(home, share, b), pos(b.pos), val(b.val) {
    x.update(home, share, b.x);
  }
  
public:
  virtual bool status(const Space* home) const {
    for (pos = 0; pos < x.size(); ++pos) {
      if (!x[pos].assigned()) {
	int w = 4;
	for (Int::ViewValues<Int::IntView> vals(x[pos]); vals(); ++vals) {
	  if (layer[vals.val()] < w) {
	    val = vals.val();
	    if ((w = layer[vals.val()]) == 0) break;
	  }
	}
	return true;
      }
    }
    // No non-assigned variables left
    return false;
  }
  virtual BranchingDesc* description(const Space* home) const {
    assert(pos >= 0 && pos < x.size() && val >= 1 && val < 52);
    return new PosValDesc<int>(this, pos, val);
  }
  virtual ExecStatus commit(Space* home, const BranchingDesc* d, unsigned int a) {
    const PosValDesc<int> *desc = static_cast<const PosValDesc<int>*>(d);
    pos = val = -1;
    if (a)
      return me_failed(x[desc->pos()].nq(home, desc->val())) ? ES_FAILED : ES_OK;
    else 
      return me_failed(x[desc->pos()].eq(home, desc->val())) ? ES_FAILED : ES_OK;
  }
  virtual Actor* copy(Space *home, bool share) {
    return new (home) BlackHoleBranch(home, share, *this);
  }
  static void post(Space* home, IntVarArgs x) {
    ViewArray<Int::IntView> xv(home, x);
    (void) new (home) BlackHoleBranch(home, xv);
  }
};


/**
 * \brief %Example: Black Hole Patience
 *
 * This example solves instances of the black-hole patience game. 
 * 
 * The model of the problem is mostly taken from "Search in the
 * Patience Game 'Black Hole'", by Ian P. Gent, Chris Jefferson, Tom
 * Kelsey, Inês Lynce, Ian Miguel, Peter Nightingale, Barbara
 * M. Smith, and S. Armagan Tarim. 
 *
 * The smart version breaks the conditional symmetry identified in the
 * above paper. 
 *
 * \ingroup Example
 *
 */
class BlackHole : public Example {
protected:
  /// Array of letters
  IntVarArray x, y;

  /// Return a string representing the card of value val
  std::string
  card(int val) {
    const char* suit = "SCHD";
    std::ostringstream o;
    o << std::setw(2) << (1 + (val%13)) << suit[val/13];
    return o.str();
  }

public:
  /// Actual model
  BlackHole(const Options& opt) 
    : x(this, 52, 0,51), y(this, 52, 0,51) 
  {
    // Black ace at bottom
    rel(this, x[0], IRT_EQ, 0);

    // x is order and y is placement
    channel(this, x, y, opt.icl);

    // Build table for accessing the rank of a card
    IntArgs modtable(52);
    for (int i = 0; i < 52; ++i) {
      modtable[i] = i%13;
    }
    // The placement rules: the absolute value of the difference
    // between two consecutive cards is 1 or 12.
    for (int i = 0; i < 51; ++i) {
      IntVar x1(this, 0, 12), x2(this, 0, 12);
      element(this, modtable, x[i], x1, ICL_DOM);
      element(this, modtable, x[i+1], x2, ICL_DOM);
      const int dr[2] = {1, 12};
      IntVar diff(this, IntSet(dr, 2));
      post(this, abs(this, minus(this, x1, x2, ICL_DOM), ICL_DOM) 
	   == diff, ICL_DOM);
    }

    // Set up instance
    for (int i = 17; i--; ) {
      for (int j = 2; j--; ) {
	// A card must be played before the one under it.
	post(this, y[layout[i][j]] < y[layout[i][j+1]]);
      }
    }

    // Compute and break the conditional symmetries that are dependent
    // on the current layout.
    if (!opt.naive) {
      BoolVar tval(this, 1, 1); 
      // For all ranks
      for (int r = 13; r--; ) {
	// For all pairs of suits
	for (int s1 = 4; s1--; ) {
	  for (int s2 = s1; s2--; ) {
	    int c1 = 13*s1 + r,
	      c2 = 13*s2 + r;
	    // The ace of spades is already placed
	    if (c1 == 0 || c2 == 0) continue;
	    int o1 = c1, o2 = c2;
	    if (pile[c1] > pile[c2] && layer[c2] >= layer[c1]) 
	      std::swap(o1, o2);
	    // Pileas are ahndled by the rules of the game
	    if (pile[c1] == pile[c2]) continue;
	    // cond is the condition for the symmetry
	    MiniModel::BoolExpr cond = tval;
	    // Both cards played after the ones on top of them
	    for (int i = 0; i < layer[o1]; ++i)
	      cond = cond && ~(y[layout[pile[o1]][i]] < y[o2]);
	    for (int i = 0; i < layer[o2]; ++i)
	      cond = cond && ~(y[layout[pile[o2]][i]] < y[o1]);
	    // Both cards played before the ones under them
	    for (int i = layer[o1]+1; i < 3; ++i)
	      cond = cond && ~(y[o2] < y[layout[pile[o1]][i]]);
	    for (int i = layer[o2]+1; i < 3; ++i)
	      cond = cond && ~(y[o1] < y[layout[pile[o2]][i]]);

	    // If cond is fulfilled, then we can order the cards
	    post(this, tt(!cond || ~(y[o1] < y[o2])));
	  }
	}
      }
    }

    BlackHoleBranch::post(this, x);      
  }
  /// Print instance and solution
  virtual void
  print(void) {
    for (int i = 0; i < 17; i++) {
      for (int j = 0; j < 3; j++)
	std::cout << card(layout[i][j]) << " ";
      if ((i+1) % 3 == 0) 
	std::cout << std::endl;
      else
	std::cout << "\t";
    }
    std::cout << std::endl << std::endl;
    
    for (int i = 0; i < 52; ++i) {
      std::cout << card(x[i].min()) << " ";
      if ((i + 1) % 13 == 0)
	std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }

  /// Constructor for cloning \a s
  BlackHole(bool share, BlackHole& s) : Example(share,s) {
    x.update(this, share, s.x);
    y.update(this, share, s.y);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new BlackHole(share,*this);
  }
};

/** \brief Main-function
 *  \relates BlackHole
 */
int
main(int argc, char** argv) {
  Options opt("Black Hole patience");
  opt.solutions  = 1;
  opt.naive      = false;
  opt.icl        = ICL_DOM;
  opt.parse(argc,argv);
  // Generates the new board
  generate(opt.size);
  Example::run<BlackHole,DFS>(opt);
  return 0;
}

// STATISTICS: example-any
