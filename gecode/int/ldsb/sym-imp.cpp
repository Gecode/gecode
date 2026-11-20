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

namespace Gecode::Int::LDSB {
    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    VariableSymmetryImp<IntView>::symmetric(Literal l, const ViewArray<IntView>& x) const {
        (void)x;
        if (indices.valid(l._variable) && indices.get(l._variable)) {
            int n = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(indices); i(); ++i)
                n++;
            ArgArray<Literal> lits(n);
            int j = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(indices); i(); ++i)
                lits[j++] = Literal(i.val(), l._value);
            return lits;
        }
        return ArgArray<Literal>(0);
    }
    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    VariableSymmetryImp<BoolView>::symmetric(Literal l, const ViewArray<BoolView>& x) const {
        (void)x;
        if (indices.valid(l._variable) && indices.get(l._variable)) {
            int n = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(indices); i(); ++i)
                n++;
            ArgArray<Literal> lits(n);
            int j = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(indices); i(); ++i)
                lits[j++] = Literal(i.val(), l._value);
            return lits;
        }
        return ArgArray<Literal>(0);
    }

    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    ValueSymmetryImp<IntView>::symmetric(Literal l, const ViewArray<IntView>& x) const {
        (void)x;
        if (values.valid(l._value) && values.get(l._value)) {
            int n = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(values); i(); ++i)
                n++;
            ArgArray<Literal> lits(n);
            int j = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(values); i(); ++i)
                lits[j++] = Literal(l._variable, i.val());
            return lits;
        }
        return ArgArray<Literal>(0);
    }
    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    ValueSymmetryImp<BoolView>::symmetric(Literal l, const ViewArray<BoolView>& x) const {
        (void)x;
        if (values.valid(l._value) && values.get(l._value)) {
            int n = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(values); i(); ++i)
                n++;
            ArgArray<Literal> lits(n);
            int j = 0;
            for (Iter::Values::BitSetOffset<Support::BitSetOffset<Space>> i(values); i(); ++i)
                lits[j++] = Literal(l._variable, i.val());
            return lits;
        }
        return ArgArray<Literal>(0);
    }

    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    ValueSequenceSymmetryImp<Int::IntView>::symmetric(Literal l, const ViewArray<IntView>& x) const {
        (void)x;
        Region region;
        Support::DynamicStack<Literal, Region> s(region);
        std::pair<int, int> location = find_var(values, n_values, seq_size, l._value);
        if (location.first == -1)
            return dynamic_stack_to_arg_array(s);
        unsigned int seq_num = location.first;
        unsigned int seq_pos = location.second;
        if (!dead_sequences.get(seq_num)) {
            for (unsigned int seq = 0; seq < n_seqs; seq++) {
                if (seq == seq_num)
                    continue;
                if (dead_sequences.get(seq))
                    continue;
                s.push(Literal(l._variable, get_val(seq, seq_pos)));
            }
        }
        return dynamic_stack_to_arg_array(s);
    }
    /// Compute symmetric literals
    template <>
    ArgArray<Literal>
    ValueSequenceSymmetryImp<BoolView>::symmetric(Literal l, const ViewArray<BoolView>& x) const {
        (void)x;
        Region region;
        Support::DynamicStack<Literal, Region> s(region);
        std::pair<int, int> location = find_var(values, n_values, seq_size, l._value);
        if (location.first == -1)
            return dynamic_stack_to_arg_array(s);
        unsigned int seq_num = location.first;
        unsigned int seq_pos = location.second;
        if (!dead_sequences.get(seq_num)) {
            for (unsigned int seq = 0; seq < n_seqs; seq++) {
                if (seq == seq_num)
                    continue;
                if (dead_sequences.get(seq))
                    continue;
                s.push(Literal(l._variable, get_val(seq, seq_pos)));
            }
        }
        return dynamic_stack_to_arg_array(s);
    }

} // namespace Gecode::Int::LDSB

// STATISTICS: int-branch
