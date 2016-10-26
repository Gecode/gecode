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
class NimFiboOptions : public Options {
public:
  int n; /// Parameter to be given on the command line
  /// Print strategy or not
  Gecode::Driver::BoolOption _printStrategy;
  /// Use cut or not
  Gecode::Driver::BoolOption _cut;
  /// Initialize options for example with name \a s
  NimFiboOptions(const char* s, int n0)
    : Options(s), n(n0),
      _printStrategy("-printStrategy","Print strategy",false),
      _cut("-cut","Use cut in problem model",true) {
        add(_printStrategy);
        add(_cut);
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
              << "\t\tnumber of initial matchs" << std::endl;
  }
  /// Return true if the strategy must be printed
  bool printStrategy(void) const {
    return _printStrategy.value();
  }
  /// Return true if cut must be used
  bool cut(void) const {
    return _cut.value();
  }
};

/// Succeed the space
static void gf_success(Space& home) {
  Space::Branchers b(home);
  while (b()) {
    BrancherHandle bh(b.brancher());
    ++b;
    bh.kill(home);
  }
}

/// Dummy function
static void gf_dummy(Space& ) { }

/// Adding cut
static void cut(Space& home, const BoolExpr& expr) {
  BoolVar o(home,0,1);
  rel(home, o == expr);
  when(home, o, &gf_success, &gf_dummy);
}

class QCSPNimFibo : public Script, public QSpaceInfo {
  IntVarArray X;

public:

  QCSPNimFibo(const NimFiboOptions& opt) : Script(opt), QSpaceInfo()
  {
    std::cout << "Loading problem" << std::endl;
    if (!opt.printStrategy()) strategyMethod(0); // disable build and print strategy
    using namespace Int;
    // Number of matches
    int NMatchs = opt.n;
    int nIterations = (NMatchs%2)?NMatchs:NMatchs+1;

    IntVarArgs x;
    X = IntVarArray(*this,nIterations,1,NMatchs-1);
    x << X[0];

    BoolVar o_im1(*this,1,1);
    BoolExpr cutExpr1(BoolVar(*this,1,1));
    BoolExpr cutExpr2(BoolVar(*this,1,1));
    branch(*this, X[0], INT_VAR_NONE(), INT_VAL_MIN());

    for (int i=1; i < nIterations; i++) {
      BoolVar oi(*this,0,1), o1(*this,0,1), o2(*this,0,1);

      x << X[i];

      rel(*this, X[i], IRT_LQ, expr(*this, 2*X[i-1]), o1);
      linear(*this, x, IRT_LQ, NMatchs, o2);

      if (i%2) {
        // Universal Player
        setForAll(*this,X[i]);
        rel(*this, o_im1 == ((o1 && o2) >> oi));
        // Adding Cut
        if (opt.cut())
          cut(*this, cutExpr1 && cutExpr2 && !(o1 && o2));  // Universal player can't play
        branch(*this, X[i], INT_VAR_NONE(), INT_VAL_MIN());
      } else {
        // Existantial Player
        rel(*this, o_im1 == (o1 && o2 && oi));
        branch(*this, X[i], INT_VAR_NONE(), INT_VAL_MIN());
      }
      cutExpr1 = cutExpr1 && o1;
      cutExpr2 = o2;
      o_im1 = oi;
    }
  }

  QCSPNimFibo(bool share, QCSPNimFibo& p)
    : Script(share,p), QSpaceInfo(*this,share,p)
  {
    X.update(*this,share,p.X);
  }

  virtual Space* copy(bool share) { return new QCSPNimFibo(share,*this); }


  void print(std::ostream& os) const {
    strategyPrint(os);
  }
};

int main(int argc, char* argv[])
{

  NimFiboOptions opt("QCSP Nim-Fibo",3);
  opt.parse(argc,argv);
  Script::run<QCSPNimFibo,QDFS,NimFiboOptions>(opt);

  return 0;
}
