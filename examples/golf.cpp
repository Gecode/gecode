/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Mikael Lagerkivst, 2017
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
#include <gecode/set.hh>

using namespace Gecode;

/// \brief %Options for %Golf example
class GolfOptions : public Options {
protected:
  Driver::IntOption _w; //< Number of weeks
  Driver::IntOption _g; //< Number of groups
  Driver::IntOption _s; //< Number of players per group
public:
  /// Constructor
  GolfOptions(void)
    : Options("Golf"),
      _w("w","number of weeks",9),
      _g("g","number of groups",8),
      _s("s","number of players per group",4) {
    add(_w);
    add(_g);
    add(_s);
  }
  /// Return number of weeks
  int w(void) const { return _w.value(); }
  /// Return number of groups
  int g(void) const { return _g.value(); }
  /// Return number of players per group
  int s(void) const { return _s.value(); }
};

/**
 * \brief %Example: %Golf tournament
 *
 * Schedule a golf tournament. This is problem 010 from csplib.
 *
 * Note that "Modeling and Programming with Gecode" uses this example
 * as a case study.
 *
 * \ingroup Example
 *
 */
class Golf : public Script {
public:
  /// Model variants
  enum {
    MODEL_PLAIN,   ///< A simple model
    MODEL_SYMMETRY ///< Model with symmetry breaking
  };
  /// Propagation
  enum {
    PROP_SET,   ///< Propagation of pair play amount using set variables
    PROP_INT,   ///< Propagation of pair play amount using int variables and distinct
    PROP_MIXED, ///< Propagation of pair play amount using both set and int variables
  };
  int g; ///< Number of groups in a week
  int s; ///< Number of players in a group
  int w; ///< Number of weeks

  /// The sets representing the groups
  SetVarArray groups;

  /// Actual model
  Golf(const GolfOptions& opt)
    : Script(opt),
      g(opt.g()), s(opt.s()), w(opt.w()),
      groups(*this,g*w,IntSet::empty,0,g*s-1,
             static_cast<unsigned int>(s),static_cast<unsigned int>(s)) {
    Matrix<SetVarArray> schedule(groups,g,w);

    // Groups in one week must be disjoint
    SetVar allPlayers(*this, 0,g*s-1, 0,g*s-1);
    for (int i=0; i<w; i++)
      rel(*this, setdunion(schedule.row(i)) == allPlayers);
    
    // No two golfers play in the same group more than once
    if (opt.propagation() == PROP_SET || opt.propagation() == PROP_MIXED) {
      // Cardinality of intersection between two groups is at most one
      for (int i=0; i<groups.size()-1; i++)
        for (int j=i+1; j<groups.size(); j++)
          rel(*this, cardinality(groups[i] & groups[j]) <= 1);
    }
    if (opt.propagation() == PROP_INT || opt.propagation() == PROP_MIXED) {
      // Set up table mapping from pairs (p1,p2) (where p1<p2) of players to
      // unique integer identifiers
      int playerCount = g * s;
      TupleSet ts(3);
      int pairCount=0;
      for (int p1=0; p1<playerCount-1; p1++)
        for (int p2=p1+1; p2<playerCount; p2++)
          ts.add({p1, p2, pairCount++});
      ts.finalize();

      // Collect pairs of golfers into pairs
      IntVarArgs pairs;
      for (int i=0; i<groups.size()-1; i++) {
        // Channel sorted will ensure that for i<j, group[i]<group[j],
        // ensuring that the tuple set has a valid mapping.
        IntVarArgs group(*this, s, 0, playerCount-1);
        channelSorted(*this, group, groups[i]);
        // Collect all pairs in current group
        for (int p1=0; p1<group.size()-1; ++p1) {
          for (int p2=p1+1; p2<group.size(); ++p2) {
            IntVar pair(*this, 0, pairCount);

            IntVarArgs args;
            args << group[p1] << group[p2] << pair;
            extensional(*this, args, ts);

            pairs << pair;
          }
        }
      }

      // All pairs of golfers (using the unique identifiers) must be different 
      distinct(*this, pairs, opt.ipl());
    }    

    if (opt.model() == MODEL_SYMMETRY) {

      /*
       * Redundant constraints and static symmetry breaking from
       * "Solving Kirkman's Schoolgirl Problem in a Few Seconds"
       * Nicolas Barnier, Pascal Brisset, Constraints, 10, 7-21, 2005
       *
       */

      // Redundant constraint:
      // in each week, one player plays in only one group
      for (int j=0; j<w; j++) {
         for (int p=0; p < g*s; p++) {
           BoolVarArgs b(g);
           for (int i=0; i<g; i++)
             b[i] = expr(*this, (singleton(p) <= schedule(i,j)));
           linear(*this, b, IRT_EQ, 1);
         }
       }

      // Symmetry breaking: order groups
      for (int j=0; j<w; j++) {
        IntVarArgs m(g);
        for (int i=0; i<g; i++)
          m[i] = expr(*this, min(schedule(i,j)));
        rel(*this, m, IRT_LE);
      }

      // Symmetry breaking: order weeks
      // minElem(group(w,0)\{0}) < minElem(group(w+1,0)\{0})
      {
        IntVarArgs m(w);
        for (int i=0; i<w; i++)
          m[i] = expr(*this, min(schedule(0,i)-IntSet(0,0)));
        rel(*this, m, IRT_LE);
      }

      // Symmetry breaking: value symmetry of player numbers
      precede(*this, groups, IntArgs::create(groups.size(),0));
    }

    branch(*this, groups, SET_VAR_MIN_MIN(), SET_VAL_MIN_INC());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "Tournament plan" << std::endl;
    Matrix<SetVarArray> schedule(groups,g,w);
    for (int j=0; j<w; j++) {
      os << "Week " << j << ": " << std::endl << "    ";
      os << schedule.row(j) << std::endl;
    }
  }

  /// Constructor for copying \a s
  Golf(Golf& s) : Script(s), g(s.g), s(s.s), w(s.w) {
    groups.update(*this, s.groups);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Golf(*this);
  }
};

/** \brief Main-function
 *  \relates Golf
 */
int
main(int argc, char* argv[]) {
  GolfOptions opt;
  opt.model(Golf::MODEL_PLAIN);
  opt.model(Golf::MODEL_PLAIN, "none", "no symmetry breaking");
  opt.model(Golf::MODEL_SYMMETRY, "symmetry", "static symmetry breaking");
  opt.propagation(Golf::PROP_SET);
  opt.propagation(Golf::PROP_SET, "set", "Use set intersection cardinality for pair play constraints");
  opt.propagation(Golf::PROP_INT, "int", "Use integer distinct for pair play constraints");
  opt.propagation(Golf::PROP_MIXED, "mixed", "Use set interesection cardinality and integer distinct for pair play constraints");
  opt.ipl(IPL_DOM);
  opt.solutions(1);
  opt.parse(argc,argv);
  Script::run<Golf,DFS,GolfOptions>(opt);
  return 0;
}

// STATISTICS: example-any
