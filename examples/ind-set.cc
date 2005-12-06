/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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
#include "minimodel.hh"

/** 
 * \name Graph specification for independent sets
 *
 * \relates IndSet
 */
//@{
/// Graph specification
class Graph {
public:
  const int  n_v; ///< Number of vertices
  const int  n_e; ///< Number of edges
  const int* e;   ///< Arrays of edges (as vertex pairs)
};

static const int e_20_10[] = {
  0, 4,  2,12,  12,14,  18,19,   7,10,
  9,12,  5,11,   6,15,   3,18,   7,16
};

static const Graph g_20_10 = { 20, 10, e_20_10 };

static const int e_40_20[] = {
  21,30,   11,30,   19,38,   20,25,   11,24,
  20,33,    8,39,    4, 5,    6,16,    5,32,
  0, 9,    5,24,   25,28,   36,38,   14,20,
  19,25,   11,22,   13,30,    7,36,   15,33
};

static const Graph g_40_20 = { 40, 20, e_40_20 };
//@}


/**
 * \brief %Example: Independent Sets in a %Graph
 *
 * \ingroup Example
 *
 */
class IndSet : public Example {
protected:
  /// Graph used
  const Graph& g;
  /// Whether vertex included in independent set
  BoolVarArray v;
  /// How many elements has indipendent set
  IntVar       k;
public:
  /// Actual model
  IndSet(const Options& opt)
    : g(opt.size == 0 ?  g_20_10 : g_40_20),
      v(this,g.n_v,0,1), k(this,0,g.n_e) {
    const int* e = g.e;
    const int* e1 = e++; const int* e2 = e++;
    for (int i = g.n_e; i--; )
      post(this, v[*e1]+v[*e2] <= 1);
    linear(this, v, IRT_EQ, k);
    branch(this, v, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  IndSet(bool share, IndSet& s) : Example(share,s), g(s.g) {
    v.update(this, share, s.v);
    k.update(this, share, s.k);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new IndSet(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tk = " << k << std::endl
	      << "\tv[] = {";
    for (int i = 0; i < v.size(); i++)
      std::cout << v[i] << ", ";
    std::cout << "};" << std::endl;
  }

  /// Add constraint for next better solution
  void
  constrain(Space* s) {
    rel(this, k, IRT_GR, static_cast<IndSet*>(s)->k.val());
  }
};


/** \brief Main-function
 *  \relates IndSet
 */
int
main(int argc, char** argv) {
  Options opt("IndSet");
  opt.solutions  = 0;
  opt.size       = 1;
  opt.iterations = 1000;
  opt.parse(argc,argv);
  Example::run<IndSet,BAB>(opt);
  return 0;
}

// STATISTICS: example-any

