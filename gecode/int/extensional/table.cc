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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int.hh"
#include "gecode/support/sort.hh"
#include "gecode/support/search.hh"

#include "gecode/int/extensional/table.icc"

namespace {
  typedef ::Gecode::Table::tuple tuple;

  /**
   * \brief Full tuple compare
   */
  class FullTupleCompare {
    int arity;
  public:
    FullTupleCompare(int a) : arity(a) {}
    bool
    operator()(const tuple& a, const tuple& b) {
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
    int pos;
  public:
    TuplePosCompare(int p) : pos(p) {}

    bool
    operator()(const tuple& a, const tuple& b) {
      if (a[pos] == b[pos]) return a < b;
      return a[pos] < b[pos];
    }
  };

  /**
   * \brief Tuple compared on position versus a key.
   *
   */
  class TupleKeyCompare {
    int pos;
  public:
    TupleKeyCompare(int p) : pos(p) {}

    bool
    operator()(const tuple& a, const tuple& b) {
      return a[pos] < b[pos];
    }


    forceinline bool
    operator()(const int& a, const tuple& b) {
      return a < b[pos];
    }

    forceinline bool
    operator()(const tuple& a, const int& b) {
      return a[pos] < b;
    }
  };
}


std::ostream&
operator<<(std::ostream& os, const Gecode::Table& table) {
  os << "Number of tuples: " << table.tablei->size << std::endl
     << "Tuples:" << std::endl;
  for (int i = 0; i < table.tablei->size; ++i) {
    os << '\t';
    for (int j = 0; i < table.tablei->arity; ++j) {
      os.width(3);
      os << " " << table.tablei->tuples[0][i][j];
    }
    os << std::endl;
  }
  return os;
}

namespace Gecode {

  void
  Table::TableI::finalize(void) {
    assert(!finalized());
    assert(tuples == NULL);

    // Add final largest tuple
    IntArgs ia(arity);
    for (int i = arity; i--; ) ia[i] = Limits::Int::int_max+1;
    int smin = min, smax = max;
    add(ia);
    min = smin; max = smax;

    // Domainsize
    domsize = max - min + 1;

    // Allocate tuple indexing data-structures
    tuples = Memory::bmalloc<tuple*>(arity);
    tuple* tuple_data = Memory::bmalloc<tuple>(size*arity+1);
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
    int* new_data = Memory::bmalloc<int>(size*arity);
    for (int t = size; t--; ) {
      for (int i = arity; i--; ) {
        new_data[t*arity + i] = tuples[0][t][i];
      }
    }

    //Memory::free(data);
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
    last = Memory::bmalloc<tuple*>(domsize*arity);
    for (int i = arity; i--; ) {
      TupleKeyCompare tkc(i);
      for (int d = domsize; d--; ) {
        tuple* l = Support::binarysearch(tuples[i], size, min+d, tkc);
        if (l && *l && (*l)[i] == min+d)
          last[(i*domsize) + d] = l;
        else
          last[(i*domsize) + d] = nullptr;
      }
    }
    
    assert(finalized());
  }

  Table::TableI*
  Table::TableI::copy(void) {
    assert(finalized());
    TableI* d     = new TableI();
    d->use_cnt    = use_cnt;
    d->arity      = arity;
    d->size       = size;
    d->tuples     = tuples;
    d->data       = data;
    d->excess     = -1;
    //d->_dfa       = _dfa;
    //d->dfa_exists = dfa_exists;
    d->min        = min;
    d->max        = max;
    d->domsize    = domsize;
    d->last       = last;
    d->nullptr    = nullptr;
    return d;
  }
  /*
  DFA
  Table::TableI::dfa(void) {
    assert(finalized());
    if (dfa_exists)
      return _dfa;
    dfa_exists = true;

    // Set up regular expression
    REG r;
    for (int t = size; t--; ) {
      REG tuple;
      for (int i = 0; i < arity; ++i)
        tuple = tuple + REG(tuples[0][t][i]);
      r = r | tuple;
    }
    derr << "Regular expression for table is " << r << std::endl;
    _dfa = DFA(r);
    derr << "DFA for table is " << _dfa << std::endl;
    return _dfa;
  }
  */
}

// STATISTICS: int-prop

