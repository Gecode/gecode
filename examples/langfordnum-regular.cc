/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagervkist@gecode.org>
 *
 *  Copyright:
 *     Miakel Lagerkvist, 2006
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
 * \relates LangfordNumRegular
 */
class ExtOptions : public Options {
public:
  ExtOptions(const char* s) : Options(s) {}
  int n;
  int k;
};

/**
 * \brief %Example: Langford's number problem
 *
 * This version uses regular constraints for placing the values. 
 *
 * Problem 024 in the categoy "combinatorial mathematics"
 * of http://www.csplib.org/.
 *
 * For a detailed problem analysis see
 * http://www.lclark.edu/~miller/langford.html
 *
 * \ingroup Example
 */
class LangfordNumRegular : public Example {
private:
  int n;
  int k;

  /// Problem variables
  IntVarArray y;

public:
  LangfordNumRegular(const Options& op) {

    const ExtOptions* eop = NULL;
    eop = static_cast<const ExtOptions*> (&op);
    n = eop->n;
    k = eop->k;

    y = IntVarArray(this, k * n, 1, n);

    // For placing two numbers 3 three steps apart, we construct the
    // regular expression 0*100010*, and apply it to the projection of
    // the sequence on the value.
    for (int i = 1; i <= n; ++i) {
      // Start of regular expression
      REG reg = *REG(0) + REG(1);
      // For each next number to place
      for (int ki = 1; ki < k; ++ki) {
        reg = reg + REG(0)(i, i) + REG(1);
      }
      // End of expression
      reg = reg + *REG(0);
      // Projection for value i
      BoolVarArgs cv(k*n);
      for (int cvi = k*n; cvi--; )
        cv[cvi] = post(this, ~(y[cvi] == i));
      DFA dfa = reg;
      regular(this, cv, dfa);
    } 

    // Symmetry breaking
    post(this, y[0] < y[n*k - 1]);

    // Branching
    branch(this, y, BVAR_SIZE_MIN, BVAL_MAX);
  }

  LangfordNumRegular(bool share, LangfordNumRegular& l)
    : Example(share, l), n(l.n), k(l.k) {
    y.update(this, share, l.y);
  }

  virtual Space*
  copy(bool share) {
    return new LangfordNumRegular(share, *this);
  }

  virtual void print(void){
    std::cout << "\nL(" << k << "," << n <<"):\n";

    for (int i = 0; i < y.size(); ++i)
      std::cout << y[i] << " ";

    std::cout << std::endl;
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
  Example::run<LangfordNumRegular, DFS>(o);
  return 0;
}

// STATISTICS: example-any

