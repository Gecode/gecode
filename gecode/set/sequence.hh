/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __GECODE_SET_SEQUENCE_HH__
#define __GECODE_SET_SEQUENCE_HH__

#include "gecode/set.hh"

namespace Gecode { namespace Set { namespace Sequence {

  /**
   * \namespace Gecode::Set::Sequence
   * \brief Propagators for ordered sequences of sets
   */

  /**
   * \brief %Propagator for the sequence constraint
   *
   * Requires \code #include "gecode/set/sequence.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class Seq :
    public NaryPropagator<SetView, PC_SET_ANY> {
  protected:
    /// Constructor for cloning \a p
    Seq(Space& home, bool share,Seq& p);
    /// Constructor for posting
    Seq(Space& home,ViewArray<SetView>&);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space& home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space& home, ModEventDelta med);
    /// Post propagator \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
    static ExecStatus post(Space& home,ViewArray<SetView>);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space& home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Propagator for the sequenced union constraint
   *
   * Requires \code #include "gecode/set/sequence.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class SeqU : public NaryOnePropagator<SetView,PC_SET_ANY> {
  protected:
    GLBndSet unionOfDets; //Union of determined variables dropped form x.
    /// Constructor for cloning \a p
    SeqU(Space& home, bool share,SeqU& p);
    /// Constructor for posting
    SeqU(Space& home,ViewArray<SetView>&, SetView);
    ExecStatus propagateSeqUnion(Space& home,
                                 bool& modified, ViewArray<SetView>& x,
                                 SetView& y);
    
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*     copy(Space& home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space& home, ModEventDelta med);
    /// Specification for this propagator
    GECODE_SET_EXPORT
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space& home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
    /// Post propagator \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
    static ExecStatus post(Space& home,ViewArray<SetView>, SetView);
  };


}}}

#include "gecode/set/rel.hh"
#include "gecode/set/rel-op/common.icc"
#include "gecode/set/sequence/common.icc"
#include "gecode/set/sequence/seq.icc"
#include "gecode/set/sequence/seq-u.icc"

#endif

// STATISTICS: set-prop
