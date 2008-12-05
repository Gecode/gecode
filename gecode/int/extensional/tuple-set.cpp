/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
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

#include <gecode/int.hh>

namespace {

  typedef ::Gecode::TupleSet::Tuple Tuple;

  /**
   * \brief Full Tuple compare
   */
  class FullTupleCompare {
    int arity;
  public:
    forceinline
    FullTupleCompare(int a) : arity(a) {}
    forceinline bool
    operator ()(const Tuple& a, const Tuple& b) {
      for (int i = 0; i < arity; ++i) {
        if (a[i] < b[i]) {
          return true;
        }
        if (a[i] > b[i]) {
          return false;
        }
      }
      return true;
    }
  };

  /**
   * \brief Tuple compared lexicographically on (element pos, full
   * order).
   *
   * Assumes that the tuples are sorted in order in memory.
   */
  class TuplePosCompare {
  private:
    int pos;
  public:
    forceinline
    TuplePosCompare(int p) : pos(p) {}

    forceinline bool
    operator ()(const Tuple& a, const Tuple& b) {
      if (a[pos] == b[pos]) return a < b;
      return a[pos] < b[pos];
    }
  };

}

namespace Gecode {

  void
  TupleSet::TupleSetI::finalize(void) {
    assert(!finalized());
    assert(tuples == NULL);

    // Add final largest tuple
    IntArgs ia(arity);
    for (int i = arity; i--; ) ia[i] = Int::Limits::max+1;
    int real_min = min, real_max = max;
    add(ia);
    min = real_min; max = real_max;

    // Domainsize
    domsize = static_cast<unsigned int>(max - min) + 1;

    // Allocate tuple indexing data-structures
    tuples = heap.alloc<Tuple*>(arity);
    tuple_data = heap.alloc<Tuple>(size*arity+1);
    tuple_data[size*arity] = NULL;
    nullptr = tuple_data+(size*arity);

    // Rearrange the tuples for faster comparisons.
    for (int i = arity; i--; )
      tuples[i] = tuple_data + (i * size);
    for (int i = size; i--; )
      tuples[0][i] = data + (i * arity);

    FullTupleCompare ftc(arity);
    Support::quicksort(tuples[0], size, ftc);
    assert(tuples[0][size-1][0] == ia[0]);
    int* new_data = heap.alloc<int>(size*arity);
    for (int t = size; t--; ) {
      for (int i = arity; i--; ) {
        new_data[t*arity + i] = tuples[0][t][i];
      }
    }

    heap.rfree(data);
    data = new_data;
    excess = -1;

    // Set up indexing structure
    for (int i = arity; i--; )
      for (int t = size; t--; )
        tuples[i][t] = data + (t * arity);
    for (int i = arity; i-->1; ) {      
      TuplePosCompare tpc(i);
      Support::quicksort(tuples[i], size, tpc);
    }

    // Set up initial last-structure
    last = heap.alloc<Tuple*>(domsize*arity);
    for (int i = arity; i--; ) {
      Tuple* t = tuples[i];
      for (unsigned int d = 0; d < domsize; ++d) {
        while (t && *t && (*t)[i] < static_cast<int>(min+d)) {
          ++t;
        }
        if (t && *t && (*t)[i] == static_cast<int>(min+d)) {
          last[(i*domsize) + d] = t;
          ++t;
        } else {
          last[(i*domsize) + d] = nullptr;
        }
      }
    }
    
    assert(finalized());
  }

  void
  TupleSet::TupleSetI::resize(void) {
    assert(excess == 0);
    int ndatasize = static_cast<int>(1+size*1.5);
    data = heap.realloc<int>(data, size * arity, ndatasize * arity);
    excess = ndatasize - size;
  }

  SharedHandle::Object*
  TupleSet::TupleSetI::copy(void) const {
    assert(finalized());
    TupleSetI* d  = new TupleSetI;
    d->arity      = arity;
    d->size       = size;
    d->excess     = excess;
    d->min        = min;
    d->max        = max;
    d->domsize    = domsize;

    // Table data
    d->data = heap.alloc<int>(size*arity);
    heap.copy(&d->data[0], &data[0], size*arity);

    // Indexing data
    d->tuples = heap.alloc<Tuple*>(arity);
    d->tuple_data = heap.alloc<Tuple>(size*arity+1);
    d->tuple_data[size*arity] = NULL;
    d->nullptr = d->tuple_data+(size*arity);

    // Rearrange the tuples for faster comparisons.
    for (int i = arity; i--; )
      d->tuples[i] = d->tuple_data + (i * size);
    for (int a = arity; a--; ) {
      for (int i = size; i--; ) {
        d->tuples[a][i] = d->data + (tuples[a][i]-data);
      }
    }

    // Last data
    d->last = heap.alloc<Tuple*>(domsize*arity);
    for (int i = domsize+arity; i--; ) {
      d->last[i] = d->tuple_data + (last[i]-tuple_data);
    }

    return d;
  }

  TupleSet::TupleSetI::~TupleSetI(void) {
    excess = -2;
    heap.rfree(tuples);
    heap.rfree(tuple_data);
    heap.rfree(data);
    heap.rfree(last);
  }  


  TupleSet::TupleSet(Reflection::VarMap& vm, Reflection::Arg* arg) {
    if (arg->isSharedReference()) {
      TupleSetI* d = 
        static_cast<TupleSetI*>(vm.getSharedObject(arg->toSharedReference()));
      object(d);
      return;
    }
    
    Reflection::IntArrayArg* a = arg->toSharedObject()->toIntArray();

    // All done... Construct the table

    int arity = (*a)[0];
    int n_tuples = (a->size() - 1) / arity;
    assert(n_tuples*arity == a->size()-1);
    int pos = 1;
    IntArgs ia(arity);
    for (int i = n_tuples; i--; ) {
      for (int j = 0; j < arity; ++j) {
        ia[j] = (*a)[pos++];
      }
      add(ia);
    }
    finalize();

    vm.putMasterObject(object());
  }

  Reflection::Arg*
  TupleSet::spec(Reflection::VarMap& vm) const {
    int sharedIndex = vm.getSharedIndex(object());
    if (sharedIndex >= 0)
      return Reflection::Arg::newSharedReference(sharedIndex);
    Reflection::IntArrayArg* a = 
      Reflection::Arg::newIntArray(static_cast<int>(1+arity()*tuples()));

    (*a)[0] = arity();

    int pos = 1;
    for (int i = 0; i < tuples(); ++i) {
      for (int j = 0; j < arity(); ++j) {
        (*a)[pos++] = (*this)[i][j];
      }
    }

    vm.putMasterObject(object());
    return Reflection::Arg::newSharedObject(a);    
  }

}

// STATISTICS: int-prop

