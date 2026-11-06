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

#include <gecode/int/ldsb.hh>
#include <gecode/int/branch.hh>

#include <map>

namespace Gecode { namespace Int { namespace LDSB {

    std::pair<int, int>
    find_var(const int* indices, unsigned int n_values, unsigned int seq_size, int index) {
        unsigned int seq = 0;
        unsigned int pos = 0;
        for (unsigned int i = 0U; i < n_values; i++) {
            if (indices[i] == index)
                return std::pair<int, int>(seq, pos);
            pos++;
            if (pos == seq_size) {
                pos = 0;
                seq++;
            }
        }
        return std::pair<int, int>(-1, -1);
    }

    bool
    is_empty(const SymmetryHandle& s) {
        if (VariableSymmetryObject* varref = dynamic_cast<VariableSymmetryObject*>(s.ref))
            return varref->nxs == 0;
        if (ValueSymmetryObject* valref = dynamic_cast<ValueSymmetryObject*>(s.ref))
            return valref->values.size() == 0;
        if (VariableSequenceSymmetryObject* varseqref = dynamic_cast<VariableSequenceSymmetryObject*>(s.ref))
            return varseqref->nxs == 0;
        if (ValueSequenceSymmetryObject* valseqref = dynamic_cast<ValueSequenceSymmetryObject*>(s.ref))
            return valseqref->values.size() == 0;
        return false;
    }

}}} // namespace Gecode::Int::LDSB

namespace Gecode {
    using namespace Int::LDSB;

    SymmetryHandle
    variable_symmetry(const IntVarArgs& vars) {
        ArgArray<VarImpBase*> a(vars.size());
        for (int i = 0; i < vars.size(); i++)
            a[i] = vars[i].varimp();
        return SymmetryHandle(new VariableSymmetryObject(a));
    }
    SymmetryHandle
    variable_symmetry(const BoolVarArgs& vars) {
        ArgArray<VarImpBase*> a(vars.size());
        for (int i = 0; i < vars.size(); i++)
            a[i] = vars[i].varimp();
        return SymmetryHandle(new VariableSymmetryObject(a));
    }
    SymmetryHandle
    variable_symmetry(const IntVarArgs& x, const IntArgs& indices) {
        IntVarArgs xs(indices.size());
        for (int i = 0; i < indices.size(); i++)
            xs[i] = x[indices[i]];
        return variable_symmetry(xs);
    }
    SymmetryHandle
    value_symmetry(const IntArgs& vs) {
        return SymmetryHandle(new ValueSymmetryObject(IntSet(vs)));
    }
    SymmetryHandle
    value_symmetry(const IntSet& vs) {
        return SymmetryHandle(new ValueSymmetryObject(vs));
    }
    SymmetryHandle
    value_symmetry(IntVar x) {
        return value_symmetry(IntSet(x.min(), x.max()));
    }
    SymmetryHandle
    variable_sequence_symmetry(const IntVarArgs& vars, int ss) {
        ArgArray<VarImpBase*> a(vars.size());
        for (int i = 0; i < vars.size(); i++)
            a[i] = vars[i].varimp();
        return SymmetryHandle(new VariableSequenceSymmetryObject(a, ss));
    }
    SymmetryHandle
    variable_sequence_symmetry(const BoolVarArgs& vars, int ss) {
        ArgArray<VarImpBase*> a(vars.size());
        for (int i = 0; i < vars.size(); i++)
            a[i] = vars[i].varimp();
        return SymmetryHandle(new VariableSequenceSymmetryObject(a, ss));
    }
    SymmetryHandle
    value_sequence_symmetry(const IntArgs& vs, int ss) {
        return SymmetryHandle(new ValueSequenceSymmetryObject(vs, ss));
    }

    SymmetryHandle
    values_reflect(int lower, int upper) {
        int n = (upper - lower + 1) / 2;
        IntArgs a(n * 2);
        int i = lower;
        int j = upper;
        int k = 0;
        while (i < j) {
            a[k] = j;
            a[n + k] = i;
            i++;
            j--;
            k++;
        }
        return value_sequence_symmetry(a, n);
    }
    SymmetryHandle
    values_reflect(IntVar x) {
        return values_reflect(x.min(), x.max());
    }
} // namespace Gecode

