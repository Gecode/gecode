/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004,2005
 *     Christian Schulte, 2004,2005
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

namespace Gecode { namespace Set { namespace Select {

  /**
   * \brief Pairs of an index and a variable
   *
   */
  template <class View>
  class IdxView {
  public:
    /// The index
    int idx;
    /// The variable
    View var;
    /// Allocate \a n elements
    static IdxView* allocate(Space* home,int n);
  };

  /**
   * \brief An array of IndexView pairs
   *
   */
  template <class View>
  class IdxViewArray {
  private:
    /// The actual array
    IdxView<View>* xs;
    /// The size of the array
    int n;
  public:
    /// Default constructor
    IdxViewArray(void);
    /// Copy constructor
    IdxViewArray(const IdxViewArray<View>&);
    /// Initialize from specification \a spec with variables \a vars
    IdxViewArray(Space* home, const Reflection::VarMap& vars,
                 Reflection::Arg* spec);

    /// Construct an IdxViewArray from \a x
    IdxViewArray(Space* home, const SetVarArgs& x);

    /// Return the current size
    int size(void) const;
    /// Set the size to \a n
    void size(int n);

    /// Access element \a n
    IdxView<View>& operator[](int n);
    /// Access element \a n
    const IdxView<View>& operator[](int) const;

    /**
     * Subscribe propagator \a p with propagation condition \a pc
     * to all elements of the array.
     */
    void subscribe(Space* home,Propagator* p, PropCond pc, bool process=true);
    /**
     * Cancel subscription of propagator \a p with propagation condition \a pc
     * for all elements of the array.
     */
    void cancel(Space* home, Propagator* p, PropCond pc);

    /// Cloning
    void update(Space* home, bool share, IdxViewArray<View>& x);
    
    // Specification
    Reflection::Arg* spec(Space* home, Reflection::VarMap& m);
  };

}}}

#include "gecode/set/select/idxarray.icc"

// STATISTICS: set-prop

