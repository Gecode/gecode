/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/examples/sat.cpp
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Raphael Reischuk <reischuk@cs.uni-sb.de>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Raphael Reischuk, 2008
 *     Guido Tack, 2008
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <fstream>
#include <string>
#include <vector>

#include <quacode/qspaceinfo.hh>
#include <gecode/driver.hh>


using namespace Gecode;

/** \brief Options for %SAT problems
 *
 */
class QDimacsOptions : public Options {
public:
  /// Print strategy or not
  Gecode::Driver::BoolOption _printStrategy;
  /// Parameter to decide between optimized quantified constraints or usual ones
  Driver::BoolOption _qConstraint;
  /// Name of the QDIMACS file to parse
  std::string filename;
  /// Initialize options with file name \a s
  QDimacsOptions(const char* s, bool _qConstraint0)
    : Options(s),
      _printStrategy("-printStrategy","Print strategy",false),
      _qConstraint("-quantifiedConstraints",
                   "whether to use quantified optimized constraints",
                   _qConstraint0)
  {
    add(_printStrategy);
    add(_qConstraint);
  }
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    // Parse regular options
    Options::parse(argc,argv);
    // Filename, should be at position 1
    if (argc == 1) {
      help();
      exit(1);
    }
    filename = argv[1];
    argc--;
  }
  /// Return true if the strategy must be printed
  bool printStrategy(void) const {
    return _printStrategy.value();
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(string) " << std::endl
              << "\t\tqdimacs file to parse (.cnf)" << std::endl;
  }
};

/**
 * \brief %Example: CNF QSAT solver
 *
 * QSAT finds models of Quantified Boolean variables such
 * that a set of clauses is satisfied or shows that
 * no such model exists.
 *
 * This example parses a Q-Dimacs CNF file in which
 * the constraints are specified. For each line of
 * the file a clause propagator is posted.
 *
 * Format of Q-Dimacs CNF files:
 *
 * A dimacs file starts with comments (each line
 * starts with c). The number of variables and the
 * number of clauses is defined by the line
 *
 *    p cnf \<variables\> \<clauses\>
 *
 * Then come the quantifiers of the binder. Each lines
 * starts with a 'a' for universal quantifier or a 'e'
 * for existential quantifier. The following number until
 * zero correspond to the id of the variables that take this
 * quantifier. All variable of the matrix which does not appear
 * here are considered as the outermost existential variables
 * of the problem.
 *
 *    a \<variable list\>
 *    e \<variable list\>
 *
 * Each of the subsequent lines specifies a clause.
 * A positive literal is denoted by a positive
 * integer, a negative literal is denoted by the
 * corresponding negative integer. Each line is
 * terminated by 0.
 *
 * c sample CNF file
 * p cnf 3 2
 * 3 -1 0
 * 1 2 -1 0
 *
 * \ingroup Example
 */
class QDimacs : public Script, public QSpaceInfo {
private:
  /// The Boolean variables
  typedef std::vector< BoolVarArray > QBoolVarArray;
  QBoolVarArray qx;
public:
  /// The actual problem
  QDimacs(const QDimacsOptions& opt) : Script(opt), QSpaceInfo() {
    if (!opt.printStrategy()) strategyMethod(0); // disable build and print strategy
    parseQDIMACS(opt.filename.c_str(),opt._qConstraint.value());
  }

  /// Constructor for cloning
  QDimacs(bool share, QDimacs& s) : Script(share,s), QSpaceInfo(*this,share,s), qx(s.qx) {
    for (unsigned int i=0; i<qx.size(); i++)
      qx[i].update(*this,share,s.qx[i]);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new QDimacs(share,*this);
  }


