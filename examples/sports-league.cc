/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *     Christian Schulte, 2007
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

#include "examples/support.hh"
#include "gecode/minimodel.hh"

#include <algorithm>

/// Entry in round robin schedule
class Play {
public:
  int h; ///< home team
  int a; ///< away team
  int g; ///< game number
};

/// Round robin schedule
class RRS {
protected:
  /// Number of teams
  const int teams;
  /// Play information
  Play* plays; 
  /// Return number of weeks
  int weeks(void) const {
    return teams-1;
  }
  /// Return number of periods
  int periods(void) const {
    return teams/2;
  }
  /// Game number for game between home team \a h and away team \a a
  int gn(int h, int a) const {
    return teams*(h-1) + a;
  }
  /// Play for period \a p and week \a w
  Play& play(int p, int w) {
    return plays[p*weeks() + w];
  }
public:
  /**
   * \brief Build a feasible schedule
   *
   * The games of the first week are fixed as:
   * \f$ \langle 1,2 \rangle \cup
   * \{\langle p + 2, t - p + 1\rangle | p \geq 1\}\f$. \n
   * The remaining games are computed by transforming a game
   * \f$ \langle h, a, g \rangle \f$ from the previous week
   * in a new game \f$ \langle h', a'\rangle \f$, where: \n
   * \f$ h' = \left\{
   *  \begin{tabular}{l c l}
   *   1 & & if $h = 1$ \\
   *   2 & & if $h = t$ \\
   *   $h + 1$ & & otherwise
   *  \end{tabular}\right.
   * \f$ and
   * \f$ a' = \left\{
   *  \begin{tabular}{l c l}
   *   2 & & if $h = t$ \\
   *   a + 1 & & otherwise
   *  \end{tabular}\right.
   * \f$
   *
   *
   */
  RRS(int t) : teams(t), plays(new Play[periods()*weeks()])  {
    // Initialize array
    for (int p=0; p<periods(); p++)
      for (int w=0; w<weeks(); w++)
        play(p,w).h = play(p,w).a = play(p,w).g = 0;
    
    // Determine the first game (week 0 period 0)
    play(0,0).h = 1;
    play(0,0).a = 2;
    play(0,0).g = 2;

    // Determine the other games of the first week
    for (int p=1; p<periods(); p++) {
      play(p,0).h = (p + 1) + 1;
      play(p,0).a = teams - (p + 1 - 2);
      play(p,0).g = gn(play(p,0).h,play(p,0).a);
    }

    // Compute the games for the subsequent weeks
    for (int w=1; w<weeks(); w++) {
      for (int p=0; p<periods(); p++) {
        if (play(p,w-1).h == teams) {
          play(p,w).h = 2;
        } else if (play(p,w-1).h == 1) {
          play(p,w).h = 1;
        } else {
          play(p,w).h = play(p,w-1).h + 1;
        }

        if (play(p,w-1).a == teams) {
          play(p,w).a = 2;
        } else {
          play(p,w).a = play(p,w-1).a + 1;
        }

        // maintain symmetry for (h,a): h < a
        if (play(p,w).h > play(p,w).a)
          std::swap(play(p,w).h,play(p,w).a);

        play(p,w).g = gn(play(p,w).h,play(p,w).a);
      }
    }

  }
  /// Home, away, and game information
  void hag(int w, IntArgs& h, IntArgs& a, IntArgs& g) {
    for (int p=0; p<periods(); p++) {
      h[p] = play(p,w).h;
      a[p] = play(p,w).a;
      g[p] = play(p,w).g;
    }
  }
  /// Delete schedule
  ~RRS(void) {
    delete [] plays;
  }
};



/**
 * \brief %Example: %Sports League Scheduling
 *
 * -# There are \f$ t \f$ teams (\f$ t \f$  even).
 * -# The season lasts \f$ t - 1 \f$ weeks.
 * -# Each game between two different teams occurs exactly once.
 * -# Every team plays one game in each week of the season.
 * -# There are \f$ \displaystyle\frac{t}{2} \f$ periods and each week
 *    every period is scheduled for one game.
 * -# No team plays more than twice in the same period over
 *    the course of the season.
 *
 * See also problem 26 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 */
class SportsLeague : public Example {
protected:
  const int teams;  ///< number of teams
  IntVarArray home; ///< home teams
  IntVarArray away; ///< away teams
  IntVarArray game; ///< game numbers

