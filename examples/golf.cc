/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

#include "gecode/set.hh"
#include "examples/support.hh"

/**
 * \name Parameters for golf tournaments
 *
 * \relates Golf
 */
//@{
/// Tournament parameters
struct Tournament {
  /// Number of groups
  int groups;
  /// Number of players in each group
  int playersInGroup;
  /// Number of weeks
  int weeks;
};
/// Tournaments
static const Tournament t[]=
  { {8,4,9},
    {5,3,7},
    {4,3,2}
  };
/// Number of tournaments
static const unsigned int n_examples = sizeof(t) / sizeof(Tournament);
//@}

/**
 * \brief %Example: Golf tournament
 *
 * Schedule a golf tournament. This is problem 010 from csplib.
 *
 * \ingroup ExProblem
 *
 */
class Golf : public Example {
public:
  /// Model variants
  enum {
    MODEL_PLAIN,   ///< A simple model
    MODEL_SYMMETRY ///< Model with symmetry breaking
  };
  enum {
    PROP_PLAIN,    ///< Propagation using intersection propagators
    PROP_DECOMPOSE ///< Propagation using union and complement propagators
  };
  int groups;          ///< Number of groups in a week
  int playersInGroup;  ///< Number of players in a group
  int weeks;           ///< Number of weeks
  int players;         ///< Overall number of players

  /// The sets representing the groups
  SetVarArray groupsS;

  /// Return group number \a g in week \a w
  SetVar& group(int w, int g) {
    return groupsS[w*groups+g];
  }

