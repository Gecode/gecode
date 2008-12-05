/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

  namespace Int { namespace Linear {

    /**
     * \brief No view serves as filler for empty view arrays
     *
     */
    class NoView : public ConstViewBase {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      NoView(void) {}
      /// Initialize
      void init(void) {}
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const { return 0; }
      /// Return maximum of domain
      int max(void) const { return 0; }
      /// Return median of domain
      int med(void) const { return 0; }
      /// Return assigned value (only if assigned)
      int val(void) const { return 0; }

      /// Return size (cardinality) of domain
      unsigned int size(void) const { return 1; }
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const { return 1; }
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const { return 0; }
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const { return 0; }
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const { return true; }
      /// Test whether view is assigned
      bool assigned(void) const { return true; }

      /// Test whether \a n is contained in domain
      bool in(int n) const { n=n; return false; }
      /// Test whether \a n is contained in domain
      bool in(double n) const { n=n; return false; }
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n) {
        (void) home; (void) n;
        return ME_INT_NONE;
      }
      //@}

      /// \name View-dependent propagator support
      //@{
      /// Schedule propagator \a p with modification event \a me
      static void schedule(Space& home, Propagator& p, ModEvent me) {
        (void) home; (void) p; (void) me;
      }
      /// Return modification event of propagator \a p for view
      static ModEvent med(const Propagator& p) {
        (void) p;
        return ME_INT_NONE;
      }
      /// Translate modification event \a me to modification event delta for view
      static ModEventDelta med(ModEvent me) {
        (void) me;
        return ME_INT_NONE;
      }
      //@}

      /// \name Dependencies
      //@{
      /// Subscribe propagator \a p with propagation condition \a pc to view
      void subscribe(Space& home, Propagator& p, PropCond pc,
                     bool process=true) {
        (void) home; (void) p; (void)  pc; (void) process;
      }
      /// Cancel subscription of propagator \a p with propagation condition \a pc to view
      void cancel(Space& home, Propagator& p, PropCond pc) {
        (void) home; (void) p; (void)  pc;
      }
      /// Subscribe advisor to view
      void subscribe(Space&, Advisor&) {};
      /// Cancel subscription of advisor
      void cancel(Space&, Advisor&) {};
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, NoView& x) {
        (void) home; (void) share; (void) x;
      }
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const {
        (void) home; (void) m;
        return NULL;
      }
      /// Type of this view
      static Support::Symbol type(void) {
        return Support::Symbol("Gecode::Int::Linear::NoView");
      }
      //@}

      /// \name View comparison
      //@{
      /// Test whether this view is the same as \a x
      bool operator ==(const NoView& x) const { (void) x; return true; }
      /// Test whether this view is not the same as \a x
      bool operator !=(const NoView& x) const { (void) x; return false; }
      /// Test whether this view is smaller than \a x (arbitrary order)
      bool operator < (const NoView& x) const { (void) x; return false; }
      /// Test whether this view is larger than \a x (arbitrary order)
      bool operator > (const NoView& x) const { (void) x; return false; }
      //@}
    };

  }}


  /**
   * \brief View array for no view (empty)
   *
   */
  template <>
  class ViewArray<Int::Linear::NoView> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (array of size 0)
    ViewArray(void) {}
    /// Allocate array with \a m variables
    ViewArray(Space& home, int m) { (void) home; (void) m; }
    /// Initialize
    ViewArray(const ViewArray<Int::Linear::NoView>&) {}
    /// Initialize
    ViewArray(Space&, const ViewArray<Int::Linear::NoView>&) {}
    /// Initialize
    ViewArray(Space&, const Reflection::VarMap&, Reflection::Arg*) {}
    /// Initialize
    const ViewArray<Int::Linear::NoView>& operator =(const ViewArray<Int::Linear::NoView>&) { return *this; }
    //@}

    /// \name Array size
    //@{
    /// Return size of array (number of elements)
    int size(void) const { return 0; }
    /// Decrease size of array (number of elements)
    void size(int n) { (void) n; }
    //@}

    /// \name Array elements
    //@{
    /// Return view at position \a i
    Int::Linear::NoView operator [](int i) {
      (void)  i;
      Int::Linear::NoView n;
      return n;
    }
    //@}

    /// \name Dependencies
    //@{
    /// Subscribe propagator \a p with propagation condition \a pc to all views
    void subscribe(Space&, Propagator& p, PropCond pc, bool process=true) {
      (void) p; (void) pc; (void) process;
    }
    /// Cancel subscription of propagator \a p with propagation condition \a pc to all views
    void cancel(Space& home, Propagator& p, PropCond pc) {
      (void) home; (void) p; (void) pc;
    }
    //@}

    /// \name Cloning
    //@{
    /**
     * \brief Update array to be a clone of array \a a
     *
     * If \a share is true, sharing is retained for all shared
     * data structures. Otherwise, for each of them an independent
     * copy is created.
     */
    void update(Space&, bool share, ViewArray<Int::Linear::NoView>& a) {
      (void) share; (void) a;
    }
    //@}

    /// \name Reflection
    //@{
    /// Return specification for this array, using variable map \a m
    Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const {
      (void) home; (void) m;
      return Reflection::Arg::newArray(0);
    }
    //@}


    /// \name Moving elements
    //@{
    /// Move assigned view from position 0 to position \a i (shift elements to the left)
    void move_fst(int i) { (void) i; }
    /// Move assigned view from position \c size()-1 to position \a i (truncate array by one)
    void move_lst(int i) { (void) i; }
    /** \brief Move view from position 0 to position \a i (shift elements to the left)
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to view at position \a i.
     */
    void move_fst(int i, Propagator& p, PropCond pc) {
      (void) i; (void) p; (void) pc;
    }
    /** \brief Move view from position \c size()-1 to position \a i (truncate array by one)
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to view at position \a i.
     */
    void move_lst(int i, Propagator& p, PropCond pc) {
      (void) i; (void) p; (void) pc;
    }
    //@}

    /// \name Dropping elements
    //@{
    /// Drop assigned views from positions 0 to \a i-1 from array
    void drop_fst(int i);
    /// Drop assigned views from positions \a i+1 to \c size()-1 from array
    void drop_lst(int i);
    /** \brief Drop views from positions 0 to \a i-1 from array
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to views at positions 0 to \a i-1.
     */
    void drop_fst(int i, Propagator& p, PropCond pc) {
      (void) i; (void) p; (void) pc;
    }
    /** \brief Drop assigned views from positions \a i+1 to \c size()-1 from array
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to views at positions \a i+1 to
     * \c size()-1.
     */
    void drop_lst(int i, Propagator& p, PropCond pc) {
      (void) i; (void) p; (void) pc;
    }
    //@}

    /// \name View equality
    //@{
    /// Test whether array has equal views
    bool equal(void) const { return false; }
    /// Test whether array contains view equal to \a y
    bool equal(const Int::Linear::NoView& y) const {
      (void) y;
      return false;
    }
    /// Remove all duplicate views from array (changes element order)
    void unique(void) {}
    //@}

    /// \name View sharing
    //@{
    /// Test whether array has shared views
    bool shared(void) const { return false; }
    /// Test whether array contains view shared with \a y
    bool shared(const Int::Linear::NoView& y) const {
      (void) y;
      return false;
    }
    //@}

  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };

}


// STATISTICS: int-prop

