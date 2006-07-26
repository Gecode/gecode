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
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(ds.top().desc(),ds.top().alt());
      ds.top().space(NULL);
      stat.lao(s);
      d = 0;
      stat.commit++;
      return s;
    }
    // General case for recomputation
    int l = lc();         // Position of last clone
    int n = ds.entries(); // Number of stack entries
    d = n - l;            // New distance, if no adaptive recomputation

    stat.commit += d;     // Number of commit operations to be performed
    stat.clone++;
    Space* s = ds[l].space()->clone(true,stat.propagate);

    if (d < a_d) {
      for (int i=l; i<n; i++)
	commit(s,i);
    } else {
      int m = l + (d >> 1); // Middle between copy and top
      int i = l;            // To iterate over all entries
      // Recompute up to middle
      for (; i<m; i++ )
	commit(s,i);
      // Skip over all rightmost branches
      for (; (i<n) && ds[i].rightmost(); i++)
	commit(s,i);
      // Is there any point to make a copy?
      if (i<n-1) {
	stat.clone++;
	ds[i].space(s->clone(true,stat.propagate));
	stat.adapt(ds[i].space());
	d = n-i;
      }
      // Finally do the remaining commits
      for (; i<n; i++)
	commit(s,i);
    }
    return s;
  }

}}

// STATISTICS: search-any
