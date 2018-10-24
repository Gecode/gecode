/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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


// Grid data
namespace {
  // Grid data
  extern const int* grids[];
  // Number of grids
  extern const unsigned int n_grids;
}


/**
 * \brief %Example: %Crossword puzzle
 *
 *  Fill crossword grids with words, that is construct a crossword
 *  puzzle. For a recent paper on this classical problem, see:
 *  Crossword Puzzles as a Constraint Problem, Anbulagan and Adi Botea,
 *  CP 2008, pages 550-554, Springer Verlag.
 *
 *  Note that "Modeling and Programming with Gecode" uses this example
 *  as a case study.
 *
 * \ingroup Example
 */
class Crossword : public Script {
protected:
  /// Width of crossword grid
  const int w;
  /// Height of crossword grid
  const int h;
  /// Letters for grid
  IntVarArray letters;
public:
  /// Which model to use
  enum {
    MODEL_ELEMENT, ///< Use element constraints per letter
    MODEL_TUPLESET ///< Use one tuple-set per word
  };
  /// Branching to use for model
  enum {
    BRANCH_WORDS_AFC,          ///< Branch on the words
    BRANCH_LETTERS_AFC,        ///< Branch on the letters
    BRANCH_LETTERS_AFC_ALL,    ///< Branch on the letters (try all values)
    BRANCH_WORDS_ACTION,       ///< Branch on the words
    BRANCH_LETTERS_ACTION,     ///< Branch on the letters
    BRANCH_LETTERS_ACTION_ALL, ///< Branch on the letters (try all values)
    BRANCH_WORDS_CHB,          ///< Branch on the words
    BRANCH_LETTERS_CHB,        ///< Branch on the letters
    BRANCH_LETTERS_CHB_ALL     ///< Branch on the letters (try all values)
  };
  /// Actual model
  Crossword(const SizeOptions& opt)
    : Script(opt),
      w(grids[opt.size()][0]), h(grids[opt.size()][1]),
      letters(*this,w*h,'a','z') {
    // Pointer into the grid specification (width and height already skipped)
    const int* g = &grids[opt.size()][2];

    // Matrix for letters
    Matrix<IntVarArray> ml(letters, w, h);

    // Set black fields to 0
    {
      IntVar z(*this,0,0);
      for (int n = *g++; n--; ) {
        int x=*g++, y=*g++;
        ml(x,y)=z;
      }
    }

    // Array of all words
    IntVarArgs allwords;

    switch (opt.model()) {
    case MODEL_ELEMENT:
      // While words of length w_l to process
      while (int w_l=*g++) {
        // Number of words of that length in the dictionary
        int n_w = dict.words(w_l);
        // Number of words of that length in the puzzle
        int n=*g++;
        
        if (n > n_w) {
          fail();
        } else {
          // Array of all words of length w_l
          IntVarArgs words(*this,n,0,n_w-1);
          allwords << words;
          
          // All words of same length must be different
          distinct(*this, words, opt.ipl());
          
          for (int d=0; d<w_l; d++) {
            // Array that maps words to a letter at a certain position (shared among all element constraints)
            IntSharedArray w2l(n_w);
            // Initialize word to letter map
            for (int i=n_w; i--; )
              w2l[i] = dict.word(w_l,i)[d];
            // Link word to letter variable
            for (int i=0; i<n; i++) {
              // Get (x,y) coordinate where word begins
              int x=g[3*i+0], y=g[3*i+1];
              // Whether word is horizontal
              bool h=(g[3*i+2] == 0);
              // Constrain the letters to the words' letters
              element(*this, w2l, words[i], h ? ml(x+d,y) : ml(x,y+d));
            }
          }
          // Skip word coordinates
          g += 3*n;
        }
      }
      break;
    case MODEL_TUPLESET:
      // While words of length w_l to process
      while (int w_l=*g++) {
        // Number of words of that length in the dictionary
        int n_w = dict.words(w_l);
        // Number of words of that length in the puzzle
        int n=*g++;
        
        if (n > n_w) {
          fail();
        } else {
          // Setup tuple-set
          TupleSet ts(w_l+1);
          {
            IntArgs w(w_l+1);
            for (int i=0; i<n_w; i++) {
              for (int d=0; d<w_l; d++)
                w[d] = dict.word(w_l,i)[d];
              w[w_l]=i;
              ts.add(w);
            }
          }
          ts.finalize();

          // Array of all words of length w_l
          IntVarArgs words(*this,n,0,n_w-1);
          allwords << words;
          
          // All words of same length must be different
          distinct(*this, words, opt.ipl());
          
          // Constraint all words in puzzle
          for (int i=0; i<n; i++) {
            // Get (x,y) coordinate where word begins
            int x=*g++, y=*g++;
            // Whether word is horizontal
            bool h=(*g++ == 0);
            // Letters in word plus word number
            IntVarArgs w(w_l+1); w[w_l]=words[i];
            if (h)
              for (int d=0; d<w_l; d++)
                w[d] = ml(x+d,y);
            else
              for (int d=0; d<w_l; d++)
                w[d] = ml(x,y+d);
            // Constrain word
            extensional(*this, w, ts);
          }
        }
      }
      break;
    }
    switch (opt.branching()) {
    case BRANCH_WORDS_AFC:
      // Branch by assigning words
      branch(*this, allwords,
             INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN(),
             nullptr, &printwords);
      break;
    case BRANCH_LETTERS_AFC:
      // Branch by assigning letters
      branch(*this, letters,
             INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN(),
             nullptr, &printletters);
      break;
    case BRANCH_LETTERS_AFC_ALL:
      // Branch by assigning letters (try all letters)
      branch(*this, letters,
             INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VALUES_MIN(),
             nullptr, &printletters);
      break;
    case BRANCH_WORDS_ACTION:
      // Branch by assigning words
      branch(*this, allwords,
             INT_VAR_ACTION_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN(),
             nullptr, &printwords);
      break;
    case BRANCH_LETTERS_ACTION:
      // Branch by assigning letters
      branch(*this, letters,
             INT_VAR_ACTION_SIZE_MAX(opt.decay()), INT_VAL_MIN(),
             nullptr, &printletters);
      break;
    case BRANCH_LETTERS_ACTION_ALL:
      // Branch by assigning letters (try all letters)
      branch(*this, letters,
             INT_VAR_ACTION_SIZE_MAX(opt.decay()), INT_VALUES_MIN(),
             nullptr, &printletters);
      break;
    case BRANCH_WORDS_CHB:
      // Branch by assigning words
      branch(*this, allwords,
             INT_VAR_CHB_SIZE_MAX(), INT_VAL_SPLIT_MIN(),
             nullptr, &printwords);
      break;
    case BRANCH_LETTERS_CHB:
      // Branch by assigning letters
      branch(*this, letters,
             INT_VAR_CHB_SIZE_MAX(), INT_VAL_MIN(),
             nullptr, &printletters);
      break;
    case BRANCH_LETTERS_CHB_ALL:
      // Branch by assigning letters (try all letters)
      branch(*this, letters,
             INT_VAR_CHB_SIZE_MAX(), INT_VALUES_MIN(),
             nullptr, &printletters);
      break;
    }
  }
  /// Print brancher information when branching on letters
  static void printletters(const Space& home, const Brancher&,
                           unsigned int a,
                           IntVar, int i, const int& n,
                           std::ostream& os) {
    const Crossword& c = static_cast<const Crossword&>(home);
    int x = i % c.w, y = i / c.w;
    os << "letters[" << x << "," << y << "] "
       << ((a == 0) ? "=" : "!=") << " "
       << static_cast<char>(n);
  }
  /// Print brancher information when branching on words
  static void printwords(const Space&, const Brancher&,
                         unsigned int a,
                         IntVar, int i, const int& n,
                         std::ostream& os) {
    os << "allwords[" << i << "] "
       << ((a == 0) ? "<=" : ">") << " "
       << n;
  }
  /// Do not perform a restart when a solution is found
  bool master(const MetaInfo& mi) {
    if (mi.type() == MetaInfo::RESTART)
      // Post no-goods
      mi.nogoods().post(*this);
    // Do not perform a restart if a solution has been found
    return false;
  }

  /// Constructor for cloning \a s
  Crossword(Crossword& s)
    : Script(s), w(s.w), h(s.h) {
    letters.update(*this, s.letters);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Crossword(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    // Matrix for letters
    Matrix<IntVarArray> ml(letters, w, h);
    for (int i=0; i<h; i++) {
      os << '\t';
      for (int j=0; j<w; j++)
        if (ml(j,i).assigned())
          if (ml(j,i).val() == 0)
            os << '*';
          else
            os << static_cast<char>(ml(j,i).val());
        else
          os << '?';
      os << std::endl;
    }
    os << std::endl << std::endl;
  }
};


/** \brief Main-function
 *  \relates Crossword
 */
int
main(int argc, char* argv[]) {
  FileSizeOptions opt("Crossword");
  opt.size(10);
  opt.ipl(IPL_VAL);
  opt.model(Crossword::MODEL_ELEMENT);
  opt.model(Crossword::MODEL_ELEMENT,"element");
  opt.model(Crossword::MODEL_TUPLESET,"tuple-set");
  opt.branching(Crossword::BRANCH_LETTERS_AFC);
  opt.branching(Crossword::BRANCH_WORDS_AFC,
                "words-afc");
  opt.branching(Crossword::BRANCH_LETTERS_AFC,
                "letters-afc");
  opt.branching(Crossword::BRANCH_LETTERS_AFC_ALL,
                "letters-afc-all");
  opt.branching(Crossword::BRANCH_WORDS_ACTION,
                "words-action");
  opt.branching(Crossword::BRANCH_LETTERS_ACTION,
                "letters-action");
  opt.branching(Crossword::BRANCH_LETTERS_ACTION_ALL,
                "letters-action-all");
  opt.branching(Crossword::BRANCH_WORDS_CHB,
                "words-chb");
  opt.branching(Crossword::BRANCH_LETTERS_CHB,
                "letters-chb");
  opt.branching(Crossword::BRANCH_LETTERS_CHB_ALL,
                "letters-chb-all");
  opt.parse(argc,argv);
  dict.init(opt.file());
  if (opt.size() >= n_grids) {
    std::cerr << "Error: size must be between 0 and "
              << n_grids-1 << std::endl;
    return 1;
  }
  Script::run<Crossword,DFS,SizeOptions>(opt);
  return 0;
}

namespace {

  /*
   * The Grid data has been provided by Peter Van Beek, to
   * quote the original README.txt:
   *
   * The files in this directory contain templates for crossword
   * puzzles. Each is a two-dimensional array. A _ indicates
   * that the associated square in the crossword template is
   * blank, and a * indicates that it is a black square that
   * does not need to have a letter inserted.
   *
   * The crossword puzzles templates came from the following
   * sources:
   *
   *    15.01, ..., 15.10
   *    19.01, ..., 19.10
   *    21.01, ..., 21.10
   *    23.01, ..., 23.10
   *
   *	Herald Tribune Crosswords, Spring, 1999
   *
   *    05.01, ..., 05.10
   *
   *	All legal 5 x 5 puzzles.
   *
   *    puzzle01, ..., puzzle19
   *
   *	Ginsberg, M.L., "Dynamic Backtracking,"
   *	Journal of Artificial Intelligence Researc (JAIR)
   *	Volume 1, pages 25-46, 1993.
   *
   *    puzzle20, ..., puzzle22
   *
   *	Ginsberg, M.L. et al., "Search Lessons Learned
   *	from Crossword Puzzles," AAAI-90, pages 210-215.
   *
   */

  /*
   * Name: 05.01, 5 x 5
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   */
  const int g0[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    0,
    // Black field coordinates

    // Length and number of words of that length
    5, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,3,0, 0,4,0, 1,0,1, 2,0,1, 3,0,1, 4,0,1,
    // End marker
    0
  };


  /*
   * Name: 05.02, 5 x 5
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(* _ _ _ _)
   */
  const int g1[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    2,
    // Black field coordinates
    0,4, 4,0,
    // Length and number of words of that length
    5, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 0,3,0, 1,0,1, 2,0,1, 3,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 1,4,0, 4,1,1,
    // End marker
    0
  };


  /*
   * Name: 05.03, 5 x 5
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ _)
   * 	(* _ _ _ _)
   * 	(* _ _ _ _)
   */
  const int g2[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    4,
    // Black field coordinates
    0,3, 0,4, 4,0, 4,1,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 1,0,1, 2,0,1, 3,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 1,3,0, 1,4,0,
    // Length and number of words of that length
    3, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 4,2,1,
    // End marker
    0
  };


  /*
   * Name: 05.04, 5 x 5
   * 	(_ _ _ * *)
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ _)
   * 	(* _ _ _ _)
   * 	(* * _ _ _)
   */
  const int g3[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    6,
    // Black field coordinates
    0,3, 0,4, 1,4, 3,0, 4,0, 4,1,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,0,1, 1,3,0, 3,1,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 2,4,0, 4,2,1,
    // End marker
    0
  };


  /*
   * Name: 05.05, 5 x 5
   * 	(_ _ _ * *)
   * 	(_ _ _ * *)
   * 	(_ _ _ _ _)
   * 	(* * _ _ _)
   * 	(* * _ _ _)
   */
  const int g4[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    8,
    // Black field coordinates
    0,3, 0,4, 1,3, 1,4, 3,0, 3,1, 4,0, 4,1,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 1,0,1, 2,3,0, 2,4,0, 3,2,1, 4,2,1,
    // End marker
    0
  };


  /*
   * Name: 05.06, 5 x 5
   * 	(* _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ *)
   */
  const int g5[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    2,
    // Black field coordinates
    0,0, 4,4,
    // Length and number of words of that length
    5, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 0,3,0, 1,0,1, 2,0,1, 3,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,1, 0,4,0, 1,0,0, 4,0,1,
    // End marker
    0
  };


  /*
   * Name: 05.07, 5 x 5
   * 	(* _ _ _ _)
   * 	(* _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ *)
   */
  const int g6[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    4,
    // Black field coordinates
    0,0, 0,1, 4,3, 4,4,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 1,0,1, 2,0,1, 3,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,4,0, 1,0,0, 1,1,0,
    // Length and number of words of that length
    3, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,1, 4,0,1,
    // End marker
    0
  };


  /*
   * Name: 05.08, 5 x 5
   * 	(* _ _ _ *)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(* _ _ _ *)
   */
  const int g7[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    4,
    // Black field coordinates
    0,0, 0,4, 4,0, 4,4,
    // Length and number of words of that length
    5, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 0,3,0, 1,0,1, 2,0,1, 3,0,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,1, 1,0,0, 1,4,0, 4,1,1,
    // End marker
    0
  };


  /*
   * Name: 05.09, 5 x 5
   * 	(* * _ _ _)
   * 	(* _ _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ _ *)
   * 	(_ _ _ * *)
   */
  const int g8[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    6,
    // Black field coordinates
    0,0, 0,1, 1,0, 3,4, 4,3, 4,4,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 1,1,0, 1,1,1, 3,0,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,1, 0,4,0, 2,0,0, 4,0,1,
    // End marker
    0
  };


  /*
   * Name: 05.10, 5 x 5
   * 	(* * _ _ _)
   * 	(* * _ _ _)
   * 	(_ _ _ _ _)
   * 	(_ _ _ * *)
   * 	(_ _ _ * *)
   */
  const int g9[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    8,
    // Black field coordinates
    0,0, 0,1, 1,0, 1,1, 3,3, 3,4, 4,3, 4,4,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,1, 0,3,0, 0,4,0, 1,2,1, 2,0,0, 2,1,0, 3,0,1, 4,0,1,
    // End marker
    0
  };


  /*
   * Name: 15.01, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * * _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ * * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g10[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    36,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 3,6, 3,7, 4,0, 4,1, 4,8, 4,12, 4,13, 4,14, 5,5, 5,9, 6,4, 6,11, 7,3, 7,11, 8,3, 8,10, 9,5, 9,9, 10,0, 10,1, 10,2, 10,6, 10,13, 10,14, 11,7, 11,8, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,5,1, 5,12,0, 12,0,1,
    // Length and number of words of that length
    7, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 3,8,1, 4,7,0, 7,4,1, 8,11,0, 11,0,1,
    // Length and number of words of that length
    6, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,11,0, 2,10,0, 3,0,1, 4,2,1, 4,6,0, 5,8,0, 6,5,1, 7,4,0, 8,4,1, 9,3,0, 10,7,1, 11,9,1,
    // Length and number of words of that length
    5, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,5,1, 0,9,0, 1,5,1, 5,0,0, 5,0,1, 5,1,0, 5,10,1, 5,13,0, 5,14,0, 9,0,1, 9,10,1, 10,5,0, 10,9,0, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,8,0, 0,11,1, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,11,1, 2,0,1, 6,0,1, 8,11,1, 11,0,0, 11,1,0, 11,2,0, 11,6,0, 11,13,0, 11,14,0, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,7,0, 3,4,0, 4,9,1, 5,6,1, 6,5,0, 6,9,0, 6,12,1, 7,0,1, 7,12,1, 8,0,1, 9,6,1, 9,10,0, 10,3,1, 12,7,0, 12,8,0,
    // End marker
    0
  };


  /*
   * Name: 15.02, 15 x 15
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ * * *)
   * 	(* * * _ _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ _ * * *)
   * 	(* * * _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   */
  const int g11[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    34,
    // Black field coordinates
    0,5, 0,10, 1,5, 1,10, 2,5, 2,10, 3,4, 3,9, 4,3, 4,8, 4,13, 4,14, 5,0, 5,7, 6,6, 6,10, 7,5, 7,9, 8,4, 8,8, 9,7, 9,14, 10,0, 10,1, 10,6, 10,11, 11,5, 11,10, 12,4, 12,9, 13,4, 13,9, 14,4, 14,9,
    // Length and number of words of that length
    15, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,12,0,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,11,0, 5,3,0, 5,13,0,
    // Length and number of words of that length
    7, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,8,1, 9,0,1,
    // Length and number of words of that length
    6, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 5,1,1, 6,0,1, 8,9,1, 9,8,0, 9,8,1,
    // Length and number of words of that length
    5, 14,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,7,0, 1,0,1, 2,0,1, 3,10,1, 7,0,1, 7,10,1, 10,7,0, 10,14,0, 11,0,1, 12,10,1, 13,10,1, 14,10,1,
    // Length and number of words of that length
    4, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,6,1, 0,8,0, 0,11,1, 0,13,0, 0,14,0, 1,6,1, 1,11,1, 2,6,1, 2,11,1, 3,0,1, 3,5,0, 3,5,1, 4,4,0, 4,4,1, 4,9,1, 5,14,0, 6,0,0, 6,11,1, 7,10,0, 8,0,1, 8,9,0, 10,2,1, 10,7,1, 11,0,0, 11,1,0, 11,6,0, 11,6,1, 11,11,0, 11,11,1, 12,0,1, 12,5,1, 13,0,1, 13,5,1, 14,0,1, 14,5,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,9,0, 3,10,0, 4,0,1, 4,9,0, 5,8,0, 6,7,0, 6,7,1, 7,6,0, 7,6,1, 8,5,0, 8,5,1, 9,4,0, 10,12,1, 12,5,0, 12,10,0,
    // End marker
    0
  };


  /*
   * Name: 15.03, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g12[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    36,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,8, 4,0, 4,1, 4,2, 4,7, 4,12, 4,13, 4,14, 5,6, 6,5, 6,11, 7,4, 7,10, 8,3, 8,9, 9,8, 10,0, 10,1, 10,2, 10,7, 10,12, 10,13, 10,14, 11,6, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 3,0,1, 5,7,1, 7,5,0, 7,11,0, 9,0,1, 11,7,1,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 3,9,1, 5,0,1, 9,3,0, 9,9,0, 9,9,1, 11,0,1,
    // Length and number of words of that length
    5, 22,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 1,5,1, 2,5,1, 4,8,0, 5,0,0, 5,1,0, 5,2,0, 5,7,0, 5,12,0, 5,13,0, 5,14,0, 6,0,1, 6,6,0, 6,6,1, 7,5,1, 8,4,1, 8,10,1, 10,8,0, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,7,0, 0,11,1, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 3,4,0, 3,10,0, 4,3,1, 4,8,1, 7,0,1, 7,11,1, 8,4,0, 8,10,0, 10,3,1, 10,8,1, 11,0,0, 11,1,0, 11,2,0, 11,7,0, 11,12,0, 11,13,0, 11,14,0, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 6,12,1, 8,0,1, 12,6,0,
    // End marker
    0
  };


  /*
   * Name: 15.04, 15 x 15
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _)
   */
  const int g13[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    32,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,0, 3,5, 3,11, 4,6, 5,3, 5,9, 6,4, 6,8, 6,13, 6,14, 8,0, 8,1, 8,6, 8,10, 9,5, 9,11, 10,8, 11,3, 11,9, 11,14, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    15, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,7,0, 0,12,0, 7,0,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 4,7,1, 7,13,0, 10,0,1,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,13,0, 0,14,0, 4,0,1, 9,0,0, 9,1,0, 9,6,0, 10,9,1,
    // Length and number of words of that length
    5, 22,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,1, 0,9,0, 1,5,1, 2,5,1, 3,6,1, 3,10,0, 4,5,0, 4,11,0, 5,4,1, 5,10,1, 6,3,0, 6,9,0, 7,4,0, 9,0,1, 9,6,1, 10,5,0, 10,11,0, 11,4,1, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 22,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,6,0, 0,11,1, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 3,1,1, 4,0,0, 6,0,1, 6,9,1, 7,14,0, 8,2,1, 8,11,1, 11,8,0, 11,10,1, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,5,0, 0,11,0, 3,4,0, 3,12,1, 5,0,1, 5,6,0, 6,5,1, 7,8,0, 8,7,1, 9,10,0, 9,12,1, 11,0,1, 12,3,0, 12,9,0, 12,14,0,
    // End marker
    0
  };


