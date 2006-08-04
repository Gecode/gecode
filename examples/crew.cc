/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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


#include "gecode/set.hh"
#include "examples/support.hh"

typedef enum {
  Tom, David, Jeremy, Ron,
  Joe, Bill, Fred,
  Bob, Mario, Ed,
  Carol, Janet, Tracy,
  Marilyn, Carolyn, Cathy,
  Inez, Jean, Heather, Juliet
} Employees;

const Employees employees[] =
  { Tom, David, Jeremy, Ron,
    Joe, Bill, Fred,
    Bob, Mario, Ed,
    Carol, Janet, Tracy,
    Marilyn, Carolyn, Cathy,
    Inez, Jean, Heather, Juliet };
const int lastEmployee = Juliet;

const char*
employeeToName(Employees e) {
  switch(e) {
  case Tom : return "Tom";
  case David : return "David";
  case Jeremy: return "Jeremy";
  case Ron: return "Ron";
  case Joe: return "Joe";
  case Bill: return "Bill";
  case Fred: return "Fred";
  case Bob: return "Bob";
  case Mario: return "Mario";
  case Ed: return "Ed";
  case Carol: return "Carol";
  case Janet: return "Janet";
  case Tracy: return "Tracy";
  case Marilyn: return "Marilyn";
  case Carolyn: return "Carolyn";
  case Cathy: return "Cathy";
  case Inez: return "Inez";
  case Jean: return "Jean";
  case Heather: return "Heather";
  case Juliet: return "Juliet";
  }
  return "None";
}

// these have to be sorted!
const Employees stewards[] =
  {Tom, David, Jeremy, Ron, Joe, Bill, Fred, Bob, Mario, Ed};
const int noOfStewards = 10;
const Employees hostesses[] =
  { Carol, Janet, Tracy, Marilyn, Carolyn, Cathy, Inez,
    Jean, Heather, Juliet };
const int noOfHostesses = 10;
const Employees frenchSpeaking[] =
  { Bill, Inez, Jean, Juliet };
const int noOfFrenchSpeaking = 4;
const Employees germanSpeaking[] =
  { Tom, Jeremy, Mario, Cathy, Juliet };
const int noOfGermanSpeaking = 5;
const Employees spanishSpeaking[] =
  { Joe, Bill, Fred, Mario, Marilyn, Inez, Heather };
const int noOfSpanishSpeaking = 7;

const int flights[][7] =
  { {1,4,1,1,1,1,1},
    {2,5,1,1,1,1,1},
    {3,5,1,1,1,1,1},
    {4,6,2,2,1,1,1},
    {5,7,3,3,1,1,1},
    {6,4,1,1,1,1,1},
    {7,5,1,1,1,1,1},
    {8,6,1,1,1,1,1},
    {9,6,2,2,1,1,1},
    {10,7,3,3,1,1,1} };

const int len = 10;


/**
 * \brief %Example: Airline crew allocation
 *
 * Assign 20 flight attendants to 10 flights. Each flight needs a certain
 * number of cabin crew, and they have to speak certain languages.
 * Every cabin crew member has two flights off after an attended flight.
 *
 * \ingroup Example
 *
 */
class Crew : public Example {
public:
  SetVarArray root;

  Crew(const Options& o) :
    root(this,len,IntSet::empty,0,lastEmployee)
  {
    IntSet stewardsDS((int*)stewards,noOfStewards);
    IntSet hostessesDS((int*)hostesses,noOfHostesses);
    IntSet spanishDS((int*)spanishSpeaking, noOfSpanishSpeaking);
    IntSet frenchDS((int*)frenchSpeaking, noOfFrenchSpeaking);
    IntSet germanDS((int*)germanSpeaking, noOfGermanSpeaking);

    for (int i=0; i<len; i++) {
      IntVarArray ia(this,5,0,lastEmployee);
      SetVar team = root[i];
      const int* flight = flights[i];

      const int N        = flight[1];
      const int NStew    = flight[2];
      const int NHost    = flight[3];
      const int NFrench  = flight[4];
      const int NSpanish = flight[5];
      const int NGerman  = flight[6];

      cardinality(this, team,N,N);
      SetVar stewardsInFS(this);
      SetVar hostessesInFS(this);
      SetVar spanishInFS(this);
      SetVar frenchInFS(this);
      SetVar germanInFS(this);

      rel(this, team, SOT_INTER, stewardsDS, SRT_EQ, stewardsInFS);
      rel(this, team, SOT_INTER, hostessesDS, SRT_EQ, hostessesInFS);
      rel(this, team, SOT_INTER, spanishDS, SRT_EQ, spanishInFS);
      rel(this, team, SOT_INTER, frenchDS, SRT_EQ, frenchInFS);
      rel(this, team, SOT_INTER, germanDS, SRT_EQ, germanInFS);

      cardinality(this, stewardsInFS, ia[0]);
      cardinality(this, hostessesInFS, ia[1]);
      cardinality(this, spanishInFS, ia[2]);
      cardinality(this, frenchInFS, ia[3]);
      cardinality(this, germanInFS, ia[4]);

      rel(this, ia[0], IRT_GQ, NStew);
      rel(this, ia[1], IRT_GQ, NHost);
      rel(this, ia[2], IRT_GQ, NSpanish);
      rel(this, ia[3], IRT_GQ, NFrench);
      rel(this, ia[4], IRT_GQ, NGerman);

    }

    for (int i=0; i<len-2; i++) {
      rel(this, root[i], SRT_DISJ, root[i+1]);
      rel(this, root[i], SRT_DISJ, root[i+2]);
    }
    rel(this, root[len-2], SRT_DISJ, root[len-1]);

    branch(this, root, SETBVAR_NONE, SETBVAL_MIN);
  }

  Crew(bool share, Crew& s)
    : Example(share,s) {
    root.update(this,share,s.root);
  }

  virtual
  Space *copy(bool share) {
    return new Crew(share,*this);
  }

  virtual void
  print(void) {
    for (int i=0; i<len; i++) {
      SetVarGlbValues d(root[i]);

      std::cout << "\tFlight " << i+1 << ":" << std::endl;
      std::cout << "\t\tCrew\tStew.\tHost.\tFrench\tSpanish\tGerman"
		<< std::endl << "\t";
      for (int j=1; j<7; j++)
        std::cout << "\t" << flights[i][j];
      std::cout << std::endl;

      std::cout << "\t\tSchedule:" << std::endl << "\t\t";
      for (;d();++d) {
        std::cout << employeeToName((Employees)d.val()) << " ";
      }
      std::cout << std::endl << std::endl;
    }

  }
};

int
main(int argc, char** argv) {
  Options o("Crew");
  o.iterations = 100;
  o.parse(argc,argv);
  Example::run<Crew,DFS>(o);
  return 0;
}


// STATISTICS: example-any

