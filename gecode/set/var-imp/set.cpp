/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Set {

    /*
     * "Standard" tell operations
     *
     */
    ModEvent
    SetVarImp::card_min_full(Space& home) {
        ModEvent me = ME_SET_CARD;
        if (card_min() == lub.size()) {
            glb.become(home, lub);
            me = ME_SET_VAL;
        }
        SetDelta d;
        return notify(home, me, d);
    }

    ModEvent
    SetVarImp::card_max_full(Space& home) {
        ModEvent me = ME_SET_CARD;
        if (card_max() == glb.size()) {
            lub.become(home, glb);
            me = ME_SET_VAL;
        }
        SetDelta d;
        return notify(home, me, d);
    }

    ModEvent
    SetVarImp::process_lub_change(Space& home, SetDelta& d) {
        ModEvent me = ME_SET_LUB;
        if (card_max() > lub.size()) {
            lub.card(lub.size());
            if (card_min() > card_max()) {
                glb.become(home, lub);
                glb.card(glb.size());
                lub.card(glb.size());
                return fail(home);
            }
            me = ME_SET_CLUB;
        }
        if (card_max() == lub.size() && card_min() == card_max()) {
            glb.become(home, lub);
            me = ME_SET_VAL;
            assert(d.glb_min() == 1);
            assert(d.glb_max() == 0);
        }
        return notify(home, me, d);
    }

    ModEvent
    SetVarImp::process_glb_change(Space& home, SetDelta& d) {
        ModEvent me = ME_SET_GLB;
        if (card_min() < glb.size()) {
            glb.card(glb.size());
            if (card_min() > card_max()) {
                glb.become(home, lub);
                glb.card(glb.size());
                lub.card(glb.size());
                return fail(home);
            }
            me = ME_SET_CGLB;
        }
        if (card_min() == glb.size() && card_min() == card_max()) {
            lub.become(home, glb);
            me = ME_SET_VAL;
            assert(d.lub_min() == 1);
            assert(d.lub_max() == 0);
        }
        return notify(home, me, d);
    }

    /*
     * Copying variables
     *
     */

    forceinline
    SetVarImp::SetVarImp(Space& home, SetVarImp& x)
        : SetVarImpBase(home, x) {
        lub.update(home, x.lub);
        glb.card(x.card_min());
        lub.card(x.card_max());
        if (x.assigned()) {
            glb.become(home, lub);
        } else {
            glb.update(home, x.glb);
        }
    }

    SetVarImp*
    SetVarImp::perform_copy(Space& home) {
        return new (home) SetVarImp(home, *this);
    }

    /*
     * Dependencies
     *
     */
    void
    SetVarImp::subscribe(Space& home, Propagator& p, PropCond pc, bool schedule) {
        SetVarImpBase::subscribe(home, p, pc, assigned(), schedule);
    }
    void
    SetVarImp::subscribe(Space& home, Advisor& a, bool fail) {
        SetVarImpBase::subscribe(home, a, assigned(), fail);
    }
    void
    SetVarImp::reschedule(Space& home, Propagator& p, PropCond pc) {
        SetVarImpBase::reschedule(home, p, pc, assigned());
    }

}} // namespace Gecode::Set

// STATISTICS: set-var
