/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
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

#include "test/cpltset.hh"
#include "test/log.hh"
#include "gecode/iter.hh"
#include <algorithm>

#ifdef GECODE_HAVE_SET_VARS
#else 
CountableSet::CountableSet(const IntSet& d0) : d(d0), //curd(IntSet::empty),
					       cur(0) {
  IntSetRanges isr(d);
  lubmax =
    static_cast<unsigned int>(pow(static_cast<double>(2.0),
				  static_cast<int>(Iter::Ranges::size(isr))));
}

void CountableSet::operator++(void) {
  cur++;
}

void CountableSet::init(const IntSet& d0) {
  d = d0;
  cur = 0;
  IntSetRanges isr(d);
  lubmax =
    static_cast<unsigned int>(pow(static_cast<double>(2.0),
				  static_cast<int>(Iter::Ranges::size(isr))));
}

int CountableSet::val(void) const {
  return cur;
}

SetAssignment::SetAssignment(int n0, const IntSet& d0, int _withInt)
  : n(n0), dsv(new CountableSet[n]), ir(_withInt, d0), done(false), lub(d0), withInt(_withInt) {
  for (int i=n; i--; )
    dsv[i].init(lub);
}

void
SetAssignment::reset(void) {
  done = false;
  for (int i=n; i--; )
    dsv[i].init(lub);
  ir.reset();
}

void
SetAssignment::operator++(void) {
  int i = n-1;
  while (true) {
    ++dsv[i];
    if (dsv[i]())
      return;
    dsv[i].init(lub);
    --i;
    if (i<0) {
      if (withInt==0) {
	done = true;
	return;
      }
      ++ir;
      if (ir()) {
	i = n-1;
	for (int j=n; j--; )
	  dsv[j].init(lub);
      } else {
	done = true;
	return;
      }
    }
  }
}

std::ostream&
operator<<(std::ostream& os, const SetAssignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++) {
    CountableSetRanges csv(a.lub, a[i]);
    os << csv << ((i!=n-1) ? "," : "}");
  }
  if (a.withInt > 0)
    os << a.ints();
  return os;
}
#endif 

#define FORCE_FIX                               \
do {                                            \
  if (Test::randgen(opt.fixprob) == 0) {        \
    Log::fixpoint();                            \
    if (status() == SS_FAILED) return;          \
  }                                             \
} while(false)

class BddTestSpace : public Space {
public:
  BuddyMgr mgr;
  CpltSetVarArray x;
  IntVarArray y;
private:
  const Options opt;
  int withInt;
public:
  BddTestSpace(int n, IntSet& d, int _withInt, const Options& o, int ivs=10000, int ics=1000) 
    : mgr(ivs, ics), 
      x(this, mgr.manager(), n, IntSet::empty, d), y(this, _withInt, d),
      opt(o), withInt(_withInt) {  
    Log::initial(x, "x");
  }
  BddTestSpace(bool share, BddTestSpace& s) 
    : Space(share,s), opt(s.opt), withInt(s.withInt) {
    x.update(this, share, s.x);
    y.update(this, share, s.y);
    mgr.update(this, share, s.mgr);
  }
  // ~BddTestSpace(void) { std::cerr << "BddTestSpace destruct\n";mgr.dispose(); }

  BuddyMgr& manager(void) { return mgr;}
  //void manager(BuddyMgr& m) { mgr.update(this, true, m); }
  
  virtual Space* copy(bool share) {
    return new BddTestSpace(share,*this);
  }
  bool failed(void) {
    return status() == SS_FAILED;
  }
  void assign(const SetAssignment& a) {
    for (int i=a.size(); i--; ) {
      CountableSetRanges csv(a.lub, a[i]);
      IntSet ai(csv);
      Log::assign(Log::mk_name("x", i), ai);
      dom(this, x[i], SRT_EQ, ai);
      FORCE_FIX;
    }
    for (int i=withInt; i--; ) {
      Log::assign(Log::mk_name("y", i), a.ints()[i]);
      rel(this, y[i], IRT_EQ, a.ints()[i]);
      FORCE_FIX;
    }
  }

