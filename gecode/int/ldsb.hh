/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_INT_LDSB_HH
#define GECODE_INT_LDSB_HH

#include <gecode/int.hh>

#include <compare>

/**
 * \namespace Gecode::Int::LDSB
 * \brief Symmetry breaking for integer variables
 */
namespace Gecode { namespace Int { namespace LDSB {

    /// A Literal is a pair of variable index and value.
    class Literal {
    public:
        /// Constructor for an empty literal
        Literal();
        /// Constructor
        Literal(int idx, int _val);

        /// Variable index.  The ViewArray that the index is meant
        /// for is assumed to be known by context.
        int _variable;
        /// The value of the literal.  For int and bool variables,
        /// this is the value itself; for set variables, this is
        /// one of the possible elements of the set.
        int _value;

        /// Order lexicographically by variable index, then by value
        auto operator<=>(const Literal&) const = default;
    };

    /**
     * \brief Find the location of an integer in a collection of
     * sequences.
     *
     *
     * Given an array \a indices of integers (of length \a n_values),
     * which represents a collection of sequences each of size \a
     * seq_size, find the location of the first occurrence of the value
     * \a index.  Returns a pair of sequence number and position within
     * that sequence (both zero-based).
     */
    GECODE_INT_EXPORT
    std::pair<int, int> find_var(const int* indices, unsigned int n_values, unsigned int seq_size, int index);
}}} // namespace Gecode::Int::LDSB

namespace Gecode {
    /// Traits of %ArgArray<VarImpBase*>
    template <>
    class ArrayTraits<ArgArray<VarImpBase*>> {
    public:
        using StorageType = ArgArray<VarImpBase*>;
        using ValueType = VarImpBase*;
        using ArgsType = ArgArray<VarImpBase*>;
    };

    /// An array of literals
    using LiteralArgs = ArgArray<Int::LDSB::Literal>;
    /// Traits of %LiteralArgs
    template <>
    class ArrayTraits<LiteralArgs> {
    public:
        using StorageType = LiteralArgs;
        using ValueType = Int::LDSB::Literal;
        using ArgsType = LiteralArgs;
    };
} // namespace Gecode

namespace Gecode { namespace Int { namespace LDSB {
    /// Implementation of a symmetry at the modelling level
    class GECODE_INT_EXPORT SymmetryObject {
    public:
        /// Number of references that point to this symmetry object.
        int nrefs;
        /// Default constructor
        SymmetryObject();
        /// Destructor
        virtual ~SymmetryObject();
    };
    /// Implementation of a variable symmetry at the modelling level
    class GECODE_INT_EXPORT VariableSymmetryObject : public SymmetryObject {
    public:
        /// Array of variables in symmetry
        VarImpBase** xs;
        /// Number of variables in symmetry
        int nxs;
        /// Constructor for creation
        VariableSymmetryObject(ArgArray<VarImpBase*> vars);
        /// Destructor
        ~VariableSymmetryObject() override;
    };
    /// Implementation of a value symmetry at the modelling level
    class GECODE_INT_EXPORT ValueSymmetryObject : public SymmetryObject {
    public:
        /// Set of symmetric values
        IntSet values;
        /// Constructor for creation
        ValueSymmetryObject(IntSet vs);
    };
    /// Implementation of a variable sequence symmetry at the modelling level
    class GECODE_INT_EXPORT VariableSequenceSymmetryObject : public SymmetryObject {
    public:
        /// Array of variables in symmetry
        VarImpBase** xs;
        /// Number of variables in symmetry
        int nxs;
        /// Size of each sequence in symmetry
        int seq_size;
        /// Constructor for creation
        VariableSequenceSymmetryObject(ArgArray<VarImpBase*> vars, int ss);
        /// Destructor
        ~VariableSequenceSymmetryObject() override;
    };
    /// Implementation of a value sequence symmetry at the modelling level
    class GECODE_INT_EXPORT ValueSequenceSymmetryObject : public SymmetryObject {
    public:
        /// Array of values in symmetry
        IntArgs values;
        /// Size of each sequence in symmetry
        int seq_size;
        /// Constructor for creation
        ValueSequenceSymmetryObject(const IntArgs& vs, int ss);
    };

    /**
     * \brief Test whether the symmetry \a s describes neither variables nor
     * values
     *
     * Such a symmetry maps nothing, so branching ignores it.  The test does
     * not allocate.
     */
    GECODE_INT_EXPORT bool is_empty(const SymmetryHandle& s);

    /// Implementation of a single symmetry.
    template <class View>
    class SymmetryImp {
    public:
        /// Compute symmetric literals
        virtual ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const = 0;
        /// Left-branch update
        virtual void update(Literal) = 0;
        /// Copy function
        virtual SymmetryImp<View>* copy(Space& home) const = 0;
        /// Disposal
        virtual size_t dispose(Space& home) = 0;
        /// Unused destructor
        virtual ~SymmetryImp();
        /// Placement new operator
        static void* operator new(size_t s, Space& home);
        /// Return memory to space
        static void operator delete(void*, Space&);
        /// Needed for exceptions
        static void operator delete(void*);
    };
    /// Implementation of a variable symmetry.
    template <class View>
    class VariableSymmetryImp : public SymmetryImp<View> {
    protected:
        /// Symmetric variable indices
        Support::BitSetOffset<Space> indices;

