/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include "examples/support.hh"

#ifdef GECODE_HAVE_SET_VARS
#include "set.hh"
#endif

#include "minimodel.hh"

// include the example specifications
#include "examples/sudoku.icc"

IntVarArgs unionOfArgs(const IntVarArgs as, const IntVarArgs bs) {
  IntVarArgs res(as.size() + bs.size());
  for (int i=as.size(); i--;)
    res[i] = as[i];
  for (int i=bs.size(); i--;)
    res[as.size()+i] = bs[i];
  return res;
}

#ifdef GECODE_HAVE_SET_VARS
void same(Space* home, IntVarArgs as, IntVarArgs bs) {
  SetVar u(home, IntSet::empty, 1, 81);
  rel(home, SOT_DUNION, as, u);
  rel(home, SOT_DUNION, bs, u);
}
#endif

/**
 * \brief %Example: Some %Sudoku puzzles
 *
 * Does not really require any explanation...
 *
 * \ingroup Example
 *
 */
class Sudoku : public Example {
protected:
  static const int n = 3;
  IntVarArray x;
public:
  /// Actual model
  Sudoku(const Options& opt)
    : x(this,n*n*n*n,1,n*n) {
    const int nn = n*n;
    MiniModel::Matrix<IntVarArray> m(x, nn, nn);

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

#ifdef GECODE_HAVE_SET_VARS
    if (!opt.naive) {

      // Implied constraints linking squares and rows
      for (int i=0; i<nn; i+=n)
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
#endif

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
	if (examples[opt.size][i][j] != 0)
	  rel(this, m(i,j), IRT_EQ, examples[opt.size][i][j]);
    
    branch(this, x, BVAR_MIN_MIN, BVAL_SPLIT_MIN);
  }
  
  /// Constructor for cloning \a s
  Sudoku(bool share, Sudoku& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Sudoku(share,*this);
  }
  
  /// Print solution
  virtual void
  print(void) {
    std::cout << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      std::cout << x[i] << " ";
      if((i+1)%(n*n) == 0)
	std::cout << std::endl << '\t';
    }
    std::cout << std::endl;
  }
};


/** \brief Main-function
 *  \relates Sudoku
 */
int
main(int argc, char** argv) {
  Options opt("Sudoku");
  opt.iterations = 1000;
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
  Example::run<Sudoku,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

