/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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
  int groups;
  int playersInGroup;
  int weeks;
  int players;

  SetVarArray groupsS;
  IntVarArray groupsSInv;

  SetVar& group(int w, int g) {
    return groupsS[w*groups+g];
  }
  IntVar& groupInv(int w, int p) {
    return groupsSInv[w*players+p];
  }

  Golf(const Options& o) :
    groups(t[o.size].groups),
    playersInGroup(t[o.size].playersInGroup),
    weeks(t[o.size].weeks),
    players(groups*playersInGroup),
    groupsS(this,groups*weeks,IntSet::empty,0,players-1,
            playersInGroup,playersInGroup),
    groupsSInv(this, weeks*players, 0, groups-1) {

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
              for (int i=(w+1)*groups; i<weeks*groups; i++) {
                SetVar atMostOne(this,IntSet::empty,0,players-1,0,1);
          rel(this, v, SOT_INTER, groupsS[i], SRT_EQ, atMostOne);
              }
      }
    }

    if (!o.naive) {

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
      // groupInv(w,p) is player p's group in week w
      for (int w=0; w<weeks; w++) {
        for (int p=0; p<players; p++) {
          SetVar thisPlayer(this, p,p, 0, players-1);
          SetVarArgs thisWeek(groups);
          for (int g=0; g<groups; g++)
            thisWeek[g] = group(w,g);
          selectSet(this, thisWeek, groupInv(w,p), thisPlayer);
        }
      }

      // Symmetry breaking: order players
      // For all p<groups : groupInv(w,p) <= p
      for (int w=0; w<weeks; w++) {
        for (int p=0; p<groups; p++) {
          rel(this, groupInv(w,p), IRT_LQ, p);
        }
      }

    }

    branch(this, groupsS, SETBVAR_MIN_UNKNOWN_ELEM, SETBVAL_MIN);
  }

  Golf(bool share, Golf& s) : Example(share,s),
      groups(s.groups), playersInGroup(s.playersInGroup),
      weeks(s.weeks), players(s.players) {
    groupsS.update(this, share, s.groupsS);
  }

  virtual Space*
  copy(bool share) {
    return new Golf(share,*this);
  }

  virtual void
  print(void) {

    std::cout << "Tournament plan" << std::endl;

    for (int w=0; w<weeks; w++) {
      std::cout << "Week " << w << ": " << std::endl << "    ";
      for (int g=0; g<groups; g++) {
        SetVarGlbValues glb(group(w,g));
        std::cout << "(" << glb.val();
        ++glb;
        while(glb()) {
          std::cout << " " << glb.val();
          ++glb;
        }
        std::cout << ")";
        if (g < groups-1) std::cout << " ";
        if (g > 0 && g % 4 == 0) std::cout << std::endl << "    ";
      }
      std::cout << std::endl;
    }

  }
};

int
main(int argc, char** argv) {
  Options o("Golf");
  o.parse(argc,argv);
  o.solutions(1);
  if (o.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and " << n_examples - 1
              << std::endl;
    return 1;
  }
  Example::run<Golf,DFS>(o);
  return 0;
}

// STATISTICS: example-any