    public:
        /// Constructor for creation
        VariableSymmetryImp(Space& home, int* _indices, unsigned int n);
        /// Copy constructor
        VariableSymmetryImp(Space& home, const VariableSymmetryImp& other);
        /// Disposal
        size_t dispose(Space& home) override;
        /// Left-branch update
        void update(Literal) override;
        /// Compute symmetric literals
        ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const override;
        /// Copy function
        SymmetryImp<View>* copy(Space& home) const override;
    };
    /// Implementation of a value symmetry.
    template <class View>
    class ValueSymmetryImp : public SymmetryImp<View> {
    public:
        /// Symmetric values
        Support::BitSetOffset<Space> values;
        /// Constructor for creation
        ValueSymmetryImp(Space& home, int* vs, unsigned int n);
        /// Copy constructor
        ValueSymmetryImp(Space& home, const ValueSymmetryImp<View>& other);
        /// Disposal
        size_t dispose(Space& home) override;
        /// Left-branch update
        void update(Literal) override;
        /// Compute symmetric literals
        ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const override;
        /// Copy function
        SymmetryImp<View>* copy(Space& home) const override;
    };
    /// Implementation of a variable sequence symmetry.
    template <class View>
    class VariableSequenceSymmetryImp : public SymmetryImp<View> {
    protected:
        /// Array of variable indices
        unsigned int* indices;
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
        int* lookup;
        /// Size of lookup
        unsigned int lookup_size;

        /// Get the value in the specified sequence at the specified
        /// position.  (Both are zero-based.)
        int get_val(unsigned int sequence, unsigned int position) const;

    public:
        /// Constructor for creation
        VariableSequenceSymmetryImp(Space& home, const int* _indices, unsigned int n, unsigned int seqsize);
        /// Copy constructor
        VariableSequenceSymmetryImp(Space& home, const VariableSequenceSymmetryImp& s);
        /// Disposal
        size_t dispose(Space& home) override;
        /// Search left-branch update
        void update(Literal) override;
        /// Compute symmetric literals
        ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const override;
        /// Copy function
        SymmetryImp<View>* copy(Space& home) const override;
    };
    /// Implementation of a value sequence symmetry.
    template <class View>
    class ValueSequenceSymmetryImp : public SymmetryImp<View> {
    protected:
        /// Set of sequences
        int* values;
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
        int get_val(unsigned int sequence, unsigned int position) const;

    public:
        ValueSequenceSymmetryImp(const ValueSequenceSymmetryImp<View>&) = delete;
        /// Constructor for creation
        ValueSequenceSymmetryImp(Space& home, const int* _values, unsigned int n, unsigned int seqsize);
        /// Copy constructor
        ValueSequenceSymmetryImp(Space& home, const ValueSequenceSymmetryImp<View>& vss);
        /// Disposal
        size_t dispose(Space& home) override;
        /// Left-branch update
        void update(Literal) override;
        /// Compute symmetric literals
        ArgArray<Literal> symmetric(Literal, const ViewArray<View>&) const override;
        /// Copy function
        SymmetryImp<View>* copy(Space& home) const override;
    };

    /// %Choice storing position and value, and symmetric literals to be
    ///  excluded on the right branch.
    template <class Val>
    class GECODE_VTABLE_EXPORT LDSBChoice : public PosValChoice<Val> {
    private:
        /// Set of literals to be excluded
        const Literal* const _literals;
        /// Number of literals
        const int _nliterals;

    public:
        /// Initialize choice for brancher \a b, position \a p, value \a
        /// n, and set of literals \a literals (of size \a nliterals)
        LDSBChoice(const Brancher& b, unsigned int a, const Pos& p, const Val& n, const Literal* literals,
                   int nliterals);
        /// Destructor
        ~LDSBChoice() override;
        /// Return literals
        const Literal* literals() const;
        /// Return number of literals
        int nliterals() const;
        /// Archive into \a e
        void archive(Archive& e) const override;
    };

    /**
     * \brief Symmetry-breaking brancher with generic view and value
     * selection
     *
     * Implements view-based branching for an array of views (of type
     * \a View) and value (of type \a Val).
     *
     */
    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    class LDSBBrancher : public ViewValBrancher<View, n, Val, a, Filter, Print> {
    public:
        using typename ViewValBrancher<View, n, Val, a, Filter, Print>::Var;
        /// Array of symmetry implementations, nullptr when \a _nsyms is 0
        SymmetryImp<View>** _syms;
        /// Number of symmetry implementations
        int _nsyms;
        // Position of variable that last choice was created for
        int _prev_pos;

    protected:
        /// Constructor for cloning \a b
        LDSBBrancher(Space& home, LDSBBrancher& b);
        /// Constructor for creation
        LDSBBrancher(Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc,
                     SymmetryImp<View>** syms, int nsyms, BranchFilter<Var> bf, VarValPrint<Var, Val> vvp);

    public:
        /// Return choice
        const Choice* choice(Space& home) override;
        /// Return choice
        const Choice* choice(const Space& home, Archive& e) override;
        /// Perform commit for choice \a c and alternative \a b
        ExecStatus commit(Space& home, const Choice& c, unsigned int b) override;
        /// Perform cloning
        Actor* copy(Space& home) override;
        /// Delete brancher and return its size
        size_t dispose(Space& home) override;
        /// Brancher post function
        static void post(Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc,
                         SymmetryImp<View>** syms, int nsyms, BranchFilter<Var> bf, VarValPrint<Var, Val> vvp);
    };

    /// Post LDSB brancher
    template <class View, int n, class Val, unsigned int a>
    void post_ldsb_brancher(Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc,
                            SymmetryImp<View>** syms, int nsyms, BranchFilter<typename View::VarType> bf,
                            VarValPrint<typename View::VarType, Val> vvp);

    /// Exclude value \a v from variable view \a x
    template <class View>
    ModEvent prune(Space& home, View x, int v);

}}} // namespace Gecode::Int::LDSB

#include <gecode/int/ldsb/brancher.hpp>
#include <gecode/int/ldsb/sym-imp.hpp>

#endif

// STATISTICS: int-branch