  bool assigned(void) const {
    // std::cerr << "assigned ?\n";
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
	return false;
    for (int i=y.size(); i--; )
      if (!y[i].assigned())
	return false;
    return true;
  }

  void removeFromLub(int v, CpltSetVar& x, int i, const SetAssignment& a) {
    CpltSetVarUnknownRanges ur(x);
    CountableSetRanges air(a.lub, a[i]);
    Iter::Ranges::Diff<CpltSetVarUnknownRanges, CountableSetRanges> diff(ur, air);
    Iter::Ranges::ToValues<Iter::Ranges::Diff
      <CpltSetVarUnknownRanges, CountableSetRanges> > diffV(diff);
    for (int j=0; j<v; j++, ++diffV);
    Log::prune(x, Log::mk_name("x", i), SRT_DISJ, diffV.val());
    // std::cerr << " x [" << i << "] SRT_DISJ " << diffV.val() << "\n";
    dom(this, x, SRT_DISJ, diffV.val());
    Log::prune_result(x);      
  }

  void addToGlb(int v, CpltSetVar& x, int i, const SetAssignment& a) {
    CpltSetVarUnknownRanges ur(x);
    CountableSetRanges air(a.lub, a[i]);
    Iter::Ranges::Inter<CpltSetVarUnknownRanges, CountableSetRanges>
      inter(ur, air);
    Iter::Ranges::ToValues<Iter::Ranges::Inter
      <CpltSetVarUnknownRanges, CountableSetRanges> > interV(inter);
    for (int j=0; j<v; j++, ++interV);
    Log::prune(x, Log::mk_name("x", i), SRT_SUP, interV.val());
    // std::cerr << " x ["<<i<<"] SRT_SUP " << interV.val() << "\n";
    dom(this, x, SRT_SUP, interV.val());
    Log::prune_result(x);      
  }

  bool fixprob(BddTest& st, bool r, BoolVar& b) {
    // std::cerr << "fixprob\n";
    Log::fixpoint();				
    if (status() == SS_FAILED) 
      return true;
    // std::cerr << "clone the space\n";
    // std::cerr << "************** FP TEST ************\n";
    BddTestSpace* c = static_cast<BddTestSpace*>(clone());
    Log::print(c->x, "x");
    if (c->y.size() > 0) Log::print(c->y, "y");
    if (!r) {
      // std::cerr << "not reified post\n";
      st.post(c,c->x,c->y);
      Log::fixpoint();
      if (c->status() == SS_FAILED) {
	// std::cerr << "stat failed\n";
	Log::print(c->x, "x");
	if (c->y.size() > 0) Log::print(c->y, "y");
	delete c;
	return false;
      }
      // std::cerr << "testing sizes !!!\n";
      for (int i=x.size(); i--; ) {
 	// std::cerr << "x["<<i<<"]=" << x[i] << " ";
 	// std::cerr << "("<<x[i].glbSize() << "," << x[i].lubSize() <<")"<<"\n";
 	if (x[i].glbSize() != c->x[i].glbSize() ||
 	    x[i].lubSize() != c->x[i].lubSize() ||
 	    x[i].cardMin() != c->x[i].cardMin() ||
 	    x[i].cardMax() != c->x[i].cardMax()) {
 	  // std::cerr << "sizes differ: " << x[i] << " != " << c->x[i] << "\n";
 	  Log::print(c->x, "x");
 	  if (c->y.size() > 0) Log::print(c->y, "y");
 	  delete c;
 	  return false;
 	}
      }
    } else {
      Log::print(b, "b");
      BoolVar cb(c,0,1);
      Log::initial(cb, "cb");
      st.post(c,c->x,c->y,cb);
      Log::fixpoint();
      if (c->status() == SS_FAILED) {
	Log::print(c->x, "x");
	if (c->y.size() > 0) Log::print(c->y, "y");
	Log::print(cb, "cb");
	delete c;
	return false;
      }
      if (cb.size() != b.size()) {
	Log::print(c->x, "x");
	if (c->y.size() > 0) Log::print(c->y, "y");
	Log::print(cb, "cb");
	delete c;
	return false;
      }
       for (int i=x.size(); i--; )
 	if (x[i].glbSize() != c->x[i].glbSize() ||
 	    x[i].lubSize() != c->x[i].lubSize() ||
 	    x[i].cardMin() != c->x[i].cardMin() ||
 	    x[i].cardMax() != c->x[i].cardMax() ) {
 	  Log::print(c->x, "x");
 	  if (c->y.size() > 0) Log::print(c->y, "y");
 	  Log::print(cb, "cb");
 	  delete c;
 	  return false;
 	}
    }
    delete c;
    return true;
  }

