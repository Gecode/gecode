/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include "test/bdd.hh"

static IntSet ds_012(0,2);
static IntSet ds_1012(-1,2);


// BE CAREFUL with number of variables and domain size
// current instance ( |ds_012 = {-1#2}| = 4 and |x| = 2 + 2 = 4 ) => 2^16
// takes at least 5 min to test
class BddSelectUnion : public BddTest {
private:
  int selector_pos;
  int union_pos;
  int xsize;
public:
  // using cache size of 10000 gives fastest time of 0m48s per iteration
  BddSelectUnion(const char* t) : BddTest(t, 4, ds_012, false, 0, 100, 10000), xsize(4) {
    union_pos = xsize - 1;
    selector_pos = xsize - 2;
  }
  virtual bool solution(const SetAssignment& x) const {
    // std::cout << "solution...";
    int selected = 0;
    // count the number of selected sets
    CountableSetValues count_sel(x.lub, x[selector_pos]);
    for ( ; count_sel(); ++count_sel, selected++);
    
    CountableSetValues xunion(x.lub, x[union_pos]);
    if (selected==0) {
      bool valid = !xunion();
      // std::cout << (valid ? "valid\n" : "INVALID\n");
      return valid;
    }

    GECODE_AUTOARRAY(CountableSetRanges, sel, selected);
    CountableSetValues selector(x.lub, x[selector_pos]);
    for (int i=selected; i--;++selector) {
      if (selector.val()>=selector_pos || selector.val()<0) {
	// std::cout << "INVALID\n";
	return false;
      }
      sel[i].init(x.lub, x[selector.val()]);
    }
    Iter::Ranges::NaryUnion<CountableSetRanges> u(sel, selected);

    CountableSetRanges z(x.lub, x[union_pos]);
    bool valid = Iter::Ranges::equal(u,z);
    // std::cout << (valid ? "valid\n" : "INVALID\n");
    return valid;
  }
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // access the space manager
    assert(this->manager().object() != NULL);
    BuddyMgr lm = this->manager();
    // lm.debug();

    int vars = selector_pos;
    CpltSetVarArgs xs(vars);
    for (int i=vars; i--;)
      xs[i]=x[i];
    Gecode::selectUnion(home, xs, x[selector_pos], x[union_pos], SCL_DOM);
  }
};
BddSelectUnion _bddselectUnion("Select::SelectUnion");


class BddFindNonEmptySub : public BddTest {
private:
  int selector_pos;
  int union_pos;
  int xsize;
public:
  BddFindNonEmptySub(const char* t) : BddTest(t, 4, ds_1012,false, 0, 800, 1000), xsize(4) {
    /// using 1012 leads to 5min testtime for ONE iteration !
    union_pos = xsize - 1;
    selector_pos = xsize - 2;   
  }
  virtual bool solution(const SetAssignment& x) const {
    int selected = 0;
    // count the number of selected sets
    CountableSetValues count_sel(x.lub, x[selector_pos]);
    for ( ; count_sel(); ++count_sel, selected++);
   
    CountableSetRanges xunion(x.lub, x[union_pos]);
    if (selected==0) {
      GECODE_AUTOARRAY(CountableSetRanges, sel, selector_pos);
      
      bool valid = true;
      for (int i = selector_pos; i--; ) {
	sel[i].init(x.lub, x[i]);
	CountableSetRanges t(x.lub, x[union_pos]);
	bool non_empty_union = t();
	bool non_empty = sel[i]();
	bool subset = Iter::Ranges::subset(sel[i], t);
	
	if (!non_empty_union) {
	  return true;
	} else {
	  valid &= (!non_empty || !subset);
	}
      }
      return valid;
    }
    
    if (!xunion()) {
      return false;
    }
    // some sets have been selected
    GECODE_AUTOARRAY(CountableSetRanges, sel, selected);
    
    CountableSetValues selector(x.lub, x[selector_pos]);
    for (int i=selected; i--;++selector) {
      if (selector.val()>=selector_pos || selector.val()<0) {
	return false;
      }
      sel[i].init(x.lub, x[selector.val()]);
    }

    Iter::Ranges::NaryUnion<CountableSetRanges> u(sel, selected);

    bool select_valid = true;
      CountableSetValues select_val(x.lub, x[selector_pos]);
      for (int i = 0; i < selector_pos; i++) {
	CountableSetRanges cur_sel(x.lub, x[i]);
	bool subset = false;
	bool non_empty = false;
	CountableSetRanges tunion(x.lub, x[union_pos]);
	non_empty = cur_sel();
	subset = Iter::Ranges::subset(cur_sel, tunion);

	if (select_val()) {
	  if (i == select_val.val()) {
	    select_valid &= (subset && non_empty);
	    ++select_val;
	  } else {
	    select_valid &= (!non_empty || !subset);
	  }
	} else {
	  select_valid &= (!non_empty || !subset);
	}
      }

    bool valid = select_valid && Iter::Ranges::subset(u, xunion);
    return valid;
  }

  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    int vars = selector_pos;
    CpltSetVarArgs xs(vars);
    for (int i=vars; i--;)
      xs[i]=x[i];

    // access the space manager
    assert(this->manager().object() != NULL);
    BuddyMgr lm = this->manager();
    // lm.debug();


    Gecode::findNonEmptySub(home, xs, x[selector_pos], x[union_pos], SCL_DOM);
  }
};
BddFindNonEmptySub _bddfindNonEmptySub("Select::FindNonEmptySub");


// STATISTICS: test-bdd