  /*
   * Name: 15.05, 15 x 15
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(* * * * _ _ _ * * * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * * * *)
   * 	(_ _ _ _ _ * * _ _ _ _ _ _ _ *)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(* _ _ _ _ _ _ _ * * _ _ _ _ _)
   * 	(* * * * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * * * _ _ _ * * * *)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   */
  const int g14[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    44,
    // Black field coordinates
    0,4, 0,8, 0,9, 1,4, 1,9, 2,4, 2,9, 3,4, 3,9, 4,3, 4,11, 4,12, 4,13, 4,14, 5,0, 5,1, 5,6, 5,10, 6,5, 6,6, 6,10, 7,4, 7,10, 8,4, 8,8, 8,9, 9,4, 9,8, 9,13, 9,14, 10,0, 10,1, 10,2, 10,3, 10,11, 11,5, 11,10, 12,5, 12,10, 13,5, 13,10, 14,5, 14,6, 14,10,
    // Length and number of words of that length
    15, 1,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0,
    // Length and number of words of that length
    10, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 5,12,0,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    1,8,0, 4,4,1, 7,6,0, 10,4,1,
    // Length and number of words of that length
    6, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 9,9,0,
    // Length and number of words of that length
    5, 21,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,6,0, 0,10,0, 0,10,1, 1,10,1, 2,10,1, 3,10,1, 5,3,0, 5,11,0, 6,0,1, 7,5,1, 8,10,1, 10,4,0, 10,8,0, 10,13,0, 10,14,0, 11,0,1, 12,0,1, 13,0,1, 14,0,1,
    // Length and number of words of that length
    4, 38,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,3,0, 0,11,0, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,5,1, 2,0,1, 2,5,1, 3,0,1, 3,5,1, 4,9,0, 5,2,1, 5,11,1, 5,13,0, 5,14,0, 6,0,0, 6,1,0, 6,11,1, 7,0,1, 7,5,0, 7,11,1, 8,0,1, 9,0,1, 9,9,1, 11,0,0, 11,1,0, 11,2,0, 11,3,0, 11,6,1, 11,11,0, 11,11,1, 12,6,1, 12,11,1, 13,6,1, 13,11,1, 14,11,1,
    // Length and number of words of that length
    3, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 4,0,1, 4,4,0, 5,7,1, 6,7,1, 8,5,1, 8,10,0, 9,5,1, 10,12,1, 14,7,1,
    // End marker
    0
  };


  /*
   * Name: 15.06, 15 x 15
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   */
  const int g15[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    30,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,7, 4,3, 4,11, 5,8, 6,4, 6,9, 7,0, 7,1, 7,2, 7,12, 7,13, 7,14, 8,5, 8,10, 9,6, 10,3, 10,11, 11,7, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    9, 3,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 6,8,0, 7,3,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 5,0,1, 7,9,0, 9,7,1,
    // Length and number of words of that length
    7, 19,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,2,0, 0,12,0, 0,13,0, 0,14,0, 3,0,1, 3,8,1, 4,4,1, 4,7,0, 8,0,0, 8,1,0, 8,2,0, 8,12,0, 8,13,0, 8,14,0, 10,4,1, 11,0,1, 11,8,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 5,9,1, 9,0,1, 9,5,0,
    // Length and number of words of that length
    5, 14,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,8,0, 1,5,1, 2,5,1, 3,10,0, 5,3,0, 5,11,0, 6,10,1, 7,4,0, 8,0,1, 10,6,0, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,3,0, 0,11,0, 0,11,1, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 6,0,1, 6,5,1, 8,6,1, 8,11,1, 11,3,0, 11,11,0, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 3,4,0, 4,0,1, 4,12,1, 9,10,0, 10,0,1, 10,12,1, 12,7,0,
    // End marker
    0
  };


  /*
   * Name: 15.07, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g16[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    32,
    // Black field coordinates
    0,4, 0,9, 1,4, 1,9, 2,9, 3,7, 4,0, 4,1, 4,6, 4,10, 5,5, 5,12, 5,13, 5,14, 6,4, 7,3, 7,11, 8,10, 9,0, 9,1, 9,2, 9,9, 10,4, 10,8, 10,13, 10,14, 11,7, 12,5, 13,5, 13,10, 14,5, 14,10,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 5,6,0, 6,5,1, 8,0,1,
    // Length and number of words of that length
    9, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1, 6,12,0, 12,6,1,
    // Length and number of words of that length
    7, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,11,0, 3,0,1, 3,8,1, 4,7,0, 7,4,1, 8,3,0, 8,11,0, 11,0,1, 11,8,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,9,0, 5,6,1, 6,5,0, 9,3,1,
    // Length and number of words of that length
    5, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,1, 0,12,0, 0,13,0, 0,14,0, 1,10,1, 2,10,1, 5,0,1, 9,10,1, 10,0,0, 10,1,0, 10,2,0, 10,9,0, 12,0,1, 13,0,1, 14,0,1,
    // Length and number of words of that length
    4, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,5,1, 0,6,0, 0,10,0, 1,0,1, 1,5,1, 2,4,0, 4,2,1, 4,11,1, 5,0,0, 5,1,0, 6,0,1, 6,13,0, 6,14,0, 8,11,1, 9,10,0, 10,0,1, 10,9,1, 11,4,0, 11,8,0, 11,13,0, 11,14,0, 13,6,1, 13,11,1, 14,6,1, 14,11,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 4,7,1, 5,10,0, 7,0,1, 7,4,0, 7,12,1, 10,5,1, 12,7,0,
    // End marker
    0
  };


  /*
   * Name: 15.08, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g17[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    39,
    // Black field coordinates
    0,4, 0,9, 1,4, 1,9, 2,4, 2,9, 3,5, 3,11, 4,0, 4,1, 4,2, 4,6, 4,10, 5,3, 5,12, 5,13, 5,14, 6,4, 6,8, 7,7, 8,6, 8,10, 9,0, 9,1, 9,2, 9,11, 10,4, 10,8, 10,12, 10,13, 10,14, 11,3, 11,9, 12,5, 12,10, 13,5, 13,10, 14,5, 14,10,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,9,0, 4,5,0, 5,4,1, 9,3,1,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 7,0,1, 7,8,1, 8,7,0,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 6,9,1, 8,0,1, 9,6,0,
    // Length and number of words of that length
    5, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,10,1, 0,12,0, 0,13,0, 0,14,0, 1,10,1, 2,10,1, 3,0,1, 3,6,1, 4,11,0, 6,3,0, 10,0,0, 10,1,0, 10,2,0, 10,11,0, 11,4,1, 11,10,1, 12,0,1, 13,0,1, 14,0,1,
    // Length and number of words of that length
    4, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,5,1, 0,6,0, 0,10,0, 1,0,1, 1,5,1, 2,0,1, 2,5,1, 4,11,1, 5,0,0, 5,1,0, 5,2,0, 6,0,1, 6,12,0, 6,13,0, 6,14,0, 8,11,1, 10,0,1, 11,4,0, 11,8,0, 11,12,0, 11,13,0, 11,14,0, 12,6,1, 12,11,1, 13,6,1, 13,11,1, 14,6,1, 14,11,1,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 3,4,0, 3,12,1, 4,3,1, 4,7,1, 5,0,1, 5,6,0, 5,10,0, 6,5,1, 7,4,0, 7,8,0, 8,7,1, 9,10,0, 9,12,1, 10,5,1, 10,9,1, 11,0,1, 12,3,0, 12,9,0,
    // End marker
    0
  };


  /*
   * Name: 15.09, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g18[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    38,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,7, 4,0, 4,1, 4,2, 4,6, 4,12, 4,13, 4,14, 5,5, 5,9, 6,4, 6,8, 7,3, 7,11, 8,6, 8,10, 9,5, 9,9, 10,0, 10,1, 10,2, 10,8, 10,12, 10,13, 10,14, 11,7, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    7, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,11,0, 3,0,1, 3,8,1, 4,7,0, 7,4,1, 8,3,0, 8,11,0, 11,0,1, 11,8,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 6,9,1, 8,0,1, 9,6,0,
    // Length and number of words of that length
    5, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,5,1, 0,9,0, 1,5,1, 2,5,1, 3,10,0, 4,7,1, 5,0,0, 5,0,1, 5,1,0, 5,2,0, 5,10,1, 5,12,0, 5,13,0, 5,14,0, 7,4,0, 9,0,1, 9,10,1, 10,3,1, 10,5,0, 10,9,0, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,6,0, 0,11,1, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 6,0,1, 8,11,1, 11,0,0, 11,1,0, 11,2,0, 11,8,0, 11,12,0, 11,13,0, 11,14,0, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 3,4,0, 4,3,1, 5,6,0, 5,6,1, 6,5,0, 6,5,1, 6,9,0, 7,0,1, 7,8,0, 7,12,1, 8,7,1, 9,6,1, 9,10,0, 10,9,1, 12,7,0,
    // End marker
    0
  };


  /*
   * Name: 15.10, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * * * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * * * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g19[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    35,
    // Black field coordinates
    0,4, 0,9, 1,4, 1,9, 2,4, 2,9, 3,4, 4,0, 4,1, 4,6, 4,11, 4,12, 4,13, 4,14, 5,5, 6,5, 6,10, 7,7, 8,4, 8,9, 9,9, 10,0, 10,1, 10,2, 10,3, 10,8, 10,13, 10,14, 11,10, 12,5, 12,10, 13,5, 13,10, 14,5, 14,10,
    // Length and number of words of that length
    10, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,3,0, 0,8,0, 3,5,1, 5,6,0, 5,11,0, 5,12,0, 11,0,1,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,6,1, 9,0,1,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 7,0,1, 7,8,1, 8,7,0,
    // Length and number of words of that length
    6, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,10,0, 9,4,0,
    // Length and number of words of that length
    5, 18,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,1, 1,10,1, 2,10,1, 3,9,0, 5,0,0, 5,0,1, 5,1,0, 5,13,0, 5,14,0, 6,0,1, 7,5,0, 8,10,1, 9,10,1, 10,9,0, 12,0,1, 13,0,1, 14,0,1,
    // Length and number of words of that length
    4, 38,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,5,1, 0,6,0, 0,11,0, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,5,1, 2,0,1, 2,5,1, 3,0,1, 4,2,1, 4,4,0, 4,7,1, 6,6,1, 6,11,1, 7,10,0, 8,0,1, 8,5,1, 10,4,1, 10,9,1, 11,0,0, 11,1,0, 11,2,0, 11,3,0, 11,8,0, 11,11,1, 11,13,0, 11,14,0, 12,6,1, 12,11,1, 13,6,1, 13,11,1, 14,6,1, 14,11,1,
    // End marker
    0
  };


  /*
   * Name: 19.01, 19 x 19
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g20[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    60,
    // Black field coordinates
    0,4, 0,9, 0,14, 1,4, 1,9, 1,14, 2,4, 2,14, 3,7, 3,12, 4,0, 4,1, 4,6, 4,11, 4,17, 4,18, 5,5, 5,10, 6,4, 6,9, 6,15, 7,3, 7,8, 7,14, 8,7, 8,13, 9,0, 9,1, 9,2, 9,6, 9,12, 9,16, 9,17, 9,18, 10,5, 10,11, 11,4, 11,10, 11,15, 12,3, 12,9, 12,14, 13,8, 13,13, 14,0, 14,1, 14,7, 14,12, 14,17, 14,18, 15,6, 15,11, 16,4, 16,14, 17,4, 17,9, 17,14, 18,4, 18,9, 18,14,
    // Length and number of words of that length
    9, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,16,0, 2,5,1, 10,2,0, 10,16,0, 16,5,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,13,0, 5,11,1, 11,5,0, 13,0,1,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,8,0, 3,0,1, 8,0,1, 10,12,1, 12,10,0, 12,15,0, 15,12,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,15,0, 3,13,1, 13,3,0, 15,0,1,
    // Length and number of words of that length
    5, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,0, 4,12,0, 4,12,1, 5,0,1, 5,11,0, 6,10,0, 6,10,1, 7,9,0, 7,9,1, 8,8,0, 8,8,1, 8,14,1, 9,7,0, 9,7,1, 10,0,1, 10,6,0, 10,6,1, 11,5,1, 12,4,1, 13,14,1, 14,2,1, 14,8,0, 14,13,0,
    // Length and number of words of that length
    4, 70,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,5,1, 0,6,0, 0,10,1, 0,11,0, 0,15,1, 0,17,0, 0,18,0, 1,0,1, 1,5,1, 1,10,1, 1,15,1, 2,0,1, 2,9,0, 2,15,1, 3,8,1, 3,14,0, 4,2,1, 4,7,0, 4,7,1, 5,0,0, 5,1,0, 5,6,0, 5,6,1, 5,17,0, 5,18,0, 6,0,1, 6,5,0, 6,5,1, 7,4,0, 7,4,1, 7,15,0, 7,15,1, 8,3,0, 8,14,0, 9,13,0, 10,0,0, 10,1,0, 10,12,0, 10,17,0, 10,18,0, 11,0,1, 11,11,0, 11,11,1, 12,4,0, 12,10,1, 12,15,1, 13,9,0, 13,9,1, 14,8,1, 14,13,1, 15,0,0, 15,1,0, 15,7,0, 15,7,1, 15,12,0, 15,17,0, 15,18,0, 16,0,1, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,0,1, 18,5,1, 18,10,1, 18,15,1,
    // Length and number of words of that length
    3, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,12,0, 3,4,0, 6,16,1, 7,0,1, 9,3,1, 9,13,1, 11,16,1, 12,0,1, 13,14,0, 16,6,0, 16,11,0,
    // End marker
    0
  };


  /*
   * Name: 19.02, 19 x 19
   * 	(_ _ _ _ _ * * _ _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ * _ _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ _ _ * * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * * * _ _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ _ * _ _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ _ * * * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * * _ _ _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ _ * _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ _ * * _ _ _ _ _)
   */
  const int g21[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    65,
    // Black field coordinates
    0,4, 0,9, 0,14, 1,4, 1,9, 1,14, 2,4, 3,7, 3,12, 4,3, 4,7, 4,8, 4,12, 4,13, 5,0, 5,1, 5,6, 5,11, 5,16, 5,17, 5,18, 6,0, 6,6, 6,10, 6,18, 7,5, 7,10, 7,15, 8,5, 8,10, 8,15, 9,4, 9,9, 9,14, 10,3, 10,8, 10,13, 11,3, 11,8, 11,13, 12,0, 12,8, 12,12, 12,18, 13,0, 13,1, 13,2, 13,7, 13,12, 13,17, 13,18, 14,5, 14,6, 14,10, 14,11, 14,15, 15,6, 15,11, 16,14, 17,4, 17,9, 17,14, 18,4, 18,9, 18,14,
    // Length and number of words of that length
    14, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 16,0,1,
    // Length and number of words of that length
    13, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 6,16,0,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,7,0, 6,11,0,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,15,0, 2,9,0, 2,14,0, 3,0,1, 5,12,0, 6,1,0, 6,11,1, 6,17,0, 7,6,0, 10,4,0, 10,9,0, 12,1,1, 12,3,0, 12,13,0, 15,12,1,
    // Length and number of words of that length
    6, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,10,0, 3,4,0, 3,13,1, 10,14,0, 13,8,0, 15,0,1,
    // Length and number of words of that length
    5, 30,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,6,0, 0,11,0, 0,16,0, 0,17,0, 0,18,0, 4,14,1, 5,3,0, 5,8,0, 5,13,0, 6,1,1, 7,0,0, 7,0,1, 7,18,0, 8,0,1, 9,5,0, 9,10,0, 9,15,0, 10,14,1, 11,14,1, 12,13,1, 14,0,0, 14,0,1, 14,1,0, 14,2,0, 14,7,0, 14,12,0, 14,17,0, 14,18,0,
    // Length and number of words of that length
    4, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,3,0, 0,5,1, 0,8,0, 0,10,1, 0,13,0, 0,15,1, 1,0,1, 1,5,1, 1,10,1, 1,15,1, 2,0,1, 3,8,1, 5,2,1, 5,7,1, 5,12,1, 7,6,1, 7,11,1, 8,6,1, 8,11,1, 9,0,1, 9,5,1, 9,10,1, 9,15,1, 10,4,1, 10,9,1, 11,4,1, 11,9,1, 13,3,1, 13,8,1, 13,13,1, 15,5,0, 15,7,1, 15,10,0, 15,15,0, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,0,1, 18,5,1, 18,10,1, 18,15,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,12,0, 4,0,1, 4,4,1, 4,9,1, 6,7,1, 7,16,1, 8,16,1, 10,0,1, 11,0,1, 12,9,1, 14,7,1, 14,12,1, 14,16,1, 16,6,0, 16,11,0,
    // End marker
    0
  };


  /*
   * Name: 19.03, 19 x 19
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   */
  const int g22[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    54,
    // Black field coordinates
    0,6, 0,12, 1,6, 1,12, 2,6, 2,12, 3,3, 3,9, 3,15, 4,4, 4,9, 4,14, 5,5, 5,13, 6,0, 6,1, 6,2, 6,8, 6,16, 6,17, 6,18, 7,7, 7,11, 8,6, 8,10, 9,3, 9,4, 9,14, 9,15, 10,8, 10,12, 11,7, 11,11, 12,0, 12,1, 12,2, 12,10, 12,16, 12,17, 12,18, 13,5, 13,13, 14,4, 14,9, 14,14, 15,3, 15,9, 15,15, 16,6, 16,12, 17,6, 17,12, 18,6, 18,12,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,9,0, 9,5,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,10,0, 8,11,1, 10,0,1, 11,8,0,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,11,0, 3,12,0, 5,6,1, 6,5,0, 6,9,1, 6,13,0, 7,0,1, 7,12,1, 9,6,0, 11,0,1, 11,12,1, 12,3,1, 12,7,0, 12,11,0, 13,6,1,
    // Length and number of words of that length
    6, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,8,0, 0,13,1, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,13,1, 2,0,1, 2,13,1, 8,0,1, 10,13,1, 13,0,0, 13,1,0, 13,2,0, 13,10,0, 13,16,0, 13,17,0, 13,18,0, 16,0,1, 16,13,1, 17,0,1, 17,13,1, 18,0,1, 18,13,1,
    // Length and number of words of that length
    5, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,7,1, 0,13,0, 1,7,1, 2,7,1, 3,4,1, 3,6,0, 3,10,1, 4,3,0, 4,15,0, 5,0,1, 5,14,1, 6,3,1, 7,0,0, 7,1,0, 7,2,0, 7,16,0, 7,17,0, 7,18,0, 10,3,0, 10,15,0, 11,12,0, 12,11,1, 13,0,1, 13,14,1, 14,5,0, 14,13,0, 15,4,1, 15,10,1, 16,7,1, 17,7,1, 18,7,1,
    // Length and number of words of that length
    4, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,14,0, 4,0,1, 4,5,1, 4,10,1, 4,15,1, 5,4,0, 5,14,0, 10,4,0, 10,14,0, 14,0,1, 14,5,1, 14,10,1, 14,15,1, 15,4,0, 15,14,0,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,15,0, 3,0,1, 3,16,1, 7,8,0, 7,8,1, 8,7,0, 8,7,1, 8,11,0, 9,0,1, 9,10,0, 9,16,1, 10,9,1, 11,8,1, 15,0,1, 15,16,1, 16,3,0, 16,9,0, 16,15,0,
    // End marker
    0
  };


  /*
   * Name: 19.04, 19 x 19
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ * * * _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * * * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ * * * _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   */
  const int g23[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    65,
    // Black field coordinates
    0,5, 0,13, 1,5, 1,13, 2,5, 2,13, 3,3, 3,7, 3,11, 3,15, 4,4, 4,8, 4,9, 4,10, 4,14, 5,0, 5,1, 5,2, 5,16, 5,17, 5,18, 6,6, 6,12, 7,3, 7,7, 7,11, 7,15, 8,4, 8,9, 8,14, 9,4, 9,8, 9,9, 9,10, 9,14, 10,4, 10,9, 10,14, 11,3, 11,7, 11,11, 11,15, 12,6, 12,12, 13,0, 13,1, 13,2, 13,16, 13,17, 13,18, 14,4, 14,8, 14,9, 14,10, 14,14, 15,3, 15,7, 15,11, 15,15, 16,5, 16,13, 17,5, 17,13, 18,5, 18,13,
    // Length and number of words of that length
    13, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,5,0, 3,13,0, 5,3,1, 13,3,1,
    // Length and number of words of that length
    7, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,1, 1,6,1, 2,6,1, 6,0,0, 6,1,0, 6,2,0, 6,16,0, 6,17,0, 6,18,0, 16,6,1, 17,6,1, 18,6,1,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,12,0, 6,0,1, 6,13,1, 12,0,1, 12,13,1, 13,6,0, 13,12,0,
    // Length and number of words of that length
    5, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,14,1, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,14,1, 2,0,1, 2,14,1, 6,7,1, 7,6,0, 7,12,0, 12,7,1, 14,0,0, 14,1,0, 14,2,0, 14,16,0, 14,17,0, 14,18,0, 16,0,1, 16,14,1, 17,0,1, 17,14,1, 18,0,1, 18,14,1,
    // Length and number of words of that length
    4, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,8,0, 0,9,0, 0,10,0, 0,14,0, 4,0,1, 4,15,1, 5,8,0, 5,10,0, 8,0,1, 8,5,1, 8,10,1, 8,15,1, 9,0,1, 9,15,1, 10,0,1, 10,5,1, 10,8,0, 10,10,0, 10,10,1, 10,15,1, 14,0,1, 14,15,1, 15,4,0, 15,8,0, 15,9,0, 15,10,0, 15,14,0,
    // Length and number of words of that length
    3, 52,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,7,0, 0,11,0, 0,15,0, 3,0,1, 3,4,1, 3,8,1, 3,12,1, 3,16,1, 4,3,0, 4,5,1, 4,7,0, 4,11,0, 4,11,1, 4,15,0, 5,4,0, 5,9,0, 5,14,0, 7,0,1, 7,4,1, 7,8,1, 7,12,1, 7,16,1, 8,3,0, 8,7,0, 8,11,0, 8,15,0, 9,5,1, 9,11,1, 11,0,1, 11,4,0, 11,4,1, 11,8,1, 11,9,0, 11,12,1, 11,14,0, 11,16,1, 12,3,0, 12,7,0, 12,11,0, 12,15,0, 14,5,1, 14,11,1, 15,0,1, 15,4,1, 15,8,1, 15,12,1, 15,16,1, 16,3,0, 16,7,0, 16,11,0, 16,15,0,
    // End marker
    0
  };


