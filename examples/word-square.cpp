/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Håkan Kjellerstrand <hakank@bonetmail.com>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Håkan Kjellerstrand, 2009
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <climits>
#include <examples/scowl.hpp>

using namespace Gecode;

/**
 * \brief %Example: Word-square puzzle
 *
 *  From http://en.wikipedia.org/wiki/Word_square:
 *  A word square is a special case of acrostic. It consists of a set of words,
 *  all having the same number of letters as the total number of words (the
 *  "order" of the square); when the words are written out in a square grid
 *  horizontally, the same set of words can be read vertically.
 *
 * \ingroup ExProblem
 *
 */
class WordSquare : public Script {
protected:
  /// Which word (position of word in dictionary) for columns
  IntVarArray words;
public:
  /// Actual model
  WordSquare(const SizeOptions& opt)
    : words(*this,opt.size(),0,n_words[opt.size()]-1) {
    const int w_l = words.size();
    const int n_w = n_words[w_l];

    IntArgs w(w_l*n_w);
    Matrix<IntArgs> mw(w, n_w, w_l);

    for (int i=n_w;  i--; )
      for (int j=w_l; j--; )
        mw(i,j) = dict[w_l][i][j];
        
    distinct(*this, words);

    // Convenience variables
    IntVarArgs col(w_l);
    for (int i=w_l; i--; )
      col[i].init(*this,i,i);

    for (int i=0; i<w_l; i++)
      for (int j=i+1; j<w_l; j++) {
        // w[j,words[i]] ==  w[i,words[j]]
        IntVar c(*this, CHAR_MIN, CHAR_MAX);
        element(*this, mw, words[i], col[j], c);
        element(*this, mw, words[j], col[i], c);
      }

    // Symmetry breaking: the last word must be later
    rel(*this, words[0], IRT_LE, words[w_l-1]);

    // Split the available words into two halves
    branch(*this, words, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MIN);
  }
  /// Constructor for cloning \a s
  WordSquare(bool share, WordSquare& s) : Script(share,s) {
    words.update(*this, share, s.words);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new WordSquare(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\twords=" << words << std::endl
       << std::endl;
    for (int i=0; i<words.size(); i++)
      if (words[i].assigned()) {
        os << "\t\t" << dict[words.size()][words[i].val()] << std::endl;
      } else {
        os << "\t\t..." << std::endl;
      } 
    os << std::endl << std::endl;
  }

};

/** \brief Main-function
 *  \relates WordSquare
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("WordSquare");
  opt.size(4);
  opt.parse(argc,argv);
  if (opt.size() > max_word_len) {
    std::cerr << "Error: size must be between 0 and "
              << max_word_len << std::endl;
    return 1;
  }
  Script::run<WordSquare,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
