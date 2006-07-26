/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include "gecode/search.hh"

namespace Gecode { namespace Search {

  Space*
  ReCoStack::recompute(unsigned int& d, EngineCtrl& stat) {
    assert(!empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((top().space() != NULL) && top().rightmost()) {
      Space* s = top().space();
      s->commit(top().desc(),top().alt());
      top().space(NULL);
      stat.lao(s);
      d = 0;
      stat.commit++;
      return s;
    }
    // General case for recomputation
    int l = lc();      // Position of last clone
    int n = entries(); // Number of stack entries
    d = n - l;         // New distance, if no adaptive recomputation kicks in 

    stat.commit += d;  // Number of commit operations to be performed
    stat.clone++;
    Space* s = operator[](l).space()->clone(true,stat.propagate);

    if (d < a_d) {
      for (int i=l; i<n; i++)
	s->commit(operator[](i).desc(),operator[](i).alt());
    } else {
      int m = l + (d >> 1); // Middle between copy and top
      int i = l;            // To iterate over all entries
      // Recompute up to middle
      for (; i<m; i++ )
	s->commit(operator[](i).desc(),operator[](i).alt());
      // Skip over all rightmost branches
      for (; (i<n) && operator[](i).rightmost(); i++)
	s->commit(operator[](i).desc(),operator[](i).alt());
      // Is there any point to make a copy?
      if (i<n-1) {
	stat.clone++;
	operator[](i).space(s->clone(true,stat.propagate));
	stat.adapt(operator[](i).space());
	d = n-i;
      }
      // Finally do the remaining commits
      for (; i<n; i++)
	s->commit(operator[](i).desc(),operator[](i).alt());
    }
    return s;
  }

}}

// STATISTICS: search-any
