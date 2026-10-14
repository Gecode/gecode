/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace Gecode;
using Board = std::array<int,16>;

/// Options for the initial scramble and bounded solution path.
class PuzzleOptions : public Options {
  Driver::UnsignedIntOption _scramble;
  Driver::UnsignedIntOption _limit;
public:
  enum { MODEL_OPEN, MODEL_FIXED };
  PuzzleOptions(void)
    : Options("FifteenPuzzle"),
      _scramble("scramble","number of random slides from the goal",12),
      _limit("limit","maximum number of solution moves",12) {
    add(_scramble);
    add(_limit);
    model(MODEL_OPEN);
    model(MODEL_OPEN,"open","construct states as the path grows");
    model(MODEL_FIXED,"fixed","construct all bounded states initially");
    solutions(0);
    seed(1);
  }
  unsigned int
  scramble(void) const {
    return _scramble.value();
  }
  int
  limit(void) const {
    Int::Limits::check(16*(static_cast<long long int>(_limit.value())+1),
                       "FifteenPuzzle::limit");
    return static_cast<int>(_limit.value());
  }
};

static Board
goal(void) {
  return {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
}

static std::vector<int>
neighbors(int blank) {
  std::vector<int> result;
  if (blank >= 4)
    result.push_back(blank-4);
  if (blank % 4 != 0)
    result.push_back(blank-1);
  if (blank % 4 != 3)
    result.push_back(blank+1);
  if (blank < 12)
    result.push_back(blank+4);
  return result;
}

static Board
initial_board(const PuzzleOptions& opt) {
  Board board = goal();
  Rnd random(opt.seed());
  int blank = 15;
  int previous = -1;
  for (unsigned int i=0; i<opt.scramble(); i++) {
    std::vector<int> choices = neighbors(blank);
    choices.erase(std::remove(choices.begin(),choices.end(),previous),
                  choices.end());
    int next = choices[random(static_cast<unsigned int>(choices.size()))];
    std::swap(board[blank],board[next]);
    previous = blank;
    blank = next;
  }
  return board;
}

static int
manhattan(const Board& board) {
  int distance = 0;
  for (int i=0; i<16; i++) {
    if (board[i] != 0) {
      const int target = board[i]-1;
      distance += std::abs(i/4-target/4) + std::abs(i%4-target%4);
    }
  }
  return distance;
}

static Board
values(const IntVarArgs& tiles) {
  Board result;
  for (int i=0; i<16; i++)
    result[i] = tiles[i].val();
  return result;
}

static void
print_board(std::ostream& os, const Board& board) {
  for (int i=0; i<16; i++) {
    os << std::setw(3) << board[i];
    if (i % 4 == 3)
      os << '\n';
  }
}

static void
post_start(Home home, const IntVarArgs& tiles, IntVar blank,
           const Board& initial) {
  for (int i=0; i<16; i++)
    rel(home,tiles[i] == initial[i]);
  const int zero = static_cast<int>(
    std::find(initial.begin(),initial.end(),0)-initial.begin());
  rel(home,blank == zero);
}

// Both representations use these rules between successive states.
static void
post_slide(Home home,
           const IntVarArgs& before, IntVar old_blank,
           const IntVarArgs& after, IntVar new_blank) {
  static const TupleSet adjacent = [] {
    TupleSet table(2);
    for (int i=0; i<16; i++) {
      for (int j : neighbors(i))
        table.add(IntArgs({i,j}));
    }
    table.finalize();
    return table;
  }();
  extensional(home,IntVarArgs({old_blank,new_blank}),adjacent);
  rel(home,element(after,old_blank) == element(before,new_blank));
  rel(home,element(after,new_blank) == 0);
  for (int i=0; i<16; i++) {
    rel(home,((old_blank != i) && (new_blank != i)) >>
             (before[i] == after[i]));
  }
}

static void
post_goal(Home home, const IntVarArgs& tiles, IntVar length,
          int states) {
  const Board target = goal();
  BoolExpr solved(tiles[0] == target[0]);
  for (int i=1; i<16; i++)
    solved = solved && (tiles[i] == target[i]);
  rel(home,(length >= states) >> ((length == states) == solved));
}

/**
 * \brief %Example: solve the 15-puzzle with an open list of states
 *
 * Start from a solvable board obtained by randomly sliding from the goal.
 * Each path position is the blank position in one board; the tile list holds
 * sixteen variables per board. The transition on the path constructs and
 * constrains the next board when that position is materialized.
 *
 * The goal closes the path. Length counts states including the start, so
 * branch and bound minimizes moves by minimizing length minus one.
 * The independent move limit bounds search and need not equal the scramble.
 * Defaults use twelve slides, seed one, and a twelve-move limit. Select
 * -model fixed to compare with an array representation; -scramble, -seed,
 * and -limit control the instance and search bound independently.
 *
 * \ingroup Example
 */
class OpenPuzzle : public Script {
  OpenIntVarList tiles;
  OpenIntVarList path;
public:
  OpenPuzzle(const PuzzleOptions& opt)
    : Script(opt),
      tiles(*this,IntSet(0,15),16*(opt.limit()+1)),
      path(*this,IntSet(0,15),
        [](Space& home, OpenIntVarList x, int i) {
          static_cast<OpenPuzzle&>(home).transition(x,i);
        },opt.limit()+1) {
    const Board initial = initial_board(opt);
    rel(*this,tiles.length() == 16*path.length());
    tiles.materialize(*this,16);
    IntVar start = path.get(*this,0);
    post_start(*this,state(0),start,initial);
    post_goal(*this,state(0),path.length(),1);
    rel(*this,path.length() >= manhattan(initial)+1);
    branch(*this,path,INT_VAR_NONE(),INT_VAL_MIN());
  }

  void
  transition(OpenIntVarList x, int i) {
    // The constructor posts the starting board unconditionally.
    if (i == 0)
      return;
    tiles.materialize(*this,16*(i+1));
    if (failed())
      return;
    post_slide(*this,state(i-1),x[i-1],state(i),x[i]);
    post_goal(*this,state(i),x.length(),i+1);
  }

  OpenPuzzle(OpenPuzzle& s) : Script(s) {
    tiles.update(*this,s.tiles);
    path.update(*this,s.path);
  }
  Space*
  copy(void) override {
    return new OpenPuzzle(*this);
  }
  IntVarArgs
  state(int i) const {
    IntVarArgs result(16);
    for (int j=0; j<16; j++)
      result[j] = tiles[16*i+j];
    return result;
  }
  void
  constrain(const Space& solution) override {
    const OpenPuzzle& best = static_cast<const OpenPuzzle&>(solution);
    rel(*this,path.length() < best.path.length().val());
  }
  void
  print(std::ostream& os) const override {
    os << "Moves: " << path.length().val()-1 << "\nBlank positions:";
    for (int i=0; i<path.known_length(); i++)
      os << ' ' << path[i].val();
    os << '\n';
  }
};

/**
 * \brief Bounded representation using the same slide and goal rules
 *
 * All boards are constructed initially, including the unused suffix after
 * the goal. The branch filter considers only the next blank position and
 * stops when the assigned prefix reaches the goal.
 *
 * \ingroup Example
 */
class FixedPuzzle : public Script {
  IntVarArray tiles;
  IntVarArray path;
  IntVar length;
public:
  FixedPuzzle(const PuzzleOptions& opt)
    : Script(opt),
      tiles(*this,16*(opt.limit()+1),0,15),
      path(*this,opt.limit()+1,0,15),
      length(*this,1,opt.limit()+1) {
    const Board initial = initial_board(opt);
    post_start(*this,state(0),path[0],initial);
    post_goal(*this,state(0),length,1);
    for (int i=1; i<=opt.limit(); i++) {
      post_slide(*this,state(i-1),path[i-1],state(i),path[i]);
      post_goal(*this,state(i),length,i+1);
    }
    rel(*this,length >= manhattan(initial)+1);
    branch(*this,path,INT_VAR_NONE(),INT_VAL_MIN(),
      [](const Space& home, IntVar, int i) {
        const FixedPuzzle& s = static_cast<const FixedPuzzle&>(home);
        const int g = s.depth();
        return values(s.state(g)) != goal() && i == g+1;
      });
  }

  FixedPuzzle(FixedPuzzle& s) : Script(s) {
    tiles.update(*this,s.tiles);
    path.update(*this,s.path);
    length.update(*this,s.length);
  }
  Space*
  copy(void) override {
    return new FixedPuzzle(*this);
  }
  IntVarArgs
  state(int i) const {
    IntVarArgs result(16);
    for (int j=0; j<16; j++)
      result[j] = tiles[16*i+j];
    return result;
  }
  int
  depth(void) const {
    int i = 0;
    while (i+1 < path.size() && path[i+1].assigned())
      i++;
    return i;
  }
  void
  constrain(const Space& solution) override {
    const FixedPuzzle& best = static_cast<const FixedPuzzle&>(solution);
    rel(*this,length < best.length.val());
  }
  void
  print(std::ostream& os) const override {
    os << "Moves: " << length.val()-1 << "\nBlank positions:";
    for (int i=0; i<length.val(); i++)
      os << ' ' << path[i].val();
    os << '\n';
  }
};

/** \brief Main-function
 *  \relates OpenPuzzle
 */
int
main(int argc, char* argv[]) {
  PuzzleOptions opt;
  opt.parse(argc,argv);
  if (opt.mode() == SM_SOLUTION) {
    std::cout << "Initial board (seed " << opt.seed() << "):\n";
    print_board(std::cout,initial_board(opt));
  }
  if (opt.model() == PuzzleOptions::MODEL_OPEN)
    Script::run<OpenPuzzle,BAB,PuzzleOptions>(opt);
  else
    Script::run<FixedPuzzle,BAB,PuzzleOptions>(opt);
  return 0;
}

// STATISTICS: example-any
