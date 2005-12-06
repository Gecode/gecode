/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date: 2005-10-20 15:28:46 +0200 (Thu, 20 Oct 2005) $ by $Author: zayenz $
 *     $Revision: 2391 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "set.hh"
#include "examples/support.hh"
#include "minimodel.hh"

#include "examples/sudoku.icc"

/**
 * \brief %Example: Some %Sudoku puzzles using set constraints
 *
 * Does not really require any explanation...
 *
 * \ingroup Example
 *
 */
class SudokuSet : public Example {
protected:
  static const int n = 3;
  SetVarArray x;
public:
  /// Actual model
  SudokuSet(const Options& opt)
    : x(this,n*n,IntSet::empty,1,n*n*n*n,9,9) {

    const int nn = n*n;

    IntSet row[9];
    IntSet col[9];
    IntSet block[9];

    // Set up the row and column set constants
    for (int i=0; i<9; i++) {
      IntSet dsr((i*nn)+1, (i*nn)+9);
      row[i] = dsr;

      int dsc_arr[9] = { 1+i, 10+i, 19+i, 28+i, 37+i, 46+i, 55+i, 64+i, 73+i };
      IntSet dsc(dsc_arr, 9);

      col[i] = dsc;
    }

    // Set up the block set constants
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++) {
	int dsb_arr[9] = {
	  (j*27)+(i*3)+1, (j*27)+(i*3)+2, (j*27)+(i*3)+3,
	  (j*27)+(i*3)+10, (j*27)+(i*3)+11, (j*27)+(i*3)+12,
	  (j*27)+(i*3)+19, (j*27)+(i*3)+20, (j*27)+(i*3)+21
	};
	IntSet dsb(dsb_arr, 9);
 	block[i*3+j]=dsb;
      }

    // All x must be pairwise disjoint
    for (int i=0; i<nn-1; i++)
      for (int j=i+1; j<nn; j++)
 	rel(this, x[i], SRT_DISJ, x[j]);
    distinct(this, x, nn);

    // The x must intersect in exactly one element with each
    // row, column, and block
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++) {
	SetVar inter_row(this, IntSet::empty, 1, 9*9, 1, 1);
	rel(this, x[i], SOT_INTER, row[j], SRT_EQ, inter_row);
	SetVar inter_col(this, IntSet::empty, 1, 9*9, 1, 1);
	rel(this, x[i], SOT_INTER, col[j], SRT_EQ, inter_col);
	SetVar inter_block(this, IntSet::empty, 1, 9*9, 1, 1);
	rel(this, x[i], SOT_INTER, block[j], SRT_EQ, inter_block);
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
	if (examples[opt.size][i][j] != 0) {
	  int idx = examples[opt.size][i][j]-1;
	  dom(this, x[idx], SRT_SUP, (i+1)+(j*nn) );
	}
    
    branch(this, x, SETBVAR_NONE, SETBVAL_MIN);
  }
  
  /// Constructor for cloning \a s
  SudokuSet(bool share, SudokuSet& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuSet(share,*this);
  }
  
  /// Print solution
  virtual void
  print(void) {
    std::cout << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      for (int j=0; j<n*n; j++) {
	if (x[j].contains(i+1)) {
	  std::cout << j+1 << " ";
	  break;
	}
      }
      if((i+1)%(n*n) == 0)
	std::cout << std::endl << '\t';
    }
    std::cout << std::endl;
  }
};


/** \brief Main-function
 *  \relates SudokuSet
 */
int
main(int argc, char** argv) {
  Options opt("Sudoku (Set)");
  opt.iterations = 100;
  opt.size       = 0;
  opt.icl        = ICL_DOM;
  opt.solutions  = 1;
  opt.naive      = true;
  opt.parse(argc,argv);
  if (opt.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and " 
	      << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<SudokuSet,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

