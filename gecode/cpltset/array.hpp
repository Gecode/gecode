/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

namespace Gecode {
  
  /// Traits of CpltSetVarArray
  template <>
  class ArrayTraits<CpltSetVarArray> {
  public:
    typedef CpltSetVarArgs   storage_type;
    typedef CpltSetVar       value_type;
    typedef CpltSetVarArgs   args_type;
  };
  /// Traits of CpltSetVarArgs
  template <>
  class ArrayTraits<CpltSetVarArgs> {
  public:
    typedef CpltSetVarArgs storage_type;
    typedef CpltSetVar     value_type;
    typedef CpltSetVarArgs args_type;
  };

  /*
   * Implementation
   *
   */

  forceinline
  CpltSetVarArray::CpltSetVarArray(void) {}

  forceinline
  CpltSetVarArray::CpltSetVarArray(const CpltSetVarArray& a)
    : VarArray<CpltSetVar>(a) {}

  namespace CpltSet {
    /**
     * \brief Ordering all declared bdd variables \f$ x_0, \dots, x_{n-1}\f$ 
     * such that 
     * \f$ \forall i\in \{0, \dots, n - 1\}: x_{0_{1}}
     * \prec x_{{n-1}_{1}}, \dots, x_{0_{k-1}} \prec x_{{n-1}_{k-1}}\f$
     */
    GECODE_CPLTSET_EXPORT void
    setVariableOrderFromArray(Space& home, const CpltSetVarArray& x);
  }
}

// STATISTICS: cpltset-other
