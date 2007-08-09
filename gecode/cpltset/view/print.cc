/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"
using namespace Gecode;
using namespace Gecode::CpltSet;

template <class I>
static void
printRange(std::ostream& os, I& i) {
  // std::cout << "inside test is running: " << bdd_isrunning() << "\n";
  // std::cout << "printImp\n";
  os << "{";
  if (i()) {
    if (i.min() == i.max()) {
      os << i.min();
    } else {
      os << i.min() << "#"<< i.max();
    }
    ++i;
  }
  while(i()) {
    os << ",";
    if (i.min() == i.max()) {
      os << i.min();
    } else {
      os << i.min() << "#"<< i.max();
    }
    ++i;
  }
  os << "}";
}

template <class I>
static void
printValue(std::ostream& os, I& i) {
  os << "{";
  if (i()) {
    os << i.val();
    ++i;
  }
  while(i()) {
    os << ",";
    os << i.val();
    ++i;
  }
  os << "}";
}

template <class I, class J>
static void
printVar(bool assigned, std::ostream& os, I& glb, J& lub) {
  if (assigned) {
    printRange(os, lub);
  } else {
    // std::cout << "not assigned print glb and lub";
    printRange(os, glb);
    os << "..";
    printRange(os, lub);
  }
}

/**
 * \brief Print set variable view
 * \relates Gecode::CpltSet::CpltSetView
 */

std::ostream&
operator<<(std::ostream& os, const CpltSetView& x) {
  BMI* mgr = x.manager();
  CpltSetOutput l = mgr->level();
  bool assigned = x.assigned();
  // std::cout << "print level = " << l << "\n";
  switch(l) {
  case CPLTSET_DOM: 
    {
      if (assigned) {
	GlbValues<CpltSetView> glb(x);
	printValue(os, glb);
      } else {
	DomValues<CpltSetView> dom(x);
	printValue(os, dom);
      }
    }
    break;
  case CPLTSET_BND:
    {
      Gecode::Set::GlbRanges<CpltSetView> glb(x);
      if (assigned) {
	printRange(os, glb);
      } else {
	Gecode::Set::LubRanges<CpltSetView> lub(x);
	printVar(assigned, os, glb, lub);
      }
    }
    break;
  case CPLTSET_BND_DOM:
    {
      os << "Conv(_";
      if (assigned) {
	GlbValues<CpltSetView> glb(x);
	printValue(os, glb);
      } else {
	DomValues<CpltSetView> dom(x);
	printValue(os, dom);
      }
      os << "_) = ";
      Gecode::Set::GlbRanges<CpltSetView> glb(x);
      if (assigned) {
	printRange(os, glb);
      } else {
	Gecode::Set::LubRanges<CpltSetView> lub(x);
	printVar(assigned, os, glb, lub);
      }
    }
    break;
  }
  return os;
}

/**
 * \brief Print set variable view
 */

std::ostream&
operator<<(std::ostream& os, const SplitCpltSetView& x) {
  os << x.base();
  return os;
}

std::ostream&
operator<<(std::ostream& os, const BndCpltSetView& x) {
  os << x.base();
  return os;
}

std::ostream&
operator<<(std::ostream& os, const CrdCpltSetView& x) {
  assert(x.cardMin() <= x.cardMax());
  
  // test whether cardinality was channeled correctly
  BMI* mgr = x.manager();
  // unsigned int off = x.offset();
  // unsigned int tab = x.table_width();
  // GecodeBdd dummy = BDDTOP;
  GecodeBdd dom   = x.base().variable()->bdd_domain();
  int l = 0;
  int u = 0;
  getcardbounds(mgr, dom, l, u);
  // card_count(mgr, dom, dummy, 0, off, tab, l, u);

  // end test

  os << x.base() << "["<< x.cardMin()<<"#"<<x.cardMax()<<"]";
  os << "CC["<< l<<".."<<u<<"]";
//   assert(x.cardMin() >= l); // does not work for crdbddview
//   assert(x.cardMax() <= u);
  return os;
}

std::ostream&
operator<<(std::ostream& os, const LexCpltSetView& x) {
  os << x.base();
  return os;
}

std::ostream&
operator<<(std::ostream& os, const SingletonCpltSetView& x) {
  os << x.base();
  return os;
}

std::ostream&
operator<<(std::ostream& os, const SetCpltSetView& x) {
  os << x.base();
  return os;
}

std::ostream&
operator<< (std::ostream& os, const CpltSetVar& x) {
  BMI* mgr = x.manager();
  CpltSetOutput l = mgr->level();
  // std::cout << "print level = " << l << "\n";
  bool assigned = x.assigned();
  switch(l) {
  case CPLTSET_DOM: 
    {
      if (assigned) {
	CpltSetVarGlbValues glb(x);
	printValue(os, glb);
      } else {
	CpltSetVarDomValues dom(x);
	printValue(os, dom);
      }
    }
    break;
  case CPLTSET_BND:
    {
      CpltSetVarGlbRanges glb(x);
      if (assigned) {
	printRange(os, glb);
      } else {
	CpltSetVarLubRanges lub(x);
	printVar(assigned, os, glb, lub);
      }
    }
    break;
  case CPLTSET_BND_DOM:
    {
      os << "Conv(_";
      if (assigned) {
	CpltSetVarGlbValues glb(x);
	printValue(os, glb);
      } else {
	CpltSetVarDomValues dom(x);
	printValue(os, dom);
      }
      os << "_) = ";
      CpltSetVarGlbRanges glb(x);
      if (assigned) {
	printRange(os, glb);
      } else {
	CpltSetVarLubRanges lub(x);
	printVar(assigned, os, glb, lub);
      }
    }
    break;
  }
  return os;
}

std::ostream&
operator<< (std::ostream& os, BMI* m) {
//   os << "print mgr ";
//   std::cout << m << "\n";
  m->debug(os);
  return os;
}

// STATISTICS: bdd-var
