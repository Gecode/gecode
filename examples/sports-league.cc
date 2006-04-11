/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */	

#include "examples/support.hh"
#include "gecode/minimodel.hh"

/**
 * \brief Triple forming an entry in the round robin schedule
 * \relates SportsLeague
 *
 */

struct Play {
  /// number of the home team
  int h;
  /// number of the away team
  int a;
  /// game number 
  int g;
};

typedef PrimArgArray<Play> RRSArray;

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
 * \ingroup Example
 */
class SportsLeague : public Example {
private:
  /// require t to be even and t!=4 (has only failures)
  /// number of considered teams
  int t;            
  /// number of weeks
  int weeks; 
  /// inserting a dummy week according to Régin
  int eweeks;
  /// number of periods
  int periods;
  /// the whole season
  int season;
  /// the maximum value a game number can take
  int value;
  /// variables for home teams
  IntVarArray home;
  /// variables for away teams
  IntVarArray away;
  /// variables for game numbers
  IntVarArray game;
  /// \brief Round robin schedule
  RRSArray rrs_array;
protected:
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
    for (int d = digit(t) - digit(n); d--; ) {
      std::cout << " ";
    }
    std::cout << n;
  }

  void blankv(int n) {
    for (int d = digit(value) - digit(n); d--; ) {
      std::cout << " ";
    }
    std::cout << n;
  }

  void blank_only(int n) {
    for (int i = digit(n); i--; ) {
      std::cout << " ";
    }
  }