namespace Gecode { namespace Int { namespace LDSB {

    /// Map from variable implementation to index
    class VariableMap : public std::map<VarImpBase*, int> {};

    /*
     * The duplication in create_int_sym/create_bool_sym is undesirable,
     * and so is the use of dynamic_cast to tell the symmetries
     * apart.
     */

    /// Create an integer symmetry implementation from a symmetry handle
    SymmetryImp<IntView>*
    create_int_sym(Space& home, const SymmetryHandle& s, VariableMap variable_map) {
        VariableSymmetryObject* varref = dynamic_cast<VariableSymmetryObject*>(s.ref);
        ValueSymmetryObject* valref = dynamic_cast<ValueSymmetryObject*>(s.ref);
        VariableSequenceSymmetryObject* varseqref = dynamic_cast<VariableSequenceSymmetryObject*>(s.ref);
        ValueSequenceSymmetryObject* valseqref = dynamic_cast<ValueSequenceSymmetryObject*>(s.ref);
        if (varref) {
            int n = varref->nxs;
            int* indices = home.alloc<int>(n);
            for (int i = 0; i < n; i++) {
                VariableMap::const_iterator index = variable_map.find(varref->xs[i]);
                if (index == variable_map.end())
                    throw LDSBUnbranchedVariable("VariableSymmetryObject::createInt");
                indices[i] = index->second;
            }
            return new (home) VariableSymmetryImp<IntView>(home, indices, n);
        }
        if (valref) {
            int n = valref->values.size();
            int* vs = home.alloc<int>(n);
            int i = 0;
            for (IntSetValues v(valref->values); v(); ++v) {
                vs[i] = v.val();
                i++;
            }
            return new (home) ValueSymmetryImp<IntView>(home, vs, n);
        }
        if (varseqref) {
            int n = varseqref->nxs;
            int* indices = home.alloc<int>(n);
            for (int i = 0; i < n; i++) {
                VariableMap::const_iterator index = variable_map.find(varseqref->xs[i]);
                if (index == variable_map.end())
                    throw LDSBUnbranchedVariable("VariableSequenceSymmetryObject::createInt");
                indices[i] = index->second;
            }
            return new (home) VariableSequenceSymmetryImp<IntView>(home, indices, n, varseqref->seq_size);
        }
        if (valseqref) {
            unsigned int n = valseqref->values.size();
            int* vs = home.alloc<int>(n);
            for (unsigned int i = 0; i < n; i++)
                vs[i] = valseqref->values[i];
            return new (home) ValueSequenceSymmetryImp<IntView>(home, vs, n, valseqref->seq_size);
        }
        GECODE_NEVER;
        return nullptr;
    }

    /// Create a boolean symmetry implementation from a symmetry handle
    SymmetryImp<BoolView>*
    create_bool_sym(Space& home, const SymmetryHandle& s, VariableMap variable_map) {
        VariableSymmetryObject* varref = dynamic_cast<VariableSymmetryObject*>(s.ref);
        ValueSymmetryObject* valref = dynamic_cast<ValueSymmetryObject*>(s.ref);
        VariableSequenceSymmetryObject* varseqref = dynamic_cast<VariableSequenceSymmetryObject*>(s.ref);
        ValueSequenceSymmetryObject* valseqref = dynamic_cast<ValueSequenceSymmetryObject*>(s.ref);
        if (varref) {
            int n = varref->nxs;
            int* indices = home.alloc<int>(n);
            for (int i = 0; i < n; i++) {
                VariableMap::const_iterator index = variable_map.find(varref->xs[i]);
                if (index == variable_map.end())
                    throw LDSBUnbranchedVariable("VariableSymmetryObject::createBool");
                indices[i] = index->second;
            }
            return new (home) VariableSymmetryImp<BoolView>(home, indices, n);
        }
        if (valref) {
            int n = valref->values.size();
            int* vs = home.alloc<int>(n);
            int i = 0;
            for (IntSetValues v(valref->values); v(); ++v) {
                vs[i] = v.val();
                i++;
            }
            return new (home) ValueSymmetryImp<BoolView>(home, vs, n);
        }
        if (varseqref) {
            int n = varseqref->nxs;
            int* indices = home.alloc<int>(n);
            for (int i = 0; i < n; i++) {
                VariableMap::const_iterator index = variable_map.find(varseqref->xs[i]);
                if (index == variable_map.end())
                    throw LDSBUnbranchedVariable("VariableSequenceSymmetryObject::createBool");
                indices[i] = index->second;
            }
            return new (home) VariableSequenceSymmetryImp<BoolView>(home, indices, n, varseqref->seq_size);
        }
        if (valseqref) {
            unsigned int n = valseqref->values.size();
            int* vs = home.alloc<int>(n);
            for (unsigned int i = 0; i < n; i++)
                vs[i] = valseqref->values[i];
            return new (home) ValueSequenceSymmetryImp<BoolView>(home, vs, n, valseqref->seq_size);
        }
        GECODE_NEVER;
        return nullptr;
    }

