/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
 *
 *  Last modified:
 *     $Date: 2007-04-13 11:39:43 +0200 (fre, 13 apr 2007) $ by $Author: zayenz $
 *     $Revision: 4378 $
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

#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <iomanip>

/* Input format
 * # min max
 * # nbvariables
 * # nbrelations
 * #   *-> arity nbtuples
 * #        *-> tuple
 * # nbconstraints
 * #   *-> table variables^arity
 */

namespace {
  int mind, maxd;
  int nvars, ntabs, ncons;
  Table* tab;
  std::vector<std::vector<int> > cons;

  void read_table(std::istream& in, Table& tab) {
    int ntuples;
    int arity;
    in >> arity >> ntuples;
    for (int i = ntuples; i--; ) { // Add to table
      IntArgs t(arity);
      for (int i = 0; i < arity; ++i) {
        in >> t[i];
        //std::cerr << t[i] << " ";
      }
      //std::cerr << std::endl;
      tab.add(t);
    }
    //std::cerr << std::endl;
    tab.finalize();
  }

  void read_spec(std::istream& in) {
    in >> mind >> maxd >> nvars >> ntabs;
    tab = Memory::bmalloc<Table>(ntabs);
    for (int i = 0; i<ntabs; ++i) {
      //std::cerr << "Reading table " << i << std::endl;
      new (tab+i) Table();
      read_table(in, tab[i]);
    }
    in >> ncons;
    //std::cerr << "Number of constraints: " << ncons << std::endl;
    for (int i = ncons; i--; ) {
      std::vector<int> con;
      int table;
      in >> table;
      con.push_back(table);
      //std::cerr << "Constraint " << i << " on table " << table << std::endl;
      for (int i = 0; i < tab[table].arity(); ++i) {
        int val;
        in >> val;
        con.push_back(val);
        //std::cerr << val << " ";
      }
      //std::cerr << std::endl;
      cons.push_back(con);
    }
  }
}


/**
 * \brief %Example: Satisfiability of set of sat-clauses
 *
 * \ingroup ExProblem
 *
 */
class Extensional : public Example {
protected:
  /// Array of Boolean variables
  IntVarArray x;
public:
  /// Model
  Extensional(const Options& opt)
    : x(this,nvars,mind,maxd) {

    ExtensionalAlgorithm ea = EA_BASIC;
    if (!opt.naive) {
      ea = EA_INCREMENTAL;
    }
    
    for (int i = cons.size(); i--; ) {
      IntVarArgs iva(tab[cons[i][0]].arity());
      for (int j = 0; j < tab[cons[i][0]].arity(); ++j) {
        iva[j] = x[cons[i][j+1]];
        //std::cerr << cons[i][j+1] << " ";
      }
      //std::cerr << " on " << cons[i][0] << std::endl;
      extensional(this, iva, tab[cons[i][0]], ea);
    }
    //std::cerr << "constraints posted" << std::endl;
    branch(this, x, BVAR_SIZE_MIN, BVAL_MIN);
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
  Extensional(bool share, Extensional& s)
    : Example(share,s) {
    x.update(this,share,s.x);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Extensional(share,*this);
  }

};

/** \brief Main-function
 *  \relates Extensional
 */
int
main(int argc, char** argv) {
  Options opt("Extensional");
  opt.solutions = 1;
  opt.size      = 1000000;
#if defined(SYSTEM_ADVISOR_IMPROVE_CHEAP) || defined(SYSTEM_ADVISOR_IMPROVE_EXPENSIVE)
  opt.naive     = false;
#else
  opt.naive     = true;
#endif
  opt.parse(argc,argv);
  if (opt.size == 1000000)
    read_spec(std::cin);
  else {
    if (opt.size < 0 || opt.size >= 20) {
      std::cerr << "size must be between 0 and 19 or 100000 (=read from standard input)" << std::endl;
      return -1;
    }
    std::ostringstream s;
    s << "examples/tables/" << opt.size << ".tab";
    std::ifstream file(s.str().c_str());
    read_spec(file);
  }
  
  
  Example::run<Extensional,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

