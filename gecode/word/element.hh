/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
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
 */

#ifndef GECODE_WORD_ELEMENT_HH
#define GECODE_WORD_ELEMENT_HH

#include <gecode/word.hh>
#include <gecode/word/rel.hh>
#include <gecode/int/idx-view.hh>

namespace Gecode { namespace Int {
  template<>
  class ViewToVarArg<Word::WordView> {
  public:
    typedef WordVarArgs argtype;
  };
}}

namespace Gecode { namespace Word { namespace Element {

  /**
   * \brief Word-array element propagator
   *
   * Prunes unsupported indices and computes the cube hull of the remaining
   * candidate words for the result.
   */
  class View : public Propagator {
  protected:
    Int::IdxViewArray<WordView> x;
    Int::IntView i;
    WordView y;
    View(Home home, Int::IdxViewArray<WordView>& x,
         Int::IntView i, WordView y);
    View(Space& home, View& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual void reschedule(Space& home);
    virtual size_t dispose(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, Int::IdxViewArray<WordView>& x,
                           Int::IntView i, WordView y);
  };

}}}

#include <gecode/word/element/view.hpp>

#endif

// STATISTICS: word-prop
