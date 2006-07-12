/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date: 2005-07-28 22:52:19 +0200 (Thu, 28 Jul 2005) $ by $Author: schulte $
 *     $Revision: 2072 $
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
    while ((*l != 0) && (j < li_max)) {
      li[j++] = *(l++);
    }
    if (j < li_max)
      li[j++] = ':';
    if (j < li_max)
      li[j++] = ' ';
    while ((*i != 0) && (j < li_max)) {
      li[j++] = *(i++);
    }
    li[j] = 0;
  }
  const char*
  Exception::what(void) const throw() {
    return &li[0];
  }

}

// STATISTICS: kernel-other
