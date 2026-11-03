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

namespace Gecode {

    /*
     * Constructors and access
     *
     */

    forceinline
    SetVar::SetVar(void) {}

    forceinline
    SetVar::SetVar(const Set::SetView& y)
        : VarImpVar<Set::SetVarImp>(y.varimp()) {}

    /*
     * Variable information
     *
     */

    forceinline unsigned int
    SetVar::glb_size(void) const {
        return x->glb_size();
    }

    forceinline unsigned int
    SetVar::lub_size(void) const {
        return x->lub_size();
    }

    forceinline unsigned int
    SetVar::unknown_size(void) const {
        return x->lub_size() - x->glb_size();
    }

    forceinline bool
    SetVar::contains(int i) const {
        return x->known_in(i);
    }

    forceinline bool
    SetVar::not_contains(int i) const {
        return x->known_out(i);
    }

    forceinline unsigned int
    SetVar::card_min(void) const {
        return x->card_min();
    }

    forceinline unsigned int
    SetVar::card_max(void) const {
        return x->card_max();
    }

    forceinline int
    SetVar::lub_min(void) const {
        return x->lub_min();
    }

    forceinline int
    SetVar::lub_max(void) const {
        return x->lub_max();
    }

    forceinline int
    SetVar::glb_min(void) const {
        return x->glb_min();
    }

    forceinline int
    SetVar::glb_max(void) const {
        return x->glb_max();
    }

    /*
     * Range and value iterators for set variables
     *
     */

    forceinline
    SetVarGlbRanges::SetVarGlbRanges(void) {}

    forceinline
    SetVarGlbRanges::SetVarGlbRanges(const SetVar& s)
        : iter(s.varimp()) {}

    forceinline bool
    SetVarGlbRanges::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarGlbRanges::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarGlbRanges::min(void) const {
        return iter.min();
    }

    forceinline int
    SetVarGlbRanges::max(void) const {
        return iter.max();
    }

    forceinline unsigned int
    SetVarGlbRanges::width(void) const {
        return iter.width();
    }

    forceinline
    SetVarLubRanges::SetVarLubRanges(void) {}

    forceinline
    SetVarLubRanges::SetVarLubRanges(const SetVar& s)
        : iter(s.varimp()) {}

    forceinline bool
    SetVarLubRanges::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarLubRanges::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarLubRanges::min(void) const {
        return iter.min();
    }

    forceinline int
    SetVarLubRanges::max(void) const {
        return iter.max();
    }

    forceinline unsigned int
    SetVarLubRanges::width(void) const {
        return iter.width();
    }

    forceinline
    SetVarUnknownRanges::SetVarUnknownRanges(void) {}

    forceinline
    SetVarUnknownRanges::SetVarUnknownRanges(const SetVar& s) {
        iter.init(s.varimp());
    }

    forceinline bool
    SetVarUnknownRanges::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarUnknownRanges::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarUnknownRanges::min(void) const {
        return iter.min();
    }

    forceinline int
    SetVarUnknownRanges::max(void) const {
        return iter.max();
    }

    forceinline unsigned int
    SetVarUnknownRanges::width(void) const {
        return iter.width();
    }

    forceinline
    SetVarGlbValues::SetVarGlbValues(const SetVar& x) {
        SetVarGlbRanges ivr(x);
        iter.init(ivr);
    }

    forceinline bool
    SetVarGlbValues::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarGlbValues::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarGlbValues::val(void) const {
        return iter.val();
    }

    forceinline
    SetVarLubValues::SetVarLubValues(const SetVar& x) {
        SetVarLubRanges ivr(x);
        iter.init(ivr);
    }

    forceinline bool
    SetVarLubValues::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarLubValues::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarLubValues::val(void) const {
        return iter.val();
    }

    forceinline
    SetVarUnknownValues::SetVarUnknownValues(const SetVar& x) {
        SetVarUnknownRanges ivr(x);
        iter.init(ivr);
    }

    forceinline bool
    SetVarUnknownValues::operator()(void) const {
        return iter();
    }

    forceinline void
    SetVarUnknownValues::operator++(void) {
        ++iter;
    }

    forceinline int
    SetVarUnknownValues::val(void) const {
        return iter.val();
    }

} // namespace Gecode

// STATISTICS: set-var
