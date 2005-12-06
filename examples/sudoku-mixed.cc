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

/// Concatenates two argument arrays
IntVarArgs unionOfArgs(const IntVarArgs as, const IntVarArgs bs) {
  IntVarArgs res(as.size() + bs.size());
  for (int i=as.size(); i--;)
    res[i] = as[i];
  for (int i=bs.size(); i--;)
    res[as.size()+i] = bs[i];
  return res;
}

/**
 * \brief Implements the "same" constraint
 *
 * Posts the constraint \f$\biguplus_i \{a_i\}=\biguplus_i \{b_i\}\f$
 */
void same(Space* home, IntVarArgs a, IntVarArgs b) {
  SetVar u(home, IntSet::empty, 1, 81);
  rel(home, SOT_DUNION, a, u);
  rel(home, SOT_DUNION, b, u);
}

/**
 * \brief %Example: Some %Sudoku puzzles using finite domain and set constraints
 *
 * The problem does not really require any explanation...
 *
 * This implementation combines both the finite domain and the finite set
 * model using channelling constraints.
 *
 * \ingroup Example
 *
 */
class SudokuMixed : public Example {
protected:
  static const int n = 3;
  SetVarArray x;
public:
  /// Actual model
  SudokuMixed(const Options& opt)
    : x(this,n*n,IntSet::empty,1,n*n*n*n,9,9) {

    const int nn = n*n;

    /********************************************************
     *
     * Finite Domain model
     *
     */
    IntVarArray y(this,n*n*n*n,1,n*n);

    MiniModel::Matrix<IntVarArray> m(y, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(this, m.row(i), opt.icl);
      distinct(this, m.col(i), opt.icl);
    }
    
    // Constraints for squares
    for (int i=0; i<nn; i+=n)
      for (int j=0; j<nn; j+=n) {
	distinct(this, m.slice(i, i+n, j, j+n), opt.icl);
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
	if (examples[opt.size][i][j] != 0)
	  rel(this, m(i,j), IRT_EQ, examples[opt.size][i][j]);    

    // Implied constraints linking squares and rows
    for (int i=0; i<nn; i+=n) {
      for (int j=0; j<nn; j+=n) {
	MiniModel::Matrix<IntVarArgs> block = m.slice(i, i+n, j, j+n);
	for (int r1=0; r1<2; r1++)
	  for (int r2=r1+1; r2<3; r2++) {
	    IntVarArgs b1 = unionOfArgs(block.col(r1), block.col(r2));
	    IntVarArgs b2 = unionOfArgs(block.row(r1), block.row(r2));
	    IntVarArgs r(0);
	    IntVarArgs c(0);
	    switch (r1+r2) {
	    case 1:
	      r = unionOfArgs(m.slice(0,i,j+2,j+3), m.slice(i+n,nn,j+2,j+3));
	      c = unionOfArgs(m.slice(i+2,i+3,0,j), m.slice(i+2,i+3,j+n,nn));
	      assert(r.size()==6);
	      break;
	    case 2:
	      r = unionOfArgs(m.slice(0,i,j+1,j+2), m.slice(i+n,nn,j+1,j+2));
	      c = unionOfArgs(m.slice(i+1,i+2,0,j), m.slice(i+1,i+2,j+n,nn));
	      assert(r.size()==6);
	      break;
	    case 3:
	      r = unionOfArgs(m.slice(0,i,j+0,j+1), m.slice(i+n,nn,j+0,j+1));
	      c = unionOfArgs(m.slice(i+0,i+1,0,j), m.slice(i+0,i+1,j+n,nn));
	      assert(r.size()==6);
	      break;
	    default:
	      assert(false);
	    }
	    same(this, r, b1);
	    same(this, c, b2);
	  }
      }
    }

    /********************************************************
     *
     * Channelling between the models
     *
     */

    IntSet is0(0,0);
    SetVar dummySet0(this, is0, is0);
    IntVar dummyInt0(this, 0, 0);
    SetVarArgs xs(nn+1);
    xs[0] = dummySet0;
    for (int i=0; i<nn; i++)
      xs[i+1] = x[i];
    IntVarArgs ys(nn*nn+1);
    ys[0] = dummyInt0;
    for (int i=0; i<nn*nn; i++)
      ys[i+1] = y[i];

    channelVarVal(this, ys, xs);

    gcc(this, y, 9, ICL_DOM);

    
    /********************************************************
     *
     * Finite set model
     *
     */

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
  SudokuMixed(bool share, SudokuMixed& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuMixed(share,*this);
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
 *  \relates SudokuMixed
 */
int
main(int argc, char** argv) {
  Options opt("Sudoku (Mixed Model)");
  opt.iterations = 200;
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
  Example::run<SudokuMixed,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

