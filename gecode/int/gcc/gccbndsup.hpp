/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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

namespace Gecode { namespace Int { namespace GCC {

  /// \name GCC-Bnd-Support
  //@{

  /**
   * \brief Class for computing unreachable values in the
   *  value GCC propagator
   *
   */
  class UnReachable {
  public:
    unsigned int minb;
    unsigned int maxb;
    unsigned int eq;
    unsigned int le;
    unsigned int gr;
  };

  /**
   * \brief Bounds consistency check for cardinality variables.
   */

  template<class Card, bool shared>
  inline ExecStatus
  prop_card(Space& home, ViewArray<IntView>& x, ViewArray<Card>& k, bool& mod) {
    int n = x.size();
    int m = k.size();
    Region r(home);
    UnReachable* rv = r.alloc<UnReachable>(m);
    for(int i = m; i--; )
      rv[i].minb = rv[i].maxb = rv[i].le = rv[i].gr = rv[i].eq = 0;

    for (int i = n; i--; ) {
      int min_idx = 0;
      int max_idx = 0;
      min_idx = lookupValue(k,x[i].min());
      if (min_idx == -1) {
        return ES_FAILED;
      }
      if (x[i].assigned()) {
        rv[min_idx].minb++;
        rv[min_idx].maxb++;
        rv[min_idx].eq++;
      } else {
        max_idx = lookupValue(k,x[i].max());
        if (max_idx == -1) {
          return ES_FAILED;
        }
        // count the number of variables
        // with lower bound k[min_idx].card()
        rv[min_idx].minb++;

        // count the number of variables
        // with upper bound k[max_idx].card()
        rv[max_idx].maxb++;
      }
    }

    rv[0].le = 0;
    int c_min = 0;
    for (int i = 1; i < m; i++) {
      rv[i].le = c_min + rv[i - 1].maxb;
      c_min += rv[i - 1].maxb;
    }

    rv[m-1].gr = 0;
    int c_max = 0;
    for (int i = m-1; i--; ) {
      rv[i].gr = c_max + rv[i + 1].minb;
      c_max += rv[i + 1].minb;
    }

    for (int i = m; i--; ) {
      int reachable = x.size() - rv[i].le - rv[i].gr;
      if (!k[i].assigned()) {
        ModEvent me = k[i].lq(home, reachable);
        if (me_failed(me))
          return ES_FAILED;
        mod |= (me_modified(me) && (k[i].max() != reachable));
        mod |= shared && me_modified(me);

        if (rv[i].eq > 0) {
          ModEvent me = k[i].gq(home, static_cast<int>(rv[i].eq));
          if (me_failed(me))
            return ES_FAILED;
          mod |= (me_modified(me) &&
                  (k[i].min() != static_cast<int>(rv[i].eq)));
          mod |= shared && me_modified(me);
        }
      } else {
        // check validity of the cardinality value
        int v = k[i].max();
        if ( !( static_cast<int>(rv[i].eq) <= v && v <= reachable) )
          return ES_FAILED;
      }
    }

    return ES_OK;
  }


  /** \brief Consistency check, whether the cardinality values are feasible.
   */
  template<class Card>
  inline bool
  card_consistent(ViewArray<IntView>& x, ViewArray<Card>& k) {
    int smin = 0;
    int smax = 0;
    for (int i = k.size(); i--; ) {
      smax += k[i].max();
      smin += k[i].min();
    }
    // Consistent if number of variables within cardinality bounds
    return (smin <= x.size()) && (x.size() <= smax);
  }

  /**
   * \brief Maps domain bounds to their position in hall[].bounds.
   */
  class Rank {
  public:
    /**
     * \brief \f$ rank[i].min = z
     * \Leftrightarrow min(x_i) = hall[z].bounds \f$
     */
    int min;
    /**
     * \brief \f$ rank[i].max = z
     * \Leftrightarrow max(x_i) = hall[z].bounds \f$
     */
    int max;
  };

  /**
   * \brief Compares two indices \a i, \a j of two views
   * \a \f$ x_i \f$ \f$ x_j\f$ according to the
   * ascending order of the views upper bounds
   *
   */

  template<class View>
  class MaxInc {
  protected:
    ViewArray<View> x;
  public:
    MaxInc(const ViewArray<View>& x0) : x(x0) {}
    forceinline bool
    operator ()(const int i, const int j) {
      return x[i].max() < x[j].max();
    }
  };

  /**
   * \brief Compares two indices \a i, \a j of two views
   * \a \f$ x_i \f$ \f$ x_j\f$ according to the
   * ascending order of the views lower bounds
   *
   */

