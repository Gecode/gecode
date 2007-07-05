/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

#include "gecode/set.hh"
#include "gecode/set/distinct.hh"

/*
 * These propagators implement the scheme discussed in
 *
 * Andrew Sadler and Carment Gervet: Global Reasoning on Sets.
 * FORMUL'01 workshop in conjunction with CP 2001.
 *
 * Todo: make the propagators incremental.
 */

namespace Gecode { namespace Set { namespace Distinct {

  /*
   * n-ary distinct with fixed cardinalities propagator
   *
   */

  static ModEvent
  nosubset(Space* home, SetView x, SetView y) {
    GlbRanges<SetView> xglb(x);
    GlbRanges<SetView> yglb(y);
    Iter::Ranges::Diff<GlbRanges<SetView>, GlbRanges<SetView> >
      diff(xglb, yglb);
    if (!diff())
      return ME_SET_FAILED;
    if (diff.min() != diff.max())
      return ME_SET_NONE;
    int a = diff.min();
    ++diff;
    if (diff())
      return ME_SET_NONE;
    return y.exclude(home, a);
  }

  Actor*
  Distinct::copy(Space* home, bool share) {
    return new (home) Distinct(home,share,*this);
  }

  Reflection::ActorSpec&
  Distinct::spec(Space* home, Reflection::VarMap& m) {
    Reflection::ActorSpec& s =
      NaryPropagator<SetView, PC_SET_ANY>::spec(home, m);
    return s << c;
  }

  const char*
  Distinct::name(void) const {
    return "set.distinct.Distinct";
  }

  ExecStatus
  Distinct::propagate(Space* home) {

    int curClique = x.size();
    int oldCliques = curClique;

    while (curClique>1) {
      
      unsigned int li = x[0].glbSize();
      curClique--;
      SetView tmp = x[curClique];
      x[curClique] = x[0];
      x[0] = tmp;

      GECODE_AUTOARRAY(LubRanges<SetView>, cliqueLubs, oldCliques);
      cliqueLubs[0].init(x[curClique]);
      unsigned int ki = 1; // size of the clique

      // Find the clique of x[i], i.e. all sets with the same glb.
      // The clique will be moved to the end of the array, but before
      // oldCliques.
      for (int j=0; j<curClique; ) {
        GlbRanges<SetView> xjglb(x[j]);
        GlbRanges<SetView> xiglb(x[curClique]);
        if (Iter::Ranges::equal(xiglb, xjglb)) {
          curClique--;
          SetView tmp2 = x[curClique];
          x[curClique] = x[j];
          x[j] = tmp2;
          cliqueLubs[ki].init(x[curClique]);
          ki++;
        } else {
          j++;
        }
      }

      Iter::Ranges::NaryUnion<LubRanges<SetView> > cliqueLub(cliqueLubs, ki);
      unsigned int ui = Iter::Ranges::size(cliqueLub);

      unsigned int possible = bin.c(ui-li, c-li);

      if (possible < ki)
        return ES_FAILED;

      if (possible == ki) {
        for (int i=curClique; i--; ) {
          GECODE_ME_CHECK(nosubset(home, x[curClique], x[i]));
        }
        for (int i=oldCliques; i<x.size(); i++) {
          GECODE_ME_CHECK(nosubset(home, x[curClique], x[i]));
        }
      }

      oldCliques = curClique;
    }

    return ES_NOFIX;
  }

}}}

// STATISTICS: set-prop
