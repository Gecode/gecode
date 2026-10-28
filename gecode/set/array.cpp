/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *     Gabor Szokoli <szokoli@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#include <gecode/set.hh>

namespace Gecode {

    SetVarArray::SetVarArray(Space& home, int n) : VarArray<SetVar>(home, n) {
        for (int i = size(); i--;)
            x[i] = SetVar(home);
    }

    SetVarArray::SetVarArray(Space& home, int n, int lb_min, int lb_max, int ub_min, int ub_max, unsigned int min_card,
                             unsigned int max_card)
        : VarArray<SetVar>(home, n) {
        Set::Limits::check(lb_min, "SetVarArray::SetVarArray");
        Set::Limits::check(lb_max, "SetVarArray::SetVarArray");
        Set::Limits::check(ub_min, "SetVarArray::SetVarArray");
        Set::Limits::check(ub_max, "SetVarArray::SetVarArray");
        Set::Limits::check(max_card, "SetVarArray::SetVarArray");
        unsigned int glb_size = (lb_min <= lb_max ? static_cast<unsigned int>(lb_max - lb_min + 1) : 0U);
        unsigned int lub_size = (ub_min <= ub_max ? static_cast<unsigned int>(ub_max - ub_min + 1) : 0U);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || lb_min < ub_min || lb_max > ub_max)
            throw Set::VariableEmptyDomain("SetVarArray::SetVarArray");
        for (int i = size(); i--;)
            x[i] = SetVar(home, lb_min, lb_max, ub_min, ub_max, min_card, max_card);
    }

    SetVarArray::SetVarArray(Space& home, int n, const IntSet& glb, int ub_min, int ub_max, unsigned int min_card,
                             unsigned int max_card)
        : VarArray<SetVar>(home, n) {
        Set::Limits::check(glb, "SetVarArray::SetVarArray");
        Set::Limits::check(ub_min, "SetVarArray::SetVarArray");
        Set::Limits::check(ub_max, "SetVarArray::SetVarArray");
        Set::Limits::check(max_card, "SetVarArray::SetVarArray");
        IntSetRanges glbr(glb);
        unsigned int glb_size = Iter::Ranges::size(glbr);
        unsigned int lub_size = (ub_min <= ub_max ? static_cast<unsigned int>(ub_max - ub_min + 1) : 0U);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || glb.min() < ub_min ||
            glb.max() > ub_max)
            throw Set::VariableEmptyDomain("SetVarArray::SetVarArray");
        for (int i = size(); i--;)
            x[i] = SetVar(home, glb, ub_min, ub_max, min_card, max_card);
    }

    SetVarArray::SetVarArray(Space& home, int n, int lb_min, int lb_max, const IntSet& lub, unsigned int min_card,
                             unsigned int max_card)
        : VarArray<SetVar>(home, n) {
        Set::Limits::check(lb_min, "SetVarArray::SetVarArray");
        Set::Limits::check(lb_max, "SetVarArray::SetVarArray");
        Set::Limits::check(lub, "SetVarArray::SetVarArray");
        Set::Limits::check(max_card, "SetVarArray::SetVarArray");
        Iter::Ranges::Singleton glbr(lb_min, lb_max);
        IntSetRanges lubr(lub);
        IntSetRanges lubr_s(lub);
        unsigned int glb_size = (lb_min <= lb_max ? static_cast<unsigned int>(lb_max - lb_min + 1) : 0U);
        unsigned int lub_size = Iter::Ranges::size(lubr_s);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVarArray::SetVarArray");
        for (int i = size(); i--;)
            x[i] = SetVar(home, lb_min, lb_max, lub, min_card, max_card);
    }

    SetVarArray::SetVarArray(Space& home, int n, const IntSet& glb, const IntSet& lub, unsigned int min_card,
                             unsigned int max_card)
        : VarArray<SetVar>(home, n) {
        Set::Limits::check(glb, "SetVarArray::SetVarArray");
        Set::Limits::check(lub, "SetVarArray::SetVarArray");
        Set::Limits::check(max_card, "SetVarArray::SetVarArray");
        IntSetRanges glbr(glb);
        IntSetRanges glbr_s(glb);
        unsigned int glb_size = Iter::Ranges::size(glbr_s);
        IntSetRanges lubr(lub);
        IntSetRanges lubr_s(lub);
        unsigned int lub_size = Iter::Ranges::size(lubr_s);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVar");
        for (int i = size(); i--;)
            x[i] = SetVar(home, glb, lub, min_card, max_card);
    }

    SetVarArgs::SetVarArgs(Space& home, int n, int lb_min, int lb_max, int ub_min, int ub_max, unsigned int min_card,
                           unsigned int max_card)
        : VarArgArray<SetVar>(n) {
        Set::Limits::check(lb_min, "SetVarArgs::SetVarArgs");
        Set::Limits::check(lb_max, "SetVarArgs::SetVarArgs");
        Set::Limits::check(ub_min, "SetVarArgs::SetVarArgs");
        Set::Limits::check(ub_max, "SetVarArgs::SetVarArgs");
        Set::Limits::check(max_card, "SetVarArgs::SetVarArgs");
        unsigned int glb_size = (lb_min <= lb_max ? static_cast<unsigned int>(lb_max - lb_min + 1) : 0U);
        unsigned int lub_size = (ub_min <= ub_max ? static_cast<unsigned int>(ub_max - ub_min + 1) : 0U);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || lb_min < ub_min || lb_max > ub_max)
            throw Set::VariableEmptyDomain("SetVarArgs::SetVarArgs");
        for (int i = size(); i--;)
            a[i] = SetVar(home, lb_min, lb_max, ub_min, ub_max, min_card, max_card);
    }

    SetVarArgs::SetVarArgs(Space& home, int n, const IntSet& glb, int ub_min, int ub_max, unsigned int min_card,
                           unsigned int max_card)
        : VarArgArray<SetVar>(n) {
        Set::Limits::check(glb, "SetVarArgs::SetVarArgs");
        Set::Limits::check(ub_min, "SetVarArgs::SetVarArgs");
        Set::Limits::check(ub_max, "SetVarArgs::SetVarArgs");
        Set::Limits::check(max_card, "SetVarArgs::SetVarArgs");
        IntSetRanges glbr(glb);
        unsigned int glb_size = Iter::Ranges::size(glbr);
        unsigned int lub_size = (ub_min <= ub_max ? static_cast<unsigned int>(ub_max - ub_min + 1) : 0U);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || glb.min() < ub_min ||
            glb.max() > ub_max)
            throw Set::VariableEmptyDomain("SetVarArgs::SetVarArgs");
        for (int i = size(); i--;)
            a[i] = SetVar(home, glb, ub_min, ub_max, min_card, max_card);
    }

    SetVarArgs::SetVarArgs(Space& home, int n, int lb_min, int lb_max, const IntSet& lub, unsigned int min_card,
                           unsigned int max_card)
        : VarArgArray<SetVar>(n) {
        Set::Limits::check(lb_min, "SetVarArgs::SetVarArgs");
        Set::Limits::check(lb_max, "SetVarArgs::SetVarArgs");
        Set::Limits::check(lub, "SetVarArgs::SetVarArgs");
        Set::Limits::check(max_card, "SetVarArgs::SetVarArgs");
        Iter::Ranges::Singleton glbr(lb_min, lb_max);
        IntSetRanges lubr(lub);
        IntSetRanges lubr_s(lub);
        unsigned int glb_size = (lb_min <= lb_max ? static_cast<unsigned int>(lb_max - lb_min + 1) : 0U);
        unsigned int lub_size = Iter::Ranges::size(lubr_s);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVarArgs::SetVarArgs");
        for (int i = size(); i--;)
            a[i] = SetVar(home, lb_min, lb_max, lub, min_card, max_card);
    }

    SetVarArgs::SetVarArgs(Space& home, int n, const IntSet& glb, const IntSet& lub, unsigned int min_card,
                           unsigned int max_card)
        : VarArgArray<SetVar>(n) {
        Set::Limits::check(glb, "SetVarArgs::SetVarArgs");
        Set::Limits::check(lub, "SetVarArgs::SetVarArgs");
        Set::Limits::check(max_card, "SetVarArgs::SetVarArgs");
        IntSetRanges glbr(glb);
        IntSetRanges glbr_s(glb);
        unsigned int glb_size = Iter::Ranges::size(glbr_s);
        IntSetRanges lubr(lub);
        IntSetRanges lubr_s(lub);
        unsigned int lub_size = Iter::Ranges::size(lubr_s);
        if (min_card > max_card || min_card > lub_size || max_card < glb_size || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVar");
        for (int i = size(); i--;)
            a[i] = SetVar(home, glb, lub, min_card, max_card);
    }

} // namespace Gecode

// STATISTICS: set-other