  template<class View>
  class MinInc {
  protected:
    ViewArray<View> x;
  public:
    MinInc(const ViewArray<View>& x0) : x(x0) {}
    forceinline bool
    operator ()(const int i, const int j) {
      return x[i].min() < x[j].min();
    }
  };

  /**
   * \brief Compares two cardinality views
   * \a \f$ x_i \f$ \f$ x_j\f$ according to the index
   *
   */

  template<class Card>
  class MinIdx {
  public:
    forceinline bool
    operator ()(const Card& x, const Card& y) {
      return x.card() < y.card();
    }
  };

  /**
   * \brief Partial sum structure for constant
   *  time computation of the maximal capacity of an interval.
   *
   */
  template<class Card>
  class PartialSum {
  private:
    /// sum[i] contains the partial sum from 0 to i
    int* sum;
    /// the size of the sum
    int size;
  public:
    /// \name Constructor, initializer, and destructor
    //@{
    PartialSum(void);
    void init(ViewArray<Card>&, bool);
    void dispose(void);
    ~PartialSum(void);
    //@}
    /// \name Access
    //@{
    bool initialized(void) const;
    int firstValue;
    int lastValue;
    int sumup(int, int);
    int minValue(void);
    int maxValue(void);
    int skipNonNullElementsRight(int);
    int skipNonNullElementsLeft(int);
    void* operator new(size_t s);
    void operator delete(void* p);
    bool check_update_max(ViewArray<Card>& k);
    bool check_update_min(ViewArray<Card>& k);
    int getsize(void) const;
    size_t allocated(void) const;
    //@}
    void print(void);
  };

  /// \brief Deallocate memory
  template<class Card>
  forceinline void
  PartialSum<Card>::dispose(void){
    if (sum != NULL)
      heap.free<int>(sum,size);
    sum = NULL;
    size = 0;
  }

  /// \brief Destructor
  template<class Card>
  forceinline
  PartialSum<Card>::~PartialSum(void){
    dispose();
  }

  /// \brief Constructor
  template<class Card>
  forceinline
  PartialSum<Card>::PartialSum(void) : sum(NULL), size(0) {}

  template<class Card>
  forceinline bool
  PartialSum<Card>::initialized(void) const {
    return sum != NULL;
  }

  /**
   * \brief Default initialization
   *
   * \param first is the miminum value the variables can take
   * \param count is equal to the size \f$ |k| \f$,
   *        where \a k denotes the array of cardinalities used
   *        in the global cardinality propagator
   * \param elements contains the upper and lower cardinalities for every value
   * \param up denotes the direction whether we sumup the lower or upper cardinality bounds
   *        If \a up is true we sumup the upper bounds, otherwise the lower bounds.
   *
   */
  template<class Card>
  inline void
  PartialSum<Card>::init(ViewArray<Card>& elements, bool up) {
    int i = 0;
    int j = 0;

    // Determine number of holes in the index set
    int holes = 0;
    for (i = 1; i < elements.size(); i++) {
      if (elements[i].card() != elements[i-1].card() + 1)
        holes += elements[i].card()-elements[i-1].card()-1;
    }

    // we add three elements at the beginning and two at the end
    size  = elements.size() + holes + 5;

    // memory allocation
    sum = heap.alloc<int>(2*size);
    int* ds  = &sum[size];

    int first = elements[0].card();

    /*
     * firstValue and lastValue are sentinels
     * indicating whether an end of the sum has been reached
     *
     */
    firstValue = first - 3;
    lastValue  = first + elements.size() + holes + 1;

    // the first three elements
    for (i = 3; i--; )
      sum[i] = i;

    /*
     * copy the bounds into sum, filling up holes with zeroes
     */
    int prevCard = elements[0].card()-1;
    i = 0;
    for (j = 2; j < elements.size() + holes + 2; j++){
      if (elements[i].card() != prevCard + 1) {
        sum[j + 1] = sum[j];
      } else if (up) {
        sum[j + 1] = sum[j] + elements[i].max();
        i++;
      } else {
        sum[j + 1] = sum[j] + elements[i].min();
        i++;
      }
      prevCard++;
    }
    sum[j + 1] = sum[j] + 1;
    sum[j + 2] = sum[j + 1] + 1;

    // Compute distances, eliminating zeroes
    i = elements.size() + holes + 3;
    j = i + 1;
    for ( ; i > 0; ){
      while(sum[i] == sum[i - 1]) {
        ds[i] = j;
        i--;
      }
      ds[j] = i;
      i--;
      j = ds[j];
    }
    ds[j] = 0;
    ds[0] = 0;
  }

