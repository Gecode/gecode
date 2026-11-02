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

namespace Gecode { namespace Set { namespace LDSB {

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::LDSBSetBrancher(
        Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc, SymmetryImp<View>** syms,
        int nsyms, BranchFilter<typename LDSBSetBrancher<View, n, Val, a, Filter, Print>::Var> bf,
        VarValPrint<typename LDSBSetBrancher<View, n, Val, a, Filter, Print>::Var, Val> vvp)
        : LDSBBrancher<View, n, Val, a, Filter, Print>(home, x, vs, vsc, syms, nsyms, std::move(bf), std::move(vvp)),
          _prev_pos(-1), _copied_syms(nullptr), _n_copied_syms(0), _stable(false) {
        // Put all the value symmetries at the end of the list.
        int seen = 0;
        int dest = this->_nsyms - 1;
        for (int i = 0; i < this->_nsyms - seen; i++) {
            if (dynamic_cast<ValueSymmetryImp<View>*>(this->_syms[i])) {
                SymmetryImp<View>* t = this->_syms[i];
                this->_syms[i] = this->_syms[dest];
                this->_syms[dest] = t;
                dest--;
                seen++;
            }
        }
        _n_value_symmetries = seen;
        _n_non_value_symmetries = this->_nsyms - seen;
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::LDSBSetBrancher(Space& home,
                                                                     LDSBSetBrancher<View, n, Val, a, Filter, Print>& b)
        : LDSBBrancher<View, n, Val, a, Filter, Print>(home, b), _prev_pos(b._prev_pos),
          _n_non_value_symmetries(b._n_non_value_symmetries), _n_value_symmetries(b._n_value_symmetries),
          _n_copied_syms(b._n_copied_syms), _left_branch_values(b._left_branch_values), _stable(b._stable) {
        if (_n_copied_syms > 0) {
            _copied_syms = home.alloc<ValueSymmetryImp<View>*>(_n_copied_syms);
            for (int i = 0; i < _n_copied_syms; i++)
                _copied_syms[i] = static_cast<ValueSymmetryImp<View>*>(b._copied_syms[i]->copy(home));
        } else {
            _copied_syms = nullptr;
        }
    }

    /**
     * \brief Bulk update of a value symmetry \a s, using \a usedValues
     *
     * Calculates the intersection and difference of the values in the
     * symmetry and \a usedValues, updates the symmetry to eliminate the
     * used values, and makes a new symmetry containing the intersection
     * values, if there are at least two.  Returns the new symmetry, or
     * nullptr if the intersection has fewer than two elements.
     */
    template <class View>
    ValueSymmetryImp<View>*
    special_update(Space& home, ValueSymmetryImp<View>* s, IntSet used_values) {
        // Calculate intersection and difference.
        IntArgs intersection;
        IntArgs difference;
        for (int i = s->values.next(s->values.offset()); i <= s->values.max_bit(); i = s->values.next(i + 1)) {
            if (used_values.in(i))
                intersection << i;
            else
                difference << i;
        }

        for (IntSetValues v(used_values); v(); ++v) {
            s->update(Literal(0, v.val()));
        }

        if (intersection.size() < 2)
            return nullptr;
        int* a = new int[intersection.size()];
        for (int i = 0; i < intersection.size(); i++) {
            a[i] = intersection[i];
        }
        ValueSymmetryImp<View>* ns = new (home) ValueSymmetryImp<View>(home, a, intersection.size());
        delete[] a;
        return ns;
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    void
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::update_part1(Space& home, int choice_pos) {
        if (_n_value_symmetries > 0) {
            // If this is a different variable from the last commit, restore
            // the old value symmetries and update the copy.
            if (choice_pos != _prev_pos) {
                if (_prev_pos != -1) {
                    int i = 0;
                    for (int j = _n_non_value_symmetries; j < this->_nsyms; j++) {
                        this->_syms[j] = _copied_syms[i];
                        i++;
                    }

                    for (int i = 0; i < _n_copied_syms; i++) {
                        ValueSymmetryImp<View>* ns = special_update(home, _copied_syms[i], _left_branch_values);
                        if (ns) {
                            this->_syms = home.realloc<SymmetryImp<View>*>(this->_syms, this->_nsyms, this->_nsyms + 1);
                            this->_syms[this->_nsyms] = ns;
                            this->_nsyms++;
                            this->_n_value_symmetries++;
                        }
                    }
                }

                // Reset for current variable, make copy of value symmetries
                _left_branch_values = IntSet::empty;
                _prev_pos = choice_pos;
                if (_n_copied_syms > 0)
                    home.free(_copied_syms, _n_copied_syms);
                _n_copied_syms = _n_value_symmetries;
                _copied_syms = home.alloc<ValueSymmetryImp<View>*>(_n_copied_syms);
                {
                    int i = 0;
                    for (int j = _n_non_value_symmetries; j < this->_nsyms; j++) {
                        ValueSymmetryImp<View>* vsi = static_cast<ValueSymmetryImp<View>*>(this->_syms[j]);
                        _copied_syms[i] = static_cast<ValueSymmetryImp<View>*>(vsi->copy(home));
                        i++;
                    }
                }
            }
        }
    }

    // Compute choice
    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    const Choice*
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::choice(Space& home) {
        // Making the PVC here is not so nice, I think.
        const Choice* c = ViewValBrancher<View, n, Val, a, Filter, Print>::choice(home);
        const PosValChoice<Val>* pvc = static_cast<const PosValChoice<Val>*>(c);

        // Compute symmetries.

        int choice_pos = pvc->pos().pos;
        delete c;

        assert(!_stable);
        update_part1(home, choice_pos);

        return LDSBBrancher<View, n, Val, a, Filter, Print>::choice(home);
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    ExecStatus
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::commit(Space& home, const Choice& c, unsigned int b) {
        const LDSBChoice<Val>& pvc = static_cast<const LDSBChoice<Val>&>(c);
        int choice_pos = pvc.pos().pos;
        int choice_val = pvc.val();

        if (!_stable)
            update_part1(home, choice_pos);

        if (b == 0) {
            IntArgs ia;
            for (IntSetValues v(_left_branch_values); v(); ++v) {
                ia << v.val();
            }
            ia << choice_val;
            _left_branch_values = IntSet(ia);

            // Post the branching constraint.
            ExecStatus from_base = ViewValBrancher<View, n, Val, a, Filter, Print>::commit(home, c, b);
            GECODE_ES_CHECK(from_base);
            for (int i = 0; i < this->_nsyms; i++)
                this->_syms[i]->update(Literal(choice_pos, choice_val));
        } else if (b == 1) {
            // Post the branching constraint.
            ExecStatus from_base = ViewValBrancher<View, n, Val, a, Filter, Print>::commit(home, c, b);
            GECODE_ES_CHECK(from_base);

            // Post prunings.
            int nliterals = pvc.nliterals();
            const Literal* literals = pvc.literals();
            for (int i = 0; i < nliterals; i++) {
                const Literal& l = literals[i];
                ModEvent me = prune<View>(home, this->x[l._variable], l._value);
                GECODE_ME_CHECK(me);
            }
        }

        return ES_OK;
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    Actor*
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::copy(Space& home) {
        return new (home) LDSBSetBrancher<View, n, Val, a, Filter, Print>(home, *this);
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    size_t
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::dispose(Space& home) {
        _left_branch_values.~IntSet();
        (void)LDSBBrancher<View, n, Val, a, Filter, Print>::dispose(home);
        return sizeof(LDSBSetBrancher<View, n, Val, a, Filter, Print>);
    }

    template <class View, int n, class Val, unsigned int a, class Filter, class Print>
    forceinline void
    LDSBSetBrancher<View, n, Val, a, Filter, Print>::post(
        Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc, SymmetryImp<View>** syms,
        int nsyms, BranchFilter<typename LDSBSetBrancher<View, n, Val, a, Filter, Print>::Var> bf,
        VarValPrint<typename LDSBSetBrancher<View, n, Val, a, Filter, Print>::Var, Val> vvp) {
        (void)new (home) LDSBSetBrancher<View, n, Val, a, Filter, Print>(home, x, vs, vsc, syms, nsyms, std::move(bf),
                                                                         std::move(vvp));
    }

    template <class View, int n, class Val, unsigned int a>
    forceinline void
    post_ldsb_set_brancher(Home home, ViewArray<View>& x, ViewSel<View>* vs[n], ValSelCommitBase<View, Val>* vsc,
                           SymmetryImp<View>** syms, int nsyms, BranchFilter<typename View::VarType> bf,
                           VarValPrint<typename View::VarType, Val> vvp) {
        if (bf) {
            if (vvp) {
                LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherPrint<View, Val>>::post(
                    home, x, vs, vsc, syms, nsyms, std::move(bf), std::move(vvp));
            } else {
                LDSBSetBrancher<View, n, Val, a, BrancherFilter<View>, BrancherNoPrint<View, Val>>::post(
                    home, x, vs, vsc, syms, nsyms, std::move(bf), std::move(vvp));
            }
        } else {
            if (vvp) {
                LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherPrint<View, Val>>::post(
                    home, x, vs, vsc, syms, nsyms, std::move(bf), std::move(vvp));
            } else {
                LDSBSetBrancher<View, n, Val, a, BrancherNoFilter<View>, BrancherNoPrint<View, Val>>::post(
                    home, x, vs, vsc, syms, nsyms, std::move(bf), std::move(vvp));
            }
        }
    }

}}} // namespace Gecode::Set::LDSB

// STATISTICS: set-branch