  /// Post constraints according to QDIMACS file \a f
  void parseQDIMACS(const char* f, bool usedQuantifiedConstraint) {
    BoolVarArray x;
    std::vector<int> x_rk;
    std::vector<Gecode::TQuantifier> x_quant;
    int variables = 0;
    int clauses = 0;
    std::ifstream dimacs(f);
    if (!dimacs) {
      std::cerr << "error: file '" << f << "' not found" << std::endl;
      exit(1);
    }
    std::cout << "Solving problem from QDIMACS file '" << f << "'"
              << std::endl;
    std::string line;
    int c = 0;
    while (dimacs.good()) {
      std::getline(dimacs,line);
      // Comments (ignore them)
      if (line[0] == 'c' || line == "") {
      }
      // Line has format "p cnf <variables> <clauses>"
      else if (variables == 0 && clauses == 0 &&
               line[0] == 'p' && line[1] == ' ' &&
               line[2] == 'c' && line[3] == 'n' &&
               line[4] == 'f' && line[5] == ' ') {
        int i = 6;
        while (line[i] >= '0' && line[i] <= '9') {
          variables = 10*variables + line[i] - '0';
          i++;
        }
        i++;
        while (line[i] >= '0' && line[i] <= '9') {
          clauses = 10*clauses + line[i] - '0';
          i++;
        }
        x = BoolVarArray(*this, variables, 0, 1);
        x_rk.resize(variables,Int::Limits::max);
        x_quant.resize(variables,EXISTS);
        std::cout << "(" << variables << " variables, "
                  << clauses << " clauses)" << std::endl << std::endl;
      }
      // Line has format "a <variables>" or "e <variables>" or "r <variables>"
      else if ( (line[0] == 'a' && line[1] == ' ') ||
                (line[0] == 'e' && line[1] == ' ') ||
                (line[0] == 'r' && line[1] == ' ') ) {
        if (line[0] == 'r') {
          std::cerr << "format error in dimacs file, \"r\" quantifier not recognized for now" << std::endl;
          std::cerr << "context: '" << line << "'" << std::endl;
          std::exit(EXIT_FAILURE);
        }
        BoolVarArgs tmp;
        Gecode::TQuantifier quant = (line[0]=='e')?EXISTS:FORALL;
        std::vector<int> values;
        int i = 2;
        while (line[i] != 0) {
          if (line[i] == ' ') {
            i++;
            continue;
          }
          int value = 0;
          while (line[i] >= '0' && line[i] <= '9') {
            value = 10 * value + line[i] - '0';
            i++;
          }
          if (value != 0) {
            values.push_back(value);
            tmp << x[value-1];
            x_rk[value-1] = qx.size();
            x_quant[value-1] = quant;
            if (quant == FORALL) setForAll(*this,x[value-1]);
            i++;
          }
        }

        std::cout << "( " << values.size() << " ";
        std::cout << ((quant==EXISTS)?"existential":"universal");
        std::cout << "variables )"<< std::endl;
        qx.push_back( BoolVarArray(*this,tmp) );
      }
      // Parse regular clause
      else if (variables > 0 &&
      ((line[0] >= '0' && line[0] <= '9') || line[0] == '-' || line[0] == ' ')) {
        c++;
        std::vector<int> pos;
        std::vector<int> neg;
        int i = 0;
        while (line[i] != 0) {
          if (line[i] == ' ') {
            i++;
            continue;
          }
          bool positive = true;
          if (line[i] == '-') {
            positive = false;
            i++;
          }
          int value = 0;
          while (line[i] >= '0' && line[i] <= '9') {
            value = 10 * value + line[i] - '0';
            i++;
          }
          if (value != 0) {
            if (positive)
              pos.push_back(value-1);
            else
              neg.push_back(value-1);
            i++;
          }
        }

        // Create positive BoolVarArgs
        BoolVarArgs positives(pos.size());
        QBoolVarArgs qpositives(pos.size());
        for (int i=pos.size(); i--;) {
          positives[i] = x[pos[i]];
          qpositives[i] = QBoolVar(x_quant[pos[i]],x[pos[i]],x_rk[pos[i]]);
        }

        BoolVarArgs negatives(neg.size());
        QBoolVarArgs qnegatives(neg.size());
        for (int i=neg.size(); i--;) {
          negatives[i] = x[neg[i]];
          qnegatives[i] = QBoolVar(x_quant[neg[i]],x[neg[i]],x_rk[neg[i]]);
        }

        // Post propagators
        if (usedQuantifiedConstraint)
          qclause(*this, BOT_OR, qpositives, qnegatives, 1);
        else
          clause(*this, BOT_OR, positives, negatives, 1);
      }
      else {
        std::cerr << "format error in dimacs file" << std::endl;
        std::cerr << "context: '" << line << "'" << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
    dimacs.close();
    if (clauses != c) {
      std::cerr << "error: number of specified clauses seems to be wrong."
                << std::endl;
      std::exit(EXIT_FAILURE);
    }

    for (unsigned int i=0; i<qx.size(); i++)
      branch(*this, qx[i], INT_VAR_NONE(), INT_VAL_MIN());

    // FIN DESCRIPTION PB
  }
};


/** \brief Main-function
 */
int main(int argc, char* argv[]) {

  QDimacsOptions opt("QDIMACS",true);
  opt.parse(argc,argv);

  // Check whether all arguments are successfully parsed
  if (argc > 1) {
    std::cerr << "Could not parse all arguments." << std::endl;
    opt.help();
    std::exit(EXIT_FAILURE);
  }

  // Run SAT solver
  Script::run<QDimacs,DFS,QDimacsOptions>(opt);
  return 0;
}

// STATISTICS: example-any