  static BoolVar unused;

  bool prune(const SetAssignment& a, BddTest& st, bool r, BoolVar& b=unused) {
    // std::cerr << "pruning variables\n";
    bool setsAssigned = true;
    for (int j=x.size(); j--; )
      if (!x[j].assigned()) {
	setsAssigned = false;
	break;
      }
    bool intsAssigned = true;
    for (int j=y.size(); j--; )
      if (!y[j].assigned()) {
	intsAssigned = false;
	break;
      }

    // std::cerr << "sel var\n";
    // Select variable to be pruned
    int i;
    if (intsAssigned) {
      i = Test::randgen(x.size());
    } else if (setsAssigned) {
      i = Test::randgen(y.size());
    } else {
      i = Test::randgen(x.size()+y.size());
    }

    if (setsAssigned || i>=x.size()) {
      if (i>=x.size())
	i = i-x.size();
      while (y[i].assigned()) {
	i = (i+1) % y.size();
      }
      // Prune int var

      // std::cerr << "sel int var\n";
      // Select mode for pruning
      int m=Test::randgen(3);
      if ((m == 0) && (a.ints()[i] < y[i].max())) {
	int v=a.ints()[i]+1+
	  Test::randgen(static_cast<unsigned int>(y[i].max()-a.ints()[i]));
	assert((v > a.ints()[i]) && (v <= y[i].max()));
	Log::prune(y[i], Log::mk_name("y", i), IRT_LE, v);
	rel(this, y[i], IRT_LE, v);
	Log::prune_result(y[i]);
      } else if ((m == 1) && (a.ints()[i] > y[i].min())) {
	int v=y[i].min()+
	  Test::randgen(static_cast<unsigned int>(a.ints()[i]-y[i].min()));
	assert((v < a.ints()[i]) && (v >= y[i].min()));
	Log::prune(y[i], Log::mk_name("y", i), IRT_GR, v);
	rel(this, y[i], IRT_GR, v);
	Log::prune_result(y[i]);
      } else  {
	int v;
	Int::ViewRanges<Int::IntView> it(y[i]);
	unsigned int skip = Test::randgen(y[i].size()-1);
	while (true) {
	  if (it.width() > skip) {
	    v = it.min() + skip;
	    if (v == a.ints()[i]) {
	      if (it.width() == 1) {
		++it; v = it.min();
	      } else if (v < it.max()) {
		++v;
	      } else {
		--v;
	      }
	    }
	    break;
	  }
	  skip -= it.width();
	  ++it;
	}
	Log::prune(y[i], Log::mk_name("y", i), IRT_NQ, v);
	rel(this, y[i], IRT_NQ, v);
	Log::prune_result(y[i]);
      }
      if (Test::randgen(opt.fixprob) == 0 && !fixprob(st, r, b))
	return false;
      return true;
    }
    // std::cerr << "sel set var\n";
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    // std::cerr << "choose x["<<i<<"]=" << x[i] << "\n";
    CpltSetVarUnknownRanges ur1(x[i]);
    CountableSetRanges air1(a.lub, a[i]);
    Iter::Ranges::Diff<CpltSetVarUnknownRanges, CountableSetRanges>
      diff(ur1, air1);
    CpltSetVarUnknownRanges ur2(x[i]);
    CountableSetRanges air2(a.lub, a[i]);
    Iter::Ranges::Inter<CpltSetVarUnknownRanges, CountableSetRanges>
      inter(ur2, air2);

    CountableSetRanges aisizer(a.lub, a[i]);
    unsigned int aisize = Iter::Ranges::size(aisizer);

    // std::cerr << "prune setvar\n";
    // Select mode for pruning
    int m=Test::randgen(5);
    if (m==0 && inter()) {
      int v = Test::randgen(Iter::Ranges::size(inter));
      // std::cerr << " add to glb\n";
      addToGlb(v, x[i], i, a);
    } else if (m==1 && diff()) {
      int v = Test::randgen(Iter::Ranges::size(diff));
      // std::cerr << " remove from lub\n";
      removeFromLub(v, x[i], i, a);
    } else  if (m==2 && x[i].cardMin() < aisize) {
      unsigned int newc = x[i].cardMin() + 1 + 
 	Test::randgen(aisize - x[i].cardMin());
      assert( newc > x[i].cardMin() );
      assert( newc <= aisize );
      Log::prune(x[i], Log::mk_name("x", i), newc, Limits::Set::card_max);
      // std::cerr << " cardinalityboth"<< newc <<","<< Limits::Set::card_max<<"\n";
      cardinality(this, x[i], newc, Limits::Set::card_max);
      Log::prune_result(x[i]);
    } else if (m==3 && x[i].cardMax() > aisize) {
      unsigned int newc = x[i].cardMax() - 1 - 
 	Test::randgen(x[i].cardMax() - aisize);
      assert( newc < x[i].cardMax() );
      assert( newc >= aisize );
      Log::prune(x[i], Log::mk_name("x", i), 0, newc);
      // std::cerr << " cardinalitymax 0," << newc << "\n";
      cardinality(this, x[i], 0, newc);
      // std::cerr << " did cardmax 0," << newc << "\n";
      Log::prune_result(x[i]);
      // std::cerr << "logged prune result\n";
    } else
      {
	if (inter()) {
	  int v = Test::randgen(Iter::Ranges::size(inter));
	  // std::cerr << " inter add to glb \n";
	  addToGlb(v, x[i], i, a);
	} else {
	  int v = Test::randgen(Iter::Ranges::size(diff));
	  // std::cerr << " inter remove from lub \n";
	  removeFromLub(v, x[i], i, a);
	}
      }
    // do not output if failure detected
    if (!this->failed()) {
      // std::cerr << "pruning done with x["<< i<<"]=" << x[i] << "\n";
    } else {
      // std::cerr << "pruning done \n";
    }
    // end out
    if (Test::randgen(opt.fixprob) == 0 && !fixprob(st, r, b))
      return false;
    // std::cerr << "end setvar\n";
    return true;
  }
  
};