  /// Actual model
  Golf(const SizeOptions& opt) :
    groups(t[opt.size()].groups),
    playersInGroup(t[opt.size()].playersInGroup),
    weeks(t[opt.size()].weeks),
    players(groups*playersInGroup),
    groupsS(this,groups*weeks,IntSet::empty,0,players-1,
            playersInGroup,playersInGroup) {

    SetVar allPlayers(this, 0, players-1, 0, players-1);

    // Groups in one week must be disjoint
    for (int w=0; w<weeks; w++) {
      SetVarArgs p(groups);
      for (int g=0; g < groups; g++)
               p[g] = group(w,g);

      rel(this,SOT_DUNION,p,allPlayers);
    }

    // No two golfers play in the same group more than once
    for (int w=0; w<weeks; w++) {
      for (int g=0; g<groups; g++) {
        SetVar v = group(w,g);
        SetVar vcompl;
        if (opt.propagation() == PROP_DECOMPOSE) {
          vcompl = SetVar(this,IntSet::empty,
                          Set::Limits::min,Set::Limits::max);
          rel(this, v, SRT_CMPL, vcompl);
        }
        for (int i=(w+1)*groups; i<weeks*groups; i++) {
          if (opt.propagation() == PROP_PLAIN) {
            SetVar atMostOne(this,IntSet::empty,0,players-1,0,1);
            rel(this, v, SOT_INTER, groupsS[i], SRT_EQ, atMostOne);
          } else {
            SetVar atMostOneC(this,IntSet::empty,
                              Set::Limits::min,
                              Set::Limits::max,
                              Set::Limits::card-1,
                              Set::Limits::card);
            SetVar groupsSiC(this,IntSet::empty,
                             Set::Limits::min,Set::Limits::max);
            rel(this, groupsS[i], SRT_CMPL, groupsSiC);
            rel(this, vcompl, SOT_UNION, groupsSiC, SRT_EQ, atMostOneC);
          }
        }
      }
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
      for (int w=0; w < weeks; w++) {
         for (int p=0; p < players; p++) {
           BoolVarArgs bs(groups);
           for (int g=0; g<groups; g++) {
            BoolVar b(this,0,1);
            dom(this, group(w,g), SRT_SUP, p, b);
            bs[g] = b;
          }
           linear(this, bs, IRT_EQ, 1);
         }
       }

      // Redundant constraint:
      // any two groups has at most one player in common
      atmostOne(this, groupsS, playersInGroup);

      // Symmetry breaking: order groups
      for (int w=0; w<weeks; w++) {
        for (int g=0; g<groups-1; g++) {
          IntVar minG1(this, 0, players-1);
          IntVar minG2(this, 0, players-1);
          SetVar g1 = group(w,g);
          SetVar g2 = group(w,g+1);
          min(this, g1, minG1);
          min(this, g2, minG2);
          rel(this, minG1, IRT_LE, minG2);
        }
      }

      // Symmetry breaking: order weeks
      // minElem(group(w,0)\{0}) < minElem(group(w+1,0)\{0})
      for (int w=0; w<weeks-1; w++) {
        SetVar g1(this, IntSet::empty, 1, players-1);
        SetVar g2(this, IntSet::empty, 1, players-1);
        rel(this, g1, SOT_DUNION, IntSet(0,0), SRT_EQ, group(w,0));
        rel(this, g2, SOT_DUNION, IntSet(0,0), SRT_EQ, group(w+1,0));
        IntVar minG1(this, 0, players-1);
        IntVar minG2(this, 0, players-1);
        min(this, g1, minG1);
        min(this, g2, minG2);
        rel(this, minG1, IRT_LE, minG2);
      }

      // Initialize the dual variables:
      // groupsSInv[w*players+p] is player p's group in week w
      IntVarArray groupsSInv(this, weeks*players, 0, groups-1);
      for (int w=0; w<weeks; w++) {
        for (int p=0; p<players; p++) {
          SetVar thisPlayer(this, p,p, 0, players-1);
          SetVarArgs thisWeek(groups);
          for (int g=0; g<groups; g++)
            thisWeek[g] = group(w,g);
          selectSet(this, thisWeek, groupsSInv[w*players+p], thisPlayer);
        }
      }

      // Symmetry breaking: order players
      // For all p<groups : groupsSInv[w*players+p] <= p
      for (int w=0; w<weeks; w++) {
        for (int p=0; p<groups; p++) {
          rel(this, groupsSInv[w*players+p], IRT_LQ, p);
        }
      }

    }

    branch(this, groupsS, SET_VAR_MIN_UNKNOWN_ELEM, SET_VAL_MIN);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "Tournament plan" << std::endl;

    for (int w=0; w<weeks; w++) {
      os << "Week " << w << ": " << std::endl << "    ";
      for (int g=0; g<groups; g++) {
        if (group(w,g).assigned()) {          
          bool first = true;
          os << "(";
          for (SetVarGlbValues glb(group(w,g)); glb(); ++glb) {
            if (first) first = false; else os << " ";
            os << glb.val();
          }
          os << ")";          
        } else {
          os << "(" << group(w,g) << ")";
        }
        if (g < groups-1) os << " ";
        if (g > 0 && g % 4 == 0) os << std::endl << "    ";
      }
      os << std::endl;
    }
  }

  /// Constructor for copying \a s
  Golf(bool share, Golf& s) : Example(share,s),
      groups(s.groups), playersInGroup(s.playersInGroup),
      weeks(s.weeks), players(s.players) {
    groupsS.update(this, share, s.groupsS);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Golf(share,*this);
  }

  /// Make variables available for visualisation
  virtual void
  getVars(Gecode::Reflection::VarMap& vm, bool registerOnly) {
    vm.putArray(this, groupsS, "groupsS", registerOnly);
  }
};

/** \brief Main-function
 *  \relates Golf
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Golf");
  opt.model(Golf::MODEL_PLAIN);
  opt.model(Golf::MODEL_PLAIN, "none", "no symmetry breaking");
  opt.model(Golf::MODEL_SYMMETRY, "symmetry", "static symmetry breaking");
  opt.propagation(Golf::PROP_PLAIN);
  opt.propagation(Golf::PROP_PLAIN, "plain", "intersection constraints");
  opt.propagation(Golf::PROP_DECOMPOSE, "decompose",
                  "union and complement constraints");
  opt.solutions(1);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and " << n_examples - 1
              << std::endl;
    return 1;
  }
  Example::run<Golf,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

