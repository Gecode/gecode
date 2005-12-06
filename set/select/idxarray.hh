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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
    void subscribe(Space* home,Propagator* p, PropCond pc);
    /**
     * Cancel subscription of propagator \a p with propagation condition \a pc
     * for all elements of the array.
     */
    void cancel(Propagator* p, PropCond pc);

    /// Cloning
    void update(Space* home, bool share, IdxViewArray<View>& x);
  };

}}}

#include "set/select/idxarray.icc"

// STATISTICS: set-prop

