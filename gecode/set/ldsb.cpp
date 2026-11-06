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

#include <gecode/set/ldsb.hh>
#include <gecode/set/branch.hh>
#include <map>

namespace Gecode {
    using namespace Int::LDSB;
    /*
     * Implementation of some SymmetryHandle methods.
     */

    SymmetryHandle
    variable_symmetry(const SetVarArgs& x) {
        ArgArray<VarImpBase*> a(x.size());
        for (int i = 0; i < x.size(); i++)
            a[i] = x[i].varimp();
        return SymmetryHandle(new VariableSymmetryObject(a));
    }
    SymmetryHandle
    variable_sequence_symmetry(const SetVarArgs& x, int ss) {
        ArgArray<VarImpBase*> a(x.size());
        for (int i = 0; i < x.size(); i++)
            a[i] = x[i].varimp();
        return SymmetryHandle(new VariableSequenceSymmetryObject(a, ss));
    }
} // namespace Gecode

namespace Gecode::Int::LDSB {
    template <>
    ModEvent
    prune<Set::SetView>(Space& home, Set::SetView x, int v) {
        return x.exclude(home, v);
    }
} // namespace Gecode::Int::LDSB

namespace Gecode::Set::LDSB {

    /// Map from variable implementation to index
    class VariableMap : public std::map<VarImpBase*, int> {};

    /*
     * The create_set_sym function is an almost exact copy of
     * create_int_sym/create_bool_sym.
     */
    static SymmetryImp<SetView>*
    create_set_sym(Space& home, const SymmetryHandle& s, VariableMap variable_map) {
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
                    throw Int::LDSBUnbranchedVariable("VariableSymmetryObject::createSet");
                indices[i] = index->second;
            }
            return new (home) VariableSymmetryImp<SetView>(home, indices, n);
        }
        if (valref) {
            int n = valref->values.size();
            int* vs = home.alloc<int>(n);
            int i = 0;
            for (IntSetValues v(valref->values); v(); ++v) {
                vs[i] = v.val();
                i++;
            }
            return new (home) ValueSymmetryImp<SetView>(home, vs, n);
        }
        if (varseqref) {
            int n = varseqref->nxs;
            int* indices = home.alloc<int>(n);
            for (int i = 0; i < n; i++) {
                VariableMap::const_iterator index = variable_map.find(varseqref->xs[i]);
                if (index == variable_map.end())
                    throw Int::LDSBUnbranchedVariable("VariableSequenceSymmetryObject::createSet");
                indices[i] = index->second;
            }
            return new (home) VariableSequenceSymmetryImp<SetView>(home, indices, n, varseqref->seq_size);
        }
        if (valseqref) {
            unsigned int n = valseqref->values.size();
            int* vs = home.alloc<int>(n);
            for (unsigned int i = 0; i < n; i++)
                vs[i] = valseqref->values[i];
            return new (home) ValueSequenceSymmetryImp<SetView>(home, vs, n, valseqref->seq_size);
        }
        GECODE_NEVER;
        return nullptr;
    }

    /**
     * \brief Create implementations of the non-empty symmetries in \a syms
     *
     * Stores their number in \a nsyms and returns nullptr if there are none.
     */
    static SymmetryImp<SetView>**
    create_set_syms(Space& home, const Symmetries& syms, const VariableMap& variable_map, int& nsyms) {
        nsyms = 0;
        for (const auto& sym : syms)
            if (!is_empty(sym))
                nsyms++;
        if (nsyms == 0)
            return nullptr;
        SymmetryImp<SetView>** array = home.alloc<SymmetryImp<SetView>*>(nsyms);
        int created = 0;
        for (const auto& sym : syms)
            if (!is_empty(sym))
                array[created++] = create_set_sym(home, sym, variable_map);
        return array;
    }
} // namespace Gecode::Set::LDSB

namespace Gecode {

    using namespace Set::LDSB;

    void
    branch(Home home, const SetVarArgs& x, SetVarBranch vars, const SetValBranch& vals, const Symmetries& syms,
           SetBranchFilter bf, SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.expand(home, x);
        ViewArray<SetView> xv(home, x);
        ViewSel<SetView>* vs[1] = {Branch::view_selection(home, vars)};

        // Construct mapping from each variable in the array to its index
        // in the array.
        VariableMap variable_map;
        for (int i = 0; i < x.size(); i++)
            variable_map[x[i].varimp()] = i;

        // Convert the modelling-level Symmetries object into an array of
        // SymmetryImp objects.
        int n = 0;
        SymmetryImp<SetView>** array = create_set_syms(home, syms, variable_map, n);

        post_ldsb_set_brancher<SetView, 1, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals), array, n,
                                                   std::move(bf), std::move(vvp));
    }

    void
    branch(Home home, const SetVarArgs& x, TieBreak<SetVarBranch> vars, const SetValBranch& vals,
           const Symmetries& syms, SetBranchFilter bf, SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.a.expand(home, x);
        if ((vars.a.select() == SetVarBranch::SEL_NONE) || (vars.a.select() == SetVarBranch::SEL_RND))
            vars.b = SET_VAR_NONE();
        vars.b.expand(home, x);
        if ((vars.b.select() == SetVarBranch::SEL_NONE) || (vars.b.select() == SetVarBranch::SEL_RND))
            vars.c = SET_VAR_NONE();
        vars.c.expand(home, x);
        if ((vars.c.select() == SetVarBranch::SEL_NONE) || (vars.c.select() == SetVarBranch::SEL_RND))
            vars.d = SET_VAR_NONE();
        vars.d.expand(home, x);
        if (vars.b.select() == SetVarBranch::SEL_NONE) {
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
            SymmetryImp<SetView>** array = create_set_syms(home, syms, variable_map, n);

            ViewArray<SetView> xv(home, x);
            ValSelCommitBase<SetView, int>* vsc = Branch::value_selection_commit(home, vals);
            if (vars.c.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[2] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b)};
                post_ldsb_set_brancher<SetView, 2, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            } else if (vars.d.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[3] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c)};
                post_ldsb_set_brancher<SetView, 3, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            } else {
                ViewSel<SetView>* vs[4] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c), Branch::view_selection(home, vars.d)};
                post_ldsb_set_brancher<SetView, 4, int, 2>(home, xv, vs, vsc, array, n, std::move(bf), std::move(vvp));
            }
        }
    }

} // namespace Gecode

// STATISTICS: set-branch
