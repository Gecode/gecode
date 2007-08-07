/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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
#include "gecode/minimodel.hh"
#include "examples/sudoku.icc"
#include "gecode/bdd/bddexample.hh"

/**
 * \brief %Example: Some %Sudoku puzzles using set constraints
 *
 * Does not really require any explanation...
 *
 * \ingroup Example
 *
 */
class SudokuBddConst : public BddExample {
protected:
  // example instance
  int instance;
  // n order of the sudoku
  const int n;
  // nn number of cells per block 3*3=9
  const int s;
  BddVarArray x;

public:
  /// Actual model
  SudokuBddConst(const Options& opt)
    : BddExample(opt), instance(opt.size), n(example_size(examples[instance])), s(n*n), 
      x(this, this->m.object(), s, IntSet::empty, 1, s*s) {

    GECODE_AUTOARRAY(int, rows, s*s);
    GECODE_AUTOARRAY(int, cols, s*s);
    GECODE_AUTOARRAY(int, blocks, s*s);

    // build fixed row and column vars
    for (int j = 0; j < s; j++) {
      int zi = j * s;
      int ci = 0;
      for (int z = j * s + 1; z <= (j + 1) * s; z++, zi++, ci++) {
        rows[zi] = z;
        cols[zi] = j + 1 + ci * s;
      }
    }

    for (int b = 1; b <= s; b++) {
      // lines of blocks
      int lob = n*n*n * ( (b - 1) / n) ;  
      // columns of blocks
      int cob = n*( (b-1) % n); 
      int zi = (b - 1) * s;
      for (int v = 0; v < n; v++) {
        for (int w = 0; w < n; w++) {
          blocks[zi] = lob + cob + (v * s) + w + 1;
          zi++;
        }
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<s; i++) {
      for (int j=0; j<s; j++) {
        if (int idx = value_at(examples[instance], s, i, j)) {
          // since this framework defines matrices in (C, R) = (column, row) 
          // we change the access
          dom(this, x[idx-1], SRT_SUP, (j*s)+(i+1), opt.scl );
	  // this is done in disjointglb
	  for (int z = 0; z < s; z++) {
	    if (z != idx - 1) {
	      dom(this, x[z], SRT_DISJ, (j*s)+(i+1), opt.scl );
	    }
	  }
        }
      }
    }

    if (opt.mode != EM_TIME) {
      //print grid coverage
      coverage();
    }

    for (int i = 0; i < s; i++) {
      cardinality(this, x[i], s);
    }

    // All x must be pairwise disjoint since every position 
    // in the grid can only be used once

    if ( s == 9 ) {
      partition(this, x);
    } else {
      for (int i = 0; i < s - 1; i++) {
	for (int j = i + 1; j < s; j++) {
	  rel(this, x[i], SRT_DISJ, x[j], opt.scl);
	}
      }
    }

    for (int j = 0; j < s; j++) {
      for (int i = 0; i < s; i++) {
	int* rowi = &rows[i*s];
	IntSet is(rowi, s);
	exactly(this, x[j], is, 1, opt.scl);
      }
    }
    
    for (int j = 0; j < s; j++) {
      for (int i = 0; i < s; i++) {
	int* coli = &cols[i*s];
	IntSet is(coli, s);
	exactly(this, x[j], is, 1, opt.scl);
      }
    }

    for (int j = 0; j < s; j++) {
      for (int i = 0; i < s; i++) {
	int* blocki = &blocks[i*s];
	IntSet is(blocki, s);
	exactly(this, x[j], is, 1, opt.scl);
      }
    }
   
    if (s > 0) {
      if (opt.naive) {
	branch(this, x, BDD_BVAR_NONE, BDD_BVAL_MIN_UNKNOWN);
      } else {
	branch(this, x, BDD_BVAR_MIN_CARD, BDD_BVAL_MIN_UNKNOWN);
      }
    }
  }

  /// Constructor for cloning \a s
  SudokuBddConst(bool share, SudokuBddConst& sbc) : 
    BddExample(share,sbc), instance(sbc.instance), n(sbc.n), s(sbc.s) {
    x.update(this, share, sbc.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuBddConst(share,*this);
  }

  // compute how many digits number a has
  int digit(int a) {
    int r = 1;
    while (a /= 10) r++;
    return r;
  }

  /// Print the variables and the respective grid positions they fill in
  virtual void
  printvars(void) {
    std::cout << "\nprintvars\n";
    for (int i = 0; i < n*n; i++) {
      std::cout << "Val: " << i + 1 << " occurs at:\t";
      std::cout << x[i] << " " << x[i].glbSize() << " * " << x[i].lubSize() << "\n\n";
    }
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "Solution\n";
    std::cout << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      bool emp = true;
      for (int j=0; j<n*n; j++) {
        int p = j + 1; // value at position
        int v = i + 1; // position
        if (x[j].contains(v)) {
          emp = false;
          if (p < 10) {
            int diffrow = i / (n*n);
            int diffcol = (i % (n*n));
            int idx = value_at(examples[instance], n*n, diffrow, diffcol);
            int diffd = digit(n*n) - digit(p);
            for (int k = 0; k < diffd; k++) 
              std::cout << " ";
            if (idx == p) {
              // matches partial assignment
              //std::cout << "M";
              //std::cout << " ";
	      std::cout << p;
            } else {
              // could infer something new
              std::cout << p;
            }
            std::cout << " ";
          } else {
            int diffrow = i / (n*n);
            int diffcol = (i % (n*n));
            int idx = value_at(examples[instance], n*n, diffrow, diffcol);
            int diffd = digit(n*n) - digit(p);
            for (int k = 0; k < diffd; k++) 
              std::cout << " ";
            if (idx == p) {
              // matches partial assignment
	      //               for (int k = 0; k < (digit(p)); k++) 
	      //                 //std::cout << "M";
	      //                 //std::cout << " ";
	      std::cout << p;
            } else {
              // could infer something new
              std::cout << p;
            }
            std::cout << " ";
          }
          break;
        }
      }
      if (emp) {
        for (int k = 0; k < digit(n*n); k++) 
          std::cout << ".";
	//std::cout << " ";
        std::cout << " ";
      }
      if((i+1)%(n*n) == 0)
        std::cout << std::endl << '\t';
    }

    // printvars();
  }

  /// Print the percentage of filled-in grid positions

  void coverage(void) {
    std::cout << "coverage\t";
    // Fill-in predefined fields
    int nn = n*n;
    int all = 0;
    int emp = 0;
    std::cout << '\t';
    for (int i=0; i<nn; i++) {
      for (int j=0; j<nn; j++) {
	all++;
        int idx = value_at(examples[instance], nn, i, j);
        if (!idx) {
	  emp++;
	}
      }
    }
    double res = ((double) emp / (double) all);
    std::cout << "covered: " << emp << "/"<< all << "= " << std::setprecision(6) << res << "\n";
  }

  /// Print the grid as specified by the problem instance

  void printpartial(void) {
    std::cout << "printpartial\n";
    // Fill-in predefined fields
    int nn = n*n;
    std::cout << '\t';
    for (int i=0; i<nn; i++) {
      for (int j=0; j<nn; j++) {
        int idx = value_at(examples[instance], nn, j, i);
        if (idx) {
          int diffd = digit(n*n) - digit(idx);
          for (int k = 0; k < diffd; k++) 
            std::cout << " ";
          std::cout << idx<< " ";// << "("<< i<< ","<<j<<") ";
        } else {
          for (int k = 0; k < digit(n*n); k++) {
            std::cout << ".";
	    //std::cout << " ";
          }
          std::cout << " ";//<< "("<< i<< ","<<j<<") ";
        }
      }
      std::cout << "\n\t";
    }
    std::cout << "\n";
  }
};


/** \brief Main-function
 *  \relates SudokuBddConst
 */
int
main(int argc, char** argv) {

  BddOptions opt("Sudoku (Bdd)");
  opt.iterations = 1;
  opt.size       = 0;
  opt.solutions  = 1;
  opt.naive      = true;

  // extra options for the bddexample
  // initial bdd nodes in the table
  opt.initvarnum = 5000000;
  // initial cache size for bdd operations
  opt.initcache  = 2000000;

  // granularity of debugging output
  opt.level      = Gecode::BDD_BND;

  // print final measurement of bdd nodes used
  // setting debug to true will give a more detailed output like this
  /*
  Buddy-Information:
        registered vars: 9
        allocated nodes: 200003
        used for vars:   729
        free nodes:      145682
        used nodes:      54321
  */
  opt.debug      = true;

  // consistency level for set constraints
  opt.scl        = SCL_DOM;

  opt.parse(argc,argv);

  if (opt.size <18) {
    opt.initvarnum = 200000;
    // initial cache size for bdd operations
    opt.initcache  = 8000;
  }
    
  if (opt.size >= n_examples) {
    std::cout << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }

  Example::run<SudokuBddConst,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