  /*
   * Name: 19.05, 19 x 19
   * 	(_ _ _ _ * * _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ * _ _ _ _ * * *)
   * 	(* * * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ * * *)
   * 	(* * * _ _ _ _ * _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ * * _ _ _ _)
   */
  const int g24[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    70,
    // Black field coordinates
    0,4, 0,10, 0,15, 1,4, 1,10, 1,15, 2,4, 2,10, 2,15, 3,6, 3,11, 4,0, 4,1, 4,2, 4,7, 4,8, 4,12, 4,16, 4,17, 4,18, 5,0, 5,8, 5,12, 5,13, 6,5, 6,13, 7,3, 7,10, 7,15, 8,6, 8,11, 9,0, 9,1, 9,2, 9,7, 9,11, 9,16, 9,17, 9,18, 10,7, 10,12, 11,3, 11,8, 11,15, 12,5, 12,13, 13,5, 13,6, 13,10, 13,18, 14,0, 14,1, 14,2, 14,6, 14,10, 14,11, 14,16, 14,17, 14,18, 15,7, 15,12, 16,3, 16,8, 16,14, 17,3, 17,8, 17,14, 18,3, 18,8, 18,14,
    // Length and number of words of that length
    19, 1,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0,
    // Length and number of words of that length
    16, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,14,0, 3,4,0,
    // Length and number of words of that length
    7, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 3,12,1, 5,1,1, 6,6,1, 8,12,1, 10,0,1, 12,6,1, 12,15,0, 13,11,1, 15,0,1,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 3,0,1, 7,4,1, 8,0,1, 10,13,1, 11,9,1, 13,13,0, 15,13,1,
    // Length and number of words of that length
    5, 18,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,13,0, 1,5,1, 2,5,1, 5,14,1, 6,0,1, 6,8,0, 6,14,1, 7,5,0, 7,13,0, 8,10,0, 12,0,1, 12,14,1, 13,0,1, 14,5,0, 16,9,1, 17,9,1, 18,9,1,
    // Length and number of words of that length
    4, 62,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,7,0, 0,8,0, 0,11,1, 0,12,0, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 3,7,1, 3,10,0, 3,15,0, 4,3,1, 4,6,0, 4,11,0, 5,1,0, 5,2,0, 5,7,0, 5,16,0, 5,17,0, 5,18,0, 6,12,0, 7,11,1, 8,7,1, 9,3,1, 9,6,0, 9,12,1, 10,0,0, 10,1,0, 10,2,0, 10,8,1, 10,11,0, 10,16,0, 10,17,0, 11,4,1, 11,7,0, 11,12,0, 12,3,0, 12,8,0, 14,12,1, 15,0,0, 15,1,0, 15,2,0, 15,6,0, 15,8,1, 15,10,0, 15,11,0, 15,16,0, 15,17,0, 15,18,0, 16,4,1, 16,15,1, 17,4,1, 17,15,1, 18,4,1, 18,15,1,
    // Length and number of words of that length
    3, 25,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,11,0, 0,16,1, 1,16,1, 2,16,1, 4,9,1, 4,13,1, 5,9,1, 6,0,0, 7,0,1, 7,16,1, 8,3,0, 8,15,0, 9,8,1, 10,18,0, 11,0,1, 11,16,1, 13,7,1, 14,3,1, 14,7,1, 16,0,1, 16,7,0, 16,12,0, 17,0,1, 18,0,1,
    // End marker
    0
  };


  /*
   * Name: 19.06, 19 x 19
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(* _ _ _ * _ _ _ _ _ _ _ _ _ _ _ * * *)
   * 	(* * * _ _ _ * * _ _ _ * * _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ * * _ _ _ * * _ _ _ * * *)
   * 	(* * * _ _ _ _ _ _ _ _ _ _ _ * _ _ _ *)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g25[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    74,
    // Black field coordinates
    0,3, 0,4, 0,9, 0,14, 0,15, 1,4, 1,9, 1,14, 1,15, 2,4, 2,15, 3,11, 3,12, 4,0, 4,1, 4,2, 4,3, 4,7, 4,11, 4,16, 4,17, 4,18, 5,5, 5,6, 5,10, 6,4, 6,9, 6,14, 7,4, 7,8, 7,14, 8,7, 8,13, 9,0, 9,1, 9,2, 9,6, 9,12, 9,16, 9,17, 9,18, 10,5, 10,11, 11,4, 11,10, 11,14, 12,4, 12,9, 12,14, 13,8, 13,12, 13,13, 14,0, 14,1, 14,2, 14,7, 14,11, 14,15, 14,16, 14,17, 14,18, 15,6, 15,7, 16,3, 16,14, 17,3, 17,4, 17,9, 17,14, 18,3, 18,4, 18,9, 18,14, 18,15,
    // Length and number of words of that length
    11, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 3,15,0, 5,3,0, 15,8,1,
    // Length and number of words of that length
    10, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 16,4,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,13,0, 5,11,1, 11,5,0, 13,0,1,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 8,0,1, 10,12,1, 12,10,0,
    // Length and number of words of that length
    6, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,13,1, 15,0,1,
    // Length and number of words of that length
    5, 22,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,6,0, 0,10,0, 4,12,0, 5,0,1, 5,11,0, 6,10,0, 7,9,0, 7,9,1, 8,8,0, 8,8,1, 8,14,1, 9,7,0, 9,7,1, 10,0,1, 10,6,0, 10,6,1, 11,5,1, 13,14,1, 14,8,0, 14,12,0, 14,13,0,
    // Length and number of words of that length
    4, 58,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,2,0, 0,5,1, 0,7,0, 0,10,1, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,5,1, 1,10,1, 2,0,1, 2,9,0, 2,14,0, 4,12,1, 5,0,0, 5,1,0, 5,2,0, 5,16,0, 5,17,0, 5,18,0, 6,0,1, 6,5,0, 6,5,1, 6,10,1, 6,15,1, 7,0,1, 7,15,1, 9,13,0, 10,0,0, 10,1,0, 10,2,0, 10,16,0, 10,17,0, 10,18,0, 11,0,1, 11,15,1, 12,0,1, 12,5,1, 12,10,1, 12,15,1, 13,4,0, 13,9,0, 14,3,1, 15,0,0, 15,1,0, 15,2,0, 15,11,0, 15,16,0, 15,17,0, 15,18,0, 16,15,1, 17,5,1, 17,10,1, 17,15,1, 18,5,1, 18,10,1,
    // Length and number of words of that length
    3, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,11,0, 0,12,0, 0,16,1, 1,3,0, 1,16,1, 2,16,1, 3,4,0, 4,4,1, 4,8,1, 5,7,0, 5,7,1, 6,6,0, 7,5,1, 8,4,0, 8,14,0, 9,3,1, 9,13,1, 10,12,0, 11,11,0, 11,11,1, 13,9,1, 13,14,0, 14,8,1, 14,12,1, 15,15,0, 16,0,1, 16,6,0, 16,7,0, 17,0,1, 18,0,1, 18,16,1,
    // End marker
    0
  };


  /*
   * Name: 19.07, 19 x 19
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * * _ _ _ _)
   * 	(* * * * _ _ _ * _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * * _ _ _ * * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * * _ _ _ * * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ * _ _ _ * * * *)
   * 	(_ _ _ _ * * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   */
  const int g26[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    70,
    // Black field coordinates
    0,4, 0,9, 0,14, 1,4, 1,9, 1,14, 2,4, 2,9, 2,14, 3,3, 3,4, 3,16, 3,17, 3,18, 4,7, 4,11, 4,15, 5,0, 5,1, 5,6, 5,11, 5,15, 6,5, 6,10, 6,14, 7,4, 7,8, 7,9, 7,13, 8,3, 8,7, 8,12, 8,17, 8,18, 9,7, 9,11, 10,0, 10,1, 10,6, 10,11, 10,15, 11,5, 11,9, 11,10, 11,14, 12,4, 12,8, 12,13, 13,3, 13,7, 13,12, 13,17, 13,18, 14,3, 14,7, 14,11, 15,0, 15,1, 15,2, 15,14, 15,15, 16,4, 16,9, 16,14, 17,4, 17,9, 17,14, 18,4, 18,9, 18,14,
    // Length and number of words of that length
    15, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 4,16,0,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,5,1, 15,3,1,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,12,0, 11,6,0,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,13,0, 4,0,1, 9,0,1, 9,12,1, 12,5,0, 12,10,0, 14,12,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,0, 13,8,0, 13,13,0,
    // Length and number of words of that length
    5, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,6,0, 6,0,1, 7,14,1, 11,0,1, 12,14,1, 14,12,0, 14,17,0, 14,18,0,
    // Length and number of words of that length
    4, 66,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,5,1, 0,7,0, 0,10,1, 0,11,0, 0,15,0, 0,15,1, 1,0,1, 1,5,1, 1,10,1, 1,15,1, 2,0,1, 2,5,1, 2,10,1, 2,15,1, 3,9,0, 4,3,0, 4,17,0, 4,18,0, 5,2,1, 5,7,1, 6,0,0, 6,1,0, 6,6,0, 6,6,1, 6,15,0, 6,15,1, 7,0,1, 7,5,0, 7,10,0, 7,14,0, 8,4,0, 8,8,0, 8,8,1, 8,13,0, 8,13,1, 9,3,0, 9,12,0, 9,17,0, 9,18,0, 10,2,1, 10,7,1, 11,0,0, 11,1,0, 11,15,0, 11,15,1, 12,0,1, 12,9,0, 12,9,1, 13,8,1, 13,13,1, 15,3,0, 15,7,0, 15,11,0, 16,0,1, 16,5,1, 16,10,1, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,0,1, 18,5,1, 18,10,1, 18,15,1,
    // Length and number of words of that length
    3, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,16,0, 0,17,0, 0,18,0, 3,0,1, 3,14,0, 4,4,0, 4,8,1, 4,12,1, 4,16,1, 5,7,0, 5,12,1, 5,16,1, 6,11,0, 6,11,1, 7,5,1, 7,10,1, 8,0,1, 8,4,1, 8,9,0, 9,8,1, 10,7,0, 10,12,1, 10,16,1, 11,6,1, 11,11,0, 11,11,1, 12,5,1, 12,14,0, 13,0,1, 13,4,0, 13,4,1, 14,0,1, 14,4,1, 14,8,1, 15,16,1, 16,0,0, 16,1,0, 16,2,0, 16,15,0,
    // End marker
    0
  };


  /*
   * Name: 19.08, 19 x 19
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * * _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ * * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g27[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    66,
    // Black field coordinates
    0,4, 0,9, 0,14, 1,4, 1,9, 1,14, 2,4, 2,9, 2,14, 3,6, 4,0, 4,1, 4,2, 4,7, 4,11, 4,12, 4,16, 4,17, 4,18, 5,8, 5,13, 6,4, 6,9, 6,14, 7,4, 7,10, 8,5, 8,11, 8,15, 9,0, 9,1, 9,2, 9,6, 9,12, 9,16, 9,17, 9,18, 10,3, 10,7, 10,13, 11,8, 11,14, 12,4, 12,9, 12,14, 13,5, 13,10, 14,0, 14,1, 14,2, 14,6, 14,7, 14,11, 14,16, 14,17, 14,18, 15,12, 16,4, 16,9, 16,14, 17,4, 17,9, 17,14, 18,4, 18,9, 18,14,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,7,1, 15,0,1,
    // Length and number of words of that length
    10, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 9,15,0,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,15,0, 5,0,1, 7,11,1, 11,0,1, 11,3,0, 11,13,0, 13,11,1,
    // Length and number of words of that length
    7, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,10,0, 12,8,0,
    // Length and number of words of that length
    6, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 15,13,1,
    // Length and number of words of that length
    5, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,13,0, 4,6,0, 5,7,0, 5,14,1, 6,8,0, 7,5,1, 7,9,0, 8,0,1, 8,6,1, 8,10,0, 9,7,1, 9,11,0, 10,8,1, 10,12,0, 10,14,1, 11,9,1, 13,0,1, 14,5,0, 14,10,0,
    // Length and number of words of that length
    4, 74,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,5,1, 0,7,0, 0,10,1, 0,11,0, 0,12,0, 0,15,1, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,5,1, 1,10,1, 1,15,1, 2,0,1, 2,5,1, 2,10,1, 2,15,1, 4,3,1, 5,0,0, 5,1,0, 5,2,0, 5,9,1, 5,12,0, 5,16,0, 5,17,0, 5,18,0, 6,0,1, 6,5,1, 6,10,1, 6,13,0, 6,15,1, 7,0,1, 7,14,0, 8,4,0, 9,5,0, 10,0,0, 10,1,0, 10,2,0, 10,6,0, 10,16,0, 10,17,0, 10,18,0, 11,15,1, 12,0,1, 12,5,1, 12,10,1, 12,15,1, 13,6,1, 14,12,1, 15,0,0, 15,1,0, 15,2,0, 15,6,0, 15,7,0, 15,11,0, 15,16,0, 15,17,0, 15,18,0, 16,0,1, 16,5,1, 16,10,1, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,0,1, 18,5,1, 18,10,1, 18,15,1,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 3,4,0, 3,9,0, 3,14,0, 4,8,1, 4,13,1, 5,11,0, 8,12,1, 8,16,1, 9,3,1, 9,13,1, 10,0,1, 10,4,1, 11,7,0, 13,4,0, 13,9,0, 13,14,0, 14,3,1, 14,8,1, 16,12,0,
    // End marker
    0
  };


  /*
   * Name: 19.09, 19 x 19
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ * * _ _ _ _ * *)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * * _ _ _ * _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ * _ _ _ * * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(* * _ _ _ _ * * _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   */
  const int g28[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    66,
    // Black field coordinates
    0,4, 0,9, 0,14, 1,4, 1,9, 1,14, 2,4, 3,7, 3,11, 3,15, 4,0, 4,1, 4,2, 4,7, 4,11, 4,12, 4,16, 4,17, 4,18, 5,6, 5,10, 6,5, 6,9, 6,14, 7,4, 7,8, 7,14, 8,7, 8,13, 9,0, 9,1, 9,2, 9,6, 9,12, 9,16, 9,17, 9,18, 10,5, 10,11, 11,4, 11,10, 11,14, 12,4, 12,9, 12,13, 13,8, 13,12, 14,0, 14,1, 14,2, 14,6, 14,7, 14,11, 14,16, 14,17, 14,18, 15,3, 15,7, 15,11, 16,14, 17,4, 17,9, 17,14, 18,4, 18,9, 18,14,
    // Length and number of words of that length
    15, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 4,15,0,
    // Length and number of words of that length
    14, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 16,0,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,13,0, 5,11,1, 11,5,0, 13,0,1,
    // Length and number of words of that length
    7, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 3,0,1, 8,0,1, 10,12,1, 12,10,0, 15,12,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 5,0,1, 13,13,0, 13,13,1,
    // Length and number of words of that length
    5, 18,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,10,0, 5,11,0, 6,0,1, 6,10,0, 7,9,0, 7,9,1, 8,8,0, 8,8,1, 8,14,1, 9,7,0, 9,7,1, 10,0,1, 10,6,1, 11,5,1, 12,14,1, 14,8,0, 14,12,0,
    // Length and number of words of that length
    4, 62,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,5,1, 0,10,1, 0,12,0, 0,15,1, 0,16,0, 0,17,0, 0,18,0, 1,0,1, 1,5,1, 1,10,1, 1,15,1, 2,0,1, 2,9,0, 2,14,0, 3,4,0, 4,3,1, 5,0,0, 5,1,0, 5,2,0, 5,12,0, 5,16,0, 5,17,0, 5,18,0, 6,10,1, 6,15,1, 7,0,1, 7,15,1, 10,0,0, 10,1,0, 10,2,0, 10,6,0, 10,16,0, 10,17,0, 10,18,0, 11,0,1, 11,15,1, 12,0,1, 12,5,1, 12,14,0, 13,4,0, 13,9,0, 14,12,1, 15,0,0, 15,1,0, 15,2,0, 15,6,0, 15,16,0, 15,17,0, 15,18,0, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,0,1, 18,5,1, 18,10,1, 18,15,1,
    // Length and number of words of that length
    3, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,11,0, 0,15,0, 3,8,1, 3,12,1, 3,16,1, 4,8,1, 4,13,1, 5,7,0, 5,7,1, 6,6,0, 6,6,1, 7,5,0, 7,5,1, 8,4,0, 8,14,0, 9,3,1, 9,13,0, 9,13,1, 10,12,0, 11,11,0, 11,11,1, 12,10,1, 13,9,1, 14,3,1, 14,8,1, 15,0,1, 15,4,1, 15,8,1, 16,3,0, 16,7,0, 16,11,0,
    // End marker
    0
  };


  /*
   * Name: 19.10, 19 x 19
   * 	(_ _ _ * * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * * _ _ _ _ *)
   * 	(* * * _ _ _ * _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * * _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(* _ _ _ _ _ _ * _ _ _ _ * * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * * _ _ _ _ * _ _ _ _ _ _ *)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ * * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ * _ _ _ * * *)
   * 	(* _ _ _ _ * * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * * _ _ _)
   */
  const int g29[] = {
    // Width and height of crossword grid
    19, 19,
    // Number of black fields
    70,
    // Black field coordinates
    0,4, 0,8, 0,9, 0,14, 0,15, 1,4, 1,9, 1,14, 2,4, 2,9, 2,14, 3,0, 3,7, 3,12, 4,0, 4,1, 4,6, 4,11, 4,12, 4,17, 4,18, 5,5, 5,10, 5,15, 6,4, 6,10, 6,15, 7,3, 7,8, 7,14, 8,7, 8,13, 9,0, 9,1, 9,6, 9,12, 9,17, 9,18, 10,5, 10,11, 11,4, 11,10, 11,15, 12,3, 12,8, 12,14, 13,3, 13,8, 13,13, 14,0, 14,1, 14,6, 14,7, 14,12, 14,17, 14,18, 15,6, 15,11, 15,18, 16,4, 16,9, 16,14, 17,4, 17,9, 17,14, 18,3, 18,4, 18,9, 18,10, 18,14,
    // Length and number of words of that length
    19, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,16,0,
    // Length and number of words of that length
    13, 1,
    // Coordinates where words start and direction (0 = horizontal)
    3,9,0,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,13,0, 11,5,0,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 8,0,1, 10,12,1, 12,15,0,
    // Length and number of words of that length
    6, 6,
    // Coordinates where words start and direction (0 = horizontal)
    1,8,0, 3,1,1, 3,13,1, 12,10,0, 15,0,1, 15,12,1,
    // Length and number of words of that length
    5, 17,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,0, 5,0,1, 5,11,0, 6,5,1, 7,9,1, 8,8,1, 8,14,1, 9,7,0, 9,7,1, 10,0,1, 10,6,1, 11,5,1, 12,9,1, 13,14,1, 14,8,0, 14,13,0,
    // Length and number of words of that length
    4, 78,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,1,0, 0,6,0, 0,10,1, 0,11,0, 0,17,0, 0,18,0, 1,0,1, 1,5,1, 1,10,1, 1,15,0, 1,15,1, 2,0,1, 2,5,1, 2,10,1, 2,15,1, 3,8,1, 3,14,0, 4,2,1, 4,7,0, 4,7,1, 4,13,1, 5,0,0, 5,1,0, 5,6,0, 5,6,1, 5,11,1, 5,12,0, 5,17,0, 5,18,0, 6,0,1, 6,5,0, 6,11,1, 7,4,0, 7,4,1, 7,10,0, 7,15,0, 7,15,1, 8,3,0, 8,8,0, 8,14,0, 9,2,1, 9,13,0, 9,13,1, 10,0,0, 10,1,0, 10,6,0, 10,12,0, 10,17,0, 10,18,0, 11,0,1, 11,11,0, 11,11,1, 12,4,0, 12,4,1, 12,15,1, 13,4,1, 13,9,1, 14,2,1, 14,3,0, 14,8,1, 14,13,1, 15,0,0, 15,1,0, 15,7,0, 15,7,1, 15,12,0, 15,17,0, 16,0,1, 16,5,1, 16,10,1, 16,15,1, 17,0,1, 17,5,1, 17,10,1, 17,15,1, 18,5,1, 18,15,1,
    // Length and number of words of that length
    3, 18,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,5,1, 0,7,0, 0,12,0, 0,16,1, 3,4,0, 5,16,1, 6,16,1, 7,0,1, 11,16,1, 12,0,1, 13,0,1, 13,14,0, 16,6,0, 16,11,0, 16,18,0, 18,0,1, 18,11,1,
    // End marker
    0
  };


