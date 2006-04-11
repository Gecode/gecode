/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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
 * \brief Extended Options taking two parameters instead of only one
 *
 * This is needed in order to parameters
 *
 * \relates LangfordNum
 */
class ExtOptions : public Options {
public:
  ExtOptions(const char* s) : Options(s) {}
  int n;
  int k;
};

/**
 * \brief %Example: Langford's number problem

 * Problem 024 in the categoy "combinatorial mathematics" 
 * of http://www.csplib.org/.
 *
 * For a detailed problem analysis see 
 * http://www.lclark.edu/~miller/langford.html
 *
 * \ingroup Example
 */
class LangfordNum : public Example {
private:
  int n;
  int k;

  /// Problem variables
  /// pos contains the position of the values in seqence 
  IntVarArray pos;
  IntVarArray y;
  /// pi makes the permutation explicit
  IntVarArray pi;

public:

  /**
   * \brief Constrain x to be a permutation of \f$ S_{kn} = \{0, \dots, n*k - 1\}\f$
   *        
   * \f$ \forall i, j\in S_{kn}, i\neq j: x_i \neq x_j \f$
   */
  
  void adiff_skn(Space* home, IntVarArray& x, IntVarArray& pi) {
    
    IntVarArray skn(home, k * n);
    for (int i = k * n; i--; ) {
      skn[i].init(home, i, i);
    }

    for (int i = k * n; i--; ) {
      pi[i].init (home, 0, k * n - 1);
    }
    
    sortedness(home, x, skn, pi);
  }

  /** 
   * \brief Returns the position of the j-th occurence of value \f$ v =(i + 1)\f$
   *
   */
  IntVar& p(int i, int j) {
    return pos[i * k + j];
  }

  IntVar& ys(int i, int j) {
    return y[i * k + j];
  }
  
  /**
   * \brief The occurences of a value v in the Langford sequence are v numbers apart.
   *
   * Let \f$ \#(i, v) \f$ denote the position of the i-th occurence of value v 
   * in the Langford Sequence. Then this function posts the constraint that
   * \f$ \forall i, j \in \{1, \dots, k\}, i \neq j: 
   *     \forall v \in \{1, \dots, n\}: \#(i, v) + (v + 1) = \#(j, v)\f$
   *
   */
  
  void distance(Space* home) {
    // p(i, j) denotes the j-th occurence of value i + 1
    for (int i = 0; i < n; i++) {
      int v = i + 1;
      for (int j = 0; j < k - 1; j++) {
	post(home, p(i, j) + (v + 1) == p(i, j + 1));
      }
    }
  }

  LangfordNum(const Options& op) {
    
    const ExtOptions* eop = NULL;
    eop = reinterpret_cast<const ExtOptions*> (&op);
    n = eop->n;
    k = eop->k;

//     seq  = IntVarArray(this, k * n);
    y    = IntVarArray(this, k * n);
    pos  = IntVarArray(this, k * n);
    pi   = IntVarArray(this, k * n);

    IntSet dom_val(1, n);
    IntSet dom_idx(0, k * n - 1);

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < k; j++) {
	p(i, j).init(this, dom_idx);
	y[i*k + j].init(this, dom_val);
      }
    }
    
    distance(this);
    adiff_skn(this, pos, pi);
    
    rel(this, y[0], IRT_LE, y[n*k - 1]);

    // dual  using gcc
    // gcc(this, y, k, eop->icl);
    // lack of distance constraints for the dual model

    // channeling
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < k; j++) {
	element(this, y, p(i, j), IntVar(this, i + 1, i + 1));
      }
    }

    branch(this, pos, BVAR_SIZE_MIN, BVAL_MIN);    
  }

  LangfordNum(bool share, LangfordNum& l)
    : Example(share, l), n(l.n), k(l.k) {
    pos.update(this, share, l.pos);
    pi.update (this, share, l.pi);
    y.update(this, share, l.y);

  }

  virtual Space* 
  copy(bool share) {
    return new LangfordNum(share, *this);
  }

  virtual void print(void){
    std::cout << "\nL(" << k << "," << n <<"):\n";
    
    GECODE_AUTOARRAY(int, values, k*n);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < k; j++) {
	values[i * k + j] = i + 1;
      }
    }

    std::cout << "seq: ";
    for(int c = 0; c < k * n; c++) {
      int i = 0;
      while (pos[i].assigned() && pos[i].val() != c) {
	i++;
      }
      std::cout << values[i] << " ";
    }
    std::cout <<"\n";

//     std::cout << "val: ";
//     for (int i = 0; i < k * n; i++) {
//       std::cout << values[i] << " ";
//     }
//     std::cout <<"\n";

//     std::cout << "y  : ";
//     for (int i = 0; i < k * n; i++) {
//       std::cout << y[i] << " ";
//     }
//     std::cout <<"\n";

    std::cout << "pos: ";
    for (int i = 0; i < k * n; i++) {
      std::cout << pos[i] << " ";
    }
    std::cout <<"\n";

//     std::cout << "pi : ";
//     for (int i = 0; i < k * n; i++) {
//       std::cout << pi[i] << " ";
//     }
//     std::cout <<"\n";

  }
};

int main(int argc, char** argv){
  ExtOptions o("Langford Numbers");
  if (argc < 2) {
    std::cerr << "specify parameters k and n\n";
    std::cerr << "usage is: ./langfordnum k n [gecode options] \n";
    return -1;
  }
  char* name = argv[0];
  o.k = atoi(argv[1]);
  o.n = atoi(argv[2]);
  argv[2] = name;
  argv++; 
  argv++;
  if (o.k < 1) {
    std::cerr << "k must be at least 1!\n";
    return -1;
  }
  if (o.k > o.n) {
    std::cerr << "n must be at least k!\n";
    return -1;
  }
  o.parse(argc - 2, argv);
  Example::run<LangfordNum, DFS>(o);
  return 0;
}

// STATISTICS: example-any

