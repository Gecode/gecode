/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Håkan Kjellerstrand <hakank@bonetmail.com>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Håkan Kjellerstrand, 2009
 *     Mikael Lagerkvist, 2009
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

#include "examples/scowl.hpp"

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
 * \ingroup Example
 *
 */
class WordSquare : public Script {
protected:
  /// Length of words
  const int w_l;
  /// The array of letters
  IntVarArray letters;
public:
  /// Branching variants
  enum {
    BRANCH_WORDS,   ///< Branch on word variables
    BRANCH_LETTERS, ///< Branch on letter variables
  };
  /// Actual model
  WordSquare(const SizeOptions& opt)
    : w_l(opt.size()), letters(*this, w_l*w_l) {

    // Initialize letters
    Matrix<IntVarArray> ml(letters, w_l, w_l);
    for (int i=0; i<w_l; i++)
      for (int j=i; j<w_l; j++)
        ml(i,j) = ml(j,i) = IntVar(*this, 'a','z');
    
    // Number of words with that length
    const int n_w = dict.words(w_l);

    // Initialize word array
    IntVarArgs words(*this, w_l, 0, n_w-1);

    // All words must be different
    distinct(*this, words);

    // Link words with letters
    for (int i=0; i<w_l; i++) {
      // Map each word to i-th letter in that word
      IntSharedArray w2l(n_w);
      for (int n=n_w; n--; )
        w2l[n]=dict.word(w_l,n)[i];
      for (int j=0; j<w_l; j++)
        element(*this, w2l, words[j], ml(i,j));
    }

    // Symmetry breaking: the last word must be later in the wordlist
    rel(*this, words[0], IRT_LE, words[w_l-1]);

    switch (opt.branching()) {
    case BRANCH_WORDS:
      // Branch by assigning words
      branch(*this, words, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MIN);
      break;
    case BRANCH_LETTERS:
      // Branch by assigning letters
      branch(*this, letters, INT_VAR_SIZE_AFC_MIN, INT_VAL_MIN);
      break;
    }
  }
  /// Constructor for cloning \a s
  WordSquare(bool share, WordSquare& s) 
    : Script(share,s), w_l(s.w_l) {
    letters.update(*this, share, s.letters);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new WordSquare(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> ml(letters, w_l, w_l);
    for (int i=0; i<w_l; i++) {
      os << "\t\t";
      for (int j=0; j<w_l; j++)
        if (ml(i,j).assigned()) {
          os << static_cast<char>(ml(i,j).val());
        } else {
          os << ".";
        }
      os << std::endl;
    }
    os << std::endl;
  }
};


/** \brief Main-function
 *  \relates WordSquare
 */
int
main(int argc, char* argv[]) {
  FileSizeOptions opt("WordSquare");
  opt.size(6);
  opt.branching(WordSquare::BRANCH_LETTERS);
  opt.branching(WordSquare::BRANCH_WORDS,   "words");
  opt.branching(WordSquare::BRANCH_LETTERS, "letters");
  opt.parse(argc,argv);
  dict.init(opt.file());
  if (opt.size() > static_cast<unsigned int>(dict.len())) {
    std::cerr << "Error: size must be between 0 and "
              << dict.len() << std::endl;
    return 1;
  }
  Script::run<WordSquare,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
