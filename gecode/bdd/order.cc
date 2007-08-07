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

#include "gecode/bdd.hh"
using namespace Gecode::Bdd;

namespace Gecode {

  void 
  hls_order(const BddVarArray& x) {
    // std::cout << "start order\n";
    BddMgr* mgr = x[0].manager();
    unsigned int var_in_tab = mgr->allocated();
    
    // std::cout << "var_in_tab = " << var_in_tab << "\n";

    // constraint order as specified by Hawkins, Lagoon and Stuckey
    GECODE_AUTOARRAY(int, hls_order, var_in_tab);
    
    unsigned int min_offset = x[0].offset();
    unsigned int max_width  = x[0].table_width();
    for (int i = 0; i < x.size(); i++) {
      if (x[i].offset() < min_offset) {
	min_offset = x[i].offset();
      }
      if (x[i].table_width() > max_width) {
	max_width = x[i].table_width();
      }
    }
    
//     std::cout << "max_width = " << max_width  << " min_offset = " << min_offset << "\n";
    

    // we do not care about variables 
    // that are not in the scope of the constraint
    int c = 0;
    for (unsigned int i = 0; i < min_offset; i++, c++) {
      hls_order[i] = i;
    }
    // ordering those that lie in the scope
    for (unsigned int f = 0; f < max_width; f++) {
      for (int i = 0; i < x.size(); i++) {
	int xo = x[i].offset();
	// std::cout << "order " << xo << "\n";
	int xw = x[i].table_width();
	int cur= xo + f;
	if (cur <= xo + xw) {
	  hls_order[c] = cur;
	  c++;
	}
      }
    }

    // IMPORTANT DO NOT FORGET REMAINING LARGER VARIABLES
    for (unsigned int i = c; i < var_in_tab; i++, c++) {
      hls_order[i] = i;
    }   
    // std::cout << "set order\n";
    mgr->setorder(&hls_order[0]);
    // std::cout << "ordering done\n";
  }

}

// STATISTICS: bdd-other