  /*
   * Name: 21.01, 21 x 21
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * * * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * * _ _ _ _ _ _)
   * 	(* * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * *)
   * 	(_ _ _ _ _ _ * * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * * * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g30[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    68,
    // Black field coordinates
    0,4, 0,10, 0,16, 1,4, 1,10, 1,16, 2,4, 2,16, 3,8, 3,14, 4,0, 4,1, 4,7, 4,13, 5,6, 5,19, 5,20, 6,5, 6,11, 6,17, 7,4, 7,10, 7,11, 7,12, 7,16, 8,3, 8,9, 8,15, 9,7, 9,13, 10,0, 10,1, 10,2, 10,7, 10,13, 10,18, 10,19, 10,20, 11,7, 11,13, 12,5, 12,11, 12,17, 13,4, 13,8, 13,9, 13,10, 13,16, 14,3, 14,9, 14,15, 15,0, 15,1, 15,14, 16,7, 16,13, 16,19, 16,20, 17,6, 17,12, 18,4, 18,16, 19,4, 19,10, 19,16, 20,4, 20,10, 20,16,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,7,1, 15,2,1,
    // Length and number of words of that length
    11, 4,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 4,14,0, 6,6,0, 18,5,1,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,18,0, 11,2,0, 11,18,0,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    4,8,0, 8,12,0,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,15,0, 3,0,1, 13,5,0, 13,11,0, 13,17,0, 17,13,1,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,12,0, 4,14,1, 9,0,1, 9,14,1, 11,0,1, 11,14,1, 14,8,0, 16,0,1,
    // Length and number of words of that length
    6, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 0,17,0, 3,15,1, 5,0,1, 15,3,0, 15,9,0, 15,15,0, 15,15,1, 17,0,1,
    // Length and number of words of that length
    5, 50,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 0,11,1, 0,19,0, 0,20,0, 1,5,1, 1,11,1, 2,10,0, 3,9,1, 4,2,1, 4,8,1, 5,0,0, 5,1,0, 6,0,1, 6,6,1, 6,12,1, 7,5,0, 7,5,1, 7,17,0, 8,4,0, 8,4,1, 8,10,0, 8,10,1, 8,16,0, 8,16,1, 9,3,0, 9,8,1, 9,15,0, 10,8,1, 11,8,1, 11,19,0, 11,20,0, 12,0,1, 12,6,1, 12,12,1, 13,11,1, 14,4,1, 14,10,0, 14,10,1, 14,16,1, 16,0,0, 16,1,0, 16,8,1, 16,14,0, 16,14,1, 17,7,1, 19,5,1, 19,11,1, 20,5,1, 20,11,1,
    // Length and number of words of that length
    4, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,7,0, 0,13,0, 0,17,1, 1,0,1, 1,17,1, 2,0,1, 2,17,1, 3,4,0, 3,16,0, 5,7,0, 5,13,0, 6,19,0, 6,20,0, 7,0,1, 7,17,1, 8,11,0, 9,9,0, 10,3,1, 10,14,1, 11,0,0, 11,1,0, 12,7,0, 12,13,0, 13,0,1, 13,17,1, 14,4,0, 14,16,0, 17,7,0, 17,13,0, 17,19,0, 17,20,0, 18,0,1, 18,17,1, 19,0,1, 19,17,1, 20,0,1, 20,17,1,
    // Length and number of words of that length
    3, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 6,18,1, 7,13,1, 8,0,1, 12,18,1, 13,5,1, 14,0,1, 18,6,0, 18,12,0,
    // End marker
    0
  };


  /*
   * Name: 21.02, 21 x 21
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g31[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    72,
    // Black field coordinates
    0,4, 0,10, 0,16, 1,4, 1,10, 1,16, 2,4, 2,10, 2,16, 3,9, 3,15, 4,0, 4,1, 4,2, 4,8, 4,12, 4,18, 4,19, 4,20, 5,3, 5,7, 5,13, 6,6, 6,14, 7,5, 7,10, 7,15, 8,4, 8,9, 8,16, 9,8, 9,17, 10,0, 10,1, 10,2, 10,7, 10,13, 10,18, 10,19, 10,20, 11,3, 11,12, 12,4, 12,11, 12,16, 13,5, 13,10, 13,15, 14,6, 14,14, 15,7, 15,13, 15,17, 16,0, 16,1, 16,2, 16,8, 16,12, 16,18, 16,19, 16,20, 17,5, 17,11, 18,4, 18,10, 18,16, 19,4, 19,10, 19,16, 20,4, 20,10, 20,16,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,11,0, 9,9,0,
    // Length and number of words of that length
    9, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,17,0, 3,0,1, 12,3,0, 17,12,1,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    9,0,1, 9,9,1, 11,4,1, 11,13,1,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 5,14,1, 6,7,1, 7,6,0, 7,14,0, 14,7,1, 14,15,0, 15,0,1,
    // Length and number of words of that length
    6, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,14,0, 5,12,0, 6,0,1, 6,15,1, 8,10,1, 10,8,0, 12,5,1, 14,0,1, 14,15,1, 15,6,0, 15,14,0,
    // Length and number of words of that length
    5, 54,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,1, 0,7,0, 0,11,1, 0,13,0, 1,5,1, 1,11,1, 2,5,1, 2,11,1, 3,4,0, 3,10,1, 3,16,0, 3,16,1, 4,3,1, 4,13,1, 5,0,0, 5,1,0, 5,2,0, 5,8,1, 5,18,0, 5,19,0, 5,20,0, 6,3,0, 7,0,1, 7,16,1, 8,5,0, 8,10,0, 8,15,0, 10,8,1, 10,17,0, 11,0,0, 11,1,0, 11,2,0, 11,18,0, 11,19,0, 11,20,0, 13,0,1, 13,4,0, 13,16,0, 13,16,1, 15,8,1, 16,3,1, 16,7,0, 16,13,0, 16,13,1, 16,17,0, 17,0,1, 17,6,1, 18,5,1, 18,11,1, 19,5,1, 19,11,1, 20,5,1, 20,11,1,
    // Length and number of words of that length
    4, 50,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,8,0, 0,12,0, 0,17,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,17,1, 2,0,1, 2,17,1, 3,10,0, 4,9,0, 5,8,0, 6,7,0, 6,13,0, 7,6,1, 7,11,1, 8,0,1, 8,5,1, 8,17,1, 10,3,1, 10,14,1, 11,7,0, 11,13,0, 12,0,1, 12,12,0, 12,12,1, 12,17,1, 13,6,1, 13,11,0, 13,11,1, 14,10,0, 17,0,0, 17,1,0, 17,2,0, 17,8,0, 17,12,0, 17,18,0, 17,19,0, 17,20,0, 18,0,1, 18,17,1, 19,0,1, 19,17,1, 20,0,1, 20,17,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 0,15,0, 4,9,1, 4,15,0, 5,0,1, 5,4,1, 9,4,0, 9,16,0, 9,18,1, 11,0,1, 14,5,0, 15,14,1, 15,18,1, 16,9,1, 18,5,0, 18,11,0,
    // End marker
    0
  };


  /*
   * Name: 21.03, 21 x 21
   * 	(_ _ _ _ * * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * * _ _ _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * * _ _ _ _ * _ _ _)
   * 	(* * _ _ _ * _ _ _ _ _ * _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ * _ _ _ _ _ _ *)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(* _ _ _ _ _ _ * _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ * _ _ _ _ _ * _ _ _ * *)
   * 	(_ _ _ * _ _ _ _ * * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ _ _ * * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * * _ _ _ _)
   */
  const int g32[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    79,
    // Black field coordinates
    0,5, 0,11, 0,12, 0,17, 1,5, 1,11, 1,17, 2,11, 3,3, 3,10, 3,15, 3,16, 4,0, 4,1, 4,2, 4,8, 4,9, 4,15, 5,0, 5,4, 5,5, 5,14, 5,18, 5,19, 5,20, 6,6, 6,13, 7,7, 7,12, 8,8, 8,16, 9,0, 9,1, 9,2, 9,3, 9,9, 9,15, 9,16, 10,3, 10,10, 10,17, 11,4, 11,5, 11,11, 11,17, 11,18, 11,19, 11,20, 12,4, 12,12, 13,8, 13,13, 14,7, 14,14, 15,0, 15,1, 15,2, 15,6, 15,15, 15,16, 15,20, 16,5, 16,11, 16,12, 16,18, 16,19, 16,20, 17,4, 17,5, 17,10, 17,17, 18,9, 19,3, 19,9, 19,15, 20,3, 20,8, 20,9, 20,15,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,0,1, 18,10,1,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,12,1, 18,0,1,
    // Length and number of words of that length
    8, 12,
    // Coordinates where words start and direction (0 = horizontal)
    2,17,0, 3,11,0, 5,6,1, 6,14,0, 7,6,0, 7,13,1, 8,0,1, 10,9,0, 11,3,0, 12,13,1, 13,0,1, 15,7,1,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 6,14,1, 7,0,1, 8,9,1, 12,5,1, 13,14,1, 14,0,1, 14,13,0,
    // Length and number of words of that length
    6, 18,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,13,0, 1,12,0, 3,4,1, 4,10,0, 6,0,1, 6,7,1, 7,13,0, 8,7,0, 10,4,1, 10,11,1, 11,10,0, 14,8,0, 14,8,1, 14,15,1, 15,7,0, 15,14,0, 17,11,1,
    // Length and number of words of that length
    5, 42,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,4,0, 0,6,1, 0,14,0, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,6,1, 1,12,1, 4,3,0, 4,3,1, 4,10,1, 4,16,1, 6,4,0, 6,5,0, 6,18,0, 6,19,0, 6,20,0, 9,4,1, 9,10,1, 10,0,0, 10,1,0, 10,2,0, 10,15,0, 10,16,0, 11,6,1, 11,12,1, 12,17,0, 16,0,0, 16,0,1, 16,1,0, 16,2,0, 16,6,0, 16,6,1, 16,13,1, 16,16,0, 19,4,1, 19,10,1, 19,16,1, 20,10,1, 20,16,1,
    // Length and number of words of that length
    4, 34,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,2,0, 0,8,0, 0,9,0, 0,13,1, 3,11,1, 3,17,1, 4,16,0, 5,1,0, 5,2,0, 5,9,0, 5,15,0, 7,8,1, 8,12,0, 8,17,1, 9,8,0, 9,17,1, 11,0,1, 12,0,1, 12,5,0, 12,11,0, 12,18,0, 12,19,0, 13,4,0, 13,9,1, 17,0,1, 17,6,1, 17,11,0, 17,12,0, 17,18,0, 17,19,0, 17,20,0, 20,4,1,
    // Length and number of words of that length
    3, 26,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,10,0, 0,15,0, 0,16,0, 0,18,1, 1,18,1, 2,5,0, 3,0,1, 5,1,1, 5,8,0, 5,15,1, 6,0,0, 10,0,1, 10,18,1, 12,20,0, 13,12,0, 15,3,1, 15,17,1, 16,15,0, 17,18,1, 18,4,0, 18,5,0, 18,10,0, 18,17,0, 19,0,1, 20,0,1,
    // End marker
    0
  };


  /*
   * Name: 21.04, 21 x 21
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   */
  const int g33[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    63,
    // Black field coordinates
    0,7, 0,13, 1,7, 1,13, 2,7, 2,13, 3,3, 3,11, 3,17, 4,4, 4,10, 4,16, 5,5, 5,9, 5,15, 6,8, 6,12, 7,0, 7,1, 7,2, 7,7, 7,13, 7,18, 7,19, 7,20, 8,6, 8,14, 9,5, 9,11, 9,17, 10,4, 10,10, 10,16, 11,3, 11,9, 11,15, 12,6, 12,14, 13,0, 13,1, 13,2, 13,7, 13,13, 13,18, 13,19, 13,20, 14,8, 14,12, 15,5, 15,11, 15,15, 16,4, 16,10, 16,16, 17,3, 17,9, 17,17, 18,7, 18,13, 19,7, 19,13, 20,7, 20,13,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,14,0, 6,0,1, 6,13,1, 13,6,0, 13,14,0, 14,0,1, 14,13,1,
    // Length and number of words of that length
    7, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,14,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,14,1, 2,0,1, 2,14,1, 3,4,1, 4,3,0, 7,8,0, 7,12,0, 8,7,1, 10,17,0, 12,7,1, 14,0,0, 14,1,0, 14,2,0, 14,18,0, 14,19,0, 14,20,0, 17,10,1, 18,0,1, 18,14,1, 19,0,1, 19,14,1, 20,0,1, 20,14,1,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,12,0, 8,0,1, 8,15,1, 12,0,1, 12,15,1, 15,8,0, 15,12,0,
    // Length and number of words of that length
    5, 56,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,8,1, 0,9,0, 0,15,0, 1,8,1, 2,8,1, 3,12,1, 4,5,1, 4,11,0, 4,11,1, 4,17,0, 5,0,1, 5,4,0, 5,10,0, 5,10,1, 5,16,0, 5,16,1, 6,9,0, 6,15,0, 7,8,1, 8,0,0, 8,1,0, 8,2,0, 8,7,0, 8,13,0, 8,18,0, 8,19,0, 8,20,0, 9,0,1, 9,6,1, 9,12,1, 10,5,0, 10,5,1, 10,11,0, 10,11,1, 11,4,0, 11,4,1, 11,10,0, 11,10,1, 11,16,0, 11,16,1, 12,3,0, 12,9,0, 13,8,1, 15,0,1, 15,6,1, 15,16,1, 16,5,0, 16,5,1, 16,11,0, 16,11,1, 16,15,0, 17,4,1, 18,8,1, 19,8,1, 20,8,1,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,16,0, 3,7,0, 3,13,0, 4,0,1, 4,17,1, 7,3,1, 7,14,1, 10,0,1, 10,17,1, 13,3,1, 13,14,1, 14,7,0, 14,13,0, 16,0,1, 16,17,1, 17,4,0, 17,10,0, 17,16,0,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,11,0, 0,17,0, 3,0,1, 3,18,1, 5,6,1, 6,5,0, 6,9,1, 9,6,0, 9,14,0, 9,18,1, 11,0,1, 12,15,0, 14,9,1, 15,12,1, 17,0,1, 17,18,1, 18,3,0, 18,9,0, 18,17,0,
    // End marker
    0
  };


  /*
   * Name: 21.05, 21 x 21
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ * * * _ _ _ * * * _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * * * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ * * * _ _ _ * * * _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   */
  const int g34[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    73,
    // Black field coordinates
    0,6, 0,14, 1,6, 1,14, 2,6, 2,14, 3,3, 3,9, 3,17, 4,4, 4,10, 4,16, 5,5, 5,11, 5,15, 6,0, 6,1, 6,2, 6,6, 6,7, 6,8, 6,12, 6,13, 6,14, 6,18, 6,19, 6,20, 7,6, 7,14, 8,6, 8,14, 9,5, 9,10, 9,17, 10,4, 10,9, 10,10, 10,11, 10,16, 11,3, 11,10, 11,15, 12,6, 12,14, 13,6, 13,14, 14,0, 14,1, 14,2, 14,6, 14,7, 14,8, 14,12, 14,13, 14,14, 14,18, 14,19, 14,20, 15,5, 15,9, 15,15, 16,4, 16,10, 16,16, 17,3, 17,11, 17,17, 18,6, 18,14, 19,6, 19,14, 20,6, 20,14,
    // Length and number of words of that length
    7, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,1, 1,7,1, 2,7,1, 3,10,1, 4,3,0, 7,0,0, 7,1,0, 7,2,0, 7,7,0, 7,7,1, 7,8,0, 7,12,0, 7,13,0, 7,18,0, 7,19,0, 7,20,0, 8,7,1, 10,17,0, 12,7,1, 13,7,1, 17,4,1, 18,7,1, 19,7,1, 20,7,1,
    // Length and number of words of that length
    6, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,7,0, 0,8,0, 0,12,0, 0,13,0, 0,15,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,15,1, 2,0,1, 2,15,1, 4,9,0, 7,0,1, 7,15,1, 8,0,1, 8,15,1, 9,11,1, 11,4,1, 11,11,0, 12,0,1, 12,15,1, 13,0,1, 13,15,1, 15,0,0, 15,1,0, 15,2,0, 15,7,0, 15,8,0, 15,12,0, 15,13,0, 15,18,0, 15,19,0, 15,20,0, 18,0,1, 18,15,1, 19,0,1, 19,15,1, 20,0,1, 20,15,1,
    // Length and number of words of that length
    5, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 0,15,0, 3,4,1, 4,5,1, 4,11,1, 4,17,0, 5,0,1, 5,4,0, 5,6,1, 5,16,0, 5,16,1, 6,15,0, 9,0,1, 10,5,0, 11,4,0, 11,16,0, 11,16,1, 12,3,0, 15,0,1, 15,10,1, 15,16,1, 16,5,0, 16,5,1, 16,9,0, 16,11,1, 16,15,0, 17,12,1,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,16,0, 4,0,1, 4,17,1, 5,10,0, 6,11,0, 9,6,1, 10,0,1, 10,5,1, 10,12,1, 10,17,1, 11,9,0, 11,11,1, 12,10,0, 16,0,1, 16,17,1, 17,4,0, 17,10,0, 17,16,0,
    // Length and number of words of that length
    3, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,17,0, 3,0,1, 3,6,0, 3,14,0, 3,18,1, 5,12,1, 6,3,1, 6,5,0, 6,9,1, 6,15,1, 9,6,0, 9,14,0, 9,18,1, 11,0,1, 12,15,0, 14,3,1, 14,9,1, 14,15,1, 15,6,0, 15,6,1, 15,14,0, 17,0,1, 17,18,1, 18,3,0, 18,11,0, 18,17,0,
    // End marker
    0
  };


