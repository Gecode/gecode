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

#include <algorithm>
#include <iomanip>

using namespace Gecode;

/// Entry in round robin schedule
class Play {
public:
  int h; ///< home team
  int a; ///< away team
  int g; ///< game number
  /// Default constructor
  Play(void) : h(0), a(0), g(0) {}
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
        if (play(p,w-1).h == teams)
          play(p,w).h = 2;
        else if (play(p,w-1).h == 1)
          play(p,w).h = 1;
        else
          play(p,w).h = play(p,w-1).h + 1;
        if (play(p,w-1).a == teams)
          play(p,w).a = 2;
        else
          play(p,w).a = play(p,w-1).a + 1;

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
 * \brief %Example: %Sports league scheduling
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
 * \ingroup Example
 */
class SportsLeague : public Script {
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
  /// Home team in period \a p and week \a w
  const IntVar& h(int p, int w) const {
    return home[p*teams + w];
  }
  /// Away team in period \a p and week \a w
  IntVar& a(int p, int w) {
    return away[p*teams + w];
  }
  /// Away team in period \a p and week \a w
  const IntVar& a(int p, int w) const {
    return away[p*teams + w];
  }
  /// Return game number for game in period \a p and week \a w
  IntVar& g(int p, int w) {
    return game[p*weeks() + w];
  }
  /// Return game number for game in period \a p and week \a w
  const IntVar& g(int p, int w) const {
    return game[p*weeks() + w];
  }

public:
  /// Setup model
  SportsLeague(const SizeOptions& opt) :
    Script(opt),
    teams(opt.size()),
    home(*this, periods() * teams, 1, weeks()),
    away(*this, periods() * teams, 2, weeks()+1),
    game(*this, weeks()*periods(), 2, teams*weeks())
  {
    // Initialize round robin schedule
    RRS r(teams);

    // Domain for gamenumber of period
    for (int w=0; w<weeks(); w++) {
      IntArgs rh(periods()), ra(periods()), rg(periods());
      IntVarArgs n(*this,periods(),0,periods()-1);

      distinct(*this, n, opt.ipl());

      r.hag(w,rh,ra,rg);

      for (int p=0; p<periods(); p++) {
        element(*this, rh, n[p], h(p,w));
        element(*this, ra, n[p], a(p,w));
        element(*this, rg, n[p], g(p,w));
      }
    }

    /// (h,a) and (a,h) are the same game, focus on home (that is, h<a)
    for (int p=0; p<periods(); p++)
      for (int w=0; w<teams; w++)
        rel(*this, h(p,w), IRT_LE, a(p,w));

    // Home teams in first week are ordered
    {
      IntVarArgs h0(periods());
      for (int p=0; p<periods(); p++)
        h0[p] = h(p,0);
      rel(*this, h0, IRT_LE);
    }

    // Fix first pair
    rel(*this, h(0,0), IRT_EQ, 1);
    rel(*this, a(0,0), IRT_EQ, 2);

    /// Column constraint: each team occurs exactly once
    for (int w=0; w<teams; w++) {
      IntVarArgs c(teams);
      for (int p=0; p<periods(); p++) {
        c[2*p] = h(p,w); c[2*p+1] = a(p,w);
      }
      distinct(*this, c, opt.ipl());
    }

    /// Row constraint: no team appears more than twice
    for (int p=0; p<periods(); p++) {
      IntVarArgs r(2*teams);
      for (int t=0; t<teams; t++) {
        r[2*t]   = h(p,t);
        r[2*t+1] = a(p,t);
      }
      IntArgs values(teams);
      for (int i=1; i<=teams; i++)
        values[i-1] = i;
      count(*this, r, IntSet(2,2), values, opt.ipl());
    }

    // Redundant constraint
    for (int p=0; p<periods(); p++)
      for (int w=0; w<weeks(); w ++)
        rel(*this, teams * h(p,w) + a(p,w) - g(p,w) == teams);

    distinct(*this, game, opt.ipl());

    branch(*this, game, INT_VAR_NONE(), INT_VAL_SPLIT_MIN());
  }
  /// Constructor for cloning \a s
  SportsLeague(SportsLeague& s)
    : Script(s), teams(s.teams) {
    home.update(*this, s.home);
    away.update(*this, s.away);
    game.update(*this, s.game);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new SportsLeague(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    // print period index
    os << "\t       ";
    for (int p=0; p<periods(); p++) {
      os << "P[";
      os.width(2);
      os << p << "] ";
      }
    os << std::endl;
    // print entries
    for (int w=0; w<weeks(); w++) {
      os << "\tW[";
      os.width(2);
      os << w+1 << "]: ";
      for (int p=0; p<periods(); p++) {
        os.width(2);
        os << h(p,w).val() << '-';
        os.width(2);
        os << a(p,w).val() << " ";
      }
      os << std::endl;
    }
  }
};


/** \brief Main-function
 *  \relates SportsLeague
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Sports League Scheduling");
  opt.ipl(IPL_DOM);
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
  Script::run<SportsLeague, DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
