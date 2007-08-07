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

// #include "qtexplorer/explorer.hh"
#include "gecode/bdd/bddexample.hh"

/**
 * \name Parameters for Steiner systems
 *
 * \relates BddSteiner
 */
//@{
/// Steiner parameters
struct SteinerSystem {
  /// Number of elements that have to be in exactly one block
  int t;
  /// Cardinality of a block
  int k;
  /// Cardinality of the whole set
  int n;
};

/// Systems
static const SteinerSystem sys[]=
  { 
    // Steiner triple systems are systems with t = 2 and k = 3
    {2,3,7},  {3,4,8},  {2,3,9},  {2,3,13}, {2,4,13}, {2,3,15}, {2,4,16},
    {2,6,16}, {3,4,16}, {2,5,21}, {3,6,22}, {2,3,31}
  };

/// Number of systems
static const unsigned int n_examples = sizeof(sys) / sizeof(SteinerSystem);

/// Compute faculty of \a n
int fac(int n) {
  if (n < 2) {
    return 1;
  } else {
    return n * fac(n - 1);
  }
}

/// Compute binomial coefficient \f$ \left(n\choose k \right)\f$
int binom(int n, int k) {
  int up = 1;
  for (int i = 0; i < k; i++) {
    up *= n;
    n--;
  }
  return (up / (fac(k)));
}

/// Compute the number of blocks in the example
int blocksize(int t, int k, int n) {
  return (binom(n, t) / binom(k, t));
}

//@}


/**
 * \brief %Example: Generalized steiner sytems
 *
 * This is problem 044 from csplib.
 *
 * \ingroup Example
 *
 */

class BddSteiner : public BddExample {
public:
  int t;
  int k;
  int n;
  int len;
  BddVarArray root;

  BddSteiner(const Options& op)
    : BddExample(op),
      t(sys[op.size].t), k(sys[op.size].k), n(sys[op.size].n), 
      len(blocksize(t, k, n)), root(this, this->m.object(), len, 1, n) {
    
    if (op.mode != EM_TIME) {
      // just for a first information
      std::cout << "Steiner-System: " << t << "-" << k << "-" << n << " with "<< len << " blocks \n";
    }

    // CUDD FAILS IN VARIABLE ORDER STRANGE THINGS ARE GOING ON HERE
    
    // each block has cardinality k
    for (int i=0; i< len; i++) {
      cardinality(this, root[i], k, op.scl);
    }

    // std::cout << "cardinality done\n";
    
    // two blocks may share atmost (t - 1) elements
    // that is each t-tuple of elements can be found in exactly one block
    for (int i=0; i < len - 1 ; i++) {    
      // std::cout << "atmost(" << i << ",";
      for (int j=i+1; j < len; j++) {
	// std::cout << j <<",";
        BddVar x = root[i];
        BddVar y = root[j];
	if (op.naive) {
	  // create intermediate variable
	  BddVar z(this, this->m.object(), 1, n);

	  rel(this, x, SOT_INTER, y, SRT_EQ, z, op.scl);
	  // std::cout << "card intermediate\n";
	  cardinality(this, z, 0, (t - 1), op.scl);
	  // symmetry-breaking
	  // std::cout << "before symm\n";
	  // rel(this, x, SRT_LE, y, op.scl);
	  // std::cout << "symm\n";
	} else {
	  atmostLex(this, x, y, (t - 1), SRT_LE, op.scl);
	}
      }
      // std::cout << ")\n";
    }

    // std::cout << "branch\n";
    branch(this, root, BDD_BVAR_NONE, BDD_BVAL_MAX_UNKNOWN_EX_FIRST); 
    // needed to simulate gecode branching
    //branch(this, root, BDD_BVAR_NONE, BDD_BVAL_MIN_UNKNOWN); 
    // std::cout << "finished branching\n";
  }
  
  BddSteiner(bool share, BddSteiner& s) : 
    BddExample(share,s), t(s.t), k(s.k), n(s.n), len(s.len) { 
    root.update(this, share, s.root);
  }

  virtual Space*
  copy(bool share) {
    return new BddSteiner(share,*this);
  }

  virtual void
  print(void) {
    std::cout << "Steiner(";
    std::cout << t << "-" << k << "-" << n << ") with "<< len << " blocks \n";
    for (int i=0; i<len; i++) {
      std::cout << "\t[" << i << "] = " << root[i] << "\n";
    }
    std::cout << "\n";
  }
};

int
main(int argc, char** argv) {

#ifdef GECODE_HAVE_LIB_CUDD  
    Cudd cm(0,0);
    cm.makeVerbose();
    BddMgr m(cm);
#endif

  BddOptions o("BddSteiner");
  o.size = 0;
  o.naive = true;

  o.initvarnum = 10000;
  o.initcache  = 1000;

  o.level = Gecode::BDD_BND;
  o.debug = true;
  o.scl = Gecode::SCL_DOM;

  o.parse(argc,argv);
  if (o.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and " << n_examples - 1
	      << std::endl;
    return 1;
  }
  Example::run<BddSteiner,DFS>(o);
  return 0;
}


// STATISTICS: example-any

