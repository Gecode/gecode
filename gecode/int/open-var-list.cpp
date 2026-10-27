/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#include <gecode/int.hh>

namespace Gecode {

  OpenIntVarList::OpenIntVarList(void) {}

  OpenIntVarList::Factory
  OpenIntVarList::factory(Domain d) {
    if (!d)
      throw InvalidFunction("OpenIntVarList");
    return [d](Space& home, int i) { return IntVar(home,d(i)); };
  }

  OpenIntVarList::OpenIntVarList(Space& home, int max)
    : OpenIntVarList(home,
        IntSet(Int::Limits::min,Int::Limits::max),max) {}

  OpenIntVarList::OpenIntVarList(Space& home, const IntSet& d,
                               int max)
    : OpenIntVarList(home,[d](int) { return d; },max) {}

  OpenIntVarList::OpenIntVarList(Space& home, const IntSet& d,
                               Transition t, int max)
    : OpenIntVarList(home,[d](int) { return d; },t,max) {}

  OpenIntVarList::OpenIntVarList(Space& home, Domain d, int max)
    : OpenVarList<IntVar>(home,factory(d),max) {}

  OpenIntVarList::OpenIntVarList(Space& home, Domain d,
                               Transition t, int max)
    : OpenVarList<IntVar>(home,factory(d),t,max) {}

  OpenBoolVarList::OpenBoolVarList(void) {}

  OpenBoolVarList::OpenBoolVarList(Space& home, int max)
    : OpenVarList<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },max) {}

  OpenBoolVarList::OpenBoolVarList(Space& home, Transition t,
                                 int max)
    : OpenVarList<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },t,max) {}

}

// STATISTICS: int-other
