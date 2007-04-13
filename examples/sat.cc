/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
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

#include <iomanip>


int nvars, nclauses;


/**
 * \brief %Example: Satisfiability of set of sat-clauses
 *
 * \ingroup ExProblem
 *
 */
class SAT : public Example {
protected:
  /// Array of Boolean variables
  BoolVarArray x;
public:
  /// Model
  SAT(const Options& opt)
    : x(this,nvars,0,1) {

    BoolVarArgs y(nvars);
    for (int i = nvars; i--; )
      y[i] = post(this, !x[i]);

    //std::string line;
    //while (std::getline(std::cin, line)) {
    //  std::cout << line << std::endl;
    //}
    BoolVarArgs v(nvars);
    while (true) { // Loop over clauses
      int var;
      int csiz = 0;
      while (true) { // Loop over vars in clause
        std::cin >> var;
        if (var == 0) break; // Clause read
        v[csiz++] = var > 0 ? x[var-1] : y[-var-1];
      }
      BoolVarArgs c(csiz);
      for (int i = csiz; i--; )
        c[i] = v[i];
      if (opt.naive)
        rel(this, c, BOT_OR, 1);
      else
        rel(this, c, BOT_OR_WL, 1);
    }
    
    branch(this, x, BVAR_DEGREE_MAX, BVAL_MIN);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "x = [";
    for (int i = 0; i < x.size(); ++i)
      std::cout << x[i] << (i!=x.size()-1 ? ", " : "");
    std::cout << "]" << std::endl;
  }

  /// Constructor for cloning \a s
  SAT(bool share, SAT& s)
    : Example(share,s) {
    x.update(this,share,s.x);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new SAT(share,*this);
  }

};

/** \brief Main-function
 *  \relates SAT
 */
int
main(int argc, char** argv) {
  Options opt("SAT");
  opt.solutions = 1;
  opt.size      = 0;
  opt.naive     = true;
  opt.parse(argc,argv);

  std::string s;
  std::cin >> s; std::cin >> s;
  std::cin >> nvars >> nclauses >> std::ws;
  std::cout << "p " << s << " " << nvars << " " << nclauses << std::endl;

  Example::run<SAT,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

