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
#include <string>

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
class ConnectFourOptions : public Options {
protected:
  /// Print strategy or not
  Gecode::Driver::BoolOption _printStrategy;
  /// Model name
  Gecode::Driver::StringOption _QCSPmodel;
  /// Heuristic in branching
  Gecode::Driver::BoolOption _heuristic;
  /// File name of recorded moves
  Gecode::Driver::StringValueOption _file;
  /// Optional number of rows
  Gecode::Driver::UnsignedIntOption _row;
  /// Optional number of cols
  Gecode::Driver::UnsignedIntOption _col;
public:
  /// Initialize options for example with name \a s
  ConnectFourOptions(const char* s)
    : Options(s),
      _printStrategy("-printStrategy","Print strategy",false),
      _QCSPmodel("-QCSPmodel","Name of the model used for modeling problem",3),
      _heuristic("-heuristic","Use heuristic when branching (only for model + and ++)",true),
      _file("-file","File name of recorded moves"),
      _row("-row","Number of rows (minimum 4)",6),
      _col("-col","Number of cols (minimum 4)",7) {
    _QCSPmodel.add(1,"AllState","Model with all states as defined by P. Nightingale. Without Pure Value and heuristic setup.");
    _QCSPmodel.add(2,"AllState+","Model with all states as defined by P. Nightingale. With cut.");
    _QCSPmodel.add(3,"AllState++","Model with all states as defined by P. Nightingale. With cut and additional constraints.");
    add(_printStrategy);
    add(_QCSPmodel);
    add(_heuristic);
    add(_file);
    add(_row);
    add(_col);
  }
  /// Return true if the strategy must be printed
  bool printStrategy(void) const {
    return _printStrategy.value();
  }
  /// Return model name
  int QCSPmodel(void) const {
    return _QCSPmodel.value();
  }
  /// Return if heuristic must be used
  bool heuristic(void) const {
    return _heuristic.value();
  }
  /// Return file name
  const char *file(void) const {
    return _file.value();
  }
  /// Return number of rows
  int row(void) const {
    return _row.value();
  }
  /// Return number of cols
  int col(void) const {
    return _col.value();
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

template <int N>
struct c4Heuristic {
  static int value(const Space& _home, IntVar x, int);
};

// Template loop to avoid to write a hundred lines of code
template <int N>
struct FOR {
  static void go(IntBranchVal t[]) {
    t[N] = &c4Heuristic<N>::value;
    FOR<N-1>::go(t);
  }
};
template <>
struct FOR<0> {
  static void go(IntBranchVal t[]) {
    t[0] = &c4Heuristic<0>::value;
  }
};

class ConnectFourAllState : public Script, public QSpaceInfo {
  static const int Red   = 0;
  static const int Black = 1;
  static const int Nil   = 2;

  IntVarArray M; // Move variables (the column played)
  IntVarArray U; // Additional move variables (the column played) only usefull for simple model
  IntVarArray board;  // State of board
  IntVarArray h; // Number of token in col c
  BoolVarArray mh; // Representing if the move i was made in column c (move-here)
  IntVarArray gameWinner; // Representing winner 0 = player red wins, 1 = player black wins
  BoolVarArray line;  // Indicating the presence of line in each row, column or diagonal (numbered)
  BoolVarArray lineMove;  // Indicating the presence of line for a move
  BoolVarArray pos;  // Indicating the presence of empty slots
  BoolVarArray moveDone;  // Is true if the move k has been done

  int row;
  int col;
  int kOffset;

  const ConnectFourOptions& opt;

public:
  ConnectFourAllState(const ConnectFourOptions& _opt) : Script(_opt), QSpaceInfo(), opt(_opt)
  {
    std::cout << "Loading problem" << std::endl;
    if (!opt.printStrategy()) strategyMethod(0); // disable build and print strategy
    using namespace Int;
    // Define constants
    row = opt.row();
    col = opt.col();
    kOffset = 0;
    int nbDecisionVar = row*col;

    // Create array of heuristics, one for each brancher
    assert(nbDecisionVar <= 100);
    IntBranchVal heuristicArray[100];
    FOR<100>::go(heuristicArray);

    // Create board variables
    M = IntVarArray(*this,nbDecisionVar,0,col-1);
    if (opt.QCSPmodel() == 1) U = IntVarArray(*this,nbDecisionVar/2,0,col-1);
    board = IntVarArray(*this, nbDecisionVar*row*col, 0, 2);
    pos = BoolVarArray(*this, nbDecisionVar*row*col, 0, 1);
    h = IntVarArray(*this, nbDecisionVar*col, 0, row);
    mh = BoolVarArray(*this, nbDecisionVar*col, 0, 1);
    lineMove = BoolVarArray(*this, nbDecisionVar, 0, 1);
    gameWinner = IntVarArray(*this, nbDecisionVar, 0, 2);
    moveDone = BoolVarArray(*this, nbDecisionVar, 0, 1);

    // Test if a file was given in argument
    // We will update kOffset according to the file number of moves
    IntArgs rMoves;
    if (opt.file()) {
      std::ifstream f(opt.file());

      if (!f)
        throw Gecode::Exception("Connect four",
                                "Unable to open file");
      int move;
      while (f >> move) {
        rMoves << move;
        kOffset++;
      }

      f.close();
      assert((kOffset%2) == 0);
    }

    // Defining the player variables
    IntVarArgs m, uWm;
    for (int k=0; k<nbDecisionVar; k++)
    {
      // Post brancher
      if (k >= kOffset) {
        if ((k%2) == 1) setForAll(*this, M[k]);
        if (opt.QCSPmodel() == 1)
          branch(*this, M[k], INT_VAR_NONE(), INT_VALUES_MIN());
        else if (opt.heuristic())
          branch(*this, M[k], INT_VAR_NONE(), INT_VAL(heuristicArray[k]));
        else
          branch(*this, M[k], INT_VAR_NONE(), INT_VAL_MIN());
      }

      if (opt.QCSPmodel() == 1) {
        // Model from P. Nightingale without Pure Value and heuristic setup
        if ((k%2) == 0) m << M[k];
        else {
          if (k >= kOffset) {
            // With this simple model, we link some new existential variables to
            // the universal one if the move is legal.
            // As a result, we increase the number of branched variable and the search space
            branch(*this, U[k/2], INT_VAR_NONE(), INT_VALUES_MIN());
          }
          m << U[k/2];
          for (int i=0; i < col; i++)
            rel(*this, ((gameWinner[k-1] == Nil) && (h[(k-1)*col+i] < row) && (M[k] == i)) >> (U[k/2] == i), IPL_DOM); // Forbid illegal move
        }
      } else {
        // Model from P. Nightingale but we add cut and prune universal in order
        // to achieve same work as Pure Value. To compare with Queso, disable the
        // heurisitic has we do not have one here.
        m << M[k];
      }

      // We build the array of unwatched variables
      if (((k%2)==0) || (k<kOffset)) uWm << M[k];
      else uWm << getUnWatched(M[k]);

      // Some moves has been recorded, we play them here
      if (rMoves.size() > k) rel(*this, uWm[k] == rMoves[k], IPL_DOM);

      // Set the move-here variables
      if (k==0)
        for (int i=0; i < col; i++)
          rel(*this, (m[0] == i) == (mh[0*col+i] && moveDone[0]), IPL_DOM);
      else {
        for (int i=0; i < col; i++) {
          if (opt.QCSPmodel() <= 1) {
            // Not exactly as the article, we have drop the part with !lineMove[k-1].
            // We have to do this because it is not compatible with the constraints
            // which force the last board to be full
            // rel(*this, (!lineMove[k-1] && (h[(k-1)*col+i] < row) && (m[k] == i)) == mh[k*col+i], IPL_DOM);
            rel(*this, ((h[(k-1)*col+i] < row) && (m[k] == i)) == mh[k*col+i], IPL_DOM);
          } else { // opt.QCSPmodel() > 1
            // rel(*this, (!lineMove[k-1] && (h[(k-1)*col+i] < row) && (m[k] == i)) == (mh[k*col+i] && moveDone[k]), IPL_DOM);
            rel(*this, ((h[(k-1)*col+i] < row) && (m[k] == i)) == (mh[k*col+i] && moveDone[k]), IPL_DOM);
            // Prune for universal
            rel(*this, (h[(k-1)*col+i] == row) >> (uWm[k] != i), IPL_DOM); // Prune illegal move from universal

            // Add cut
            if ((k%2) == 1) cut(*this, (gameWinner[k-1] == Red) && moveDone[k-1]);
          }
        }
      }
    }

    // Fill the holes
    for (int k=0, offSet = 0; k<nbDecisionVar; k++, offSet += row*col)
      for (int i=0; i < col; i++)
        for (int j=0; j < row-1; j++) {
          BoolExpr be;
          be = expr(*this, board[offSet+i*row+j] != (((k%2)==0)?Black:Red));
          for (int jj=j+1; jj < row; jj++)
            be = expr(*this, be && (board[offSet+i*row+jj] == Nil));
          rel(*this, pos[offSet+i*row+j] == be, IPL_DOM);
        }

    for (int k=0, offSet = 0; k<nbDecisionVar; k++, offSet += row*col)
      if (k == 0) {
        for (int i=0; i < col; i++) {
            rel(*this, pos[offSet+i*row], IPL_DOM);
            rel(*this, pos[offSet+i*row+row-1], IPL_DOM);
            rel(*this, !mh[0*col+i] >> (board[offSet+i*row] == Nil), IPL_DOM);
            rel(*this,  mh[0*col+i] >> (board[offSet+i*row] == Red), IPL_DOM);
        }
      } else {
        for (int i=0; i < col; i++) {
          rel(*this, (h[(k-1)*col+i] == row) == !pos[offSet+i*row+row-1], IPL_DOM);
          for (int j=0; j < row; j++) {
            rel(*this, (h[(k-1)*col+i] == j) >> pos[offSet+i*row+j], IPL_DOM);
            rel(*this, (!mh[k*col+i] && (h[(k-1)*col+i] == j)) >> (board[offSet+i*row+j] == Nil), IPL_DOM);
            rel(*this, ( mh[k*col+i] && (h[(k-1)*col+i] == j)) >> (board[offSet+i*row+j] == (((k%2)==0)?Red:Black)), IPL_DOM);
          }
        }
      }

    // Map pieces from board at move i-1 to board at move k
    for (int k=1, offSet = row*col; k<nbDecisionVar; k++, offSet += row*col)
      for (int i=0; i < col; i++)
        for (int j=0; j < row; j++) {
          rel(*this, (board[(offSet-row*col)+i*row+j] == Black) >> (board[offSet+i*row+j] == Black), IPL_DOM);
          rel(*this, (board[(offSet-row*col)+i*row+j] == Red) >> (board[offSet+i*row+j] == Red), IPL_DOM);
        }

    // Link height and board state
    for (int k=0, offSet = 0; k<nbDecisionVar; k++, offSet += row*col)
      for (int i=0; i < col; i++)
        for (int j=0; j < row+1; j++)
          if (j==0)
            rel(*this, (board[offSet+i*row] == Nil) >> (h[k*col+i] == 0), IPL_DOM);
          else if (j==row)
            rel(*this, (board[offSet+i*row+j-1] != Nil) >> (h[k*col+i] == row), IPL_DOM);
          else
            rel(*this, ((board[offSet+i*row+j-1] != Nil) && (board[offSet+i*row+j] == Nil)) >> (h[k*col+i] == j), IPL_DOM);

    // Detect lines
    BoolVarArgs l;
    // Detect winning blocks
    for (int k=0, offSet = 0; k<nbDecisionVar; k++, offSet += row*col) {
      BoolVarArgs lk;
      for (int z=0; z<4; z++) { // Row(0) / Col(1) / Diag1(2) / Diag2(3)
        for (int i=0; i < col; i++) {
          for (int j=0; j < row; j++) {
            bool post = false;
            IntVarArgs x;
            if (((z%4)==0) && (i+3) < col) { // Line in row
              x << board[offSet+i*row+j] << board[offSet+(i+1)*row+j] << board[offSet+(i+2)*row+j] << board[offSet+(i+3)*row+j];
              post = true;
            }
            if (((z%4)==1) && (j+3) < row) {// Line in column
              x << board[offSet+i*row+j] << board[offSet+i*row+j+1] << board[offSet+i*row+j+2] << board[offSet+i*row+j+3];
              post = true;
            }
            if (((z%4)==2) && ((i+3) < col) && ((j+3) < row)) { // Line in diag1
              x << board[offSet+i*row+j] << board[offSet+(i+1)*row+j+1] << board[offSet+(i+2)*row+j+2] << board[offSet+(i+3)*row+j+3];
              post = true;
            }
            if (((z%4)==3) && ((i-3) >= 0) && ((j+3) < row)) { // Line in diag2
              x << board[offSet+i*row+j] << board[offSet+(i-1)*row+j+1] << board[offSet+(i-2)*row+j+2] << board[offSet+(i-3)*row+j+3];
              post = true;
            }
            if (post) {
              if ((k%2) == 0) {
                BoolVar bRed(*this,0,1);
                lk << bRed;
                l << bRed;
                if (k>0)
                  rel(*this,(lineMove[k-1] || (x[0] != Red) || (x[1] != Red) || (x[2] != Red) || (x[3] != Red)) == !bRed, IPL_DOM);
                else
                  rel(*this,((x[0] != Red) || (x[1] != Red) || (x[2] != Red) || (x[3] != Red)) == !bRed, IPL_DOM);
              } else {
                BoolVar bBlack(*this,0,1);
                lk << bBlack;
                l << bBlack;
                rel(*this,(lineMove[k-1] || (x[0] != Black) || (x[1] != Black) || (x[2] != Black) || (x[3] != Black)) == !bBlack, IPL_DOM);
              }
            }
          }
        }
      }
      if (k>0) lk << lineMove[k-1];
      rel(*this, BOT_OR, lk, lineMove[k], IPL_DOM);
    }
    line = BoolVarArray(*this, l);

    // Set GameState variables
    rel(*this, gameWinner[0] == Nil, IPL_DOM);
    for (int k=1; k < nbDecisionVar; k++) {
      rel(*this, (gameWinner[k-1] == Black) >> (gameWinner[k] == Black), IPL_DOM);
      rel(*this, (gameWinner[k-1] == Red) >> (gameWinner[k] == Red), IPL_DOM);
      rel(*this, ((gameWinner[k-1] == Nil) && !lineMove[k]) >> (gameWinner[k] == Nil), IPL_DOM);
      if ((k%2) == 0)
        rel(*this, ((gameWinner[k-1] == Nil) && lineMove[k]) >> (gameWinner[k] == Red), IPL_DOM);
      else
        rel(*this, ((gameWinner[k-1] == Nil) && lineMove[k]) >> (gameWinner[k] == Black), IPL_DOM);

      if (opt.QCSPmodel() == 3) {
        // If not winner before, only current player have a chance to win
        // the game at this move -- NOT IN INITIAL MODEL
        if ((k%2) == 0)
          rel(*this, (gameWinner[k-1] == Nil) >> (gameWinner[k] != Black), IPL_DOM);
        else
          rel(*this, (gameWinner[k-1] == Nil) >> (gameWinner[k] != Red), IPL_DOM);
      }
    }

    // For first move, symmetry is broken by removing the rightmost (upper): col - (col div 2)
    if (kOffset == 0) rel(*this, m[0], IRT_LE, col - (col / 2), IPL_DOM);

    // Force a winner at the end of the game
    rel(*this, gameWinner[nbDecisionVar-1], IRT_EQ, Red, IPL_DOM);

    if (opt.QCSPmodel() == 1) {
    // Set the last board full.
    // Useless if we prune universal, but needed for the simple model.
    // Notice that it is not compatible with the -depth argument as all board
    // doesn't have to be filled.
    for (int i=0; i < col; i++)
      for (int j=0; j < row; j++)
        rel(*this, board[(nbDecisionVar-1)*row*col+i*row+j] != Nil, IPL_DOM);
    }
  }

  ConnectFourAllState(bool share, ConnectFourAllState& p)
    : Script(share,p), QSpaceInfo(*this,share,p), row(p.row), col(p.col), kOffset(p.kOffset), opt(p.opt)
  {
    M.update(*this,share,p.M);
    if (opt.QCSPmodel() == 1) U.update(*this,share,p.U);
    board.update(*this,share,p.board);
    h.update(*this,share,p.h);
    mh.update(*this,share,p.mh);
    line.update(*this,share,p.line);
    lineMove.update(*this,share,p.lineMove);
    pos.update(*this,share,p.pos);
    gameWinner.update(*this,share,p.gameWinner);
    moveDone.update(*this,share,p.moveDone);
  }

  virtual Space* copy(bool share) { return new ConnectFourAllState(share,*this); }


  int c4Heuristic(IntVar x, int k) const {
    if (k == 0) return x.max();
    std::vector<int> boardBefore(row*col);
    int offSet = row*col*(k-1);
    for (int i=row-1; i>=0; i--)
      for (int j=0; j<col; j++) {
        assert(board[offSet+j*row+i].assigned());
        boardBefore[j*row+i] = board[offSet+j*row+i].val();
      }

    // now we have move number and
    // previous board state
    int bestScore=0;
    int bestMove=x.min();

    for (IntVarValues vv(x); vv(); ++vv) {
      int j = vv.val();
      assert(h[(k-1)*col+j].assigned());
      int i = h[(k-1)*col+j].val();

      if ((i = h[(k-1)*col+j].val()) < row) { // The column is not full
        boardBefore[j*row+i] = ((k%2)==0)?Red:Black;

        if (((k%2) == 0) && checklines<Red>(boardBefore)) return j; // Leftmost winning move
        else if (((k%2) == 1) && checklines<Black>(boardBefore)) return j; // Leftmost winning move

        int score;
        if ((k%2) == 0) { // Red player
          score = check3lines<Red>(boardBefore);
        } else { // Black player
          score = check3lines<Black>(boardBefore);
        }

        if (score > bestScore) {
          bestScore = score;
          bestMove = j;
        }
        boardBefore[j*row+i] = Nil;
      }
    }

    return bestMove;
  }

  template <int Player>
  int check3lines(std::vector<int>& board) const {
    int lines = 0;
    // Detect winning blocks
    for (int z=0; z<4; z++) { // Row(0) / Col(1) / Diag1(2) / Diag2(3)
      for (int i=0; i < col; i++) {
        for (int j=0; j < row; j++) {
          if (((z%4)==0) && (i+3) < col) { // Line in row
            if (check4for3<Player>(board[i*row+j], board[(i+1)*row+j], board[(i+2)*row+j], board[(i+3)*row+j]))
              lines++;
          }
          if (((z%4)==1) && (j+3) < row) {// Line in column
            if (check4for3<Player>(board[i*row+j], board[i*row+j+1], board[i*row+j+2], board[i*row+j+3]))
              lines++;
          }
          if (((z%4)==2) && ((i+3) < col) && ((j+3) < row)) { // Line in diag1
            if (check4for3<Player>(board[i*row+j], board[(i+1)*row+j+1], board[(i+2)*row+j+2], board[(i+3)*row+j+3]))
              lines++;
          }
          if (((z%4)==3) && ((i-3) >= 0) && ((j+3) < row)) { // Line in diag2
            if (check4for3<Player>(board[i*row+j], board[(i-1)*row+j+1], board[(i-2)*row+j+2], board[(i-3)*row+j+3]))
              lines++;
          }
        }
      }
    }
    return lines;
  }

  template <int Player>
  bool check4for3(int a, int b, int c, int d) const {
    if ((a == Player) && (b == Player) && (c == Player)) return (d == Nil);
    else if ((a == Player) && (b == Player) && (d == Player)) return (c == Nil);
    else if ((a == Player) && (c == Player) && (d == Player)) return (b == Nil);
    else if ((b == Player) && (c == Player) && (d == Player)) return (a == Nil);
    else return false;
  }

  template <int Player>
  bool checklines(std::vector<int>& board) const {
    // Detect winning blocks
    for (int z=0; z<4; z++) { // Row(0) / Col(1) / Diag1(2) / Diag2(3)
      for (int i=0; i < col; i++) {
        for (int j=0; j < row; j++) {
          if (((z%4)==0) && (i+3) < col) { // Line in row
            if ((board[i*row+j] == Player) && (board[(i+1)*row+j] == Player) && (board[(i+2)*row+j] == Player) && (board[(i+3)*row+j] == Player)) return true;
          }
          if (((z%4)==1) && (j+3) < row) {// Line in column
            if ((board[i*row+j] == Player) && (board[i*row+j+1] == Player) && (board[i*row+j+2] == Player) && (board[i*row+j+3] == Player)) return true;
          }
          if (((z%4)==2) && ((i+3) < col) && ((j+3) < row)) { // Line in diag1
            if ((board[i*row+j] == Player) && (board[(i+1)*row+j+1] == Player) && (board[(i+2)*row+j+2] == Player) && (board[(i+3)*row+j+3] == Player)) return true;
          }
          if (((z%4)==3) && ((i-3) >= 0) && ((j+3) < row)) { // Line in diag2
            if ((board[i*row+j] == Player) && (board[(i-1)*row+j+1] == Player) && (board[(i-2)*row+j+2] == Player) && (board[(i-3)*row+j+3] == Player)) return true;
          }
        }
      }
    }
    return false;
  }

  void print(std::ostream& os) const {
    strategyPrint(os);
  }
};

template <int N> int
c4Heuristic<N>::value(const Space& _home, IntVar x, int) {
  const ConnectFourAllState& home = static_cast<const ConnectFourAllState&>(_home);
  return home.c4Heuristic(x,N);
}

const int ConnectFourAllState::Red;
const int ConnectFourAllState::Black;
const int ConnectFourAllState::Nil;

int main(int argc, char* argv[])
{

  ConnectFourOptions opt("QCSP Connect-Four-Game");
  opt.parse(argc,argv);
  Script::run<ConnectFourAllState,QDFS,ConnectFourOptions>(opt);

  return 0;
}
