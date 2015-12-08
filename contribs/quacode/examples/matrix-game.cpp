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

#include <iostream>
#include <vector>

#include <quacode/qspaceinfo.hh>
#include <gecode/minimodel.hh>
#include <gecode/driver.hh>


using namespace Gecode;

#ifdef GECODE_HAS_GIST
namespace Gecode { namespace Driver {
  /// Specialization for QDFS
  template<typename S>
  class GistEngine<QDFS<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::explore(root, false, opt);
    }
  };
}}
#endif

/**
 * \brief Options taking one additional parameter
 */
class MatrixGameOptions : public Options {
protected:
  /// Optional file name
  Gecode::Driver::StringValueOption _file;
  /// Print strategy or not
  Gecode::Driver::BoolOption _printStrategy;
  /// Flag to known if we have to print initial board
  Driver::BoolOption _printBoard;
public:
  int n; /// Parameter to be given on the command line
  /// Initialize options for example with name \a s
  MatrixGameOptions(const char* s, int n0, bool pb0)
    : Options(s), _file("-file","File name for MatrixGameOptionsrix input"),
      _printStrategy("-printStrategy","Print strategy",false),
      _printBoard("-printBoard",
                   "whether to print initial board",pb0),
      n(n0) {
    add(_file);
    add(_printStrategy);
    add(_printBoard);
  }
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 2)
      return;
    n = atoi(argv[1]);
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(unsigned int) default: " << n << std::endl
              << "\t\tDepth of the matrix" << std::endl;
  }
  /// Return file name
  const char* file(void) const {
    return _file.value();
  }
  /// Return true if the strategy must be printed
  bool printStrategy(void) const {
    return _printStrategy.value();
  }
  /// Return true if we have to print initial board
  bool printBoard(void) const {
    return _printBoard.value();
  }
};

class QCSPMatrixGame : public Script, public QSpaceInfo {
  IntVarArray X;

public:

  QCSPMatrixGame(const MatrixGameOptions& opt) : Script(opt), QSpaceInfo()
  {
    std::cout << "Loading problem" << std::endl;
    if (!opt.printStrategy()) strategyMethod(0); // disable build and print strategy
    using namespace Int;
    int depth = opt.n;// Size of the matrix is 2^depth. Large values may take long to solve...
    int boardSize = (int)pow((double)2,(double)depth);
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // If a file is given we take the matrix from the file
    bool bFile = false;
    std::vector<int> tab;
    if (opt.file() != NULL) {
      std::ifstream inS(opt.file());
      if(!inS.is_open())
        bFile = false;
      else {
        bFile = true;
        int x;
        while (inS >> x) tab.push_back(x);
        boardSize = (int)sqrt(tab.size());
        depth = (int) (log(boardSize) / log(2));
      }
    }

    IntArgs board(boardSize*boardSize);
    for (int i=0; i<boardSize; i++)
      for (int j=0; j<boardSize; j++)
        if (bFile)
          board[j*boardSize+i] = tab[j*boardSize+i];
        else
          board[j*boardSize+i] = (int)( (double)rand()  /  ((double)RAND_MAX + 1) * 50 ) < 25 ? 0:1;

    int nbDecisionVar = 2*depth;
    IntArgs access(nbDecisionVar);
    access[nbDecisionVar-1]=1;
    access[nbDecisionVar-2]=boardSize;
    for (int i=nbDecisionVar-3; i>=0; i--)
      access[i]=access[i+2]*2;

    // Print initial board
    if (opt.printBoard()) {
      for (int i=0; i<boardSize; i++)
      {
        for (int j=0; j<boardSize; j++)
          std::cout << board[i*boardSize+j] << " ";
        std::cout << std::endl;
      }
      std::cout  << std::endl;
    }

    // Defining the player variables
    IntVarArgs x;
    X = IntVarArray(*this,nbDecisionVar,0,1);
    for (int i=0; i<nbDecisionVar; i++)
    {
      x << X[i];
      if ((i%2) == 1) setForAll(*this, X[i]);
    }

    // Goal constaints
    IntVar cstUn(*this,1,1);
    IntVar boardIdx(*this,0,boardSize*boardSize);
    linear(*this, access, x, IRT_EQ, boardIdx, IPL_DOM);
    element(*this, board, boardIdx, cstUn, IPL_DOM);

    branch(*this, X, INT_VAR_NONE(), INT_VAL_MIN());
  }

  QCSPMatrixGame(bool share, QCSPMatrixGame& p)
    : Script(share,p), QSpaceInfo(*this,share,p)
  {
    X.update(*this,share,p.X);
  }

  virtual Space* copy(bool share) { return new QCSPMatrixGame(share,*this); }


  void print(std::ostream& os) const {
    strategyPrint(os);
  }
};

int main(int argc, char* argv[])
{

  MatrixGameOptions opt("QCSP Matrix-Game",5,false);
  opt.parse(argc,argv);
  Script::run<QCSPMatrixGame,QDFS,MatrixGameOptions>(opt);

  return 0;
}
