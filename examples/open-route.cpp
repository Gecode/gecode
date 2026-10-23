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

using namespace Gecode;

/**
 * \brief %Example: simple routes of unknown length
 *
 * Find all simple routes from node 0 to node 4 in this directed graph:
 *   0 -> 1, 2, 4
 *   1 -> 2, 4
 *   2 -> 1, 3
 *   3 -> 2, 4
 *
 * The graph contains cycles, but distinct node variables exclude revisits.
 * A simple route uses at most the five nodes, so this finite bound preserves
 * every solution while ensuring that search terminates.
 *
 * \ingroup Example
 */
class OpenRoute : public Script {
protected:
  /// Number of graph nodes and maximal route length
  static const int node_count = 5;
  /// Node IDs in route order, including source and destination
  OpenIntVarList route;
public:
  /// Branching to use for the route
  enum {
    BRANCH_INCREMENTAL, ///< Assign the current prefix before extending it
    BRANCH_LENGTH      ///< Choose the complete route length first
  };

  /// Construct the model
  OpenRoute(const Options& opt)
    : Script(opt), route(*this,IntSet(0,node_count-1),node_count) {
    // State 5 consumes the source node. Each remaining transition consumes
    // the destination of a graph arc; only node 4 is an accepting state.
    DFA paths(5,{
      {5,0,0},
      {0,1,1}, {0,2,2}, {0,4,4},
      {1,2,2}, {1,4,4},
      {2,1,1}, {2,3,3},
      {3,2,2}, {3,4,4}
    },{4});
    extensional(*this,route,paths);
    distinct(*this,route);

    // Ordinary brancher posting order gives length-first search when wanted.
    if (opt.branching() == BRANCH_LENGTH)
      branch(*this,route.length(),INT_VAL_MIN());
    branch(*this,route,INT_VAR_NONE(),INT_VAL_MIN());
  }

  /// Constructor for cloning \a s, including the space-local open list
  OpenRoute(OpenRoute& s) : Script(s) {
    route.update(*this,s.route);
  }

  /// Perform copying during cloning
  Space*
  copy(void) override {
    return new OpenRoute(*this);
  }

  /// Print the materialized route (complete in a solution)
  void
  print(std::ostream& os) const override {
    os << "\t";
    for (int i=0; i<route.known_length(); i++) {
      if (i > 0)
        os << " -> ";
      os << route[i];
    }
    os << " (length " << route.length() << ")" << std::endl;
  }
};

/** \brief Main-function
 *  \relates OpenRoute
 */
int
main(int argc, char* argv[]) {
  Options opt("OpenRoute");
  opt.solutions(0);
  opt.branching(OpenRoute::BRANCH_INCREMENTAL);
  opt.branching(OpenRoute::BRANCH_INCREMENTAL,"incremental",
                "Assign the current prefix before extending the route");
  opt.branching(OpenRoute::BRANCH_LENGTH,"length",
                "Choose the complete route length first");
  opt.parse(argc,argv);
  Script::run<OpenRoute,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
