/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Miakel Lagerkvist, 2007
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

#include "gecode/int/extensional/table.icc"

namespace Gecode { namespace Int { namespace Extensional {
#if 0
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
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
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
#endif
}}}


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
  const bool table_debug = true;
  void
  Table::TableI::finalize(void) {
    assert(!finalized());
    assert(tuples == NULL);

    // Domainsize
    domsize = max - min + 1;

    // Allocate tuple indexing data-structures
    tuples = Memory::bmalloc<tuple*>(arity);
    tuple* tuple_data = Memory::bmalloc<tuple>(size*arity);

    // Rearrange the tuples for faster comparisons.
    for (int i = arity; i--; )
      tuples[i] = tuple_data + (i * size);
    for (int i = size; i--; )
      tuples[0][i] = data + (i * arity);
    //Support::quicksort<tuple, Int::Extensional::FullTupleCompare>
    //  (tuples[0], size, Int::Extensional::FullTupleCompare(arity));
    int* new_data = Memory::bmalloc<int>(size*arity);
    for (int t = size; t--; ) {
      for (int i = arity; i--; ) {
        new_data[t*arity + i] = data[tuples[0][t][arity] + i];
      }
    }
    Memory::free(data);
    data = new_data;
    excess = -1;

    // Set up indexing structure
    for (int i = arity; i--; )
      for (int t = size; t--; )
        tuples[i][t] = data + (t * arity);
    for (int i = arity; i-->1; ) {      
      //Support::quicksort<tuple, Int::Extensional::TuplePosCompare>
      //  (tuples[i], size, Int::Extensional::TuplePosCompare(i));
    }

    // Set up initial last-structure
    last = Memory::bmalloc<tuple**>(domsize*arity);
    for (int i = arity; i--; ) {
      for (int d = domsize; d--; ) {
        last[(i*domsize) + d] = 
          NULL;//Support::binarysearch<tuple, int, Int::Extensional::TupleKeyCompare>
        //(tuples[i], size, min+d, Int::Extensional::TupleKeyCompare(i));
      }
    }
    
    // Debug output
    if (table_debug) {
      std::cout << "Finalization finished: "  << std::endl;
      for (int i = 0; i < arity; ++i) {
        std::cout << "Index " << i << ":  ";
        for (int d = 0; d < domsize; ++d) {
          std::cout << "[" << (min+d) << "]=" << last[i][d] << "  ";
        }
        std::cout << std::endl;
      }
    }
  }

  Table::TableI*
  Table::TableI::copy(void) {
    assert(finalized());
    TableI* d  = new TableI();
    d->use_cnt = 1;
    d->arity   = arity;
    d->size    = size;
    d->tuples  = tuples;
    d->data    = data;
    d->excess  = -1;
    d->_dfa    = _dfa;
    return d;
  }

  DFA
  Table::TableI::dfa(void) {
    assert(finalized());
    if (_dfa.n_states())
      return _dfa;

    // Set up regular expression
    REG r;
    for (int t = size; t--; ) {
      REG tuple;
      for (int i = 0; i < arity; ++i)
        tuple = tuple + REG(tuples[0][t][i]);
      r = r | tuple;
    }
    if (table_debug)
      std::cout << "Regular expression for table is " << r << std::endl;
    _dfa = DFA(r);
    if (table_debug)
      std::cout << "DFA for table is " << _dfa << std::endl;
    return _dfa;
  }
}

// STATISTICS: int-prop