  template<class Card>
  void
  PartialSum<Card>::print(void) {
    for (int i=0; i < size; i++) {
      std::cerr << "(" << firstValue+i << "-" << sum[i] << ") ";
    }
    std::cerr << std::endl;
  }

  /**
   * \brief Compute the maximum capacity of an interval I
   */
  template<class Card>
  forceinline int
  PartialSum<Card>::sumup(int from, int to){
    if (from <= to) {
      return sum[to - firstValue] - sum[from - firstValue - 1];
    } else {
      assert(to - firstValue - 1 >= 0);
      assert(to - firstValue - 1 < size);
      assert(from - firstValue >= 0);
      assert(from - firstValue < size);
      return sum[to - firstValue - 1] - sum[from - firstValue];
    }
  }

  /**
   * \brief Returns the smallest bound of the variables in \a x
   *
   */
  template<class Card>
  forceinline int
  PartialSum<Card>::minValue(void){
    return firstValue + 3;
  }

  /**
   * \brief Returns the largest bound of the variables in \a x
   *
   */

  template<class Card>
  forceinline int
  PartialSum<Card>::maxValue(void){
    return lastValue - 2;
  }


  /**
   * \brief Skip neigboured array entries if their values do
   *        not differ.
   *
   */
  template<class Card>
  forceinline int
  PartialSum<Card>::skipNonNullElementsRight(int value) {
    value -= firstValue;
    int* ds  = &sum[size];
    return (ds[value] < value ? value : ds[value]) + firstValue;
  }

  /**
   * \brief Skip neigboured array entries if their values do
   *        not differ.
   *
   */
  template<class Card>
  forceinline int
  PartialSum<Card>::skipNonNullElementsLeft(int value) {
    value -= firstValue;
    int* ds  = &sum[size];
    return (ds[value] > value ? ds[ds[value]] : value) + firstValue;
  }

  /**
   * \brief Check whether the values in the
   *        partial sum structure containting
   *        the upper cardinality bounds differ
   *        from the actual upper bounds of the
   *        cardinalities.
   */

  template<class Card>
  inline bool
  PartialSum<Card>::check_update_max(ViewArray<Card>& k){
    int j = 0;
    for (int i = 3; i < size - 2; i++) {
      int max = 0;
      if (k[j].card() == i+firstValue) {
        max = k[j++].max();
      }
      if ((sum[i] - sum[i - 1]) != max) {
        return true;
      }
    }
    return false;
  }

  /**
   * \brief Check whether the values in the
   *        partial sum structure containting
   *        the lower cardinality bounds differ
   *        from the actual lower bounds of the
   *        cardinalities.
   */

  template<class Card>
  inline bool
  PartialSum<Card>::check_update_min(ViewArray<Card>& k){
    int j = 0;
    for (int i = 3; i < size - 2; i++) {
      int min = 0;
      if (k[j].card() == i+firstValue) {
        min = k[j++].min();
      }
      if ((sum[i] - sum[i - 1]) != min) {
        return true;
      }
    }
    return false;
  }

  /// \brief Return the size of the partial sum structure.
  template<class Card>
  forceinline int
  PartialSum<Card>::getsize(void) const {
    return size;
  }
  template<class Card>
  forceinline size_t
  PartialSum<Card>::allocated(void) const {
    return sizeof(PartialSum<Card>) + 2*size*sizeof(int);
  }


  /**
   * \brief Container class provding information about the Hall
   *  structure of the problem variables.
   *
   *  This class is used to
   *  keep the number of different arrays small, that is
   *  an array of type %HallInfo replaces integer arrays for each
   *  of the class members.
   *
   */
  class HallInfo {
  public:
    /// Represents the union of all lower and upper domain bounds
    int bounds;
    /**
     * \brief critical capacity pointer
     * t represents a predecessor function where \f$ t_i \f$ denotes the
     * predecessor of i in bounds
     */
    int t;
    /**
     * \brief difference between critical capacities
     *
     * d_i is the difference between the capacities of hall[i].bounds
     * and its predecessor in bounds hall[t[i]].bounds
     *
     */
    int d;
    /**
     * \brief Hall set pointer
     *
     * If hall[i].h < i then the half-open interval
     * [hall[h[i]].bounds,hall[i].bounds) is containd in a Hall
     * set.
     * Otherwise holds a pointer to the Hall intervall it belongs to.
     */
    int h;
    /**
     * \brief Stable Set pointer
     *
     */
    int s;
    /**
     * \brief Potentially Stable Set pointer
     *
     */
    int ps;
    /**
     * \brief Bound update
     *
     * \a newBound contains either a narrowed domain bound
     * or is stores the old domain bound of a variable.
     */
    int newBound;
  };


