/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

/**
 * \brief %Example: n-%Knights tour
 *
 * Fill an n times n chess board with knights such that the
 * knights do a full tour by knights move (last knight reaches
 * first knight again). The formulation is due to Gert Smolka.
 *
 * \ingroup Example
 *
 */
class Knights : public Example {
private:
  /// Size of board
  const int n;
  /// Maps board field to number of knight on board
  IntVarArray jump;
public:

  /// Return field at position \a i, \a j
  int
  field(int i, int j) {
    return i*n+j;
  }

  /// The actual model
  Knights(const Options& opt)
    : n(opt.size), jump(this,n*n,0,n*n-1) {
    const int nn = n*n;

    // Map knight to its predecessor of succesor on board
    IntVarArgs pred(nn);
    IntVarArgs succ(nn);
    for (int i = nn; i--; ) {
      IntVar p(this,0,nn-1);
      IntVar s(this,0,nn-1);
      pred[i]=p; succ[i]=s;
    } 

    // Place the first two knights
    rel(this, jump[field(0,0)], IRT_EQ, 0);
    rel(this, jump[field(1,2)], IRT_EQ, 1);

    distinct(this, jump, opt.icl);

    for (int f = 0; f < nn; f++) {
      int i = f / n;
      int j = f % n;
      // Compute array of neighbours
      int nbs[8];
      int n_nbs = 0;

      static const int moves[8][2] = {
	{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}
      };

      for (int nij = 0; nij<8 ; nij++) {
	int id = i + moves[nij][0];
	int jd = j + moves[nij][1];
	
	if ((id >= 0) && (jd >= 0) && (id < n) && (jd < n)) {
	  int g = field(id,jd);
	  nbs[n_nbs++] = g;
	
	  BoolVar b(this,0,1);

	  rel(this, succ[f], IRT_EQ, g, b);
	  rel(this, pred[g], IRT_EQ, f, b);

	  bool_xor(this, 
		   post(this, ~(jump[g]-jump[f] == 1)),
		   post(this, ~(jump[g]-jump[f] == 1-nn)),
		   b);
	}
      }
	
      IntSet ds(nbs, n_nbs);
      dom(this, pred[f], ds);
      dom(this, succ[f], ds);
      rel(this, succ[f], IRT_NQ, pred[f]);
    }
    branch(this, succ, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  Knights(bool share, Knights& s) : Example(share,s), n(s.n) {
    jump.update(this, share, s.jump);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Knights(share,*this);
  }
  /// Print board
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
	std::cout.width(3);
	std::cout << jump[field(i,j)] << " ";
      }
      std::cout << std::endl << "\t";
    }
    std::cout << std::endl;
  }
};

/** \brief Main-function
 *  \relates Knights
 */
int
main(int argc, char** argv) {
  Options opt("Knights");
  opt.iterations = 50;
  opt.size       = 8;
  opt.c_d        = 25;
  opt.a_d        = 5;
  opt.parse(argc,argv);
  Example::run<Knights,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

