/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2008
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
#include <QtGui>
#endif

using namespace Gecode;


/** \brief Custom brancher for knight's tours using Warnsdorff's rule
 *
 * This class implements Warnsdorff's rule for finding knight's
 * tours. The next position is choosen by taking the jump that
 * minimizes the number of alternatives in the next step.
 *
 * \relates Knights
 */
class Warnsdorff : public Brancher {
protected:
  /// Views of the brancher
  ViewArray<Int::IntView> x;
  /// Next variable to branch on
  mutable int start;
  /// %Choice
  class Choice : public Gecode::Choice {
  public:
    /// Position of variable
    int pos;
    /// Value of variable
    int val;
    /** Initialize choice for brancher \a b, position \a pos0, 
     *  and value \a val0.
     */
    Choice(const Brancher& b, int pos0, int val0)
      : Gecode::Choice(b,2), pos(pos0), val(val0) {}
    /// Report size occupied
    virtual size_t size(void) const {
      return sizeof(Choice);
    }
    /// Archive into \a e
    virtual void archive(Archive& e) const {
      Gecode::Choice::archive(e);
      e << pos << val;
    }
  };
 
  /// Construct brancher
  Warnsdorff(Home home, ViewArray<Int::IntView>& xv) 
    : Brancher(home), x(xv), start(0) {}
  /// Copy constructor
  Warnsdorff(Space& home, bool share, Warnsdorff& b) 
    : Brancher(home, share, b), start(b.start) {
    x.update(home, share, b.x);
  }
public:
  /// Check status of brancher, return true if alternatives left
  virtual bool status(const Space&) const {
    // A path to follow can be at most x.size() long
    for (int n=x.size(); n--; ) {
      if (!x[start].assigned()) 
        return true;
      // Follow path of assigned variables
      start = x[start].val();
    }
    return false;
  }
  /// Return choice
  virtual Gecode::Choice* choice(Space&) {
    Int::ViewValues<Int::IntView> iv(x[start]);
    int n = iv.val();
    unsigned int min = x[n].size();
    ++iv;
    // Choose the value with the fewest neighbors
    while (iv()) {
      if (x[iv.val()].size() < min) {
        n = iv.val();
        min = x[n].size();
      }
      ++iv;
    }
    return new Choice(*this, start, n);
  }
  /// Return choice
  virtual Choice* choice(const Space&, Archive& e) {
    int pos, val;
    e >> pos >> val;
    return new Choice(*this, pos, val);
  }
  /// Perform commit for choice \a _c and alternative \a a
  virtual ExecStatus commit(Space& home, const Gecode::Choice& _c, 
                            unsigned int a) {
    const Choice& c = static_cast<const Choice&>(_c);
    if (a == 0)
      return me_failed(x[c.pos].eq(home, c.val)) ? ES_FAILED : ES_OK;
    else 
      return me_failed(x[c.pos].nq(home, c.val)) ? ES_FAILED : ES_OK;
  }
  /// Copy brancher
  virtual Actor* copy(Space& home, bool share) {
    return new (home) Warnsdorff(home, share, *this);
  }
  /// Post brancher
  static void post(Home home, const IntVarArgs& x) {
    ViewArray<Int::IntView> xv(home, x);
    (void) new (home) Warnsdorff(home, xv);
  }
  /// Delete brancher and return its size
  virtual size_t dispose(Space&) {
    return sizeof(*this);
  }
};


/// Base-class for knight's tour example
class Knights : public Script {
public:
  /// Size of board
  const int n;
  /// Maps board field to successor field
  IntVarArray succ;
  /// Propagation to use for model
  enum {
    PROP_REIFIED, ///< Use reified constraints
    PROP_CIRCUIT  ///< Use single circuit constraints
  };
  /// Branching to use for model
  enum {
    BRANCH_NAIVE,      ///< Use naive, lexicographical branching
    BRANCH_WARNSDORFF, ///< Use Warnsdorff's rule
  };
  /// Return field at position \a x, \a y
  int f(int x, int y) const {
    return x + y*n;
  }
  /// Return x coordinate at field \a f
  int x(int f) const {
    return f % n;
  }
  /// Return y coordinate at field \a f
  int y(int f) const {
    return f / n;
  }
  /// Compute set of neighbour fields
  IntSet neighbors(int i) {
    static const int moves[8][2] = {
      {-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}
    };
    int nbs[8]; int n_nbs = 0;
    for (int m=0; m<8; m++) {
      int nx = x(i) + moves[m][0], ny = y(i) + moves[m][1];
      if ((nx >= 0) && (nx < n) && (ny >= 0) && (ny < n))
        nbs[n_nbs++] = f(nx,ny);
    }
    return IntSet(nbs,n_nbs);
  }
  /// Constructor
  Knights(const SizeOptions& opt)
    : n(opt.size()), succ(*this,n*n,0,n*n-1) {
    switch (opt.branching()) {
    case BRANCH_NAIVE:
      branch(*this, succ, INT_VAR_NONE, INT_VAL_MIN);
      break;
    case BRANCH_WARNSDORFF:
      Warnsdorff::post(*this, succ);
      break;
    }
  }
  /// Constructor for cloning \a s
  Knights(bool share, Knights& s) : Script(share,s), n(s.n) {
    succ.update(*this, share, s.succ);
  }
  /// Print board
  virtual void
  print(std::ostream& os) const {
    int* jump = new int[n*n];
    {
      int j=0;
      for (int i=0; i<n*n; i++) {
        jump[j]=i; j=succ[j].min();
      }
    }
    os << "\t";
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        os.width(3);
        os << jump[f(i,j)] << " ";
        }
        os << std::endl << "\t";
    }
    os << std::endl;
    delete [] jump;
  }
};

