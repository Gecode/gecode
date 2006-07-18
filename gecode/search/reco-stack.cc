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
    unsigned int last = entries()-1;

    while (!operator[](last).space())
      last--;

    stat.clone++;
    Space* s = operator[](last).space()->clone(true,stat.propagate);

    unsigned int dist = entries() - last;
    stat.commit += dist;
    if (dist < a_d) {
      for (unsigned int i = last; i < entries(); i++)
	s->commit(operator[](i).desc(),operator[](i).alt());
      d = dist;
    } else {
      unsigned int mid  = last + (dist >> 1);
      unsigned int i    = last;

      // Recompute up to middle
      for (; i < mid; i++ )
	s->commit(operator[](i).desc(),operator[](i).alt());
      // Skip over all rightmost branches
      for (; (i < entries()) && operator[](i).rightmost(); i++)
	s->commit(operator[](i).desc(),operator[](i).alt());
      // Is there any point to make a copy?
      if (i < entries()-1) {
	stat.clone++;
	operator[](i).space(s->clone(true,stat.propagate));
	stat.adapt(operator[](i).space());
	d = entries()-i;
      } else {
	d = entries()-last;
      }
      // Finally do the remaining commits
      for (; i < entries(); i++ )
	s->commit(operator[](i).desc(),operator[](i).alt());
    }
    return s;
  }

}}

// STATISTICS: search-any
