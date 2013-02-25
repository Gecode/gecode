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

namespace Gecode {
  template <>
  inline ModEvent
  prune<Int::IntView>(Space& home, Int::IntView x, int v) {
    return x.nq(home, v);
  }

  template <>
  inline ModEvent
  prune<Int::BoolView>(Space& home, Int::BoolView x, int v) {
    return x.nq(home, v);
  }

  template<>
  class ArrayTraits<ArgArray<VarImpBase*> > {
  public:
    typedef ArgArray<VarImpBase*>     StorageType;
    typedef VarImpBase*               ValueType;
    typedef ArgArray<VarImpBase*>     ArgsType;
  };
}

namespace Gecode { namespace Int { namespace Branch {
  
  template <class T, class A>
  ArgArray<T>
  dynamicStackToArgArray(const Support::DynamicStack<T,A>& s) {
    ArgArray<T> a(s.entries());
    for (int i = 0 ; i < s.entries() ; ++i) {
      a[i] = s[i];
    }
    return a;
  }

  GECODE_INT_EXPORT
  std::pair<int,int>
  findVar(int *indices, unsigned int n_values, unsigned int seq_size, int index);
  
  class VariableMap : public std::map<VarImpBase*,int> {};
  
  class SymmetryObject {
  public:
    /// Number of references that point to this object.
    int nrefs;
    /// Default constructor
    SymmetryObject() : nrefs(1) {}
    virtual ~SymmetryObject() {}
  };

  class VariableSymmetryObject : public SymmetryObject {
  public:
    VarImpBase** xs;
    int nxs;
  public:
    VariableSymmetryObject(ArgArray<VarImpBase*> vars) {
      nxs = vars.size();
      xs = new VarImpBase*[nxs];
      for (int i = 0 ; i < nxs ; i++)
        xs[i] = vars[i];
    }
    ~VariableSymmetryObject() {
      delete [] xs;
    }
  };

  class ValueSymmetryObject : public SymmetryObject {
  public:
    IntSet values;
  public:
    ValueSymmetryObject(IntSet vs)
      : values(vs) {}
  };

  class VariableSequenceSymmetryObject : public SymmetryObject {
  public:
    VarImpBase** xs;
    int nxs;
    int seq_size;
  public:
    VariableSequenceSymmetryObject(ArgArray<VarImpBase*> vars, int ss)
      : seq_size(ss) {
      nxs = vars.size();
      xs = new VarImpBase*[nxs];
      for (int i = 0 ; i < nxs ; i++)
        xs[i] = vars[i];
    }
    ~VariableSequenceSymmetryObject() {
      delete [] xs;
    }
  };

  class ValueSequenceSymmetryObject : public SymmetryObject {
  public:
    IntArgs values;
    int seq_size;
  public:
    ValueSequenceSymmetryObject(IntArgs vs, int ss)
      : values(vs), seq_size(ss) {}
  };
}}}

namespace Gecode {
  // A SymmetryHandle is just a reference-counted pointer to a
  // SymmetryObject.
  class SymmetryHandle {
  public:
    /// Symmetry object that this handle refers to.
    Int::Branch::SymmetryObject* ref;
    /// Decrement 
    GECODE_INT_EXPORT void increment();
    GECODE_INT_EXPORT void decrement();
  public:
    /// Default constructor
    SymmetryHandle() : ref(NULL) {}
    /// Initialies with a SymmetryObject
    SymmetryHandle(Int::Branch::SymmetryObject* o) : ref(o) {}
    /// Copy constructor
    SymmetryHandle(const SymmetryHandle& h)
      : ref(h.ref) {
      if (ref != NULL)
        increment();
    }
    /// Assignment operator
    const SymmetryHandle&
    operator = (const SymmetryHandle& h) {
      if (h.ref == ref)
        return *this;
      if (ref != NULL)
        decrement();
      ref = h.ref;
      if (ref != NULL)
        increment();
      return *this;
    }
    /// Destructor
    ~SymmetryHandle() {
      if (ref != NULL)
        decrement();
    }
  };

  /*
   * LDSB modelling level classes.
   * 
   * At the modelling level, the branch function takes a reference to
   * a Symmetries object, which is a collection of SymmetryHandle
   * objects.  A SymmetryHandle is a reference-counted pointer to a
   * SymmetryObject.  The SymmetryObject has a subclass for each kind
   * of symmetry.
   */

  template<>
  class ArrayTraits<ArgArray<SymmetryHandle> > {
  public:
    typedef Symmetries     StorageType;
    typedef SymmetryHandle ValueType;
    typedef Symmetries     ArgsType;
  };

  // If this is instead a typedef, strange things happen with the
  // overloading of the "branch" function.
  class Symmetries : public ArgArray<SymmetryHandle> {};

  namespace Int { namespace Branch {
    SymmetryImp<Int::IntView>* createIntSym(Space& home, const SymmetryHandle& s, Int::Branch::VariableMap variableMap);
    SymmetryImp<Int::BoolView>* createBoolSym(Space& home, const SymmetryHandle& s, Int::Branch::VariableMap variableMap);
  }}

  GECODE_INT_EXPORT SymmetryHandle VariableSymmetry(IntVarArgs vars);
  GECODE_INT_EXPORT SymmetryHandle VariableSymmetry(BoolVarArgs vars);
  GECODE_INT_EXPORT SymmetryHandle ValueSymmetry(IntArgs vs);
  GECODE_INT_EXPORT SymmetryHandle ValueSymmetry(IntSet vs);
  GECODE_INT_EXPORT SymmetryHandle VariableSequenceSymmetry(IntVarArgs vs, int ss);
  GECODE_INT_EXPORT SymmetryHandle VariableSequenceSymmetry(BoolVarArgs vs, int ss);
  GECODE_INT_EXPORT SymmetryHandle ValueSequenceSymmetry(IntArgs vs, int ss); 

  // ************************************************************
  // Convenience functions.
  // ************************************************************

  // All variables in vars are interchangeable
  GECODE_INT_EXPORT SymmetryHandle variables_interchange(const IntVarArgs& vars);
  GECODE_INT_EXPORT SymmetryHandle variables_interchange(const BoolVarArgs& vars);

  // Specified variables in vars are interchangeable
  GECODE_INT_EXPORT SymmetryHandle variables_indices_interchange(const IntVarArgs& vars, IntArgs indices);

  /// All values in the domains of vars are interchangeable
  GECODE_INT_EXPORT SymmetryHandle values_interchange(const IntVarArgs& vars);

  /// Values reflect: min <-> max, min+1 <-> max-1, etc.
  // template <class View>
  // Symmetry<View> values_reflect(Space& home, int min, int max);

  /// Rows in the vars matrix are interchangeable
  // template <class View, class VarArray>
  // Symmetry<View> rows_interchange(Space& home, const Matrix<VarArray>& vars);
  // /// Columns in the vars matrix are interchangeable
  // template <class View, class VarArray>
  // Symmetry<View> columns_interchange(Space& home, const Matrix<VarArray>& vars);

  // /// Rows in the vars matrix reflect: row 0 to row n-1, row 1 to row
  // /// n-2, etc.
  // template <class View, class VarArray>
  // Symmetry<View> rows_reflect(Space& home, const Matrix<VarArray>&
  // vars);
}

// STATISTICS: int-branch