  /// Return number of weeks
  int weeks(void) const {
    return teams-1;
  }
  /// Return number of periods
  int periods(void) const {
    return teams/2;
  }
  /// Home team in period \a p and week \a w
  IntVar& h(int p, int w) {
    return home[p*teams + w];
  }
  /// Away team in period \a p and week \a w
  IntVar& a(int p, int w) {
    return away[p*teams + w];
  }
  /// Return game number for game in period \a p and week \a w
  IntVar& g(int p, int w) {
    return game[p*weeks() + w];
  }

public:
  /// Setup model
  SportsLeague(const SizeOptions& opt) :
    teams(opt.size()),
    home(this, periods() * teams, 1, weeks()),
    away(this, periods() * teams, 2, weeks()+1),
    game(this, weeks()*periods(), 2, teams*weeks())
  {
    // Initialize round robin schedule
    RRS r(teams);

    // Domain for gamenumber of period
    for (int w=0; w<weeks(); w++) {
      IntArgs rh(periods()), ra(periods()), rg(periods());
      IntVarArgs n(periods());

      for (int p=0; p<periods(); p++)
        n[p].init(this,0,periods()-1);
      distinct(this, n, opt.icl());

      r.hag(w,rh,ra,rg);
        
      for (int p=0; p<periods(); p++) {
        element(this, rh, n[p], h(p,w), 0);
        element(this, ra, n[p], a(p,w), 0);
        element(this, rg, n[p], g(p,w), 0);
      }
    }

    /// (h,a) and (a,h) are the same game, focus on home (that is, h<a)
    for (int p=0; p<periods(); p++)
      for (int w=0; w<teams; w++)
        rel(this, h(p,w), IRT_LE, a(p,w));

    // Home teams in first week are ordered
    for (int p=0; p<periods()-1; p++)
      rel(this, h(p,0), IRT_LE, h(p+1,0));

    // Fix first pair
    rel(this, h(0,0), IRT_EQ, 1);
    rel(this, a(0,0), IRT_EQ, 2);

    /// Column constraint: each team occurs exactly once
    for (int w=0; w<teams; w++) {
      IntVarArgs c(teams);
      for (int p=0; p<periods(); p++) {
        c[2*p] = h(p,w); c[2*p+1] = a(p,w);
      }
      distinct(this, c, opt.icl());
    }

    /// Row constraint: no team appears more than twice
    for (int p=0; p<periods(); p++) {
      IntVarArgs r(2*teams);
      for (int t=0; t<teams; t++) {
        r[2*t]   = h(p,t);
        r[2*t+1] = a(p,t);
      }
      gcc(this, r, 2, opt.icl());
    }

    // Redundant constraint
    for (int p=0; p<periods(); p++)
      for (int w=0; w<weeks(); w ++)
        post(this, teams * h(p,w) + a(p,w) - g(p,w) == teams);

    distinct(this, game, opt.icl());

    branch(this, game, BVAR_NONE, BVAL_MIN);
  }
  /// Constructor for cloning \a s
  SportsLeague(bool share, SportsLeague& s)
    : Example(share, s), teams(s.teams) {
    home.update(this, share, s.home);
    away.update(this, share, s.away);
    game.update(this, share, s.game);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new SportsLeague(share, *this);
  }
  /// Print solution
  virtual void print(void) {
    using namespace std;
    // print period index
    cout << "\t       ";
    for (int p=0; p<periods(); p++) {
      cout << "P[";
      cout.width(2);
      cout << p << "] ";
      }
    cout << endl;
    // print entries
    for (int w=0; w<weeks(); w++) {
      cout << "\tW[";
      cout.width(2);
      cout << w+1 << "]: ";
      for (int p=0; p<periods(); p++) {
        cout.width(2); 
        cout << h(p,w).val() << '-';
        cout.width(2);
        cout << a(p,w).val() << " ";
      }
      cout << endl;
    }
  }
};


/** \brief Main-function
 *  \relates SportsLeague
 */
int 
main(int argc, char* argv[]) {
  SizeOptions opt("Sports League Scheduling");
  opt.icl(ICL_DOM);
  opt.size(18);
  opt.parse(argc,argv);
  if (opt.size() < 5) {
    std::cerr<< "No Solution for less than 5 teams!" << std::endl;
    return 1;
  }
  if (opt.size() % 2 != 0) {
    std::cerr << "Number of teams has to be even!" << std::endl;
    return 1;
  }
  Example::run<SportsLeague, DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