  /**
   * \name Path compression
   *
   * Each of the nodes on the path from \a start to \a end
   * becomes a direct child of \a to.
   *
   */

  //@{
  /// \brief Path compression for potentially stable set structure
  inline void
  pathset_ps(HallInfo hall[], int start, int end, int to) {
    int k, l;
    for (l=start; (k=l) != end; hall[k].ps=to) {
      l = hall[k].ps;
    }
  }

  /// \brief Path compression for stable set structure
  inline void
  pathset_s(HallInfo hall[], int start, int end, int to) {
    int k, l;
    for (l=start; (k=l) != end; hall[k].s=to) {
      l = hall[k].s;
    }
  }

  /// \brief Path compression for capacity pointer structure
  inline void
  pathset_t(HallInfo hall[], int start, int end, int to) {
    int k, l;
    for (l=start; (k=l) != end; hall[k].t=to) {
      l = hall[k].t;
    }
  }

  /// \brief Path compression for hall pointer structure
  inline void
  pathset_h(HallInfo hall[], int start, int end, int to) {
    int k, l;
    for (l=start; (k=l) != end; hall[k].h=to) {
      l = hall[k].h;
      assert(l != k);
    }
  }
  //@}

  /**
   *  \name Path minimum
   *
   *  returns the smalles reachable index starting from i
   *
   */
  //@{
  /// \brief Path minimum for hall pointer structure
  forceinline int
  pathmin_h(const HallInfo hall[], int i) {
    while (hall[i].h < i)
      i = hall[i].h;
    return i;
  }
  /// \brief Path minimum for capacity pointer structure
  forceinline int
  pathmin_t(const HallInfo hall[], int i) {
    while (hall[i].t < i)
      i = hall[i].t;
    return i;
  }
  //@}

  /**
   *  \name Path maximum
   *
   *  returns the greatest reachable index starting from i
   */
  //@{
  /// \brief Path maximum for hall pointer structure
  forceinline int
  pathmax_h(const HallInfo hall[], int i) {
    while (hall[i].h > i)
      i = hall[i].h;
    return i;
  }


  /// \brief Path maximum for capacity pointer structure
  forceinline int
  pathmax_t(const HallInfo hall[], int i) {
    while (hall[i].t > i) {
      i = hall[i].t;
    }
    return i;
  }

  /// \brief Path maximum for stable set pointer structure
  forceinline int
  pathmax_s(const HallInfo hall[], int i) {
    while (hall[i].s > i)
      i = hall[i].s;
    return i;
  }

  /// \brief Path maximum for potentially stable set pointer structure
  forceinline int
  pathmax_ps(const HallInfo hall[], int i) {
    while (hall[i].ps > i)
      i = hall[i].ps;
    return i;
  }
  //@}
  //@}

  /** \brief Assert consistency in the cardinality specification
   *         for bounds propagation.
   *
   *         This is done by ensuring that only those values are specified with
   *         cardinalities, which are not smaller
   *         than the smallest value of all variable domains and not greater
   *         than the greatest value of all variable domains.
   */

  template<class Card>
  void
  reduce_card(Space& home, int cmin, int cmax, ViewArray<Card>& k) {
    if (cmin == cmax) {
      int idx = 0;
      while (k[idx].card() != cmax) {
        idx++;
      }
      k[0] = k[idx];
      k.size(1);
    } else {
      Region r(home);
      bool* zero = r.alloc<bool>(k.size());
      int ks = k.size();
      int zc = 0;
      for (int i = 0; i < ks; i++) {
        bool impossible  = ( (k[i].counter() == 0) &&
                             (k[i].card() < cmin ||
                              k[i].card() > cmax));

        if (impossible) {
          zero[i] = true;
          zc++;
        } else {
          zero[i] = false;
        }
      }


      if (zero[ks - 1]) {
        int m = ks;
        while(zero[m - 1]) {
          m--;
          zc--;
        }
        k.size(m);
      }

      if (zc > 0) {
        int ks = k.size();
        // if there are still zero entries left
        for (int i = 0; i < ks; i++) {
          assert(0 <= i && i < ks);
          if  (zero[i]) {
            if (i == ks - 1) {
              break;
            }
            // this cardinality does not occur
            // remove it
            // we need the next non-null entry
            int j = i + 1;
            assert(0 <= j && j < ks);
            if (j < ks) {
              while (zero[j]) {
                j++;
              }
            }
            if (j > ks - 1) {
              break;
            }
            k[i] = k[j];
            zero[j] = true;
          }
        }
        k.size(ks - zc);
      }

    }

  }

}}}

// STATISTICS: int-prop

