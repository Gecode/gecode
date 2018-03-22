/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Vincent Barichard, 2012
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

#ifndef __GECODE_FLOAT_CHANNEL_HH__
#define __GECODE_FLOAT_CHANNEL_HH__

#include <gecode/int.hh>
#include <gecode/float.hh>

/**
 * \namespace Gecode::Float::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Float { namespace Channel {

  /**
   * \brief %Propagator for bounds consistent integer part operator
   *
   * Requires \code #include <gecode/float/arithmetic.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class A, class B>
  class Channel :
    public MixBinaryPropagator<A,PC_FLOAT_BND,B,Gecode::Int::PC_INT_BND> {
  protected:
    using MixBinaryPropagator<A,PC_FLOAT_BND,B,Gecode::Int::PC_INT_BND>::x0;
    using MixBinaryPropagator<A,PC_FLOAT_BND,B,Gecode::Int::PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    Channel(Space& home, Channel& p);
    /// Constructor for creation
    Channel(Home home, A x0, B x1);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$ int(x_0) = x_1\f$
    static ExecStatus post(Home home, A x0, B x1);
  };

}}}

#include <gecode/float/channel/channel.hpp>

#endif

// STATISTICS: float-prop
