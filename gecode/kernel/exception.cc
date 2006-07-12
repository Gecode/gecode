/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "gecode/kernel.hh"

namespace Gecode {

  /*
   * Base class for exceptions
   *
   */
  Exception::Exception(const char* l, const char* i) throw() {
    int j = 0;
    while ((*l != 0) && (j < li_max))
      li[j++] = *(l++);
    if (j < li_max)
      li[j++] = ':';
    if (j < li_max)
      li[j++] = ' ';
    while ((*i != 0) && (j < li_max))
      li[j++] = *(i++);
    li[j] = 0;
  }
  const char*
  Exception::what(void) const throw() {
    return &li[0];
  }

}

// STATISTICS: kernel-other
