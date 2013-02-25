/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
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

#include <gecode/set/branch.hh>

namespace Gecode {
  /*
   * Implementation of some SymmetryHandle methods.
   */
  
  namespace Set { namespace Branch {
    using Int::Branch::VariableSymmetryObject;
    using Int::Branch::ValueSymmetryObject;
    using Int::Branch::VariableSequenceSymmetryObject;
    using Int::Branch::ValueSequenceSymmetryObject;
    using Int::Branch::VariableMap;
    /*
     * The createSetSym function is an almost exact copy of
     * createIntSym/createBoolSym.
     */
    SymmetryImp<SetView>* createSetSym(Space& home, const SymmetryHandle& s,
                                       VariableMap variableMap) {
      VariableSymmetryObject*         varref    = dynamic_cast<VariableSymmetryObject*>(s.ref);
      ValueSymmetryObject*            valref    = dynamic_cast<ValueSymmetryObject*>(s.ref);
      VariableSequenceSymmetryObject* varseqref = dynamic_cast<VariableSequenceSymmetryObject*>(s.ref);
      ValueSequenceSymmetryObject*    valseqref = dynamic_cast<ValueSequenceSymmetryObject*>(s.ref);
      if (varref) {
        int n = varref->nxs;
        int* indices = home.alloc<int>(n);
        for (int i = 0 ; i < n ; i++) {
          VariableMap::const_iterator index = variableMap.find(varref->xs[i]);
          if (index == variableMap.end())
            throw LDSBBranchingException("VariableSymmetryObject::createSet");
          indices[i] = index->second;
        }
        return new (home) VariableSymmetryImp<SetView>(home, indices, n);
      }
      if (valref) {
        int n = valref->values.size();
        int *vs = home.alloc<int>(n);
        int i = 0;
        for (IntSetValues v(valref->values) ; v() ; ++v) {
          vs[i] = v.val();
          i++;
        }
        return new (home) ValueSymmetryImp<SetView>(home, vs, n);
      }
      if (varseqref) {
        int n = varseqref->nxs;
        int* indices = home.alloc<int>(n);
        for (int i = 0 ; i < n ; i++) {
          VariableMap::const_iterator index = variableMap.find(varseqref->xs[i]);
          if (index == variableMap.end())
            throw LDSBBranchingException("VariableSequenceSymmetryObject::createSet");
          indices[i] = index->second;
        }
        return new (home) VariableSequenceSymmetryImp<SetView>(home, indices, n, varseqref->seq_size);
      }
      if (valseqref) {
        unsigned int n = valseqref->values.size();
        int *vs = home.alloc<int>(n);
        for (unsigned int i = 0 ; i < n ; i++)
          vs[i] = valseqref->values[i];
        return new (home) ValueSequenceSymmetryImp<SetView>(home, vs, n, valseqref->seq_size);
      }
      std::cerr << "Unknown symmetry type in createSetSym." << std::endl;
      abort();
    }
  }}

  SymmetryHandle VariableSymmetry(SetVarArgs vars) {
    ArgArray<VarImpBase*> a(vars.size());
    for (int i = 0 ; i < vars.size() ; i++)
      a[i] = vars[i].varimp();
    return SymmetryHandle(new Int::Branch::VariableSymmetryObject(a));
  }
  SymmetryHandle ValueSymmetry(IntArgs vs) {
    return SymmetryHandle(new Int::Branch::ValueSymmetryObject(IntSet(vs)));
  }
  SymmetryHandle ValueSymmetry(IntSet vs) {
    return SymmetryHandle(new Int::Branch::ValueSymmetryObject(vs));
  }
  SymmetryHandle VariableSequenceSymmetry(SetVarArgs vars, int ss) {
    ArgArray<VarImpBase*> a(vars.size());
    for (int i = 0 ; i < vars.size() ; i++)
      a[i] = vars[i].varimp();
    return SymmetryHandle(new Int::Branch::VariableSequenceSymmetryObject(a, ss));
  }
  SymmetryHandle ValueSequenceSymmetry(IntArgs vs, int ss) {
    return SymmetryHandle(new Int::Branch::ValueSequenceSymmetryObject(vs, ss));
  }