    /**
     * \brief Create implementations of the non-empty symmetries in \a syms
     *
     * Each implementation is created by \a create_sym.  Stores their number
     * in \a nsyms and returns nullptr if there are none.
     */
    template <class View>
    SymmetryImp<View>**
    create_syms(Space& home, const Symmetries& syms, const VariableMap& variable_map,
                SymmetryImp<View>* (*create_sym)(Space&, const SymmetryHandle&, VariableMap), int& nsyms) {
        nsyms = 0;
        for (const auto& sym : syms)
            if (!is_empty(sym))
                nsyms++;
        if (nsyms == 0)
            return nullptr;
        SymmetryImp<View>** array = home.alloc<SymmetryImp<View>*>(nsyms);
        int created = 0;
        for (const auto& sym : syms)
            if (!is_empty(sym))
                array[created++] = create_sym(home, sym, variable_map);
        return array;
    }
}}} // namespace Gecode::Int::LDSB

namespace Gecode {

    using namespace Int::LDSB;

    void
    branch(Home home, const IntVarArgs& x, IntVarBranch vars, const IntValBranch& vals, const Symmetries& syms,
           IntBranchFilter bf, IntVarValPrint vvp) {
        using namespace Int;
        if (home.failed())
            return;
        vars.expand(home, x);
        ViewArray<IntView> xv(home, x);
        ViewSel<IntView>* vs[1] = {Branch::view_selection(home, vars)};
        switch (vals.select()) {
        case IntValBranch::SEL_SPLIT_MIN:
        case IntValBranch::SEL_SPLIT_MAX:
        case IntValBranch::SEL_RANGE_MIN:
        case IntValBranch::SEL_RANGE_MAX:
        case IntValBranch::SEL_VALUES_MIN:
        case IntValBranch::SEL_VALUES_MAX:
            throw LDSBBadValueSelection("Int::LDSB::branch");
            break;
        case IntValBranch::SEL_VAL_COMMIT:
            if (vals.commit())
                throw LDSBBadValueSelection("Int::LDSB::branch");
            // If vals.commit() is valid, it means it will commit with
            // binary branching, which is OK for LDSB, so we
            // fall through
        default:
            // Construct mapping from each variable in the array to its index
            // in the array.
            VariableMap variable_map;
            for (int i = 0; i < x.size(); i++)
                variable_map[x[i].varimp()] = i;

            // Convert the modelling-level Symmetries object into an array of
            // SymmetryImp objects.
            int n = 0;
            SymmetryImp<IntView>** array = create_syms(home, syms, variable_map, create_int_sym, n);

            post_ldsb_brancher<IntView, 1, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals), array, n,
                                                   std::move(bf), std::move(vvp));
        }
    }

    void
    branch(Home home, const IntVarArgs& x, TieBreak<IntVarBranch> vars, const IntValBranch& vals,
           const Symmetries& syms, IntBranchFilter bf, IntVarValPrint vvp) {
        using namespace Int;
        if (home.failed())
            return;
        vars.a.expand(home, x);
        if ((vars.a.select() == IntVarBranch::SEL_NONE) || (vars.a.select() == IntVarBranch::SEL_RND))
            vars.b = INT_VAR_NONE();
        vars.b.expand(home, x);
        if ((vars.b.select() == IntVarBranch::SEL_NONE) || (vars.b.select() == IntVarBranch::SEL_RND))
            vars.c = INT_VAR_NONE();
        vars.c.expand(home, x);
        if ((vars.c.select() == IntVarBranch::SEL_NONE) || (vars.c.select() == IntVarBranch::SEL_RND))
            vars.d = INT_VAR_NONE();
        vars.d.expand(home, x);
        if (vars.b.select() == IntVarBranch::SEL_NONE) {
            branch(home, x, std::move(vars.a), vals, syms, std::move(bf), std::move(vvp));
        } else {
            // Construct mapping from each variable in the array to its index
            // in the array.
            VariableMap variable_map;
            for (int i = 0; i < x.size(); i++)
                variable_map[x[i].varimp()] = i;

            // Convert the modelling-level Symmetries object into an array of
            // SymmetryImp objects.
            int n = 0;
            SymmetryImp<IntView>** array = create_syms(home, syms, variable_map, create_int_sym, n);

            ViewArray<IntView> xv(home, x);
            if (vars.c.select() == IntVarBranch::SEL_NONE) {
                ViewSel<IntView>* vs[2] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b)};
                switch (vals.select()) {
                case IntValBranch::SEL_SPLIT_MIN:
                case IntValBranch::SEL_SPLIT_MAX:
                case IntValBranch::SEL_RANGE_MIN:
                case IntValBranch::SEL_RANGE_MAX:
                case IntValBranch::SEL_VALUES_MIN:
                case IntValBranch::SEL_VALUES_MAX:
                    throw LDSBBadValueSelection("Int::LDSB::branch");
                    break;
                case IntValBranch::SEL_VAL_COMMIT:
                    if (vals.commit())
                        throw LDSBBadValueSelection("Int::LDSB::branch");
                    // If vals.commit() is valid, it means it will commit with
                    // binary branching, which is OK for LDSB, so we
                    // fall through
                default:
                    post_ldsb_brancher<IntView, 2, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals),
                                                           array, n, std::move(bf), std::move(vvp));
                }
            } else if (vars.d.select() == IntVarBranch::SEL_NONE) {
                ViewSel<IntView>* vs[3] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c)};
                switch (vals.select()) {
                case IntValBranch::SEL_SPLIT_MIN:
                case IntValBranch::SEL_SPLIT_MAX:
                case IntValBranch::SEL_RANGE_MIN:
                case IntValBranch::SEL_RANGE_MAX:
                case IntValBranch::SEL_VALUES_MIN:
                case IntValBranch::SEL_VALUES_MAX:
                    throw LDSBBadValueSelection("Int::LDSB::branch");
                    break;
                case IntValBranch::SEL_VAL_COMMIT:
                    if (vals.commit())
                        throw LDSBBadValueSelection("Int::LDSB::branch");
                    // If vals.commit() is valid, it means it will commit with
                    // binary branching, which is OK for LDSB, so we
                    // fall through
                default:
                    post_ldsb_brancher<IntView, 3, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals),
                                                           array, n, std::move(bf), std::move(vvp));
                }
            } else {
                ViewSel<IntView>* vs[4] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c), Branch::view_selection(home, vars.d)};
                switch (vals.select()) {
                case IntValBranch::SEL_SPLIT_MIN:
                case IntValBranch::SEL_SPLIT_MAX:
                case IntValBranch::SEL_RANGE_MIN:
                case IntValBranch::SEL_RANGE_MAX:
                case IntValBranch::SEL_VALUES_MIN:
                case IntValBranch::SEL_VALUES_MAX:
                    throw LDSBBadValueSelection("Int::LDSB::branch");
                    break;
                case IntValBranch::SEL_VAL_COMMIT:
                    if (vals.commit())
                        throw LDSBBadValueSelection("Int::LDSB::branch");
                    // If vals.commit() is valid, it means it will commit with
                    // binary branching, which is OK for LDSB, so we
                    // fall through
                default:
                    post_ldsb_brancher<IntView, 4, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals),
                                                           array, n, std::move(bf), std::move(vvp));
                }
            }
        }
    }

    void
    branch(Home home, const BoolVarArgs& x, BoolVarBranch vars, const BoolValBranch& vals, const Symmetries& syms,
           BoolBranchFilter bf, BoolVarValPrint vvp) {
        using namespace Int;
        if (home.failed())
            return;
        vars.expand(home, x);
        ViewArray<BoolView> xv(home, x);
        ViewSel<BoolView>* vs[1] = {Branch::view_selection(home, vars)};

        // Construct mapping from each variable in the array to its index
        // in the array.
        VariableMap variable_map;
        for (int i = 0; i < x.size(); i++)
            variable_map[x[i].varimp()] = i;

        // Convert the modelling-level Symmetries object into an array of
        // SymmetryImp objects.
        int n = 0;
        SymmetryImp<BoolView>** array = create_syms(home, syms, variable_map, create_bool_sym, n);

        // Technically these "bad" value selection could in fact work with
        // LDSB, because they degenerate to binary splitting for
        // Booleans.  Nonetheless, we explicitly forbid them for
        // consistency with the integer version.
        switch (vals.select()) {
        case BoolValBranch::SEL_VAL_COMMIT:
            if (vals.commit())
                throw LDSBBadValueSelection("Int::LDSB::branch");
            // If vals.commit() is valid, it means it will commit with
            // binary branching, which is OK for LDSB, so we
            // fall through
        default:
            post_ldsb_brancher<BoolView, 1, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals), array, n,
                                                    std::move(bf), std::move(vvp));
        }
    }

    void
    branch(Home home, const BoolVarArgs& x, TieBreak<BoolVarBranch> vars, const BoolValBranch& vals,
           const Symmetries& syms, BoolBranchFilter bf, BoolVarValPrint vvp) {
        using namespace Int;
        if (home.failed())
            return;
        vars.a.expand(home, x);
        if ((vars.a.select() == BoolVarBranch::SEL_NONE) || (vars.a.select() == BoolVarBranch::SEL_RND))
            vars.b = BOOL_VAR_NONE();
        vars.b.expand(home, x);
        if ((vars.b.select() == BoolVarBranch::SEL_NONE) || (vars.b.select() == BoolVarBranch::SEL_RND))
            vars.c = BOOL_VAR_NONE();
        vars.c.expand(home, x);
        if ((vars.c.select() == BoolVarBranch::SEL_NONE) || (vars.c.select() == BoolVarBranch::SEL_RND))
            vars.d = BOOL_VAR_NONE();
        vars.d.expand(home, x);
        if (vars.b.select() == BoolVarBranch::SEL_NONE) {
            branch(home, x, std::move(vars.a), vals, syms, std::move(bf), std::move(vvp));
        } else {
            // Construct mapping from each variable in the array to its index
            // in the array.
            VariableMap variable_map;
            for (int i = 0; i < x.size(); i++)
                variable_map[x[i].varimp()] = i;

            // Convert the modelling-level Symmetries object into an array of
            // SymmetryImp objects.
            int n = 0;
            SymmetryImp<BoolView>** array = create_syms(home, syms, variable_map, create_bool_sym, n);

            // Technically these "bad" value selection could in fact work with
            // LDSB, because they degenerate to binary splitting for
            // Booleans.  Nonetheless, we explicitly forbid them for
            // consistency with the integer version.
            switch (vals.select()) {
            case BoolValBranch::SEL_VAL_COMMIT:
                if (vals.commit())
                    throw LDSBBadValueSelection("Int::LDSB::branch");
                // If vals.commit() is valid, it means it will commit with
                // binary branching, which is OK for LDSB, so we
                // fall through
            default:;
                // Do nothing and continue.
            }

            ViewArray<BoolView> xv(home, x);
            ValSelCommitBase<BoolView, int>* vsc = Branch::value_selection_commit(home, vals);
            if (vars.c.select() == BoolVarBranch::SEL_NONE) {
                ViewSel<BoolView>* vs[2] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b)};
                post_ldsb_brancher<BoolView, 2, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            } else if (vars.d.select() == BoolVarBranch::SEL_NONE) {
                ViewSel<BoolView>* vs[3] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                            Branch::view_selection(home, vars.c)};
                post_ldsb_brancher<BoolView, 3, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            } else {
                ViewSel<BoolView>* vs[4] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                            Branch::view_selection(home, vars.c), Branch::view_selection(home, vars.d)};
                post_ldsb_brancher<BoolView, 4, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            }
        }
    }

} // namespace Gecode

// STATISTICS: int-branch
