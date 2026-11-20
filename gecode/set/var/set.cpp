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

    SetVar::SetVar(Space& home) : VarImpVar<Set::SetVarImp>(new (home) Set::SetVarImp(home)) {}

    SetVar::SetVar(Space& home, int lb_min, int lb_max, int ub_min, int ub_max, unsigned int min_card,
                   unsigned int max_card)
        : VarImpVar<Set::SetVarImp>(new (home)
                                        Set::SetVarImp(home, lb_min, lb_max, ub_min, ub_max, min_card, max_card)) {
        Set::Limits::check(lb_min, "SetVar::SetVar");
        Set::Limits::check(lb_max, "SetVar::SetVar");
        Set::Limits::check(ub_min, "SetVar::SetVar");
        Set::Limits::check(ub_max, "SetVar::SetVar");
        Set::Limits::check(max_card, "SetVar::SetVar");
        if (min_card > max_card || min_card > lub_size() || max_card < glb_size() || lb_min < ub_min || lb_max > ub_max)
            throw Set::VariableEmptyDomain("SetVar::SetVar");
    }

    SetVar::SetVar(Space& home, const IntSet& glb, int ub_min, int ub_max, unsigned int min_card, unsigned int max_card)
        : VarImpVar<Set::SetVarImp>(new (home) Set::SetVarImp(home, glb, ub_min, ub_max, min_card, max_card)) {
        Set::Limits::check(glb, "SetVar::SetVar");
        Set::Limits::check(ub_min, "SetVar::SetVar");
        Set::Limits::check(ub_max, "SetVar::SetVar");
        Set::Limits::check(max_card, "SetVar::SetVar");
        if (min_card > max_card || min_card > lub_size() || max_card < glb_size() || glb.min() < ub_min ||
            glb.max() > ub_max)
            throw Set::VariableEmptyDomain("SetVar::SetVar");
    }

    SetVar::SetVar(Space& home, int lb_min, int lb_max, const IntSet& lub, unsigned int min_card, unsigned int max_card)
        : VarImpVar<Set::SetVarImp>(new (home) Set::SetVarImp(home, lb_min, lb_max, lub, min_card, max_card)) {
        Set::Limits::check(lb_min, "SetVar::SetVar");
        Set::Limits::check(lb_max, "SetVar::SetVar");
        Set::Limits::check(lub, "SetVar::SetVar");
        Set::Limits::check(max_card, "SetVar::SetVar");
        Iter::Ranges::Singleton glbr(lb_min, lb_max);
        IntSetRanges lubr(lub);
        if (min_card > max_card || min_card > lub_size() || max_card < glb_size() || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVar::SetVar");
    }

    SetVar::SetVar(Space& home, const IntSet& glb, const IntSet& lub, unsigned int min_card, unsigned int max_card)
        : VarImpVar<Set::SetVarImp>(new (home) Set::SetVarImp(home, glb, lub, min_card, max_card)) {
        Set::Limits::check(glb, "SetVar::SetVar");
        Set::Limits::check(lub, "SetVar::SetVar");
        Set::Limits::check(max_card, "SetVar::SetVar");
        IntSetRanges glbr(glb);
        IntSetRanges lubr(lub);
        if (min_card > max_card || min_card > lub_size() || max_card < glb_size() || !Iter::Ranges::subset(glbr, lubr))
            throw Set::VariableEmptyDomain("SetVar::SetVar");
    }

} // namespace Gecode

// STATISTICS: set-var