  template <>
  ArgArray<Literal>
  VariableSymmetryImp<Set::SetView>
  ::symmetric(Literal l, const ViewArray<Set::SetView>& x) const {
    (void) x;
    if (indices.valid(l.variable) && indices.get(l.variable)) {
      int n = 0;
      for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space> > i(indices) ; i() ; ++i)
        n++;
      ArgArray<Literal> lits(n);
      int j = 0;
      for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space> > i(indices) ; i() ; ++i) {
        lits[j++] = Literal(i.val(), l.value);
      }
      return lits;
    } else {
      return ArgArray<Literal>(0);
    }
  }

  template <>
  ArgArray<Literal>
  ValueSymmetryImp<Set::SetView>
  ::symmetric(Literal l, const ViewArray<Set::SetView>& x) const {
    (void) x;
    if (values.valid(l.value) && values.get(l.value)) {
      int n = 0;
      for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space> > i(values) ; i() ; ++i)
        n++;
      ArgArray<Literal> lits(n);
      int j = 0;
      for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space> > i(values) ; i() ; ++i) {
        lits[j++] = Literal(l.variable, i.val());
      }
      return lits;
    } else {
      return ArgArray<Literal>(0);
    }
  }

  // XXX: could be improved?
  bool
  equalValues(const Set::SetView& x, const Set::SetView& y) {
    unsigned int n = x.lubSize();
    if (n != y.lubSize()) return false;
    for (unsigned int i = 0 ; i < n ; i++)
      if (x.lubMinN(i) != y.lubMinN(i))
        return false;
    return true;
  }

  template <class View>
  ArgArray<Literal>
  VariableSequenceSymmetryImp<View>
  ::symmetric(Literal l, const ViewArray<View>& x) const {
    Support::DynamicStack<Literal, Heap> s(heap);
    if (l.variable < (int)lookup_size) {
      int posIt = lookup[l.variable];
      if (posIt == -1) {
        return Int::Branch::dynamicStackToArgArray(s);
      }
      unsigned int seqNum = posIt / seq_size;
      unsigned int seqPos = posIt % seq_size;
      for (unsigned int seq = 0 ; seq < n_seqs ; seq++) {
        if (seq == seqNum) {
          continue;
        }
        if (x[getVal(seq, seqPos)].assigned()) {
          continue;
        }
        bool active = true;
        const unsigned int *firstSeq = &indices[seqNum*seq_size];
        const unsigned int *secondSeq = &indices[seq*seq_size];
        for (unsigned int i = 0 ; i < seq_size ; i++) {
          const View& xv = x[firstSeq[i]];
          const View& yv = x[secondSeq[i]];
          if ((!xv.assigned() && !yv.assigned())
              || (xv.assigned() && yv.assigned() && equalValues(xv, yv))) {
            continue;
          } else {
            active = false;
            break;
          }
        }
        
        if (active) {
          s.push(Literal(secondSeq[seqPos], l.value));
        } 
      }
    }
    return Int::Branch::dynamicStackToArgArray(s);
  }

  template <>
  ArgArray<Literal>
  ValueSequenceSymmetryImp<Set::SetView>
  ::symmetric(Literal l, const ViewArray<Set::SetView>& x) const {
    (void) x;
    Support::DynamicStack<Literal, Heap> s(heap);
    std::pair<int,int> location = Int::Branch::findVar(values, n_values, seq_size, l.value);
    if (location.first == -1) return Int::Branch::dynamicStackToArgArray(s);
    unsigned int seqNum = location.first;
    unsigned int seqPos = location.second;
    for (unsigned int seq = 0 ; seq < n_seqs ; seq++) {
      if (seq == seqNum) continue;
      if (dead_sequences.get(seq)) continue;
      s.push(Literal(l.variable, getVal(seq,seqPos)));
    }
    return Int::Branch::dynamicStackToArgArray(s);
  }

  /*********************/

  // SetVariableSymmetry
  // variables_interchange(const SetVarArgs& vars) {
  //   return SetVariableSymmetry(vars);
  // }

  // SetVariableSymmetry
  // variables_indices_interchange(const SetVarArgs& vars, IntArgs indices) {
  //   SetVarArgs xs(indices.size());
  //   for (int i = 0 ; i < indices.size() ; i++)
  //     xs[i] = vars[indices[i]];
  //   return SetVariableSymmetry(xs);
  // }

  // SetValueSymmetry
  // values_interchange(const SetVarArgs& vars) {
  //   int min = vars[0].min();
  //   int max = vars[0].max();
  //   for (int i = 1 ; i < vars.size() ; i++) {
  //     min = std::min(min, vars[i].min());
  //     max = std::max(max, vars[i].max());
  //   }
  //   return SetValueSymmetry(IntSet(min, max));
  // }
}

// STATISTICS: set-branch