  /*
   * Name: 21.06, 21 x 21
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g35[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    68,
    // Black field coordinates
    0,4, 0,10, 0,16, 1,4, 1,10, 1,16, 2,4, 2,16, 3,8, 3,12, 4,0, 4,1, 4,2, 4,7, 4,13, 4,18, 4,19, 4,20, 5,6, 5,14, 6,5, 6,11, 6,15, 7,4, 7,10, 7,16, 8,3, 8,9, 8,17, 9,6, 9,12, 10,0, 10,1, 10,7, 10,13, 10,19, 10,20, 11,8, 11,14, 12,3, 12,11, 12,17, 13,4, 13,10, 13,16, 14,5, 14,9, 14,15, 15,6, 15,14, 16,0, 16,1, 16,2, 16,7, 16,13, 16,18, 16,19, 16,20, 17,8, 17,12, 18,4, 18,16, 19,4, 19,10, 19,16, 20,4, 20,10, 20,16,
    // Length and number of words of that length
    11, 4,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 5,2,0, 5,18,0, 18,5,1,
    // Length and number of words of that length
    8, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,17,0, 3,0,1, 3,13,1, 9,13,1, 11,0,1, 13,3,0, 13,11,0, 13,17,0, 17,0,1, 17,13,1,
    // Length and number of words of that length
    7, 8,
    // Coordinates where words start and direction (0 = horizontal)
    4,8,0, 5,7,1, 7,5,0, 7,15,0, 8,10,1, 10,12,0, 12,4,1, 15,7,1,
    // Length and number of words of that length
    6, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 0,15,0, 5,0,1, 5,15,1, 9,0,1, 11,15,1, 15,0,1, 15,5,0, 15,9,0, 15,15,0, 15,15,1,
    // Length and number of words of that length
    5, 54,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 0,11,1, 0,14,0, 1,5,1, 1,11,1, 2,10,0, 4,8,1, 4,12,0, 5,0,0, 5,1,0, 5,7,0, 5,13,0, 5,19,0, 5,20,0, 6,0,1, 6,6,1, 6,14,0, 6,16,1, 7,5,1, 7,11,0, 7,11,1, 8,4,0, 8,4,1, 8,10,0, 8,16,0, 9,7,1, 9,9,0, 10,2,1, 10,6,0, 10,8,1, 10,14,1, 11,0,0, 11,1,0, 11,7,0, 11,9,1, 11,13,0, 11,19,0, 11,20,0, 12,8,0, 12,12,1, 13,5,1, 13,11,1, 14,0,1, 14,10,0, 14,10,1, 14,16,1, 16,6,0, 16,8,1, 16,14,0, 19,5,1, 19,11,1, 20,5,1, 20,11,1,
    // Length and number of words of that length
    4, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,7,0, 0,13,0, 0,17,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,17,1, 2,0,1, 2,17,1, 3,4,0, 3,16,0, 4,3,1, 4,14,1, 7,0,1, 7,17,1, 13,0,1, 13,17,1, 14,4,0, 14,16,0, 16,3,1, 16,14,1, 17,0,0, 17,1,0, 17,2,0, 17,7,0, 17,13,0, 17,18,0, 17,19,0, 17,20,0, 18,0,1, 18,17,1, 19,0,1, 19,17,1, 20,0,1, 20,17,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,12,0, 3,9,1, 6,6,0, 6,12,1, 8,0,1, 8,18,1, 9,3,0, 9,17,0, 12,0,1, 12,14,0, 12,18,1, 14,6,1, 17,9,1, 18,8,0, 18,12,0,
    // End marker
    0
  };


  /*
   * Name: 21.07, 21 x 21
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * * * * * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g36[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    73,
    // Black field coordinates
    0,4, 0,10, 0,16, 1,4, 1,10, 1,16, 2,10, 3,5, 3,9, 3,15, 4,0, 4,1, 4,6, 4,14, 4,19, 4,20, 5,3, 5,11, 5,17, 6,4, 6,8, 6,12, 6,16, 7,7, 7,13, 8,6, 8,10, 8,14, 9,3, 9,10, 9,15, 10,0, 10,1, 10,2, 10,8, 10,9, 10,10, 10,11, 10,12, 10,18, 10,19, 10,20, 11,5, 11,10, 11,17, 12,6, 12,10, 12,14, 13,7, 13,13, 14,4, 14,8, 14,12, 14,16, 15,3, 15,9, 15,17, 16,0, 16,1, 16,6, 16,14, 16,19, 16,20, 17,5, 17,11, 17,15, 18,10, 19,4, 19,10, 19,16, 20,4, 20,10, 20,16,
    // Length and number of words of that length
    10, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,18,0, 2,0,1, 2,11,1, 11,2,0, 11,18,0, 18,0,1, 18,11,1,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,13,0, 4,5,0, 4,7,1, 5,4,1, 7,0,1, 7,4,0, 7,14,1, 7,16,0, 10,15,0, 13,0,1, 13,14,1, 14,7,0, 14,13,0, 15,10,1, 16,7,1,
    // Length and number of words of that length
    6, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,12,0, 4,9,0, 8,0,1, 8,15,1, 9,4,1, 11,11,0, 11,11,1, 12,0,1, 12,15,1, 15,8,0, 15,12,0,
    // Length and number of words of that length
    5, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,1, 0,11,0, 0,11,1, 0,17,0, 1,5,1, 1,11,1, 3,0,1, 3,10,0, 3,10,1, 3,16,1, 4,15,0, 5,0,0, 5,1,0, 5,12,1, 5,19,0, 5,20,0, 6,17,0, 7,8,1, 8,7,0, 8,13,0, 9,16,1, 10,3,0, 10,3,1, 10,13,1, 11,0,0, 11,0,1, 11,1,0, 11,19,0, 11,20,0, 12,5,0, 13,8,1, 13,10,0, 15,4,1, 16,3,0, 16,9,0, 16,17,0, 17,0,1, 17,6,1, 17,16,1, 19,5,1, 19,11,1, 20,5,1, 20,11,1,
    // Length and number of words of that length
    4, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,6,0, 0,14,0, 0,17,1, 0,19,0, 0,20,0, 1,0,1, 1,17,1, 2,4,0, 2,16,0, 4,2,1, 4,15,1, 6,0,1, 6,11,0, 6,17,1, 9,11,1, 11,6,1, 11,9,0, 14,0,1, 14,17,1, 15,4,0, 15,16,0, 16,2,1, 16,15,1, 17,0,0, 17,1,0, 17,6,0, 17,14,0, 17,19,0, 17,20,0, 19,0,1, 19,17,1, 20,0,1, 20,17,1,
    // Length and number of words of that length
    3, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,9,0, 0,15,0, 3,6,1, 5,0,1, 5,6,0, 5,14,0, 5,18,1, 6,3,0, 6,5,1, 6,9,1, 6,13,1, 7,8,0, 7,12,0, 8,7,1, 8,11,1, 9,0,1, 9,6,0, 9,14,0, 11,8,0, 11,12,0, 11,18,1, 12,7,1, 12,11,1, 12,17,0, 13,6,0, 13,14,0, 14,5,1, 14,9,1, 14,13,1, 15,0,1, 15,18,1, 17,12,1, 18,5,0, 18,11,0, 18,15,0,
    // End marker
    0
  };


  /*
   * Name: 21.08, 21 x 21
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * * _ _ _ * * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g37[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    76,
    // Black field coordinates
    0,4, 0,10, 0,16, 1,4, 1,10, 1,16, 2,4, 2,10, 2,16, 3,8, 3,14, 4,0, 4,1, 4,2, 4,7, 4,13, 4,18, 4,19, 4,20, 5,6, 5,12, 6,5, 6,6, 6,11, 6,17, 7,4, 7,10, 7,16, 8,3, 8,10, 8,15, 9,8, 9,9, 9,14, 10,0, 10,1, 10,2, 10,7, 10,13, 10,18, 10,19, 10,20, 11,6, 11,11, 11,12, 12,5, 12,10, 12,17, 13,4, 13,10, 13,16, 14,3, 14,9, 14,14, 14,15, 15,8, 15,14, 16,0, 16,1, 16,2, 16,7, 16,13, 16,18, 16,19, 16,20, 17,6, 17,12, 18,4, 18,10, 18,16, 19,4, 19,10, 19,16, 20,4, 20,10, 20,16,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 12,11,0,
    // Length and number of words of that length
    8, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,15,0, 3,0,1, 5,13,1, 9,0,1, 11,13,1, 13,5,0, 13,17,0, 15,0,1, 17,13,1,
    // Length and number of words of that length
    6, 14,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 0,17,0, 3,15,1, 5,0,1, 8,4,1, 9,15,1, 11,0,1, 12,11,1, 15,3,0, 15,9,0, 15,15,0, 15,15,1, 17,0,1,
    // Length and number of words of that length
    5, 61,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 0,11,1, 0,12,0, 1,5,1, 1,11,1, 2,5,1, 2,11,1, 3,9,1, 4,8,0, 4,8,1, 4,14,0, 5,0,0, 5,1,0, 5,2,0, 5,7,0, 5,7,1, 5,13,0, 5,18,0, 5,19,0, 5,20,0, 6,0,1, 6,12,0, 6,12,1, 7,5,0, 7,5,1, 7,11,1, 7,17,0, 8,4,0, 8,16,0, 8,16,1, 9,3,0, 9,15,0, 10,8,0, 10,8,1, 11,0,0, 11,1,0, 11,2,0, 11,7,0, 11,13,0, 11,18,0, 11,19,0, 11,20,0, 12,0,1, 12,6,0, 12,12,0, 13,5,1, 13,11,1, 14,4,1, 14,16,1, 15,9,1, 16,8,0, 16,8,1, 16,14,0, 17,7,1, 18,5,1, 18,11,1, 19,5,1, 19,11,1, 20,5,1, 20,11,1,
    // Length and number of words of that length
    4, 54,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,7,0, 0,13,0, 0,17,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,17,1, 2,0,1, 2,17,1, 3,4,0, 3,10,0, 3,16,0, 4,3,1, 4,14,1, 6,7,1, 7,0,1, 7,6,0, 7,11,0, 7,17,1, 8,11,1, 9,10,1, 10,3,1, 10,9,0, 10,14,0, 10,14,1, 11,7,1, 12,6,1, 13,0,1, 13,17,1, 14,4,0, 14,10,0, 14,10,1, 14,16,0, 16,3,1, 16,14,1, 17,0,0, 17,1,0, 17,2,0, 17,7,0, 17,13,0, 17,18,0, 17,19,0, 17,20,0, 18,0,1, 18,17,1, 19,0,1, 19,17,1, 20,0,1, 20,17,1,
    // Length and number of words of that length
    3, 9,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 6,18,1, 8,0,1, 9,10,0, 12,18,1, 14,0,1, 18,6,0, 18,12,0,
    // End marker
    0
  };


  /*
   * Name: 21.09, 21 x 21
   * 	(* * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * *)
   * 	(* _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ *)
   * 	(* * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * *)
   */
  const int g38[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    75,
    // Black field coordinates
    0,0, 0,1, 0,7, 0,13, 0,19, 0,20, 1,0, 1,7, 1,13, 1,20, 2,7, 2,13, 3,3, 3,11, 3,17, 4,4, 4,10, 4,16, 5,5, 5,9, 5,15, 6,8, 6,14, 7,0, 7,1, 7,2, 7,7, 7,13, 7,18, 7,19, 7,20, 8,6, 8,12, 9,5, 9,11, 9,17, 10,4, 10,10, 10,16, 11,3, 11,9, 11,15, 12,8, 12,14, 13,0, 13,1, 13,2, 13,7, 13,13, 13,18, 13,19, 13,20, 14,6, 14,12, 15,5, 15,11, 15,15, 16,4, 16,10, 16,16, 17,3, 17,9, 17,17, 18,7, 18,13, 19,0, 19,7, 19,13, 19,20, 20,0, 20,1, 20,7, 20,13, 20,19, 20,20,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,12,0, 6,0,1, 8,13,1, 12,0,1, 13,8,0, 13,14,0, 14,13,1,
    // Length and number of words of that length
    7, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,18,0, 2,0,1, 2,14,1, 3,4,1, 4,3,0, 10,17,0, 14,2,0, 14,18,0, 17,10,1, 18,0,1, 18,14,1,
    // Length and number of words of that length
    6, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 1,1,0, 1,1,1, 1,14,1, 1,19,0, 6,15,1, 8,0,1, 12,15,1, 14,0,1, 14,1,0, 14,19,0, 15,6,0, 15,12,0, 19,1,1, 19,14,1,
    // Length and number of words of that length
    5, 72,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,1, 0,5,0, 0,8,1, 0,9,0, 0,14,1, 0,15,0, 1,8,1, 2,0,0, 2,8,1, 2,20,0, 3,12,1, 4,5,1, 4,11,0, 4,11,1, 4,17,0, 5,0,1, 5,4,0, 5,10,0, 5,10,1, 5,16,0, 5,16,1, 6,9,0, 6,9,1, 6,15,0, 7,8,0, 7,8,1, 7,14,0, 8,0,0, 8,1,0, 8,2,0, 8,7,0, 8,7,1, 8,13,0, 8,18,0, 8,19,0, 8,20,0, 9,0,1, 9,6,0, 9,6,1, 9,12,0, 9,12,1, 10,5,0, 10,5,1, 10,11,0, 10,11,1, 11,4,0, 11,4,1, 11,10,0, 11,10,1, 11,16,0, 11,16,1, 12,3,0, 12,9,0, 12,9,1, 13,8,1, 14,0,0, 14,7,1, 14,20,0, 15,0,1, 15,6,1, 15,16,1, 16,5,0, 16,5,1, 16,11,0, 16,11,1, 16,15,0, 17,4,1, 18,8,1, 19,8,1, 20,2,1, 20,8,1, 20,14,1,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,16,0, 3,7,0, 3,13,0, 4,0,1, 4,17,1, 7,3,1, 7,14,1, 10,0,1, 10,17,1, 13,3,1, 13,14,1, 14,7,0, 14,13,0, 16,0,1, 16,17,1, 17,4,0, 17,10,0, 17,16,0,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,11,0, 0,17,0, 3,0,1, 3,18,1, 5,6,1, 6,5,0, 9,18,1, 11,0,1, 12,15,0, 15,12,1, 17,0,1, 17,18,1, 18,3,0, 18,9,0, 18,17,0,
    // End marker
    0
  };


  /*
   * Name: 21.10, 21 x 21
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ * _ _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ _ * _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   */
  const int g39[] = {
    // Width and height of crossword grid
    21, 21,
    // Number of black fields
    58,
    // Black field coordinates
    0,7, 0,13, 1,7, 1,13, 2,7, 2,13, 3,3, 3,17, 4,4, 4,12, 4,16, 5,5, 5,11, 5,15, 6,6, 6,10, 6,14, 7,0, 7,1, 7,2, 7,9, 7,18, 7,19, 7,20, 8,8, 8,16, 9,7, 9,15, 10,6, 10,14, 11,5, 11,13, 12,4, 12,12, 13,0, 13,1, 13,2, 13,11, 13,18, 13,19, 13,20, 14,6, 14,10, 14,14, 15,5, 15,9, 15,15, 16,4, 16,8, 16,16, 17,3, 17,17, 18,7, 18,13, 19,7, 19,13, 20,7, 20,13,
    // Length and number of words of that length
    13, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,4,1, 4,3,0, 4,17,0, 17,4,1,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 3,13,0, 7,10,1, 8,0,1, 10,7,0, 12,13,1, 13,3,1, 13,12,0,
    // Length and number of words of that length
    7, 42,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,9,0, 0,14,1, 0,18,0, 0,19,0, 0,20,0, 1,0,1, 1,14,1, 2,0,1, 2,14,1, 4,5,1, 5,4,0, 5,12,0, 6,11,0, 7,10,0, 8,9,0, 8,9,1, 9,0,1, 9,8,0, 9,8,1, 9,16,0, 10,7,1, 11,6,1, 11,14,1, 12,5,1, 14,0,0, 14,1,0, 14,2,0, 14,11,0, 14,18,0, 14,19,0, 14,20,0, 16,9,1, 18,0,1, 18,14,1, 19,0,1, 19,14,1, 20,0,1, 20,14,1,
    // Length and number of words of that length
    6, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,10,0, 0,14,0, 3,7,0, 6,0,1, 6,15,1, 7,3,1, 10,0,1, 10,15,1, 12,13,0, 13,12,1, 14,0,1, 14,15,1, 15,6,0, 15,10,0, 15,14,0,
    // Length and number of words of that length
    5, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,8,1, 0,11,0, 0,15,0, 1,8,1, 2,8,1, 5,0,1, 5,6,1, 5,16,1, 6,5,0, 8,0,0, 8,1,0, 8,2,0, 8,18,0, 8,19,0, 8,20,0, 9,16,1, 10,15,0, 11,0,1, 15,0,1, 15,10,1, 15,16,1, 16,5,0, 16,9,0, 16,15,0, 18,8,1, 19,8,1, 20,8,1,
    // Length and number of words of that length
    4, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,12,0, 0,16,0, 4,0,1, 4,17,1, 8,17,1, 12,0,1, 16,0,1, 16,17,1, 17,4,0, 17,8,0, 17,16,0,
    // Length and number of words of that length
    3, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,17,0, 3,0,1, 3,18,1, 4,13,1, 5,12,1, 5,16,0, 6,7,1, 6,11,1, 6,15,0, 7,6,0, 7,14,0, 11,6,0, 11,14,0, 12,5,0, 13,4,0, 14,7,1, 14,11,1, 15,6,1, 16,5,1, 17,0,1, 17,18,1, 18,3,0, 18,17,0,
    // End marker
    0
  };


  /*
   * Name: 23.01, 23 x 23
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ _ _ _ * * _ _ _ _ _ _)
   * 	(* * * * _ _ _ * _ _ _ * _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ * * * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * * _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ * * * _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * * * _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ * _ _ _ * _ _ _ * * * *)
   * 	(_ _ _ _ _ _ * * _ _ _ _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _)
   */
  const int g40[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    89,
    // Black field coordinates
    0,5, 0,11, 0,17, 1,5, 1,11, 1,17, 2,5, 2,11, 2,17, 3,4, 3,5, 4,3, 4,8, 4,12, 4,16, 4,21, 4,22, 5,7, 5,15, 6,0, 6,1, 6,6, 6,10, 6,14, 6,18, 7,5, 7,9, 7,13, 7,17, 7,18, 8,3, 8,8, 8,12, 8,19, 9,3, 9,8, 9,21, 9,22, 10,6, 10,11, 10,16, 11,5, 11,6, 11,7, 11,11, 11,15, 11,16, 11,17, 12,6, 12,11, 12,16, 13,0, 13,1, 13,14, 13,19, 14,3, 14,10, 14,14, 14,19, 15,4, 15,5, 15,9, 15,13, 15,17, 16,4, 16,8, 16,12, 16,16, 16,21, 16,22, 17,7, 17,15, 18,0, 18,1, 18,6, 18,10, 18,14, 18,19, 19,17, 19,18, 20,5, 20,11, 20,17, 21,5, 21,11, 21,17, 22,5, 22,11, 22,17,
    // Length and number of words of that length
    23, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,20,0,
    // Length and number of words of that length
    17, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,6,1, 19,0,1,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    9,9,1, 13,2,1,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    4,4,0, 8,18,0,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,19,0, 15,3,0,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 0,13,0, 3,11,0, 5,0,1, 5,8,1, 5,16,1, 7,10,0, 8,9,0, 8,13,0, 9,12,0, 13,11,0, 16,9,0, 16,13,0, 17,0,1, 17,8,1, 17,16,1,
    // Length and number of words of that length
    6, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,6,0, 0,10,0, 0,14,0, 0,18,0, 7,0,0, 7,1,0, 7,14,0, 8,13,1, 10,0,1, 10,8,0, 10,17,1, 10,21,0, 10,22,0, 12,0,1, 12,17,1, 14,4,1, 17,4,0, 17,8,0, 17,12,0, 17,16,0, 17,21,0, 17,22,0,
    // Length and number of words of that length
    5, 38,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,6,1, 0,7,0, 0,12,1, 0,15,0, 0,18,1, 1,0,1, 1,6,1, 1,12,1, 1,18,1, 2,0,1, 2,6,1, 2,12,1, 2,18,1, 5,16,0, 6,7,0, 6,15,0, 7,0,1, 11,0,1, 11,18,1, 12,7,0, 12,15,0, 13,6,0, 15,18,1, 18,7,0, 18,15,0, 20,0,1, 20,6,1, 20,12,1, 20,18,1, 21,0,1, 21,6,1, 21,12,1, 21,18,1, 22,0,1, 22,6,1, 22,12,1, 22,18,1,
    // Length and number of words of that length
    4, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,8,0, 0,12,0, 0,16,0, 0,21,0, 0,22,0, 3,0,1, 3,17,0, 4,4,1, 4,17,1, 5,21,0, 5,22,0, 6,2,1, 6,19,1, 7,19,1, 8,4,1, 9,4,1, 9,19,0, 10,3,0, 10,7,1, 10,12,1, 12,7,1, 12,12,1, 13,15,1, 14,0,0, 14,1,0, 14,15,1, 15,0,1, 16,0,1, 16,5,0, 16,17,1, 18,2,1, 18,15,1, 19,0,0, 19,1,0, 19,6,0, 19,10,0, 19,14,0, 19,19,0, 19,19,1,
    // Length and number of words of that length
    3, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 4,0,1, 4,5,0, 4,9,1, 4,13,1, 5,3,0, 5,8,0, 5,12,0, 6,7,1, 6,11,1, 6,15,1, 7,6,0, 7,6,1, 7,10,1, 7,14,1, 8,0,1, 8,5,0, 8,9,1, 8,17,0, 8,20,1, 9,0,1, 11,8,1, 11,12,1, 12,5,0, 12,17,0, 13,16,0, 13,20,1, 14,0,1, 14,11,1, 14,20,1, 15,6,1, 15,10,0, 15,10,1, 15,14,0, 15,14,1, 15,19,0, 16,5,1, 16,9,1, 16,13,1, 16,17,0, 18,7,1, 18,11,1, 18,20,1, 20,18,0,
    // End marker
    0
  };


  /*
   * Name: 23.02, 23 x 23
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ * * _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(* * _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ * *)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ * * _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ * * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   */
  const int g41[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    94,
    // Black field coordinates
    0,5, 0,10, 0,16, 0,22, 1,5, 1,10, 1,16, 2,5, 2,16, 3,3, 3,9, 3,14, 3,19, 4,3, 4,7, 4,8, 4,13, 4,18, 5,0, 5,1, 5,6, 5,12, 5,17, 6,5, 6,17, 6,21, 6,22, 7,4, 7,10, 7,11, 7,15, 7,16, 8,4, 8,9, 8,19, 9,8, 9,13, 9,14, 9,18, 10,0, 10,1, 10,2, 10,6, 10,7, 10,12, 10,17, 11,5, 11,17, 12,5, 12,10, 12,15, 12,16, 12,20, 12,21, 12,22, 13,4, 13,8, 13,9, 13,14, 14,3, 14,13, 14,18, 15,6, 15,7, 15,11, 15,12, 15,18, 16,0, 16,1, 16,5, 16,17, 17,5, 17,10, 17,16, 17,21, 17,22, 18,4, 18,9, 18,14, 18,15, 18,19, 19,3, 19,8, 19,13, 19,19, 20,6, 20,17, 21,6, 21,12, 21,17, 22,0, 22,6, 22,12, 22,17,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,20,0, 11,2,0,
    // Length and number of words of that length
    11, 3,
    // Coordinates where words start and direction (0 = horizontal)
    6,6,1, 11,6,1, 16,6,1,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,6,1, 13,20,0, 20,7,1,
    // Length and number of words of that length
    9, 4,
    // Coordinates where words start and direction (0 = horizontal)
    5,3,0, 8,10,1, 9,19,0, 14,4,1,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    9,0,1, 13,15,1,
    // Length and number of words of that length
    7, 7,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,11,0, 0,15,0, 8,11,0, 16,7,0, 16,11,0, 16,18,0,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,21,0, 1,17,1, 2,17,1, 7,17,1, 15,0,1, 17,1,0, 20,0,1, 21,0,1,
    // Length and number of words of that length
    5, 48,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,6,0, 0,11,1, 0,12,0, 0,17,0, 0,17,1, 1,0,1, 1,11,1, 1,22,0, 2,0,1, 2,10,0, 3,4,1, 4,14,0, 5,7,0, 5,7,1, 5,18,1, 6,0,1, 7,5,1, 7,21,0, 7,22,0, 10,18,1, 11,0,0, 11,0,1, 11,1,0, 11,18,1, 12,0,1, 13,15,0, 14,8,0, 15,13,1, 16,12,0, 16,18,1, 17,0,0, 17,0,1, 17,11,1, 18,5,0, 18,10,0, 18,16,0, 18,21,0, 18,22,0, 19,14,1, 20,18,1, 21,7,1, 21,18,1, 22,1,1, 22,7,1, 22,18,1,
    // Length and number of words of that length
    4, 72,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,1, 0,7,0, 0,8,0, 0,13,0, 0,18,0, 1,6,1, 3,10,1, 3,15,1, 3,16,0, 4,9,0, 4,9,1, 4,14,1, 4,19,0, 4,19,1, 5,2,1, 5,8,0, 5,13,0, 5,13,1, 5,18,0, 6,0,0, 6,1,0, 6,6,0, 6,12,0, 7,0,1, 7,5,0, 8,0,1, 8,5,1, 8,10,0, 8,15,0, 8,16,0, 9,4,0, 9,9,0, 9,9,1, 9,19,1, 10,8,1, 10,13,0, 10,13,1, 10,18,0, 11,6,0, 11,7,0, 11,12,0, 12,6,1, 12,11,1, 12,17,0, 13,0,1, 13,10,0, 13,10,1, 13,16,0, 13,21,0, 13,22,0, 14,4,0, 14,9,0, 14,14,0, 14,14,1, 14,19,1, 15,3,0, 15,13,0, 15,19,1, 16,6,0, 17,6,1, 17,17,1, 18,0,1, 18,5,1, 18,10,1, 19,4,0, 19,4,1, 19,9,0, 19,9,1, 19,14,0, 19,15,0, 21,13,1, 22,13,1,
    // Length and number of words of that length
    3, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,14,0, 0,19,0, 3,0,1, 3,5,0, 3,20,1, 4,0,1, 4,4,1, 6,18,1, 7,12,1, 7,17,0, 8,20,1, 9,15,1, 10,3,1, 10,8,0, 10,14,0, 12,17,1, 13,5,0, 13,5,1, 14,0,1, 15,8,1, 16,2,1, 17,17,0, 18,16,1, 18,20,1, 19,0,1, 19,20,1, 20,3,0, 20,8,0, 20,13,0, 20,19,0,
    // End marker
    0
  };


