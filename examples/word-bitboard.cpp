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

#include <gecode/driver.hh>
#include <gecode/word.hh>

using namespace Gecode;

/**
 * \brief %Example: Recover a small bitboard from attacks and piece count
 *
 * Adjacent squares are computed with shifts and bitwise OR. Search follows
 * the fixed attack mask back to the board. For larger boards, Script copy and
 * recomputation options can be worth comparing.
 *
 * \ingroup Example
 */
class WordBitboard : public Script {
private:
  /// Board, shifted attacks, their union, and occupied-square count
  WordVar board;
  WordVar left;
  WordVar right;
  WordVar attacked;
  IntVar pieces;
public:
  /// Actual model
  WordBitboard(const Options& opt)
    : Script(opt), board(*this,8), left(*this,8), right(*this,8),
      attacked(*this,8), pieces(*this,2,2) {
    shift_left(*this,board,1,left);
    logical_shift_right(*this,board,1,right);
    rel(*this,left,WOT_OR,right,attacked);
    dom(*this,attacked,0x52U);
    popcount(*this,board,pieces);

    WordVarArgs decision={left,right,board};
    branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_MSB());
    branch(*this,pieces,INT_VAL_MIN());
  }
  /// Constructor for cloning \a s
  WordBitboard(WordBitboard& s) : Script(s) {
    board.update(*this,s.board);
    left.update(*this,s.left);
    right.update(*this,s.right);
    attacked.update(*this,s.attacked);
    pieces.update(*this,s.pieces);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordBitboard(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tboard = 0x" << std::hex << board.val()
       << ", attacked = 0x" << attacked.val() << std::dec
       << ", pieces = " << pieces.val() << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordBitboard
 */
int
main(int argc, char* argv[]) {
  Options opt("WordBitboard");
  opt.parse(argc,argv);
  Script::run<WordBitboard,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