public:
  /// Access the home team in period \a p and week \a w
  IntVar& h (int p, int w) {
    return home[p * eweeks + w];
  }

  /// Access the away team in period \a p and week \a w
  IntVar& a (int p, int w) {
    return away[p * eweeks + w];
  }

  /**
   * \brief Access the game number associated with the game 
   * in period \a p and week \a w
   */
  IntVar& g (int p, int w) {
    return game[p * weeks + w];
  }
 
  /**
   * \brief Access the entry in the round robin schedule for
   *  period \a p and week \a w
   */
  Play& rrs(int p, int w) {
    return rrs_array[p * weeks + w];
  }

  /**
   * \brief Compute game numbers
   * 
   * Given the game \f$ (h,a) \f$ between a
   * home team \f$ h \f$ and an away team \f$ a \f$ this function 
   * computes the unique game number
   * \f$g = (h - 1) * t + a  \Leftrightarrow  t = t * h + a - g\f$
   */

  int gn(int h, int a) {
    return (t * (h - 1) + a);
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

  void init_rrs(void) {

    // Initialize the array 
    for(int p = 0; p < periods; p++)
      for (int w = 0; w < weeks; w++) {
	rrs(p, w).h = 0;
	rrs(p, w).a = 0;
	rrs(p, w).g = 0;
      }
    
    // Determine the first game (week 0 period 0)
    rrs(0, 0).h = 1;
    rrs(0, 0).a = 2;
    rrs(0, 0).g = 2;

    // Determine the other games of the first week
    for(int p = 1; p < periods; p++){
      rrs(p, 0).h = (p + 1) + 1;
      rrs(p, 0).a = t - (p + 1 - 2);
      rrs(p, 0).g = gn(rrs(p, 0).h, rrs(p, 0).a);
    }

    // Compute the games for the subsequent weeks
    for (int w = 1; w < weeks; w++) {
      for (int p = 0; p < periods; p++) {
	if (rrs(p, w - 1).h == t) {
	  rrs(p, w).h = 2;
	} else {
	  if (rrs(p, w - 1).h == 1) {
	    rrs(p, w).h = 1;
	  } else {
	    rrs(p, w).h = rrs(p, w - 1).h + 1;
	  }
	}
	if (rrs(p, w - 1).a == t) {
	  rrs(p, w).a = 2;
	} else {
	  rrs(p, w).a = rrs(p, w - 1).a + 1;
	}

	// maintain symmetry for (h, a): h < a
	if (rrs(p, w).h > rrs(p, w).a) {
	  int buffer  = rrs(p, w).h;
	  rrs(p, w).h = rrs(p, w).a;
	  rrs(p, w).a = buffer;
	}
	rrs(p, w).g = gn(rrs(p, w).h, rrs(p, w).a);
      }
    }
  }
  
  SportsLeague(const Options& op) :
    t       (op.size), 
    weeks   (t - 1), 
    eweeks  (t), 
    periods (t / 2), 
    season  (weeks * periods), 
    value   (t * (t - 2) + t), 
    home    (this, periods * eweeks),
    away    (this, periods * eweeks),
    game    (this, season), 
    rrs_array     (periods * weeks){

    IntSet dom_all   (1, t);
    IntSet dom_game  (2, value);
    IntSet dom_home  (1, t - 1);
    IntSet dom_away  (2, t);
    IntSet dom_index (0, periods - 1);
    
    for (int i = eweeks * periods; i--; ) {
      home[i].init(this, dom_home);
      away[i].init(this, dom_away);
    }
    for (int i = season; i--; ) {
      game[i].init(this, dom_game);
    }


    // Initialize the round robin schedule
    init_rrs();

    //    domain for the gamenumber of period
    for (int w = 0; w < weeks; w++) {
      IntArgs gamenum(periods);
      IntArgs fst(periods);
      IntArgs snd(periods);

      for(int p = 0; p < periods; p++){
	gamenum[p] = rrs(p, w).g; 
	fst[p]     = rrs(p, w).h;
	snd[p]     = rrs(p, w).a;
      }

      IntVarArray n(this, periods, 0, periods - 1);
      distinct(this, n, ICL_DOM);
	
      for(int p = 0; p < periods; p++){
	element(this, gamenum, n[p], g(p, w), op.icl);
	element(this, fst,     n[p], h(p, w), op.icl);
	element(this, snd,     n[p], a(p, w), op.icl);
      }
    }


    /**
     * Symmetrie breaking: 
     * we consider (h, a) and (a, h) as the same game and focus
     * on the home game for h, i.e. (h, a) with h < a
     */

    // fix the first pair
    rel(this, h(0,0), IRT_EQ, 1);
    rel(this, a(0,0), IRT_EQ, 2);

    for (int p = 0; p < periods; p++) {
      for (int w = 0; w < eweeks; w++) {
    	rel(this, h(p, w), IRT_LE, a(p, w));
      }
    }

    // home teams in the first week are ordered
    for (int p = 0; p < periods - 1; p++) {
      rel(this, h(p, 0), IRT_LE, h(p + 1, 0));
    }
    
    /**
     * Constraint on each column: 
     * each team occurs exactly once.
     * (you need two teams in order to form a match).
     */
    
    for(int w = 0; w < eweeks; w++ ) {
      IntVarArray col(this, t);
      int k = 0;
      for( int p = 0; p < periods; p++ ) {
	col[k] = h(p, w); 
	k++;
	col[k] = a(p, w);
	k++;
      }
      distinct(this, col, ICL_DOM);
    }

    /** 
     * Constraint on each row: 
     * no team appears more than twice
     * (you do not want a team to play more than twice a week in the same slot
     * the same time.)
     *
     */

    for(int p = 0; p < periods; p++) {
      IntVarArray row(this, 2 * eweeks);
      for (int w = 0; w < 2 * eweeks; w +=2) {
	row[w]     = h(p, w / 2);
	row[w + 1] = a(p, w / 2);
      }
      gcc(this, row, 2, op.icl); // cardvars
    }



    for(int p = 0; p < periods; p++) {
      for(int w = 0; w < weeks; w ++) {
	post(this, t * h(p, w) + 1 * a(p, w) - 1* g(p, w) == t);
      }
    }

    distinct(this, game, ICL_DOM);

    branch(this, game, BVAR_NONE, BVAL_MIN);

  }

  SportsLeague(bool share, SportsLeague& s)
    : Example(share, s), 
      t(s.t), weeks(s.weeks), eweeks(s.eweeks), 
      periods(s.periods), season(s.season), 
      value(s.value), rrs_array(s.rrs_array) {
    home.update(this, share, s.home);
    away.update(this, share, s.away);
    game.update(this, share, s.game);
  }
  
  virtual Space* 
  copy(bool share) {
    return new SportsLeague(share, *this);
  }

  virtual void print(void){
    // print feasible schedule
    // t is the number of the greatest game
    std::cout << "\nSchedule for " << t << " teams"
	      << " and "<< weeks << " weeks:" << std::endl;
    // print period index
    std::cout << " ";
    blank_only(t);
    std::cout << "     ";
    for (int p = 0; p < periods; p++) {
      blank_only(t);
      std::cout << "P[";
      blank(p);
      std::cout <<"]";
      blank_only(t);
    }
    std::cout <<"\n";

    // print entries
    for(int w = 0; w < weeks; w++){
      std::cout << "W["; 
      blank(w + 1); 
      std::cout <<"]: ";
      for(int p = 0; p < periods; p++){
	if (h(p, w).assigned() && a(p, w).assigned()) {
	  std::cout <<" ";
	  blank(h(p, w).val());
	  std::cout <<",";
	  blank(a(p, w).val());
	  std::cout <<" ";
	} else {
	  blank_only(t);
	  std::cout << " x ";
	  blank_only(t);
	}
      }
      std::cout << "\n";
    }
    std::cout << "\n";

    //print gamenumbers
    std::cout << "\nUnique game numbers:\n";
    std::cout <<"\n";
    for(int p = 0; p < periods; p++){
      std::cout << "Period[";
      blank(p + 1);
      std::cout <<"]: " ;
      for(int w = 0; w < weeks; w++){
	if (g(p, w).assigned()) {
	  blankv(g(p, w).val());
	  std::cout << " ";
	} else {
	  for (int i = digit(value); i--; ) {
	    std::cout << " ";
	  }
	  std::cout << " ";
	}
      }
      std::cout << "\n";
    }
    std::cout << "\n";

  }
};


int main(int argc, char** argv){
  Options o("Sports League Scheduling ");
  o.solutions = 1;
  o.size      = 18;
  o.parse(argc, argv);
  if (o.size == 4) {
    std::cerr<< "No Solution for t = 4!\n";
    return -1;
  } 
  if (o.size % 2 != 0) {
    std::cerr << "Number of t has to be even!\n";
    return -1;
  }
  Example::run<SportsLeague, DFS>(o);
  return 0;
}

// STATISTICS: example-any