BoolVar BddTestSpace::unused;


#define CHECK(T,M) 				\
if (!(T)) { 					\
  problem = (M); 				\
  delete s;					\
  goto failed; 					\
}

SetAssignment*
BddTest::make_assignment(void) {
  return new SetAssignment(arity,lub,withInt);
}

bool 
BddTest::run(const Options& opt) {
  const char* test    = "NONE";
  const char* problem = "NONE";
  // std::cerr << "test = " << test << "\n";
  SetAssignment* ap = make_assignment();
  SetAssignment& a = *ap;
  
  int an = 0;
  //   int cutoff = 80000;
  //   while( a() && an < cutoff) {
  //     ++a;
  //     an++;
  //   }
  
  while (a()) {
    // std::cerr << "compute solution\n";    
    bool is_sol;
    {
      BddTestSpace* search_s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());
      // std::cerr << "pass manager\n";
      manager(search_s->manager());
      // std::cerr << "passed manager\n";
      post(search_s,search_s->x, search_s->y);
    
    //   branch(search_s,search_s->x,SETBVAR_NONE,SETBVAL_MIN);
    //   Gecode::DFS<BddTestSpace> e_s(search_s);
      is_sol = solution(a);    
    // std::cerr << "finished search\n";
    // for sake of correct disposal
      delete search_s;
    }
    
    manager().clear();

    {
      test = "Assignment (after posting)";
      // std::cerr << "test = " << test << "\n";
      Log::reset();
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      post(s,s->x,s->y); s->assign(a);
      if (is_sol) {
	CHECK(!s->failed(), "Failed on solution");
	CHECK(s->propagators()==0, "No subsumtion");
      } else {
	CHECK(s->failed(), "Solved on non-solution");
      }
      delete s;
    }
    
    manager().clear();


    {
      test = "Assignment (before posting)";
      // std::cerr << "test = " << test << "\n";
      Log::reset();
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      s->assign(a); post(s,s->x,s->y);
      if (is_sol) {
	CHECK(!s->failed(), "Failed on solution");
	CHECK(s->propagators()==0, "No subsumtion");
      } else {
	CHECK(s->failed(), "Solved on non-solution");
      }
      delete s;      
    }
    
    manager().clear();

    if (reified) {
      test = "Assignment reified (before posting)";
      // std::cerr << "test = " << test << "\n";
      Log::reset();
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      BoolVar b(s,0,1); 
      s->assign(a); post(s,s->x,s->y,b);
      CHECK(!s->failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
	CHECK(b.val()==1, "Zero on solution");
      } else {
	CHECK(b.val()==0, "One on non-solution");
      }
      delete s;
      
    }
    
    manager().clear();

    if (reified) {
      test = "Assignment reified (after posting)";
      // std::cerr << "test = " << test << "\n";
      Log::reset();
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      BoolVar b(s,0,1);
      post(s,s->x,s->y,b); s->assign(a);
      CHECK(!s->failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
	CHECK(b.val()==1, "Zero on solution");
      } else {
	CHECK(b.val()==0, "One on non-solution");
      }
      delete s;      
    }
    
    manager().clear();

    {
      test = "Prune";
      // std::cerr << "Assignment = " << a << "\n";
      // std::cerr << "test = " << test << "\n";
      Log::reset();
      // std::cerr <<  "start FP-test\n";
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      // std::cerr << "*********** FIRST POST *******\n";
      post(s,s->x,s->y);
      // std::cerr << "start fp-loop\n";
      while (!s->failed() && !s->assigned()) {
      // std::cerr << "fp-looping\n";
 	if (!s->prune(a,*this,false)) {
 	  problem = "No fixpoint";
 	  delete s;
	  
 	  goto failed;
 	}
	// std::cerr << "still looping\n";
      }
      // std::cerr << "finished loop\n";
      s->assign(a);
      if (is_sol) {
	CHECK(!s->failed(), "Failed on solution");
	CHECK(s->propagators()==0, "No subsumtion");
      } else {
	CHECK(s->failed(), "Solved on non-solution");
      }
      delete s;      
    }
    
    manager().clear();

    if (reified) {
      test = "Prune reified";
      // std::cerr << "test = " << test << "\n";
      BddTestSpace* s = new BddTestSpace(arity,lub,withInt,opt, varsize(), cachesize());;
      manager(s->manager());
      BoolVar b(s,0,1);
      post(s,s->x,s->y,b);
      while (!s->assigned() && !b.assigned())
 	if (!s->prune(a,*this,true,b)) {
 	  problem = "No fixpoint";
 	  delete s;
	  
 	  goto failed;
 	}
      CHECK(!s->failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
	CHECK(b.val()==1, "Zero on solution");
      } else {
	CHECK(b.val()==0, "One on non-solution");
      }
      delete s;      
    }
    
    manager().clear();

    ++an;
    ++a;
    // std::cerr << "assignment loop\n";
  }

  delete ap;
  return true;
 failed:
  std::cout << "FAILURE" << std::endl
	    << "\t" << "Test:       " << test << std::endl
	    << "\t" << "Problem:    " << problem << std::endl
	    << "\t" << "SetAssignment: " << a << std::endl
	    << "\t" << "Assignment Nr: " << an << std::endl;
  delete ap;
  return false;
}

#undef CHECK

// STATISTICS: test-bdd