  /*
   * Name: 23.03, 23 x 23
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * * _ _ _ * * * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ _ _ _ _ * _ _ _ _ _ * * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * * _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ * * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * * _ _ _ _ _ * _ _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * * * _ _ _ * * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   */
  const int g42[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    89,
    // Black field coordinates
    0,5, 0,11, 0,16, 1,5, 1,11, 1,16, 2,5, 2,16, 3,4, 3,10, 3,15, 4,4, 4,8, 4,13, 4,14, 4,18, 4,19, 5,11, 5,17, 5,21, 5,22, 6,0, 6,1, 6,6, 6,7, 6,12, 6,17, 7,3, 7,9, 7,16, 8,4, 8,9, 9,4, 9,10, 9,14, 9,19, 10,4, 10,5, 10,10, 10,15, 10,20, 10,21, 10,22, 11,6, 11,11, 11,16, 12,0, 12,1, 12,2, 12,7, 12,12, 12,17, 12,18, 13,3, 13,8, 13,12, 13,18, 14,13, 14,18, 15,6, 15,13, 15,19, 16,5, 16,10, 16,15, 16,16, 16,21, 16,22, 17,0, 17,1, 17,5, 17,11, 18,3, 18,4, 18,8, 18,9, 18,14, 18,18, 19,7, 19,12, 19,18, 20,6, 20,17, 21,6, 21,11, 21,17, 22,6, 22,11, 22,17,
    // Length and number of words of that length
    13, 2,
    // Coordinates where words start and direction (0 = horizontal)
    8,10,1, 14,0,1,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 11,20,0,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,0,1, 17,12,1,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,20,0, 2,6,1, 13,2,0, 20,7,1,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,13,0, 9,9,0,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,8,0, 10,14,0,
    // Length and number of words of that length
    7, 10,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 3,5,0, 3,16,1, 5,18,0, 11,4,0, 13,17,0, 16,13,0, 16,19,0, 19,0,1,
    // Length and number of words of that length
    6, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,6,0, 0,7,0, 0,12,0, 0,17,1, 1,17,1, 2,17,1, 4,15,0, 7,10,1, 7,17,1, 11,0,1, 11,17,1, 13,7,0, 15,0,1, 15,7,1, 17,10,0, 17,15,0, 17,16,0, 17,21,0, 17,22,0, 20,0,1, 21,0,1, 22,0,1,
    // Length and number of words of that length
    5, 42,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,6,1, 0,17,0, 0,21,0, 0,22,0, 1,0,1, 1,6,1, 2,0,1, 3,5,1, 4,10,0, 5,12,1, 6,11,0, 6,18,1, 7,0,0, 7,1,0, 7,4,1, 7,7,0, 7,12,0, 7,17,0, 8,3,0, 9,5,1, 10,19,0, 11,5,0, 11,10,0, 11,15,0, 11,21,0, 11,22,0, 12,11,0, 13,13,1, 14,12,0, 15,14,1, 16,0,1, 17,6,1, 18,0,0, 18,1,0, 18,5,0, 19,13,1, 20,18,1, 21,12,1, 21,18,1, 22,12,1, 22,18,1,
    // Length and number of words of that length
    4, 58,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,12,1, 0,13,0, 0,14,0, 0,18,0, 0,19,0, 1,12,1, 3,0,1, 3,11,1, 3,16,0, 4,0,1, 4,9,1, 5,14,0, 5,19,0, 6,2,1, 6,8,1, 6,13,1, 6,21,0, 6,22,0, 7,6,0, 8,0,1, 8,5,1, 9,0,1, 9,15,1, 10,0,1, 10,6,1, 10,11,1, 10,16,1, 11,7,1, 11,12,1, 12,3,1, 12,8,1, 12,13,1, 12,16,0, 12,19,1, 13,0,0, 13,1,0, 13,4,1, 13,19,1, 14,3,0, 14,8,0, 14,14,1, 14,19,1, 16,6,0, 16,6,1, 16,11,1, 16,17,1, 18,10,1, 18,19,1, 19,3,0, 19,4,0, 19,8,0, 19,8,1, 19,9,0, 19,14,0, 19,19,1, 21,7,1, 22,7,1,
    // Length and number of words of that length
    3, 26,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,15,0, 2,11,0, 4,5,1, 4,15,1, 4,20,1, 5,4,0, 5,18,1, 7,0,1, 8,16,0, 9,11,1, 9,20,1, 12,6,0, 13,0,1, 13,9,1, 15,18,0, 15,20,1, 17,2,1, 18,0,1, 18,5,1, 18,11,0, 18,15,1, 20,7,0, 20,12,0, 20,18,0,
    // End marker
    0
  };


  /*
   * Name: 23.04, 23 x 23
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   */
  const int g43[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    80,
    // Black field coordinates
    0,5, 0,11, 0,17, 1,5, 1,11, 1,17, 2,5, 2,11, 2,17, 3,9, 3,13, 4,8, 4,14, 5,0, 5,1, 5,2, 5,7, 5,15, 5,20, 5,21, 5,22, 6,6, 6,10, 6,16, 7,5, 7,11, 7,17, 8,4, 8,12, 8,18, 9,3, 9,9, 9,13, 9,19, 10,8, 10,16, 11,0, 11,1, 11,2, 11,7, 11,15, 11,20, 11,21, 11,22, 12,6, 12,14, 13,3, 13,9, 13,13, 13,19, 14,4, 14,10, 14,18, 15,5, 15,11, 15,17, 16,6, 16,12, 16,16, 17,0, 17,1, 17,2, 17,7, 17,15, 17,20, 17,21, 17,22, 18,8, 18,14, 19,9, 19,13, 20,5, 20,11, 20,17, 21,5, 21,11, 21,17, 22,5, 22,11, 22,17,
    // Length and number of words of that length
    9, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,19,0, 3,0,1, 3,14,1, 14,3,0, 14,19,0, 19,0,1, 19,14,1,
    // Length and number of words of that length
    8, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,12,0, 0,18,0, 4,0,1, 4,15,1, 10,0,1, 12,15,1, 15,4,0, 15,10,0, 15,18,0, 18,0,1, 18,15,1,
    // Length and number of words of that length
    7, 14,
    // Coordinates where words start and direction (0 = horizontal)
    5,8,1, 5,14,0, 7,10,0, 8,5,0, 8,5,1, 8,11,0, 8,17,0, 9,12,0, 10,9,1, 11,8,0, 11,8,1, 12,7,1, 14,11,1, 17,8,1,
    // Length and number of words of that length
    6, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,10,0, 0,16,0, 6,0,1, 6,17,1, 10,17,1, 12,0,1, 16,0,1, 16,17,1, 17,6,0, 17,12,0, 17,16,0,
    // Length and number of words of that length
    5, 84,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,6,1, 0,7,0, 0,12,1, 0,15,0, 0,18,1, 0,20,0, 0,21,0, 0,22,0, 1,0,1, 1,6,1, 1,12,1, 1,18,1, 2,0,1, 2,6,1, 2,12,1, 2,18,1, 4,9,0, 4,9,1, 4,13,0, 5,8,0, 6,0,0, 6,1,0, 6,2,0, 6,7,0, 6,11,1, 6,15,0, 6,20,0, 6,21,0, 6,22,0, 7,0,1, 7,6,0, 7,6,1, 7,12,1, 7,18,1, 8,13,1, 9,4,0, 9,4,1, 9,14,1, 9,18,0, 11,16,0, 12,0,0, 12,1,0, 12,2,0, 12,7,0, 12,15,0, 12,20,0, 12,21,0, 12,22,0, 13,4,1, 13,14,0, 13,14,1, 14,5,1, 14,9,0, 14,13,0, 15,0,1, 15,6,1, 15,12,1, 15,18,1, 16,7,1, 18,0,0, 18,1,0, 18,2,0, 18,7,0, 18,9,1, 18,15,0, 18,20,0, 18,21,0, 18,22,0, 20,0,1, 20,6,1, 20,12,1, 20,18,1, 21,0,1, 21,6,1, 21,12,1, 21,18,1, 22,0,1, 22,6,1, 22,12,1, 22,18,1,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 3,5,0, 3,11,0, 3,17,0, 5,3,1, 5,16,1, 8,0,1, 8,19,1, 11,3,1, 11,16,1, 14,0,1, 14,19,1, 16,5,0, 16,11,0, 16,17,0, 17,3,1, 17,16,1, 19,8,0, 19,14,0,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 0,13,0, 3,10,1, 6,7,1, 7,16,0, 9,0,1, 9,10,1, 9,20,1, 10,3,0, 10,9,0, 10,13,0, 10,19,0, 13,0,1, 13,6,0, 13,10,1, 13,20,1, 16,13,1, 19,10,1, 20,9,0, 20,13,0,
    // End marker
    0
  };


  /*
   * Name: 23.05, 23 x 23
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   */
  const int g44[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    84,
    // Black field coordinates
    0,5, 0,11, 0,17, 1,5, 1,11, 1,17, 2,5, 2,11, 2,17, 3,3, 3,8, 3,14, 3,19, 4,7, 4,15, 5,0, 5,1, 5,6, 5,12, 5,16, 5,20, 5,21, 5,22, 6,5, 6,11, 6,17, 7,4, 7,10, 7,18, 8,3, 8,9, 8,14, 8,19, 9,8, 9,13, 10,7, 10,12, 10,17, 11,0, 11,1, 11,2, 11,6, 11,16, 11,20, 11,21, 11,22, 12,5, 12,10, 12,15, 13,9, 13,14, 14,3, 14,8, 14,13, 14,19, 15,4, 15,12, 15,18, 16,5, 16,11, 16,17, 17,0, 17,1, 17,2, 17,6, 17,10, 17,16, 17,21, 17,22, 18,7, 18,15, 19,3, 19,8, 19,14, 19,19, 20,5, 20,11, 20,17, 21,5, 21,11, 21,17, 22,5, 22,11, 22,17,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 12,20,0,
    // Length and number of words of that length
    9, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,13,0, 7,11,0, 9,14,1, 11,7,1, 13,0,1, 14,9,0,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 9,0,1, 13,15,1, 15,13,0,
    // Length and number of words of that length
    7, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,18,0, 4,0,1, 4,8,1, 4,16,1, 5,15,0, 7,11,1, 8,4,0, 8,18,0, 10,0,1, 11,7,0, 12,16,1, 15,5,1, 16,4,0, 16,12,0, 16,18,0, 18,0,1, 18,8,1, 18,16,1,
    // Length and number of words of that length
    5, 80,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,6,0, 0,6,1, 0,12,0, 0,12,1, 0,16,0, 0,18,1, 0,20,0, 0,21,0, 0,22,0, 1,0,1, 1,6,1, 1,12,1, 1,18,1, 2,0,1, 2,6,1, 2,12,1, 2,18,1, 3,9,1, 4,8,0, 5,7,0, 5,7,1, 6,0,0, 6,0,1, 6,1,0, 6,6,0, 6,6,1, 6,12,1, 6,16,0, 6,18,1, 6,20,0, 6,21,0, 6,22,0, 7,5,0, 7,5,1, 8,4,1, 9,3,0, 9,19,0, 10,18,1, 11,17,0, 12,0,0, 12,0,1, 12,1,0, 12,2,0, 12,6,0, 12,16,0, 12,21,0, 12,22,0, 13,15,0, 14,14,0, 14,14,1, 15,13,1, 16,0,1, 16,6,1, 16,12,1, 16,18,1, 17,11,1, 18,0,0, 18,1,0, 18,2,0, 18,6,0, 18,10,0, 18,16,0, 18,21,0, 18,22,0, 19,9,1, 20,0,1, 20,6,1, 20,12,1, 20,18,1, 21,0,1, 21,6,1, 21,12,1, 21,18,1, 22,0,1, 22,6,1, 22,12,1, 22,18,1,
    // Length and number of words of that length
    4, 38,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,15,0, 3,4,1, 3,15,1, 4,3,0, 4,14,0, 4,19,0, 5,2,1, 6,12,0, 7,0,1, 7,19,1, 8,10,0, 8,10,1, 8,15,1, 9,9,0, 9,9,1, 9,14,0, 10,8,0, 10,8,1, 10,13,0, 10,13,1, 11,12,0, 12,6,1, 12,11,1, 13,10,0, 13,10,1, 14,4,1, 14,9,1, 15,0,1, 15,3,0, 15,8,0, 15,19,0, 15,19,1, 17,17,1, 19,4,1, 19,7,0, 19,15,0, 19,15,1,
    // Length and number of words of that length
    3, 30,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,8,0, 0,14,0, 0,19,0, 3,0,1, 3,5,0, 3,11,0, 3,17,0, 3,20,1, 5,13,1, 5,17,1, 7,17,0, 8,0,1, 8,20,1, 11,3,1, 11,17,1, 13,5,0, 14,0,1, 14,20,1, 17,3,1, 17,5,0, 17,7,1, 17,11,0, 17,17,0, 19,0,1, 19,20,1, 20,3,0, 20,8,0, 20,14,0, 20,19,0,
    // End marker
    0
  };


  /*
   * Name: 23.06, 23 x 23
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * * * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   */
  const int g45[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    69,
    // Black field coordinates
    0,7, 0,15, 1,7, 1,15, 2,7, 2,15, 3,3, 3,12, 3,19, 4,4, 4,11, 4,18, 5,5, 5,10, 5,17, 6,8, 6,14, 7,0, 7,1, 7,2, 7,7, 7,15, 7,20, 7,21, 7,22, 8,6, 8,16, 9,9, 9,13, 10,3, 10,11, 10,17, 11,4, 11,10, 11,11, 11,12, 11,18, 12,5, 12,11, 12,19, 13,9, 13,13, 14,6, 14,16, 15,0, 15,1, 15,2, 15,7, 15,15, 15,20, 15,21, 15,22, 16,8, 16,14, 17,5, 17,12, 17,17, 18,4, 18,11, 18,18, 19,3, 19,10, 19,19, 20,7, 20,15, 21,7, 21,15, 22,7, 22,15,
    // Length and number of words of that length
    9, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 0,13,0, 7,8,0, 7,14,0, 8,7,1, 9,0,1, 9,14,1, 13,0,1, 13,14,1, 14,7,1, 14,9,0, 14,13,0,
    // Length and number of words of that length
    8, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,16,0, 3,4,1, 4,19,0, 6,0,1, 6,15,1, 11,3,0, 15,6,0, 15,16,0, 16,0,1, 16,15,1, 19,11,1,
    // Length and number of words of that length
    7, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,8,1, 0,16,1, 0,20,0, 0,21,0, 0,22,0, 1,0,1, 1,8,1, 1,16,1, 2,0,1, 2,8,1, 2,16,1, 4,12,0, 7,8,1, 8,0,0, 8,1,0, 8,2,0, 8,7,0, 8,15,0, 8,20,0, 8,21,0, 8,22,0, 10,4,1, 12,10,0, 12,12,1, 15,8,1, 16,0,0, 16,1,0, 16,2,0, 16,20,0, 16,21,0, 16,22,0, 20,0,1, 20,8,1, 20,16,1, 21,0,1, 21,8,1, 21,16,1, 22,0,1, 22,8,1, 22,16,1,
    // Length and number of words of that length
    6, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 3,13,1, 4,3,0, 4,5,1, 4,12,1, 5,4,0, 5,11,1, 5,18,0, 6,5,0, 8,0,1, 8,17,1, 11,17,0, 12,4,0, 12,18,0, 13,19,0, 14,0,1, 14,17,1, 17,6,1, 17,8,0, 17,14,0, 18,5,1, 18,12,1, 19,4,1,
    // Length and number of words of that length
    5, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,0, 0,17,0, 5,0,1, 5,11,0, 5,18,1, 6,9,1, 6,10,0, 9,6,0, 9,16,0, 10,12,1, 10,18,1, 11,5,1, 11,13,1, 12,0,1, 12,6,1, 12,12,0, 13,11,0, 16,9,1, 17,0,1, 17,18,1, 18,5,0, 18,12,0, 18,17,0,
    // Length and number of words of that length
    4, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,11,0, 0,18,0, 3,7,0, 3,15,0, 4,0,1, 4,19,1, 5,6,1, 6,17,0, 7,3,1, 7,16,1, 11,0,1, 11,19,1, 13,5,0, 15,3,1, 15,16,1, 16,7,0, 16,15,0, 17,13,1, 18,0,1, 18,19,1, 19,4,0, 19,11,0, 19,18,0,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,12,0, 0,19,0, 3,0,1, 3,20,1, 9,10,1, 10,0,1, 10,9,0, 10,13,0, 12,20,1, 13,10,1, 19,0,1, 19,20,1, 20,3,0, 20,10,0, 20,19,0,
    // End marker
    0
  };


