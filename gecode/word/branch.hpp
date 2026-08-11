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

#ifndef GECODE_WORD_BRANCH_HPP
#define GECODE_WORD_BRANCH_HPP

namespace Gecode {

  forceinline
  WordValBranch::WordValBranch(Select s0) : s(s0) {}
  forceinline
  WordValBranch::WordValBranch(Select s0, Rnd r)
    : ValBranch<WordVar>(r), s(s0) {}
  forceinline WordValBranch::Select
  WordValBranch::select(void) const { return s; }

  forceinline WordValBranch WORD_VAL_LSB(void) {
    return WordValBranch(WordValBranch::SEL_LSB);
  }
  forceinline WordValBranch WORD_VAL_MSB(void) {
    return WordValBranch(WordValBranch::SEL_MSB);
  }
  forceinline WordValBranch WORD_VAL_RND(Rnd r) {
    return WordValBranch(WordValBranch::SEL_RND,r);
  }

  forceinline
  WordAssign::WordAssign(Select s0) : s(s0) {}
  forceinline
  WordAssign::WordAssign(Select s0, Rnd r)
    : ValBranch<WordVar>(r), s(s0) {}
  forceinline WordAssign::Select
  WordAssign::select(void) const { return s; }

  forceinline WordAssign WORD_ASSIGN_LSB(void) {
    return WordAssign(WordAssign::SEL_LSB);
  }
  forceinline WordAssign WORD_ASSIGN_MSB(void) {
    return WordAssign(WordAssign::SEL_MSB);
  }
  forceinline WordAssign WORD_ASSIGN_RND(Rnd r) {
    return WordAssign(WordAssign::SEL_RND,r);
  }

}

#endif

