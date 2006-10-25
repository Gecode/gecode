/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include "gecode/int.hh"

#include "gecode/support/sort.hh"

namespace Gecode {

  /// Sort ranges according to increasing minimum
  class IntSet::MinInc {
  public:
    bool operator()(const Range &x, const Range &y);
  };

  forceinline bool
  IntSet::MinInc::operator()(const Range &x, const Range &y) {
    return x.min < y.min;
  }

  void
  IntSet::normalize(int n) {
    // Sort ranges
    MinInc lt_mi;
    Support::quicksort<Range>(&sar[0], n, lt_mi);
    // Conjoin continuous ranges
    int min = sar[0].min;
    int max = sar[0].max;
    int i = 1;
    int j = 0;
    while (i < n) {
      if (max+1 < sar[i].min) {
        sar[j].min = min; sar[j].max = max; j++;
        min = sar[i].min; max = sar[i].max; i++;
      } else {
        max = std::max(max,sar[i].max); i++;
      }
    }
    sar[j].min = min; sar[j].max = max;
    sar.size(j+1);
  }

  void
  IntSet::init(const int r[], int n) {
    if (n>0) {
      for (int i = n; i--; ) {
        sar[i].min=r[i]; sar[i].max=r[i];
      }
      normalize(n);
    }
  }

  void
  IntSet::init(const int r[][2], int n) {
    if (n>0) {
      int j = 0;
      for (int i = 0; i < n; i++)
        if (r[i][0] <= r[i][1]) {
          sar[j].min=r[i][0]; sar[j].max=r[i][1]; j++;
        }
      normalize(j);
    }
  }

  void
  IntSet::init(int n, int m) {
    if (n <= m) {
      sar[0].min = n; sar[0].max = m;
    } else {
      sar.size(0);
    }
  }

  const IntSet IntSet::empty;

}

std::ostream&
operator<<(std::ostream& os, const Gecode::IntSet& is) {
  os << '<';
  for (int i = 0; i < is.size(); ) {
    int min = is.min(i);
    int max = is.max(i);
    if (min == max)
      os << min;
    else
      os << min << ".." << max;
    i++;
    if (i < is.size())
      os << ',';
  }
  return os << '>';
}


// STATISTICS: int-var