  /*
   * Name: 23.07, 23 x 23
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ *)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * * _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ * * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(* _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g46[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    83,
    // Black field coordinates
    0,4, 0,10, 0,16, 0,22, 1,4, 1,10, 1,16, 2,4, 2,16, 3,8, 3,14, 3,19, 4,0, 4,1, 4,7, 4,13, 4,18, 5,6, 5,12, 5,17, 6,5, 6,10, 6,11, 6,16, 6,21, 6,22, 7,4, 7,15, 8,3, 8,9, 8,14, 8,19, 9,8, 9,18, 10,0, 10,1, 10,2, 10,6, 10,12, 10,17, 11,6, 11,11, 11,16, 12,5, 12,10, 12,16, 12,20, 12,21, 12,22, 13,4, 13,14, 14,3, 14,8, 14,13, 14,19, 15,7, 15,18, 16,0, 16,1, 16,6, 16,11, 16,12, 16,17, 17,5, 17,10, 17,16, 18,4, 18,9, 18,15, 18,21, 18,22, 19,3, 19,8, 19,14, 20,6, 20,18, 21,6, 21,12, 21,18, 22,0, 22,6, 22,12, 22,18,
    // Length and number of words of that length
    12, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,20,0, 11,2,0,
    // Length and number of words of that length
    11, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,5,1, 20,7,1,
    // Length and number of words of that length
    10, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 5,7,0, 7,5,1, 8,15,0, 13,20,0, 15,8,1,
    // Length and number of words of that length
    9, 4,
    // Coordinates where words start and direction (0 = horizontal)
    5,13,0, 9,9,0, 9,9,1, 13,5,1,
    // Length and number of words of that length
    8, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 3,0,1, 9,0,1, 13,15,1, 15,13,0, 15,19,0, 19,15,1,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,15,0, 7,16,1, 15,0,1, 16,7,0,
    // Length and number of words of that length
    6, 14,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 0,21,0, 1,17,1, 2,17,1, 5,0,1, 11,0,1, 11,17,1, 17,1,0, 17,11,0, 17,17,0, 17,17,1, 20,0,1, 21,0,1,
    // Length and number of words of that length
    5, 54,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 0,11,1, 0,12,0, 0,17,0, 0,17,1, 1,5,1, 1,11,1, 1,22,0, 3,9,1, 4,2,1, 4,8,0, 4,8,1, 5,0,0, 5,1,0, 5,7,1, 5,18,1, 6,0,1, 7,5,0, 7,10,0, 7,21,0, 7,22,0, 8,4,0, 8,4,1, 9,3,0, 9,19,0, 10,7,1, 10,18,0, 10,18,1, 11,0,0, 11,1,0, 11,12,0, 11,17,0, 12,0,1, 12,11,1, 13,21,0, 13,22,0, 14,14,0, 14,14,1, 16,18,1, 17,0,0, 17,0,1, 17,11,1, 18,5,0, 18,10,0, 18,10,1, 18,16,0, 18,16,1, 19,9,1, 21,7,1, 21,13,1, 22,1,1, 22,7,1, 22,13,1,
    // Length and number of words of that length
    4, 64,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,7,0, 0,13,0, 0,18,0, 1,0,1, 2,0,1, 2,10,0, 3,4,0, 3,15,1, 4,14,0, 4,14,1, 4,19,0, 4,19,1, 5,13,1, 5,18,0, 6,6,0, 6,6,1, 6,12,0, 6,12,1, 6,17,0, 6,17,1, 7,0,1, 7,11,0, 7,16,0, 8,10,1, 8,15,1, 9,14,0, 9,19,1, 10,8,0, 10,13,1, 11,7,1, 11,12,1, 12,6,0, 12,6,1, 12,11,0, 13,0,1, 13,5,0, 13,10,0, 13,16,0, 14,4,0, 14,4,1, 14,9,1, 15,3,0, 15,8,0, 15,19,1, 16,2,1, 16,7,1, 16,13,1, 16,18,0, 17,6,1, 17,12,0, 18,0,1, 18,5,1, 19,4,0, 19,4,1, 19,9,0, 19,15,0, 19,21,0, 19,22,0, 20,19,1, 21,19,1, 22,19,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 0,19,0, 3,16,0, 3,20,1, 8,0,1, 8,20,1, 10,3,1, 12,17,1, 14,0,1, 14,20,1, 17,6,0, 19,0,1, 20,3,0, 20,8,0, 20,14,0,
    // End marker
    0
  };


  /*
   * Name: 23.08, 23 x 23
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   */
  const int g47[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    75,
    // Black field coordinates
    0,7, 0,15, 1,7, 1,15, 2,7, 2,15, 3,3, 3,8, 3,13, 3,19, 4,4, 4,12, 4,18, 5,5, 5,10, 5,17, 6,6, 6,11, 6,16, 7,0, 7,1, 7,2, 7,9, 7,15, 7,20, 7,21, 7,22, 8,3, 8,8, 8,14, 9,7, 9,13, 9,19, 10,5, 10,12, 10,18, 11,6, 11,11, 11,16, 12,4, 12,10, 12,17, 13,3, 13,9, 13,15, 14,8, 14,14, 14,19, 15,0, 15,1, 15,2, 15,7, 15,13, 15,20, 15,21, 15,22, 16,6, 16,11, 16,16, 17,5, 17,12, 17,17, 18,4, 18,10, 18,18, 19,3, 19,9, 19,14, 19,19, 20,7, 20,15, 21,7, 21,15, 22,7, 22,15,
    // Length and number of words of that length
    8, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,14,0, 8,15,1, 14,0,1, 15,8,0,
    // Length and number of words of that length
    7, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,8,1, 0,9,0, 0,16,1, 0,20,0, 0,21,0, 0,22,0, 1,0,1, 1,8,1, 1,16,1, 2,0,1, 2,8,1, 2,16,1, 4,5,1, 5,4,0, 8,0,0, 8,1,0, 8,2,0, 8,20,0, 8,21,0, 8,22,0, 9,0,1, 11,18,0, 13,16,1, 16,0,0, 16,1,0, 16,2,0, 16,13,0, 16,20,0, 16,21,0, 16,22,0, 18,11,1, 20,0,1, 20,8,1, 20,16,1, 21,0,1, 21,8,1, 21,16,1, 22,0,1, 22,8,1, 22,16,1,
    // Length and number of words of that length
    6, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,11,0, 0,16,0, 3,7,0, 5,11,1, 6,0,1, 6,10,0, 6,17,0, 6,17,1, 7,3,1, 10,6,1, 11,0,1, 11,5,0, 11,12,0, 11,17,1, 12,11,1, 14,15,0, 15,14,1, 16,0,1, 16,17,1, 17,6,0, 17,6,1, 17,11,0, 17,16,0,
    // Length and number of words of that length
    5, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,10,0, 0,17,0, 3,14,1, 4,13,0, 4,13,1, 4,19,0, 5,0,1, 5,12,0, 5,18,0, 5,18,1, 7,10,1, 8,9,0, 8,9,1, 8,15,0, 9,8,0, 9,8,1, 9,14,0, 9,14,1, 10,0,1, 10,7,0, 10,13,0, 10,13,1, 12,5,1, 12,18,1, 13,4,0, 13,4,1, 13,10,0, 13,10,1, 14,3,0, 14,9,0, 14,9,1, 15,8,1, 17,0,1, 17,18,1, 18,5,0, 18,5,1, 18,12,0, 18,17,0, 19,4,1,
    // Length and number of words of that length
    4, 44,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,12,0, 0,18,0, 3,4,1, 3,9,1, 3,15,0, 4,0,1, 4,3,0, 4,8,0, 4,19,1, 5,6,1, 6,5,0, 6,7,1, 6,12,1, 7,6,0, 7,11,0, 7,16,0, 7,16,1, 8,4,1, 9,3,0, 10,19,0, 10,19,1, 11,7,1, 11,12,1, 12,0,1, 12,6,0, 12,11,0, 12,16,0, 13,17,0, 14,15,1, 15,3,1, 15,14,0, 15,19,0, 16,7,0, 16,7,1, 16,12,1, 17,13,1, 18,0,1, 18,19,1, 19,4,0, 19,10,0, 19,10,1, 19,15,1, 19,18,0,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,8,0, 0,13,0, 0,19,0, 3,0,1, 3,20,1, 8,0,1, 9,20,1, 13,0,1, 14,20,1, 19,0,1, 19,20,1, 20,3,0, 20,9,0, 20,14,0, 20,19,0,
    // End marker
    0
  };


  /*
   * Name: 23.09, 23 x 23
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ *)
   * 	(_ _ _ * _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _)
   * 	(* * _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ * *)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ * _ _ _)
   * 	(* _ _ _ * _ _ _ * _ _ _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _ _ * _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   */
  const int g48[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    76,
    // Black field coordinates
    0,5, 0,11, 0,17, 1,5, 1,11, 2,5, 3,6, 3,12, 3,18, 4,3, 4,9, 4,13, 4,17, 5,0, 5,4, 5,8, 5,14, 5,20, 5,21, 5,22, 6,7, 6,15, 6,19, 7,6, 7,10, 7,16, 8,5, 8,11, 8,17, 9,4, 9,12, 9,18, 10,3, 10,9, 10,15, 11,0, 11,1, 11,8, 11,14, 11,21, 11,22, 12,7, 12,13, 12,19, 13,4, 13,10, 13,18, 14,5, 14,11, 14,17, 15,6, 15,12, 15,16, 16,3, 16,7, 16,15, 17,0, 17,1, 17,2, 17,8, 17,14, 17,18, 17,22, 18,5, 18,9, 18,13, 18,19, 19,4, 19,10, 19,16, 20,17, 21,11, 21,17, 22,5, 22,11, 22,17,
    // Length and number of words of that length
    17, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,6,1, 6,20,0, 20,0,1,
    // Length and number of words of that length
    11, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,12,1, 12,21,0, 21,0,1,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,10,0, 0,16,0, 5,13,0, 6,0,1, 6,8,1, 8,6,0, 8,16,0, 9,5,1, 10,16,1, 11,9,0, 12,0,1, 13,11,1, 16,6,0, 16,8,1, 16,12,0, 16,16,1,
    // Length and number of words of that length
    6, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,15,0, 0,19,0, 2,11,0, 3,0,1, 7,0,1, 7,17,1, 11,2,1, 11,15,1, 15,0,1, 15,11,0, 15,17,1, 17,3,0, 17,7,0, 17,15,0, 19,17,1,
    // Length and number of words of that length
    5, 86,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,4,0, 0,6,1, 0,8,0, 0,12,1, 0,14,0, 0,18,1, 0,20,0, 0,21,0, 0,22,0, 1,0,1, 1,6,1, 2,0,1, 3,5,0, 3,7,1, 3,13,1, 4,4,1, 4,12,0, 4,18,0, 4,18,1, 5,3,0, 5,9,0, 5,9,1, 5,15,1, 6,0,0, 6,8,0, 6,14,0, 6,21,0, 6,22,0, 7,7,0, 7,11,1, 7,19,0, 8,0,1, 8,6,1, 8,10,0, 8,12,1, 8,18,1, 9,5,0, 9,11,0, 9,13,1, 9,17,0, 10,4,1, 10,10,1, 10,12,0, 11,3,0, 11,9,1, 11,15,0, 12,0,0, 12,1,0, 12,8,0, 12,8,1, 12,14,0, 12,14,1, 12,22,0, 13,5,1, 13,13,0, 13,19,0, 14,0,1, 14,4,0, 14,6,1, 14,10,0, 14,12,1, 14,18,1, 15,7,1, 15,17,0, 17,3,1, 17,9,1, 18,0,0, 18,0,1, 18,1,0, 18,2,0, 18,8,0, 18,14,0, 18,14,1, 18,18,0, 18,22,0, 19,5,1, 19,11,1, 20,18,1, 21,12,1, 21,18,1, 22,0,1, 22,6,1, 22,12,1, 22,18,1,
    // Length and number of words of that length
    4, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 0,13,0, 3,19,1, 9,0,1, 9,19,1, 13,0,1, 13,19,1, 19,0,1, 19,9,0, 19,13,0, 19,19,0,
    // Length and number of words of that length
    3, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,6,0, 0,12,0, 0,18,0, 1,17,0, 4,0,1, 4,6,0, 4,10,1, 4,14,1, 5,1,1, 5,5,1, 5,17,0, 6,4,0, 6,16,1, 6,20,1, 7,7,1, 7,15,0, 10,0,1, 10,4,0, 10,18,0, 12,20,1, 13,7,0, 14,18,0, 15,5,0, 15,13,1, 16,0,1, 16,4,1, 16,16,0, 17,15,1, 17,19,1, 18,6,1, 18,10,1, 18,20,1, 19,5,0, 20,4,0, 20,10,0, 20,16,0,
    // End marker
    0
  };


  /*
   * Name: 23.10, 23 x 23
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ *)
   * 	(* * _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ _ * _ _ _ * _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ * _ _ _ _ * *)
   * 	(* _ _ _ * _ _ _ _ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _ _ * _ _ _ _ _ _)
   */
  const int g49[] = {
    // Width and height of crossword grid
    23, 23,
    // Number of black fields
    67,
    // Black field coordinates
    0,6, 0,13, 0,17, 1,6, 1,13, 2,13, 3,3, 3,12, 3,19, 4,5, 4,11, 4,17, 5,4, 5,10, 5,18, 5,22, 6,0, 6,1, 6,6, 6,16, 7,7, 7,15, 8,8, 8,14, 9,9, 9,13, 9,20, 9,21, 9,22, 10,5, 10,12, 10,19, 11,4, 11,11, 11,18, 12,3, 12,10, 12,17, 13,0, 13,1, 13,2, 13,9, 13,13, 14,8, 14,14, 15,7, 15,15, 16,6, 16,16, 16,21, 16,22, 17,0, 17,4, 17,12, 17,18, 18,5, 18,11, 18,17, 19,3, 19,10, 19,19, 20,9, 21,9, 21,16, 22,5, 22,9, 22,16,
    // Length and number of words of that length
    13, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1, 10,20,0, 20,10,1,
    // Length and number of words of that length
    9, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,9,0, 0,20,0, 0,21,0, 1,14,1, 2,14,1, 6,7,1, 7,6,0, 7,16,0, 9,0,1, 13,14,1, 14,1,0, 14,2,0, 14,13,0, 16,7,1, 20,0,1, 21,0,1,
    // Length and number of words of that length
    8, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 0,14,0, 3,4,1, 4,3,0, 8,0,1, 8,15,1, 11,19,0, 14,0,1, 14,15,1, 15,8,0, 15,14,0, 19,11,1,
    // Length and number of words of that length
    7, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,7,0, 0,15,0, 5,11,1, 5,17,0, 7,0,1, 7,8,1, 7,16,1, 8,7,0, 8,15,0, 11,5,0, 15,0,1, 15,8,1, 15,16,1, 16,7,0, 16,15,0, 17,5,1,
    // Length and number of words of that length
    6, 40,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,7,1, 0,16,0, 1,0,1, 1,7,1, 3,13,0, 3,13,1, 4,12,0, 4,19,0, 5,11,0, 6,10,0, 6,17,1, 7,0,0, 7,1,0, 9,14,1, 10,6,1, 10,13,1, 10,21,0, 10,22,0, 11,5,1, 11,12,0, 11,12,1, 12,4,1, 12,11,0, 12,11,1, 13,3,0, 13,3,1, 13,10,0, 14,9,0, 16,0,1, 17,6,0, 17,21,0, 17,22,0, 19,4,1, 21,10,1, 21,17,1, 22,10,1, 22,17,1,
    // Length and number of words of that length
    5, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 0,18,0, 0,18,1, 0,22,0, 4,0,1, 4,6,1, 4,12,1, 4,18,1, 5,5,0, 5,5,1, 6,4,0, 6,18,0, 8,9,1, 9,8,0, 9,14,0, 10,0,1, 12,4,0, 12,18,0, 12,18,1, 13,17,0, 14,9,1, 17,13,1, 18,0,0, 18,0,1, 18,4,0, 18,6,1, 18,12,0, 18,12,1, 18,18,0, 18,18,1, 22,0,1,
    // Length and number of words of that length
    4, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,11,0, 2,6,0, 5,0,1, 6,2,1, 11,0,1, 11,19,1, 16,17,1, 17,16,0, 17,19,1, 19,11,0, 19,17,0,
    // Length and number of words of that length
    3, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,12,0, 0,14,1, 0,19,0, 1,17,0, 3,0,1, 3,20,1, 5,19,1, 6,22,0, 9,10,1, 10,9,0, 10,13,0, 10,20,1, 12,0,1, 13,10,1, 14,0,0, 17,1,1, 19,0,1, 19,5,0, 19,20,1, 20,3,0, 20,10,0, 20,19,0, 22,6,1,
    // End marker
    0
  };


  /*
   * Name: puzzle01, 2 x 2
   * 	(_ *)
   * 	(_ _)
   */
  const int g50[] = {
    // Width and height of crossword grid
    2, 2,
    // Number of black fields
    1,
    // Black field coordinates
    1,0,
    // Length and number of words of that length
    2, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,1,0,
    // Length and number of words of that length
    1, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 1,1,1,
    // End marker
    0
  };


  /*
   * Name: puzzle02, 3 x 3
   * 	(* _ _)
   * 	(_ _ _)
   * 	(_ _ _)
   */
  const int g51[] = {
    // Width and height of crossword grid
    3, 3,
    // Number of black fields
    1,
    // Black field coordinates
    0,0,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 1,0,1, 2,0,1,
    // Length and number of words of that length
    2, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,1, 1,0,0,
    // End marker
    0
  };


  /*
   * Name: puzzle03, 4 x 4
   * 	(_ _ _ *)
   * 	(_ _ _ _)
   * 	(_ _ _ _)
   * 	(* _ _ _)
   */
  const int g52[] = {
    // Width and height of crossword grid
    4, 4,
    // Number of black fields
    2,
    // Black field coordinates
    0,3, 3,0,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 1,0,1, 2,0,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 1,3,0, 3,1,1,
    // End marker
    0
  };


  /*
   * Name: puzzle04, 5 x 5
   * 	(_ _ _ * *)
   * 	(_ _ _ _ *)
   * 	(_ _ _ _ _)
   * 	(* _ _ _ _)
   * 	(* * _ _ _)
   */
  const int g53[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    6,
    // Black field coordinates
    0,3, 0,4, 1,4, 3,0, 4,0, 4,1,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,0,1, 1,3,0, 3,1,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 2,4,0, 4,2,1,
    // End marker
    0
  };


  /*
   * Name: puzzle05, 5 x 5
   * 	(_ _ _ _ *)
   * 	(_ _ _ * _)
   * 	(_ _ _ _ _)
   * 	(_ * _ _ _)
   * 	(* _ _ _ _)
   */
  const int g54[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    4,
    // Black field coordinates
    0,4, 1,3, 3,1, 4,0,
    // Length and number of words of that length
    5, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 1,4,0, 4,1,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,0,1, 2,3,0, 3,2,1,
    // Length and number of words of that length
    1, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 1,4,1, 3,0,1, 4,1,0,
    // End marker
    0
  };


  /*
   * Name: puzzle06, 5 x 5
   * 	(_ _ _ _ _)
   * 	(_ _ _ * _)
   * 	(_ _ _ _ _)
   * 	(_ * _ _ _)
   * 	(_ _ _ _ _)
   */
  const int g55[] = {
    // Width and height of crossword grid
    5, 5,
    // Number of black fields
    2,
    // Black field coordinates
    1,3, 3,1,
    // Length and number of words of that length
    5, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,2,0, 0,4,0, 2,0,1, 4,0,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,0,1, 2,3,0, 3,2,1,
    // Length and number of words of that length
    1, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 1,4,1, 3,0,1, 4,1,0,
    // End marker
    0
  };


  /*
   * Name: puzzle07, 6 x 6
   * 	(_ _ _ _ _ *)
   * 	(_ * _ _ _ _)
   * 	(_ _ _ * _ _)
   * 	(_ _ * _ _ _)
   * 	(_ _ _ _ * _)
   * 	(* _ _ _ _ _)
   */
  const int g56[] = {
    // Width and height of crossword grid
    6, 6,
    // Number of black fields
    6,
    // Black field coordinates
    0,5, 1,1, 2,3, 3,2, 4,4, 5,0,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 1,5,0, 5,1,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 1,2,1, 2,1,0, 4,0,1,
    // Length and number of words of that length
    3, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 2,0,1, 3,3,0, 3,3,1,
    // Length and number of words of that length
    2, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 2,4,1, 3,0,1, 4,2,0,
    // Length and number of words of that length
    1, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 1,0,1, 4,5,1, 5,4,0,
    // End marker
    0
  };


  /*
   * Name: puzzle08, 7 x 7
   * 	(_ _ _ _ * _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ * _ _ _ *)
   * 	(_ _ _ _ _ _ _)
   * 	(* _ _ _ * _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ * _ _ _ _)
   */
  const int g57[] = {
    // Width and height of crossword grid
    7, 7,
    // Number of black fields
    8,
    // Black field coordinates
    0,4, 2,2, 2,6, 3,1, 3,5, 4,0, 4,4, 6,2,
    // Length and number of words of that length
    7, 3,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 1,0,1, 5,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 3,6,0, 6,3,1,
    // Length and number of words of that length
    3, 9,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,5,0, 1,4,0, 2,3,1, 3,2,0, 3,2,1, 4,1,0, 4,1,1, 4,5,0,
    // Length and number of words of that length
    2, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,5,1, 0,6,0, 2,0,1, 4,5,1, 5,0,0, 5,4,0, 6,0,1,
    // Length and number of words of that length
    1, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 3,6,1,
    // End marker
    0
  };


  /*
   * Name: puzzle09, 7 x 7
   * 	(* * _ _ _ * *)
   * 	(* _ _ _ _ _ *)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(* _ _ _ _ _ *)
   * 	(* * _ _ _ * *)
   */
  const int g58[] = {
    // Width and height of crossword grid
    7, 7,
    // Number of black fields
    14,
    // Black field coordinates
    0,0, 0,1, 0,5, 0,6, 1,0, 1,6, 3,2, 3,4, 5,0, 5,6, 6,0, 6,1, 6,5, 6,6,
    // Length and number of words of that length
    7, 3,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 2,0,1, 4,0,1,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    1,1,0, 1,1,1, 1,5,0, 5,1,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,2,1, 0,4,0, 2,0,0, 2,6,0, 4,2,0, 4,4,0, 6,2,1,
    // Length and number of words of that length
    2, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 3,5,1,
    // Length and number of words of that length
    1, 1,
    // Coordinates where words start and direction (0 = horizontal)
    3,3,1,
    // End marker
    0
  };


  /*
   * Name: puzzle10, 7 x 7
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _)
   * 	(* * _ * _ * *)
   * 	(_ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _)
   */
  const int g59[] = {
    // Width and height of crossword grid
    7, 7,
    // Number of black fields
    9,
    // Black field coordinates
    0,3, 1,3, 3,0, 3,1, 3,3, 3,5, 3,6, 5,3, 6,3,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,4,0, 2,0,1, 4,0,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,4,1, 0,5,0, 0,6,0, 1,0,1, 1,4,1, 4,0,0, 4,1,0, 4,5,0, 4,6,0, 5,0,1, 5,4,1, 6,0,1, 6,4,1,
    // Length and number of words of that length
    1, 4,
    // Coordinates where words start and direction (0 = horizontal)
    2,3,0, 3,2,1, 3,4,1, 4,3,0,
    // End marker
    0
  };


  /*
   * Name: puzzle11, 7 x 7
   * 	(* * _ _ _ _ *)
   * 	(* _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ *)
   * 	(* _ _ _ _ * *)
   */
  const int g60[] = {
    // Width and height of crossword grid
    7, 7,
    // Number of black fields
    11,
    // Black field coordinates
    0,0, 0,1, 0,6, 1,0, 3,2, 3,3, 3,4, 5,6, 6,0, 6,5, 6,6,
    // Length and number of words of that length
    7, 2,
    // Coordinates where words start and direction (0 = horizontal)
    2,0,1, 4,0,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 1,1,0, 1,1,1, 5,0,1,
    // Length and number of words of that length
    4, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,1, 1,6,0, 2,0,0, 6,1,1,
    // Length and number of words of that length
    3, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,3,0, 0,4,0, 4,2,0, 4,3,0, 4,4,0,
    // Length and number of words of that length
    2, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 3,5,1,
    // End marker
    0
  };


  /*
   * Name: puzzle12, 8 x 8
   * 	(_ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _)
   * 	(* * * _ _ _ _ _)
   * 	(_ _ _ _ _ * * *)
   * 	(_ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _)
   */
  const int g61[] = {
    // Width and height of crossword grid
    8, 8,
    // Number of black fields
    12,
    // Black field coordinates
    0,3, 1,3, 2,3, 3,5, 3,6, 3,7, 4,0, 4,1, 4,2, 5,4, 6,4, 7,4,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 3,0,1, 3,3,0, 4,3,1,
    // Length and number of words of that length
    4, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,2,0, 0,4,1, 1,4,1, 2,4,1, 4,5,0, 4,6,0, 4,7,0, 5,0,1, 6,0,1, 7,0,1,
    // Length and number of words of that length
    3, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,5,0, 0,6,0, 0,7,0, 1,0,1, 2,0,1, 5,0,0, 5,1,0, 5,2,0, 5,5,1, 6,5,1, 7,5,1,
    // End marker
    0
  };


