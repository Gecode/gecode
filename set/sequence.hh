/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#ifndef __GECODE_SET_SEQUENCE_HH__
#define __GECODE_SET_SEQUENCE_HH__

#include "set.hh"

namespace Gecode { namespace Set { namespace Sequence {

  /**
   * \namespace Gecode::Set::Sequence
   * \brief Propagators for ordered sequences of sets
   */

  /**
   * \brief %Propagator for the sequence constraint
   *
   * Requires \code #include "set/sequence.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class Seq :
    public NaryPropagator<SetView, PC_SET_CGLB> {
  protected:
    /// Constructor for cloning \a p
    Seq(Space* home, bool share,Seq& p);
    /// Constructor for posting
    Seq(Space* home,ViewArray<SetView>&);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
    static ExecStatus post(Space* home,ViewArray<SetView>);
  };

  /**
   * \brief %Propagator for the sequenced union constraint
   *
   * Requires \code #include "set/sequence.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class SeqU : public NaryOnePropagator<SetView,PC_SET_ANY> {
  protected:
    GLBndSet unionOfDets; //Union of determined variables dropped form x.
    /// Constructor for cloning \a p
    SeqU(Space* home, bool share,SeqU& p);
    /// Constructor for posting
    SeqU(Space* home,ViewArray<SetView>&, SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*     copy(Space* home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
    static ExecStatus post(Space* home,ViewArray<SetView>, SetView);
  };


}}}

#include "set/rel-op/common.icc"
#include "set/sequence/common.icc"
#include "set/sequence/seq.icc"
#include "set/sequence/seq-u.icc"

#endif

// STATISTICS: set-prop
