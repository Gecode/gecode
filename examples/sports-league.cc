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

public:
  /// Round robin schedule for period \a p and week \a w
  Play& rrs(int p, int w) {
    return plays[p*weeks() + w];
  }
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
        rrs(p,w).h = rrs(p,w).a = rrs(p,w).g = 0;
    
    // Determine the first game (week 0 period 0)
    rrs(0,0).h = 1;
    rrs(0,0).a = 2;
    rrs(0,0).g = 2;

    // Determine the other games of the first week
    for (int p=1; p<periods(); p++) {
      rrs(p,0).h = (p + 1) + 1;
      rrs(p,0).a = teams - (p + 1 - 2);
      rrs(p,0).g = gn(rrs(p,0).h,rrs(p,0).a);
    }

    // Compute the games for the subsequent weeks
    for (int w=1; w<weeks(); w++) {
      for (int p=0; p<periods(); p++) {
        if (rrs(p,w-1).h == teams) {
          rrs(p,w).h = 2;
        } else if (rrs(p,w-1).h == 1) {
          rrs(p,w).h = 1;
        } else {
          rrs(p,w).h = rrs(p,w-1).h + 1;
        }

        if (rrs(p,w-1).a == teams) {
          rrs(p,w).a = 2;
        } else {
          rrs(p,w).a = rrs(p,w-1).a + 1;
        }

        // maintain symmetry for (h,a): h < a
        if (rrs(p,w).h > rrs(p,w).a)
          std::swap(rrs(p,w).h,rrs(p,w).a);

        rrs(p,w).g = gn(rrs(p,w).h,rrs(p,w).a);
      }
    }

  }
  /// Home information
  IntArgs home(int w) {
    IntArgs h(periods());
    for (int p=0; p<periods(); p++)
      h[p] = rrs(p,w).h;
    return h;
  }
  /*
    // Domain for gamenumber of period
    for (int w=0; w<weeks(); w++) {
      IntArgs gamenum(periods());
      IntArgs fst(periods());
      IntArgs snd(periods());
      IntVarArgs n(periods());

      for (int p=0; p<periods(); p++) {
        n[p].init(this,0,periods()-1);
        gamenum[p] = r.rrs(p,w).g;
        fst[p]     = r.rrs(p,w).h;
        snd[p]     = r.rrs(p,w).a;
      }

      distinct(this, n, opt.icl());
        
      for (int p=0; p<periods(); p++) {
        element(this, gamenum, n[p], g(p,w), 0);
        element(this, fst,     n[p], h(p,w), 0);
        element(this, snd,     n[p], a(p,w), 0);
      }
    }
  */
  /// Delete schedule
  ~RRS(void) {
    delete [] plays;
  }
};



/**
 * \brief %Example: %Sports League Scheduling
 *
 * Prob026: round robin tournaments from http://www.csplip.org
 *
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
      IntArgs gamenum(periods());
      IntArgs fst(periods());
      IntArgs snd(periods());
      IntVarArgs n(periods());

      for (int p=0; p<periods(); p++) {
        n[p].init(this,0,periods()-1);
        gamenum[p] = r.rrs(p,w).g;
        fst[p]     = r.rrs(p,w).h;
        snd[p]     = r.rrs(p,w).a;
      }

      distinct(this, n, opt.icl());
        
      for (int p=0; p<periods(); p++) {
        element(this, gamenum, n[p], g(p,w), 0);
        //        element(this, r.home(p), n[p], h(p,w), 0);
        element(this, fst, n[p], h(p,w), 0);
        element(this, snd,     n[p], a(p,w), 0);
      }
    }


    /*
     * Symmetry breaking:
     * we consider (h, a) and (a, h) as the same game and focus
     * on the home game for h, i.e. (h, a) with h < a
     */
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
      for (int w=0; w<2*teams; w+=2) {
        r[w]     = h(p, w / 2);
        r[w + 1] = a(p, w / 2);
      }
      gcc(this, r, 2, opt.icl());
    }


    for (int p=0; p<periods(); p++)
      for (int w=0; w<weeks(); w ++)
        post(this, teams * h(p,w) + a(p,w) - g(p,w) == teams);

    distinct(this, game, opt.icl());

    branch(this, game, BVAR_NONE, BVAL_MIN);
  }

  SportsLeague(bool share, SportsLeague& s)
    : Example(share, s), teams(s.teams) {
    home.update(this, share, s.home);
    away.update(this, share, s.away);
    game.update(this, share, s.game);
  }

  virtual Space*
  copy(bool share) {
    return new SportsLeague(share, *this);
  }

  // return the number of digits of n
  int digit(int n) {
    int f = n;
    int fdigit = 0;
    while (f / 10) {
      fdigit++;
      f = f / 10;
    }
    return fdigit;
  }

  // printing function that makes the schedule more readable
  void blank(int n) {
    for (int d = digit(teams) - digit(n); d--; ) {
      std::cout << " ";
    }
    std::cout << n;
  }

  void blank_only(int n) {
    for (int i = digit(n); i--; ) {
      std::cout << " ";
    }
  }

  virtual void print(void) {
    // print period index
    std::cout << " ";
    blank_only(teams);
    std::cout << "     ";
    for (int p=0; p<periods(); p++) {
      blank_only(teams);
      std::cout << "P[";
      blank(p);
      std::cout <<"]";
      blank_only(teams);
    }
    std::cout <<"\n";

    // print entries
    for (int w=0; w<weeks(); w++) {
      std::cout << "W[";
      blank(w+1);
      std::cout <<"]: ";
      for (int p=0; p<periods(); p++) {
        if (h(p, w).assigned() && a(p, w).assigned()) {
          std::cout <<" ";
          blank(h(p,w).val());
          std::cout <<",";
          blank(a(p,w).val());
          std::cout <<" ";
        } else {
          blank_only(teams);
          std::cout << " x ";
          blank_only(teams);
        }
      }
      std::cout << "\n";
    }
    std::cout << "\n";
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