  /*
   * Name: puzzle13, 9 x 9
   * 	(_ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _)
   * 	(_ _ _ * * * _ _ _)
   * 	(_ _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ _)
   * 	(_ _ _ * * * _ _ _)
   * 	(_ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _)
   */
  const int g62[] = {
    // Width and height of crossword grid
    9, 9,
    // Number of black fields
    16,
    // Black field coordinates
    0,4, 1,4, 2,4, 3,2, 3,6, 4,0, 4,1, 4,2, 4,6, 4,7, 4,8, 5,2, 5,6, 6,4, 7,4, 8,4,
    // Length and number of words of that length
    9, 2,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,0,
    // Length and number of words of that length
    4, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,5,1, 0,7,0, 0,8,0, 1,0,1, 1,5,1, 2,0,1, 2,5,1, 5,0,0, 5,1,0, 5,7,0, 5,8,0, 6,0,1, 6,5,1, 7,0,1, 7,5,1, 8,0,1, 8,5,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,6,0, 3,3,1, 3,4,0, 4,3,1, 5,3,1, 6,2,0, 6,6,0,
    // Length and number of words of that length
    2, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 3,7,1, 5,0,1, 5,7,1,
    // End marker
    0
  };


  /*
   * Name: puzzle14, 10 x 10
   * 	(* * * _ _ _ _ * * *)
   * 	(* * _ _ _ _ _ * * *)
   * 	(* _ _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ * * _ _ _)
   * 	(_ _ _ _ * * * _ _ _)
   * 	(_ _ _ * * * _ _ _ _)
   * 	(_ _ _ * * _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ _ *)
   * 	(* * * _ _ _ _ _ * *)
   * 	(* * * _ _ _ _ * * *)
   */
  const int g63[] = {
    // Width and height of crossword grid
    10, 10,
    // Number of black fields
    38,
    // Black field coordinates
    0,0, 0,1, 0,2, 0,7, 0,8, 0,9, 1,0, 1,1, 1,7, 1,8, 1,9, 2,0, 2,8, 2,9, 3,5, 3,6, 4,4, 4,5, 4,6, 5,3, 5,4, 5,5, 6,3, 6,4, 7,0, 7,1, 7,9, 8,0, 8,1, 8,2, 8,8, 8,9, 9,0, 9,1, 9,2, 9,7, 9,8, 9,9,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    1,2,0, 2,1,1, 2,7,0, 7,2,1,
    // Length and number of words of that length
    5, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 1,2,1, 2,1,0, 3,0,1, 3,8,0, 5,6,0, 6,5,1, 8,3,1,
    // Length and number of words of that length
    4, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,1, 0,4,0, 3,0,0, 3,9,0, 4,0,1, 5,6,1, 6,5,0, 9,3,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,6,0, 3,7,1, 4,7,1, 5,0,1, 6,0,1, 7,3,0, 7,4,0,
    // End marker
    0
  };


  /*
   * Name: puzzle15, 11 x 11
   * 	(_ _ _ _ * * * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _)
   * 	(* _ _ _ _ _ * _ _ _ *)
   * 	(* * * _ _ _ _ _ * * *)
   * 	(* _ _ _ * _ _ _ _ _ *)
   * 	(_ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * * * _ _ _ _)
   */
  const int g64[] = {
    // Width and height of crossword grid
    11, 11,
    // Number of black fields
    26,
    // Black field coordinates
    0,4, 0,5, 0,6, 1,5, 2,5, 3,3, 3,7, 4,0, 4,6, 4,10, 5,0, 5,1, 5,2, 5,8, 5,9, 5,10, 6,0, 6,4, 6,10, 7,3, 7,7, 8,5, 9,5, 10,4, 10,5, 10,6,
    // Length and number of words of that length
    5, 22,
    // Coordinates where words start and direction (0 = horizontal)
    0,1,0, 0,2,0, 0,8,0, 0,9,0, 1,0,1, 1,4,0, 1,6,1, 2,0,1, 2,6,1, 3,5,0, 4,1,1, 5,3,1, 5,6,0, 6,1,0, 6,2,0, 6,5,1, 6,8,0, 6,9,0, 8,0,1, 8,6,1, 9,0,1, 9,6,1,
    // Length and number of words of that length
    4, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,7,1, 0,10,0, 7,0,0, 7,10,0, 10,0,1, 10,7,1,
    // Length and number of words of that length
    3, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,7,0, 1,6,0, 3,0,1, 3,4,1, 3,8,1, 4,3,0, 4,7,0, 4,7,1, 6,1,1, 7,0,1, 7,4,0, 7,4,1, 7,8,1, 8,3,0, 8,7,0,
    // End marker
    0
  };


  /*
   * Name: puzzle16, 13 x 13
   * 	(_ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * * *)
   * 	(* * * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ * * *)
   * 	(* * * _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ * _ _ _)
   */
  const int g65[] = {
    // Width and height of crossword grid
    13, 13,
    // Number of black fields
    34,
    // Black field coordinates
    0,4, 0,9, 1,4, 1,9, 2,4, 2,9, 3,0, 3,1, 3,2, 3,7, 4,6, 4,10, 4,11, 4,12, 5,5, 6,3, 6,4, 6,8, 6,9, 7,7, 8,0, 8,1, 8,2, 8,6, 9,5, 9,10, 9,11, 9,12, 10,3, 10,8, 11,3, 11,8, 12,3, 12,8,
    // Length and number of words of that length
    7, 2,
    // Coordinates where words start and direction (0 = horizontal)
    5,6,1, 7,0,1,
    // Length and number of words of that length
    6, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,8,0, 4,0,1, 7,4,0, 7,9,0, 8,7,1,
    // Length and number of words of that length
    5, 6,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 3,8,1, 5,0,1, 7,8,1, 8,7,0, 9,0,1,
    // Length and number of words of that length
    4, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,1, 0,5,1, 0,6,0, 0,10,0, 0,11,0, 0,12,0, 1,0,1, 1,5,1, 2,0,1, 2,5,1, 3,3,1, 4,0,0, 4,1,0, 4,2,0, 5,10,0, 5,11,0, 5,12,0, 9,0,0, 9,1,0, 9,2,0, 9,6,0, 9,6,1, 10,4,1, 10,9,1, 11,4,1, 11,9,1, 12,4,1, 12,9,1,
    // Length and number of words of that length
    3, 26,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,1,0, 0,2,0, 0,7,0, 0,10,1, 1,10,1, 2,10,1, 3,4,0, 3,9,0, 4,7,0, 4,7,1, 5,6,0, 6,0,1, 6,5,0, 6,5,1, 6,10,1, 7,3,0, 7,8,0, 8,3,1, 10,0,1, 10,5,0, 10,10,0, 10,11,0, 10,12,0, 11,0,1, 12,0,1,
    // End marker
    0
  };


  /*
   * Name: puzzle17, 15 x 15
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * _ _ _ _ * _ _ _ _ _ _ * *)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ * _ _ _)
   * 	(* * * _ _ _ * * * _ _ _ * * *)
   * 	(_ _ _ * _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(* * _ _ _ _ _ _ * _ _ _ _ * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   */
  const int g66[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    45,
    // Black field coordinates
    0,3, 0,7, 0,11, 1,3, 1,7, 1,11, 2,7, 3,0, 3,1, 3,8, 3,13, 3,14, 4,5, 4,9, 5,4, 5,10, 6,3, 6,7, 7,0, 7,1, 7,2, 7,6, 7,7, 7,8, 7,12, 7,13, 7,14, 8,7, 8,11, 9,4, 9,10, 10,5, 10,9, 11,0, 11,1, 11,6, 11,13, 11,14, 12,7, 13,3, 13,7, 13,11, 14,3, 14,7, 14,11,
    // Length and number of words of that length
    7, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,2,0, 0,6,0, 0,12,0, 2,0,1, 2,8,1, 6,8,1, 8,0,1, 8,2,0, 8,8,0, 8,12,0, 12,0,1, 12,8,1,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    2,11,0, 3,2,1, 7,3,0, 11,7,1,
    // Length and number of words of that length
    5, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,4,0, 0,10,0, 4,0,1, 4,10,1, 5,5,0, 5,5,1, 5,9,0, 9,5,1, 10,0,1, 10,4,0, 10,10,0, 10,10,1,
    // Length and number of words of that length
    4, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,9,0, 2,3,0, 3,9,1, 5,0,1, 5,11,1, 9,0,1, 9,11,0, 9,11,1, 11,2,1, 11,5,0, 11,9,0,
    // Length and number of words of that length
    3, 48,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,4,1, 0,8,0, 0,8,1, 0,12,1, 0,13,0, 0,14,0, 1,0,1, 1,4,1, 1,8,1, 1,12,1, 3,7,0, 4,0,0, 4,1,0, 4,6,1, 4,8,0, 4,13,0, 4,14,0, 6,0,1, 6,4,0, 6,4,1, 6,10,0, 7,3,1, 7,9,1, 8,0,0, 8,1,0, 8,6,0, 8,8,1, 8,12,1, 8,13,0, 8,14,0, 9,7,0, 10,6,1, 12,0,0, 12,1,0, 12,6,0, 12,13,0, 12,14,0, 13,0,1, 13,4,1, 13,8,1, 13,12,1, 14,0,1, 14,4,1, 14,8,1, 14,12,1,
    // End marker
    0
  };


  /*
   * Name: puzzle18, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * * _ _ _ _)
   * 	(* * * * _ _ _ * * _ _ _ * * *)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * * _ _ _ * * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ * _ _ _ * _ _ _)
   * 	(* * * _ _ _ * * _ _ _ * * * *)
   * 	(_ _ _ _ * * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g67[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    48,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,4, 3,5, 3,9, 4,0, 4,1, 4,2, 4,6, 4,7, 4,11, 4,12, 4,13, 4,14, 5,3, 5,7, 5,11, 6,10, 7,4, 7,5, 7,9, 7,10, 8,4, 9,3, 9,7, 9,11, 10,0, 10,1, 10,2, 10,3, 10,7, 10,8, 10,12, 10,13, 10,14, 11,5, 11,9, 11,10, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    10, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 5,6,0, 6,0,1, 8,5,1,
    // Length and number of words of that length
    5, 16,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,1, 1,5,1, 2,5,1, 3,10,1, 5,0,0, 5,1,0, 5,2,0, 5,12,0, 5,13,0, 5,14,0, 10,11,0, 11,0,1, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 36,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,6,0, 0,7,0, 0,11,0, 0,11,1, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 3,0,1, 6,11,1, 7,0,1, 7,11,1, 8,0,1, 11,0,0, 11,1,0, 11,2,0, 11,3,0, 11,7,0, 11,8,0, 11,11,1, 11,12,0, 11,13,0, 11,14,0, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 30,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,9,0, 3,6,1, 3,10,0, 4,3,1, 4,4,0, 4,5,0, 4,8,1, 4,9,0, 5,0,1, 5,4,1, 5,8,1, 5,12,1, 6,3,0, 6,7,0, 6,11,0, 7,6,1, 8,5,0, 8,9,0, 8,10,0, 9,0,1, 9,4,0, 9,4,1, 9,8,1, 9,12,1, 10,4,1, 10,9,1, 11,6,1, 12,5,0, 12,9,0,
    // End marker
    0
  };


  /*
   * Name: puzzle19, 15 x 15
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ _ _ * * *)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ _ * _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ * _ _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ * _ _ _ * _ _ _ _ _)
   * 	(* * * _ _ _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ * _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ _ _ * _ _ _ _)
   */
  const int g68[] = {
    // Width and height of crossword grid
    15, 15,
    // Number of black fields
    38,
    // Black field coordinates
    0,4, 0,10, 1,4, 1,10, 2,4, 2,10, 3,8, 4,0, 4,1, 4,2, 4,6, 4,7, 4,12, 4,13, 4,14, 5,5, 5,9, 6,4, 7,3, 7,11, 8,10, 9,5, 9,9, 10,0, 10,1, 10,2, 10,7, 10,8, 10,12, 10,13, 10,14, 11,6, 12,4, 12,10, 13,4, 13,10, 14,4, 14,10,
    // Length and number of words of that length
    10, 2,
    // Coordinates where words start and direction (0 = horizontal)
    6,5,1, 8,0,1,
    // Length and number of words of that length
    8, 2,
    // Coordinates where words start and direction (0 = horizontal)
    3,0,1, 11,7,1,
    // Length and number of words of that length
    7, 5,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,11,0, 7,4,1, 8,3,0, 8,11,0,
    // Length and number of words of that length
    6, 4,
    // Coordinates where words start and direction (0 = horizontal)
    3,9,1, 4,8,0, 5,6,0, 11,0,1,
    // Length and number of words of that length
    5, 23,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,5,1, 0,9,0, 1,5,1, 2,5,1, 3,10,0, 5,0,0, 5,0,1, 5,1,0, 5,2,0, 5,7,0, 5,10,1, 5,12,0, 5,13,0, 5,14,0, 7,4,0, 9,0,1, 9,10,1, 10,5,0, 10,9,0, 12,5,1, 13,5,1, 14,5,1,
    // Length and number of words of that length
    4, 32,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,6,0, 0,7,0, 0,11,1, 0,12,0, 0,13,0, 0,14,0, 1,0,1, 1,11,1, 2,0,1, 2,11,1, 4,8,1, 6,0,1, 8,11,1, 10,3,1, 11,0,0, 11,1,0, 11,2,0, 11,7,0, 11,8,0, 11,12,0, 11,13,0, 11,14,0, 12,0,1, 12,11,1, 13,0,1, 13,11,1, 14,0,1, 14,11,1,
    // Length and number of words of that length
    3, 12,
    // Coordinates where words start and direction (0 = horizontal)
    0,8,0, 3,4,0, 4,3,1, 5,6,1, 6,5,0, 6,9,0, 7,0,1, 7,12,1, 9,6,1, 9,10,0, 10,9,1, 12,6,0,
    // End marker
    0
  };


  /*
   * Name: puzzle20, 9 x 9
   * 	(* * * _ _ _ * * *)
   * 	(* * _ _ _ _ _ * *)
   * 	(* _ _ _ _ _ _ _ *)
   * 	(_ _ _ _ * _ _ _ _)
   * 	(_ _ _ * * * _ _ _)
   * 	(_ _ _ _ * _ _ _ _)
   * 	(* _ _ _ _ _ _ _ *)
   * 	(* * _ _ _ _ _ * *)
   * 	(* * * _ _ _ * * *)
   */
  const int g69[] = {
    // Width and height of crossword grid
    9, 9,
    // Number of black fields
    29,
    // Black field coordinates
    0,0, 0,1, 0,2, 0,6, 0,7, 0,8, 1,0, 1,1, 1,7, 1,8, 2,0, 2,8, 3,4, 4,3, 4,4, 4,5, 5,4, 6,0, 6,8, 7,0, 7,1, 7,7, 7,8, 8,0, 8,1, 8,2, 8,6, 8,7, 8,8,
    // Length and number of words of that length
    7, 4,
    // Coordinates where words start and direction (0 = horizontal)
    1,2,0, 1,6,0, 2,1,1, 6,1,1,
    // Length and number of words of that length
    5, 4,
    // Coordinates where words start and direction (0 = horizontal)
    1,2,1, 2,1,0, 2,7,0, 7,2,1,
    // Length and number of words of that length
    4, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,5,0, 3,0,1, 3,5,1, 5,0,1, 5,3,0, 5,5,0, 5,5,1,
    // Length and number of words of that length
    3, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,1, 0,4,0, 3,0,0, 3,8,0, 4,0,1, 4,6,1, 6,4,0, 8,3,1,
    // End marker
    0
  };


  /*
   * Name: puzzle21, 13 x 13
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ * * * _ _ _ _ _)
   * 	(_ _ _ * * * * * * * _ _ _)
   * 	(_ _ _ _ _ * * * _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ * _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   */
  const int g70[] = {
    // Width and height of crossword grid
    13, 13,
    // Number of black fields
    41,
    // Black field coordinates
    0,4, 0,8, 1,4, 1,8, 2,4, 2,8, 3,6, 4,0, 4,1, 4,2, 4,6, 4,10, 4,11, 4,12, 5,5, 5,6, 5,7, 6,3, 6,4, 6,5, 6,6, 6,7, 6,8, 6,9, 7,5, 7,6, 7,7, 8,0, 8,1, 8,2, 8,6, 8,10, 8,11, 8,12, 9,6, 10,4, 10,8, 11,4, 11,8, 12,4, 12,8,
    // Length and number of words of that length
    6, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 3,0,1, 3,7,1, 7,3,0, 7,9,0, 9,0,1, 9,7,1,
    // Length and number of words of that length
    5, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,7,0, 5,0,1, 5,8,1, 7,0,1, 7,8,1, 8,5,0, 8,7,0,
    // Length and number of words of that length
    4, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,9,1, 0,10,0, 0,11,0, 0,12,0, 1,0,1, 1,9,1, 2,0,1, 2,9,1, 9,0,0, 9,1,0, 9,2,0, 9,10,0, 9,11,0, 9,12,0, 10,0,1, 10,9,1, 11,0,1, 11,9,1, 12,0,1, 12,9,1,
    // Length and number of words of that length
    3, 24,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 0,6,0, 1,5,1, 2,5,1, 3,4,0, 3,8,0, 4,3,1, 4,7,1, 5,0,0, 5,1,0, 5,2,0, 5,10,0, 5,11,0, 5,12,0, 6,0,1, 6,10,1, 7,4,0, 7,8,0, 8,3,1, 8,7,1, 10,5,1, 10,6,0, 11,5,1, 12,5,1,
    // End marker
    0
  };


  /*
   * Name: puzzle22, 13 x 13
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ * * * _ _ _ _ _)
   * 	(_ _ _ _ * * * * * _ _ _ _)
   * 	(_ _ _ _ _ * * * _ _ _ _ _)
   * 	(* * * _ _ _ * _ _ _ * * *)
   * 	(_ _ _ _ _ _ _ _ _ _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   * 	(_ _ _ _ * _ _ _ * _ _ _ _)
   */
  const int g71[] = {
    // Width and height of crossword grid
    13, 13,
    // Number of black fields
    37,
    // Black field coordinates
    0,4, 0,8, 1,4, 1,8, 2,4, 2,8, 4,0, 4,1, 4,2, 4,6, 4,10, 4,11, 4,12, 5,5, 5,6, 5,7, 6,4, 6,5, 6,6, 6,7, 6,8, 7,5, 7,6, 7,7, 8,0, 8,1, 8,2, 8,6, 8,10, 8,11, 8,12, 10,4, 10,8, 11,4, 11,8, 12,4, 12,8,
    // Length and number of words of that length
    13, 4,
    // Coordinates where words start and direction (0 = horizontal)
    0,3,0, 0,9,0, 3,0,1, 9,0,1,
    // Length and number of words of that length
    5, 8,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,0, 0,7,0, 5,0,1, 5,8,1, 7,0,1, 7,8,1, 8,5,0, 8,7,0,
    // Length and number of words of that length
    4, 28,
    // Coordinates where words start and direction (0 = horizontal)
    0,0,0, 0,0,1, 0,1,0, 0,2,0, 0,6,0, 0,9,1, 0,10,0, 0,11,0, 0,12,0, 1,0,1, 1,9,1, 2,0,1, 2,9,1, 6,0,1, 6,9,1, 9,0,0, 9,1,0, 9,2,0, 9,6,0, 9,10,0, 9,11,0, 9,12,0, 10,0,1, 10,9,1, 11,0,1, 11,9,1, 12,0,1, 12,9,1,
    // Length and number of words of that length
    3, 20,
    // Coordinates where words start and direction (0 = horizontal)
    0,5,1, 1,5,1, 2,5,1, 3,4,0, 3,8,0, 4,3,1, 4,7,1, 5,0,0, 5,1,0, 5,2,0, 5,10,0, 5,11,0, 5,12,0, 7,4,0, 7,8,0, 8,3,1, 8,7,1, 10,5,1, 11,5,1, 12,5,1,
    // End marker
    0
  };


  const int* grids[] = {
    &g0[0], &g1[0], &g2[0], &g3[0], &g4[0], &g5[0], &g6[0], &g7[0], &g8[0],
    &g9[0], &g10[0], &g11[0], &g12[0], &g13[0], &g14[0], &g15[0], &g16[0],
    &g17[0], &g18[0], &g19[0], &g20[0], &g21[0], &g22[0], &g23[0], &g24[0],
    &g25[0], &g26[0], &g27[0], &g28[0], &g29[0], &g30[0], &g31[0], &g32[0],
    &g33[0], &g34[0], &g35[0], &g36[0], &g37[0], &g38[0], &g39[0], &g40[0],
    &g41[0], &g42[0], &g43[0], &g44[0], &g45[0], &g46[0], &g47[0], &g48[0],
    &g49[0], &g50[0], &g51[0], &g52[0], &g53[0], &g54[0], &g55[0], &g56[0],
    &g57[0], &g58[0], &g59[0], &g60[0], &g61[0], &g62[0], &g63[0], &g64[0],
    &g65[0], &g66[0], &g67[0], &g68[0], &g69[0], &g70[0], &g71[0]
  };

  const unsigned int n_grids = 72;

}

// STATISTICS: example-any
