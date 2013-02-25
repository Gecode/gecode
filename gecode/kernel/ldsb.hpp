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
  /// Exception thrown if a variable specified in a symmetry is not
  /// to be branched on.
  class LDSBBranchingException : public Exception {
  public:
    LDSBBranchingException(const char* l);
  };
  inline
  LDSBBranchingException::LDSBBranchingException(const char* l)
    : Exception(l,"Variable in symmetry not branched on.") {}

  /// Exception thrown if a non-LDSB-compatible branching strategy is
  /// used.
  class LDSBIncompatibleBrancher : public Exception {
  public:
    LDSBIncompatibleBrancher(const char* l);
  };
  inline
  LDSBIncompatibleBrancher::LDSBIncompatibleBrancher(const char* l)
    : Exception(l,"Incompatible brancher.") {}

  /// A Literal is a pair of variable index and value.
  class Literal {
  public:
    Literal() : variable(-1), value(-1) {}
    Literal(int _var, int _val) : variable(_var), value(_val) {}

    /// Variable index.  The ViewArray that the index is meant
    /// for is assumed to be known by context.
    int variable;
    /// The value of the literal.  For int and bool variables,
    /// this is the value itself; for set variables, this is
    /// one of the possible elements of the set.
    int value;

    /// Less than.  The ordering is the lexicographical order
    /// on the (variable,value) pair.
    bool operator <(const Literal &rhs) const {
      int d = rhs.variable - variable;
      if (d > 0)       return true;
      else if (d == 0) return rhs.value > value;
      else             return false;
    }
  };

  typedef ArgArray<Literal> LiteralArgs;
  /// Traits of %LiteralArgs
  template<>
  class ArrayTraits<LiteralArgs> {
  public:
    typedef LiteralArgs   StorageType;
    typedef Literal ValueType;
    typedef LiteralArgs   ArgsType;
  };

  /// Implementation of a single symmetry.
  template <class View>
  class SymmetryImp {
  public:
    /// Compute symmetric literals
    virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const = 0;
    /// Left-branch update
    virtual void update(Literal) = 0;
    /// Copy function
    virtual SymmetryImp<View>* copy(Space& home, bool share) const = 0;
    /// Disposal
    virtual size_t dispose(Space& home) = 0;
    /// Placement new operator
    static void* operator new(size_t s, Space& home) {
      return home.ralloc(s);
    }
    /// Return memory to space
    static void  operator delete(void*,Space&) {}
    /// Needed for exceptions
    static void  operator delete(void*) {}
  protected:
    virtual void printOn(std::ostream& os) const = 0;
    friend std::ostream& operator << (std::ostream& os, const SymmetryImp& sym) {
      sym.printOn(os);
      return os;
    }
  };

  template <class View>
  class VariableSymmetryImp : public SymmetryImp<View> {
  public:
    /// Symmetric variable indices
    Support::BitSetOffset<Space> indices;
    /// Constructor for creation
    VariableSymmetryImp<View>(Space& home, int* vs, unsigned int n);
    //    VariableSymmetryImp(Space& home, const Support::BitSetOffset<Space>& bits);
    /// Copy constructor
    VariableSymmetryImp<View>(Space& home, const VariableSymmetryImp<View>& other);
    /// Disposal
    virtual size_t dispose(Space& home);
    /// Left-branch update
    void update(Literal);
    /// Compute symmetric literals
    virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const;
    /// Copy function
    SymmetryImp<View>* copy(Space& home, bool share) const;
    /// Completeness check
    virtual bool completed() const;
  protected:
    virtual void printOn(std::ostream& os) const {
      os << "VariableSymmetryImp(" << indices << ")";
    }
  };

  template <class View>
  class ValueSymmetryImp : public SymmetryImp<View>
  {
  public:
    /// Symmetric values
    Support::BitSetOffset<Space> values;
    /// Constructor for creation
    ValueSymmetryImp<View>(Space& home, int* vs, unsigned int n);
    //    ValueSymmetryImp(Space& home, const Support::BitSetOffset<Space>& bits);
    /// Copy constructor
    ValueSymmetryImp<View>(Space& home, const ValueSymmetryImp<View>& other);
    /// Disposal
    virtual size_t dispose(Space& home);
    /// Left-branch update
    void update(Literal);
    /// Compute symmetric literals
    virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const;
    /// Copy function
    SymmetryImp<View>* copy(Space& home, bool share) const;
    /// Completeness check
    virtual bool completed() const;
  protected:
    virtual void printOn(std::ostream& os) const {
      os << "ValueSymmetryImp(" << values << ")";
    }
  };

  template <class View>
  class VariableSequenceSymmetryImp : public SymmetryImp<View>
  {
  protected:
    /// Array of variable indices
    unsigned int *indices;
    /// Total number of indices (n_seqs * seq_size)
    unsigned int n_indices;
    /// Size of each sequence in symmetry
    unsigned int seq_size;
    /// Number of sequences in symmetry
    unsigned int n_seqs;

    /// Map from variable's index to its sequence and position.
    // e.g. lookup[2] == 10 indicates that the variable with index 2
    // occurs at position 10 in the "indices" array.
    // If a variable occurs more than once, only the first occurrence
    // is recorded.
    // A value of -1 indicates that the variable does not occur in
    // "indices".
    int *lookup;
    unsigned int lookup_size;

    /// Get the value in the specified sequence at the specified
    /// position.  (Both are zero-based.)
    int getVal(unsigned int sequence, unsigned int position) const;
  public:
    /// Constructor for creation
    VariableSequenceSymmetryImp<View>(Space& home, int *_indices, unsigned int n, unsigned int seqsize);
    /// Copy constructor
    VariableSequenceSymmetryImp<View>(Space& home, bool share, const VariableSequenceSymmetryImp<View>& s);
    /// Disposal
    virtual size_t dispose(Space& home);
    /// Search left-branch update
    void update(Literal);
    /// Compute symmetric literals
    virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const;
    /// Copy function
    SymmetryImp<View>* copy(Space& home, bool share) const;

    virtual void printOn(std::ostream& os) const {
      os << "VariableSequenceSymmetryImp(";
      for (unsigned int i = 0 ; i < n_seqs ; i++) {
        if (i > 0)
          os << ",";
        os << "<";
        for (unsigned int j = 0 ; j < seq_size ; j++) {
          if (j > 0)
            os << ",";
          os << getVal(i,j);
        }
        os << ">";
      }
      os << ")";
    }
  };

  template <class View>
  class ValueSequenceSymmetryImp : public SymmetryImp<View>
  {
  private:
    ValueSequenceSymmetryImp<View>(const ValueSequenceSymmetryImp<View>&);
  public:
    /// Constructor for creation
    ValueSequenceSymmetryImp<View>(Space& home, int* _values, unsigned int n, unsigned int seqsize);
    /// Copy constructor
    ValueSequenceSymmetryImp<View>(Space& home, const ValueSequenceSymmetryImp<View>& vss);
    // /// Disposal
    virtual size_t dispose(Space& home);
    /// Left-branch update
    void update(Literal);
    /// Compute symmetric literals
    virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const;
    /// Copy function
    SymmetryImp<View>* copy(Space& home, bool share) const;
    /// Completeness check
    virtual bool completed() const;
  protected:
    /// Set of sequences
    int *values;
    /// Total number of values (n_seqs * seq_size)
    unsigned int n_values;
    /// Size of each sequence in symmetry
    unsigned int seq_size;
    /// Number of sequences in symmetry
    unsigned int n_seqs;
    /// Which sequences are dead
    Support::BitSet<Space> dead_sequences;

    /// Get the value in the specified sequence at the specified
    /// position.  (Both are zero-based.)
    int getVal(unsigned int sequence, unsigned int position) const;
    
    virtual void printOn(std::ostream& os) const {
      os << "ValueSequenceSymmetryImp(";
      for (unsigned int i = 0 ; i < n_seqs ; i++) {
        if (i > 0)
          os << ",";
        if (dead_sequences.get(i))
          os << "D";
        os << "<";
        for (unsigned int j = 0 ; j < seq_size ; j++) {
          if (j > 0)
            os << ",";
          os << getVal(i,j);
        }
        os << ">";
      }
      os << ")";
    }
  };

  /*
   * Implementation of the above symmetry classes.  Note that the
   * "symmetric" method is not defined here; it must be defined in
   * variable-specific code.
   */

  template <class View>
  VariableSymmetryImp<View>
  ::VariableSymmetryImp(Space& home, int* _indices, unsigned int n)
    : indices(home, 0, 0) {
    // Find minimum and maximum value in _indices: the minimum is the
    // offset, and the maximum dictates how large the bitset needs to
    // be.
    int maximum = _indices[0];
    int minimum = _indices[0];
    for (unsigned int i = 1 ; i < n ; i++) {
      if (_indices[i] > maximum) maximum = _indices[i];
      if (_indices[i] < minimum) minimum = _indices[i];
    }
    indices.resize(home, maximum-minimum+1, minimum);

    // Set the bits for the included indices.
    for (unsigned int i = 0 ; i < n ; i++) {
      indices.set(_indices[i]);
    }
  }

  template <class View>
  VariableSymmetryImp<View>
  ::VariableSymmetryImp(Space& home, const VariableSymmetryImp& other) :
    indices(home, other.indices) {}

  template <class View>
  size_t
  VariableSymmetryImp<View>
  ::dispose(Space& home) {
    indices.dispose(home);
    return sizeof(*this);
  }

  template <class View>
  void
  VariableSymmetryImp<View>
  ::update(Literal l) {
    if (indices.valid(l.variable)) {
      indices.clear(l.variable);
    }
  }

  template <class View>
  SymmetryImp<View>*
  VariableSymmetryImp<View>
  ::copy(Space& home, bool share) const {
    (void) share;
    return new (home) VariableSymmetryImp<View>(home, *this);
  }

  template <class View>
  bool
  VariableSymmetryImp<View>
  ::completed() const {
    return false;
  }

  /*********************/

  // The minimum value in vs is the bitset's offset, and the maximum
  // dictates how large the bitset needs to be.
  template <class View>
  ValueSymmetryImp<View>
  ::ValueSymmetryImp(Space& home, int* vs, unsigned int n)
    : values(home, 0, 0) {
    // Find minimum and maximum value in vs: the minimum is the
    // offset, and the maximum dictates how large the bitset needs to
    // be.
    int maximum = vs[0];
    int minimum = vs[0];
    for (unsigned int i = 1 ; i < n ; i++) {
      if (vs[i] > maximum) maximum = vs[i];
      if (vs[i] < minimum) minimum = vs[i];
    }
    values.resize(home, maximum-minimum+1, minimum);

    // Set the bits for the included values.
    for (unsigned int i = 0 ; i < n ; i++) {
      values.set(vs[i]);
    }
  }

  template <class View>
  ValueSymmetryImp<View>
  ::ValueSymmetryImp(Space& home, const ValueSymmetryImp<View>& other)
    : values(home, other.values) { }

  template <class View>
  size_t
  ValueSymmetryImp<View>
  ::dispose(Space& home) {
    values.dispose(home);
    return sizeof(*this);
  }

  template <class View>
  void
  ValueSymmetryImp<View>
  ::update(Literal l) {
    if (values.valid(l.value))
      values.clear(l.value);
  }

  template <class View>
  SymmetryImp<View>*
  ValueSymmetryImp<View>
  ::copy(Space& home, bool share) const {
    (void) share;
    return new (home) ValueSymmetryImp(home, *this);
  }

  template <class View>
  bool
  ValueSymmetryImp<View>
  ::completed() const {
    return false;
    // bool r = values.size() < 2;
    // return r;
  }

  /*********************/

  template <class View>
  int
  VariableSequenceSymmetryImp<View>
  ::getVal(unsigned int sequence, unsigned int position) const {
    return indices[sequence*seq_size + position];
  }

  template <class View>
  VariableSequenceSymmetryImp<View>
  ::VariableSequenceSymmetryImp(Space& home, int* _indices, unsigned int n, unsigned int seqsize)
    : n_indices(n), seq_size(seqsize), n_seqs(n/seqsize) {
    indices = home.alloc<unsigned int>(n_indices);
    unsigned int max_index = _indices[0];
    for (unsigned int i = 0 ; i < n_indices ; i++) {
      indices[i] = _indices[i];
      if (indices[i] > max_index)
        max_index = indices[i];
    }

    lookup_size = max_index+1;
    lookup = home.alloc<int>(lookup_size);
    for (unsigned int i = 0 ; i < lookup_size ; i++)
      lookup[i] = -1;
    for (unsigned int i = 0 ; i < n_indices ; i++) {
      if (lookup[indices[i]] == -1)
        lookup[indices[i]] = i;
    }
  }

  template <class View>
  VariableSequenceSymmetryImp<View>
  ::VariableSequenceSymmetryImp(Space& home, bool share, const VariableSequenceSymmetryImp& s)
    : n_indices(s.n_indices), seq_size(s.seq_size), n_seqs(s.n_seqs),
      lookup_size(s.lookup_size) {
    (void) share;
    indices = home.alloc<unsigned int>(n_indices);
    memcpy(indices, s.indices, n_indices * sizeof(int));
    lookup = home.alloc<int>(lookup_size);
    memcpy(lookup, s.lookup, lookup_size * sizeof(int));
  }

  template <class View>
  size_t
  VariableSequenceSymmetryImp<View>
  ::dispose(Space& home) {
    home.free<unsigned int>(indices, n_indices);
    home.free<int>(lookup, lookup_size);
    return sizeof(*this);
  }

  template <class View>
  void
  VariableSequenceSymmetryImp<View>
  ::update(Literal l) {
    // Do nothing.
    (void) l;
  }

  template <class View>
  SymmetryImp<View>*
  VariableSequenceSymmetryImp<View>
  ::copy(Space& home, bool share) const {
    return new (home) VariableSequenceSymmetryImp<View>(home, share, *this);
  }

  /*********************/

  template <class View>
  int
  ValueSequenceSymmetryImp<View>
  ::getVal(unsigned int sequence, unsigned int position) const {
    return values[sequence*seq_size + position];
  }

  template <class View>
  ValueSequenceSymmetryImp<View>
  ::ValueSequenceSymmetryImp(Space& home, int* _values, unsigned int n, unsigned int seqsize)
    : n_values(n), seq_size(seqsize), n_seqs(n/seqsize),
      dead_sequences(home, n_seqs) {
    values = home.alloc<int>(n_values);
    for (unsigned int i = 0 ; i < n_values ; i++)
      values[i] = _values[i];
  }

  template <class View>
  ValueSequenceSymmetryImp<View>
  ::ValueSequenceSymmetryImp(Space& home, const ValueSequenceSymmetryImp<View>& vss)
    : n_values(vss.n_values),
      seq_size(vss.seq_size),
      n_seqs(vss.n_seqs),
      dead_sequences(home, vss.dead_sequences) {
    values = home.alloc<int>(n_values);
    for (unsigned int i = 0 ; i < n_values ; i++)
      values[i] = vss.values[i];
  }

  template <class View>
  size_t
  ValueSequenceSymmetryImp<View>
  ::dispose(Space& home) {
    home.free(values, n_values);
    return sizeof(*this);
  }

  template <class View>
  void
  ValueSequenceSymmetryImp<View>
  ::update(Literal l) {
    unsigned int seq = 0;
    unsigned int pos = 0;
    for (unsigned int i = 0 ; i < n_values ; i++) {
      if (values[i] == l.value) {
        dead_sequences.set(seq);
        // TODO: This can be slightly optimised.
        while (pos < seq_size) {
          i++;
          pos++;
        }
      }
      pos++;
      if (pos == seq_size) {
        pos = 0;
        seq++;
      }
    }
  }

  template <class View>
  SymmetryImp<View>*
  ValueSequenceSymmetryImp<View>
  ::copy(Space& home, bool share) const {
    (void) share;
    return new (home) ValueSequenceSymmetryImp<View>(home, *this);
  }

  template <class View>
  bool
  ValueSequenceSymmetryImp<View>
  ::completed() const {
    return false;
    // bool r = values.size() < 2;
    // return r;
  }
}

// STATISTICS: kernel-branch
