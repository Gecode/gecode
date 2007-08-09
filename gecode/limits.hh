/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#ifndef __GECODE_LIMITS_HH__
#define __GECODE_LIMITS_HH__

#include <climits>
#include <cfloat>

/*
 * Numerical limits
 *
 */

namespace Gecode { namespace Limits {

  /**
   * \namespace Gecode::Limits
   *  \brief Numerical limits
   *
   * Contains constants describing numerical limits
   * for numbers contained in integer and set variables.
   *
   */

  namespace Int {
    /**
     * \namespace Gecode::Limits::Int
     *  \brief Numerical limits for integer variables
     *
     * The integer limits are chosen such that addition and subtraction
     * of two values within the limits can be done safely without
     * numerical overflow. Also, changing the sign is always possible
     * without overflow.
     *
     */

    /// Largest allowed integer value
    const int int_max        = ((INT_MAX)>>1) - 1;
    /// Smallest allowed integer value
    const int int_min        = -int_max;

    /// Largest double that can exactly be represented
    const double double_max  =  9007199254740991.0;
    /// Smallest double that can exactly be represented
    const double double_min  = -9007199254740991.0;

  }

  namespace Set {
    /**
     * \namespace Gecode::Limits::Set
     *  \brief Numerical limits for set variables
     *
     */

    /// Largest allowed integer in integer set
    const int int_max        = ((INT_MAX)>>2) - 1;
    /// Smallest allowed integer in integer set
    const int int_min        = -int_max;
    /// Maximum cardinality of an integer set
    const unsigned int card_max       = int_max-int_min+1;
  }

}}

#endif

// STATISTICS: kernel-other