/**
 * \brief %Example: n-Knight's tour (simple model)
 *
 * Fill an n times n chess board with knight's moves such that
 * the knight does a full tour (last move reaches first move
 * again). The formulation is due to Gert Smolka.
 *
 * \ingroup Example
 *
 */
class KnightsReified : public Knights {
public:
  KnightsReified(const SizeOptions& opt) : Knights(opt) {
    const int nn = n*n;

    // Map knight to its predecessor of succesor on board
    IntVarArgs jump(nn);
    IntVarArgs pred(nn);

    for (int i = nn; i--; ) {
      IntVar p(*this,0,nn-1); pred[i]=p;
      IntVar j(*this,0,nn-1); jump[i]=j;
    }

    // Place the first two knights
    rel(*this, jump[f(0,0)], IRT_EQ, 0);
    rel(*this, jump[f(1,2)], IRT_EQ, 1);

    distinct(*this, jump, opt.icl());
    channel(*this, succ, pred, opt.icl());

    for (int f = 0; f < nn; f++) {
      IntSet ds = neighbors(f);
      for (IntSetValues i(ds); i(); ++i)
        rel(*this,
            expr(*this, (jump[i.val()]-jump[f] == 1)),
            BOT_XOR,
            expr(*this, (jump[i.val()]-jump[f] == 1-nn)),
            expr(*this, (succ[f] == i.val())));
      dom(*this, pred[f], ds);
      dom(*this, succ[f], ds);
      rel(*this, succ[f], IRT_NQ, pred[f]);
    }
  }
  /// Constructor for cloning \a s
  KnightsReified(bool share, KnightsReified& s) : Knights(share,s) {}
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new KnightsReified(share,*this);
  }
};

/**
 * \brief %Example: n-%Knights tour (model using circuit)
 *
 * Fill an n times n chess board with knights such that the
 * knights do a full tour by knights move (last knight reaches
 * first knight again).
 *
 * \ingroup Example
 *
 */
class KnightsCircuit : public Knights {
public:
  KnightsCircuit(const SizeOptions& opt) : Knights(opt) {
    // Fix the first move
    rel(*this, succ[0], IRT_EQ, f(1,2));

    circuit(*this, succ, opt.icl());

    for (int f = 0; f < n*n; f++)
      dom(*this, succ[f], neighbors(f));
  }
  /// Constructor for cloning \a s
  KnightsCircuit(bool share, KnightsCircuit& s) : Knights(share,s) {}
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new KnightsCircuit(share,*this);
  }
};

/*
 * Just to fool some scripts:
 * \brief %Example: n-Knight's tour
 *
 */

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing knight moves on a chess board
class KnightsInspector : public Gist::Inspector {
protected:
  /// The graphics scene displaying the board
  QGraphicsScene* scene;
  /// The window containing the graphics scene
  QMainWindow* mw;
  /// The size of a field on the board
  static const int unit = 30;
public:
  /// Constructor
  KnightsInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space& s) {
    const Knights& k = static_cast<const Knights&>(s);
    
    if (!scene)
      initialize();
    QList <QGraphicsItem*> itemList = scene->items();
    foreach (QGraphicsItem* i, scene->items()) {
      scene->removeItem(i);
      delete i;
    }

    for (int i=0; i<k.n; i++) {
      for (int j=0; j<k.n; j++) {
        scene->addRect(i*unit,j*unit,unit,unit);
        
        QPen pen(Qt::black, 2);
        if (!k.succ[i*k.n+j].assigned()) {
          pen.setColor(Qt::red);
          pen.setStyle(Qt::DotLine);
          pen.setWidth(0);
        }
        for (IntVarValues xv(k.succ[i*k.n+j]); xv(); ++xv) {
          int ky = xv.val() % k.n;
          int kx = xv.val() / k.n;
          scene->addLine(i*unit+unit/2,j*unit+unit/2,
                         kx*unit+unit/2,ky*unit+unit/2,
                         pen);
        }
        
      }
    }
    mw->show();    
  }
  
  /// Set up main window
  void initialize(void) {
    mw = new QMainWindow();
    scene = new QGraphicsScene();
    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHints(QPainter::Antialiasing);
    mw->setCentralWidget(view);
    mw->setAttribute(Qt::WA_QuitOnClose, false);
    mw->setAttribute(Qt::WA_DeleteOnClose, false);
    QAction* closeWindow = new QAction("Close window", mw);
    closeWindow->setShortcut(QKeySequence("Ctrl+W"));
    mw->connect(closeWindow, SIGNAL(triggered()),
                mw, SLOT(close()));
    mw->addAction(closeWindow);
  }
  
  /// Name of the inspector
  virtual std::string name(void) { return "Board"; }
  /// Finalize inspector
  virtual void finalize(void) {
    delete mw;
    mw = NULL;
  }
};
#endif

/** \brief Main-function
 *  \relates Knights
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Knights");
  opt.iterations(100);
  opt.size(8);
  opt.propagation(Knights::PROP_CIRCUIT);
  opt.propagation(Knights::PROP_REIFIED, "reified");
  opt.propagation(Knights::PROP_CIRCUIT, "circuit");
  opt.branching(Knights::BRANCH_NAIVE);
  opt.branching(Knights::BRANCH_NAIVE, "reified");
  opt.branching(Knights::BRANCH_WARNSDORFF, "warnsdorff");

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
  KnightsInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc,argv);

  if (opt.propagation() == Knights::PROP_REIFIED) {
    Script::run<KnightsReified,DFS,SizeOptions>(opt);
  } else {
    Script::run<KnightsCircuit,DFS,SizeOptions>(opt);
  }
  return 0;
}

// STATISTICS: example-any

