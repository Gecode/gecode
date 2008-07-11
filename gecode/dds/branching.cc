/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Martin Mann <mmann@informatik.uni-freiburg.de>
 *     Sebastian Will <will@informatik.uni-freiburg.de>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *     Martin Mann, 2008
 *     Sebastian Will, 2008
 *
 *  Last modified:
 *     $Date: 2007-04-05 15:25:13 +0200 (Thu, 05 Apr 2007) $ by $Author: tack $
 *     $Revision: 4322 $
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

#include "gecode/dds.hh"

namespace Gecode { namespace Decomposition {

  void
  DecompDesc::significantVars(int alt, std::vector<int>& sv) const {
    assert(alt >= 0 && (unsigned int)alt < alternatives());
      int count = 0;
      for (std::vector<int>::const_iterator it=label.begin(); 
           it!=label.end(); it++) {
        if (*it==alt) count++;
      }
      sv.resize(count);
      for (int i=label.size()-1; i>=0; i--)
        if (label[i]==alt) sv[--count]=i;
  }

}}

// STATISTICS: dds-any
