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

  template <class View, class Card, bool isView, bool shared>
  ExecStatus
  BndImp<View, Card, isView, shared>::ubc(Space& home, int& nb,
    HallInfo hall[], Rank rank[], int mu[], int nu[]){
    int rightmost = nb + 1; // rightmost accesible value in bounds
    int bsize     = nb + 2; // number of unique bounds including sentinels

    //Narrow lower bounds (UBC)

    /*
     * Initializing tree structure with the values from bounds
     * and the interval capacities of neighboured values
     * from left to right
     */


    hall[0].h = 0;
    hall[0].t = 0;
    hall[0].d = 0;

    for (int i = bsize; --i; ) {
      hall[i].h = hall[i].t = i-1;
      hall[i].d = ups.sumup(hall[i-1].bounds, hall[i].bounds - 1);
    }

    int n          = x.size();

    for (int i = 0; i < n; i++) {

      // visit intervals in increasing max order
      int x0   = rank[mu[i]].min;
      int succ = x0 + 1;
      int y    = rank[mu[i]].max;
      int z    = pathmax_t(hall, succ);
      int j    = hall[z].t;

      /* DOMINATION:
       *     v^i_j denotes
       *     unused values in the current interval. If the difference d
       *     between to critical capacities v^i_j and v^i_z
       *     is equal to zero, j dominates z
       *
       *     i.e. [hall[l].bounds, hall[nb+1].bounds] is not left-maximal and
       *     [hall[j].bounds, hall[l].bounds] is a Hall set iff
       *     [hall[j].bounds, hall[l].bounds] processing a variable x_i uses up a value in the interval
       *     [hall[z].bounds,hall[z+1].bounds] according to the intervals
       *     capacity. Therefore, if d = 0
       *     the considered value has already been used by processed variables
       *     m-times, where m = u[i] for value v_i. Since this value must not
       *     be reconsidered the path can be compressed
       */
      if (--hall[z].d == 0){
        hall[z].t = z + 1;
        z         = pathmax_t(hall, hall[z].t);
        if (z >= bsize) {
          z--;
        }
        assert(z < bsize);
        hall[z].t = j;
      }
      pathset_t(hall, succ, z, z); // path compression
      /* NEGATIVE CAPACITY:
       *     A negative capacity results in a failure.Since a
       *     negative capacity signals that the number of variables
       *     whose domain is contained in the set S is larger than
       *     the maximum capacity of S => UBC is not satisfiable,
       *     i.e. there are more variables than values to instantiate them
       */

      if (hall[z].d < ups.sumup(hall[y].bounds, hall[z].bounds - 1)){
        return ES_FAILED;
      }
      
      /* UPDATING LOWER BOUND:
       *   If the lower bound min_i lies inside a Hall interval [a,b]
       *   i.e. a <= min_i <=b <= max_i
       *   min_i is set to  min_i := b+1
       */
      if (hall[x0].h > x0) {
        int w       = pathmax_h(hall, hall[x0].h);
        int m       = hall[w].bounds;

        GECODE_ME_CHECK(x[mu[i]].gq(home, m));
        pathset_h(hall, x0, w, w); // path compression
      }

      /* ZEROTEST:
       *     (using the difference between capacity pointers)
       *     zero capacity => the difference between critical capacity
       *     pointers is equal to the maximum capacity of the interval,i.e.
       *     the number of variables whose domain is contained in the
       *     interval is equal to the sum over all u[i] for a value v_i that
       *     lies in the Hall-Intervall which can also be thought of as a
       *     Hall-Set
       *
       *    ZeroTestLemma: Let k and l be succesive critical indices.
       *          v^i_k=0  =>  v^i_k = max_i+1-l+d
       *                   <=> v^i_k = y + 1 - z + d
       *                   <=> d = z-1-y
       *    if this equation holds the interval [j,z-1] is a hall intervall
       */

      if (hall[z].d == ups.sumup(hall[y].bounds, hall[z].bounds - 1)) {
        /*
         *mark hall interval [j,z-1]
         * hall pointers form a path to the upper bound of the interval
         */
        int predj = j - 1;
        pathset_h(hall, hall[y].h, predj, y);
        hall[y].h = predj;
      }
    }

    /* Narrow upper bounds (UBC)
     * symmetric to the narrowing of the lower bounds
     */
    for (int i = 0; i < rightmost; i++) {
      hall[i].h = hall[i].t = i+1;
      hall[i].d = ups.sumup(hall[i].bounds, hall[i+1].bounds - 1);
    }
        
    for (int i = n; i--; ) {
      // visit intervals in decreasing min order
      int x0   = rank[nu[i]].max;
      int pred = x0 - 1;
      int y    = rank[nu[i]].min;
      int z    = pathmin_t(hall, pred);
      int j    = hall[z].t;
    
      // DOMINATION:
      if (--hall[z].d == 0){
        hall[z].t = z - 1;
        z         = pathmin_t(hall, hall[z].t);
        hall[z].t = j;
      }
      pathset_t(hall, pred, z, z);
    
      // NEGATIVE CAPACITY:
      if (hall[z].d < ups.sumup(hall[z].bounds,hall[y].bounds-1)){
        return ES_FAILED;
      }
    
      /* UPDATING UPPER BOUND:
       *   If the upper bound max_i lies inside a Hall interval [a,b]
       *   i.e. min_i <= a <= max_i < b
       *   max_i is set to  max_i := a-1
       */
      if (hall[x0].h < x0) {
        int w       = pathmin_h(hall, hall[x0].h);
        int m       = hall[w].bounds - 1;
        GECODE_ME_CHECK(x[nu[i]].lq(home, m));
        pathset_h(hall, x0, w, w);
      }
      // ZEROTEST
      if (hall[z].d == ups.sumup(hall[z].bounds, hall[y].bounds - 1)) {
        //mark hall interval [y,j]
        pathset_h(hall, hall[y].h, j+1, y);
        hall[y].h = j+1;
      }
    }
    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop

