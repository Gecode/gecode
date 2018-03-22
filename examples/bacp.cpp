/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *     Mikael Lagerkvist, 2010
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
#include <gecode/int/branch.hh>

#include <map>
#include <string>
#include <list>
#include <vector>

using namespace Gecode;

/// A course \relates BACP
class Course {
public:
  const char* name; ///< Course name
  int credit;       ///< Course credit
};

/// A curriculum \relates BACP
class Curriculum {
public:
  /// Number of periods
  int p;
  /// Minimum academic load
  int a;
  /// Maximum academic load
  int b;
  /// Minimum amount of courses
  int c;
  /// Maximum amount of courses
  int d;

  /// Courses
  const Course *courses;
  /// Prerequisites
  const char **prereqs;
};

namespace {

  extern const Curriculum curriculum[];
  extern const unsigned int n_examples;

}

/**
 * \brief %Example: The balanced academic curriculum problem
 *
 * This is problem 030 from http://www.csplib.org/.
 *
 * A custom value selection from "A CP Approach to the Balanced
 * Academic Curriculum Problem", J.N. Monette, P. Schaus, S. Zampelli,
 * Y. Deville, and P. Dupont is available.
 *
 * \ingroup Example
 *
 */
class BACP : public IntMinimizeScript {
protected:
  /// The curriculum to be scheduled
  const Curriculum curr;

  /// Academic load for each period
  IntVarArray l;
  /// Maximum academic load
  IntVar u;
  /// Number of courses assigned to a period
  IntVarArray q;

  /// Period to which a course is assigned
  IntVarArray x;
public:
  /// Branching variants
  enum {
    BRANCHING_NAIVE,    ///< Simple fail-first branching
    BRANCHING_LOAD,     ///< Place based on minimum-load
    BRANCHING_LOAD_REV, ///< Place based on maximum-load
  };

  /// Actual model
  BACP(const SizeOptions& opt)
    : IntMinimizeScript(opt), curr(curriculum[opt.size()]) {
    std::map<std::string, int> courseMap; // Map names to course numbers
    int maxCredit = 0;
    int numberOfCourses = 0;
    for (const Course* co=curr.courses; co->name != 0; co++) {
      courseMap[co->name] = numberOfCourses++;
      maxCredit += co->credit;
    }

    int p = curr.p;
    int a = curr.a;
    int b = curr.b;
    int c = curr.c;
    int d = curr.d;

    l = IntVarArray(*this, p, a, b);
    u = IntVar(*this, 0, maxCredit);
    q = IntVarArray(*this, p, c, d);
    x = IntVarArray(*this, numberOfCourses, 0, p-1);

    for (int j=0; j<p; j++) {
      BoolVarArgs xij(*this, numberOfCourses, 0, 1);
      IntArgs t(numberOfCourses);
      for (int i=0; i<numberOfCourses; i++) {
        rel(*this, (x[i]==j) == xij[i]);
        t[i] = curr.courses[i].credit;
      }
      // sum over all t*(xi==j) is load of period i
      linear(*this, t, xij, IRT_EQ, l[j]);
      // sum over all (xi==j) is number of courses in period i
      linear(*this, xij, IRT_EQ, q[j]);
    }

    // Precedence
    for (const char** prereq = curr.prereqs; *prereq != 0; prereq+=2)
      rel(*this, x[courseMap[*prereq]] < x[courseMap[*(prereq+1)]]);

    // Optimization criterion: minimize u
    max(*this, l, u);

    // Redundant constraints
    linear(*this, l, IRT_EQ, maxCredit);
    linear(*this, q, IRT_EQ, numberOfCourses);

    switch (opt.branching()) {
    case BRANCHING_NAIVE:
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCHING_LOAD:
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL(&load));
      break;
    case BRANCHING_LOAD_REV:
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL(&load_rev));
      break;
    }
  }
  /// Value selection function for load branching
  static int load(const Space& home, IntVar x, int) {
    const BACP& b = static_cast<const BACP&>(home);
    IntVarValues values(x);
    int val = -1;
    int best = Int::Limits::max+1;
    while (values()) {
      if (b.l[values.val()].min() < best) {
        val  = values.val();
        best = b.l[val].min();
      }
      ++values;
    }
    assert(val != -1);
    return val;
  }
  /// Value selection function for reverse load branching
  static int load_rev(const Space& home, IntVar x, int) {
    const BACP& b = static_cast<const BACP&>(home);
    IntVarValues values(x);
    int val = -1;
    int best = Int::Limits::min-1;
    while (values()) {
      if (b.l[values.val()].min() > best) {
        val  = values.val();
        best = b.l[val].min();
      }
      ++values;
    }
    assert(val != -1);
    return val;
  }
  /// Constructor for copying \a bacp
  BACP(BACP& bacp) : IntMinimizeScript(bacp),
    curr(bacp.curr) {
    l.update(*this, bacp.l);
    u.update(*this, bacp.u);
    x.update(*this, bacp.x);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new BACP(*this);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return u;
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    std::vector<std::list<int> > period(curr.p);
    for (int i=x.size(); i--;)
      period[x[i].val()].push_back(i);

    os << "Solution with load " << u.val() << ":" << std::endl;
    for (int i=0; i<curr.p; i++) {
      os << "\tPeriod "<<i+1<<": ";
      for (std::list<int>::iterator v=period[i].begin();
           v != period[i].end(); ++v) {
        os << curr.courses[*v].name << " ";
      }
      os << std::endl;
    }
    os << std::endl;
  }

};

/** \brief Main-function
 *  \relates BACP
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("BACP");
  opt.branching(BACP::BRANCHING_NAIVE);
  opt.branching(BACP::BRANCHING_NAIVE,"naive");
  opt.branching(BACP::BRANCHING_LOAD,"load");
  opt.branching(BACP::BRANCHING_LOAD_REV,"load-reverse");
  opt.size(2);
  opt.solutions(0);
  opt.iterations(20);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and " << n_examples - 1
              << std::endl;
    return 1;
  }
  IntMinimizeScript::run<BACP,BAB,SizeOptions>(opt);
  return 0;
}

namespace {
  /** \name Parameters for balanced academic curriculum
    * \relates BACP
    * @{
    */

  /// Courses for first example
  const Course courses8[] =
    {
      {"dew100", 1},
      {"fis100", 3},
      {"hcw310", 1},{"iwg101", 2},{"mat190", 4},{"mat192", 4},{"dew101", 1},
      {"fis101", 5},{"iwi131", 3},{"mat191", 4},{"mat193", 4},{"fis102", 5},{"hxwxx1", 1},
      {"iei134", 3},{"iei141", 3},{"mat194", 4},
      {"dewxx0", 1},{"hcw311", 1},{"iei132", 3},{"iei133", 3},{"iei142", 3},{"iei162", 3},
      {"iwn170", 3},{"mat195", 3},{"hxwxx2", 1},{"iei231", 4},{"iei241", 4},{"iei271", 3},{"iei281", 3},{"iwn261", 3},
      {"hfw120", 2},{"iei233", 4},{"iei238", 3},{"iei261", 3},{"iei272", 3},{"iei273", 3},{"iei161", 3},{"iei232", 3},
      {"iei262", 3},{"iei274", 3},{"iwi365", 3},{"iwn270", 3},{"hrw130", 2},{"iei218", 3},{"iei219", 3},{"iei248", 3},
      {0,0}
    };

  /// Prerequisites for first example
  const char* prereqs8[] =
    {
    "dew101","dew100",
    "fis101","fis100",
    "fis101","mat192",
    "mat191","mat190",
    "mat193","mat190",
    "mat193","mat192",
    "fis102","fis101",
    "fis102","mat193",
    "iei134","iwi131",
    "iei141","iwi131",
    "mat194","mat191 ",
    "mat194","mat193",
    "dewxx0","dew101",
    "hcw311","hcw310",
    "iei132","iei134",
    "iei133","iei134",
    "iei142","iei141",
    "mat195","mat194",
    "iei231","iei134",
    "iei241","iei142",
    "iei271","iei162",
    "iei281","mat195",
    "iei233","iei231",
    "iei238","iei231",
    "iei261","iwn261",
    "iei272","iei271",
    "iei273","iei271",
    "iei273","iei271",
    "iei161","iwn261",
    "iei161","iwn261",
    "iei232","iei273",
    "iei232","iei273",
    "iei262","iwn261",
    "iei274","iei273",
    "iei274","iei273",
    "iei219","iei232",
    "iei248","iei233",
    "iei248","iei233",
    0,0
    };

  /// Courses for second example
  const Course courses10[] = {
  {"dew100",1},
  {"fis100",3},
  {"hrwxx1",2},
  {"iwg101",2},
  {"mat021",5},
  {"qui010",3},
  {"dew101",1},
  {"fis110",5},
  {"hrwxx2",2},
  {"iwi131",3},
  {"mat022",5},
  {"dewxx0",1},
  {"fis120",4},
  {"hcw310",1},
  {"hrwxx3",2},
  {"ili134",4},
  {"ili151",3},
  {"mat023",4},
  {"hcw311",1},
  {"ili135",4},
  {"ili153",3},
  {"ili260",3},
  {"iwn261",3},
  {"mat024",4},
  {"fis130",4},
  {"ili239",4},
  {"ili245",4},
  {"ili253",4},
  {"fis140",4},
  {"ili236",4},
  {"ili243",4},
  {"ili270",3},
  {"ili280",4},
  {"ici344",4},
  {"ili263",3},
  {"ili332",4},
  {"ili355",4},
  {"iwn170",3},
  {"icdxx1",3},
  {"ili362",3},
  {"iwn270",3},
  {"icdxx2",3},
  {0,0}
  };

  /// Prerequisites for second example
  const char* prereqs10[] = {
  "dew101","dew100",
  "fis110","fis100",
  "fis110","mat021",
  "mat022","mat021",
  "dewxx0","dew101",
  "fis120","fis110",
  "fis120","mat022",
  "ili134","iwi131",
  "ili151","iwi131",
  "mat023","mat022",
  "hcw311","hcw310",
  "ili135","ili134",
  "ili153","ili134",
  "ili153","ili151",
  "mat024","mat023",
  "fis130","fis110",
  "fis130","mat022",
  "ili239","ili135",
  "ili245","ili153",
  "ili253","ili153",
  "fis140","fis120",
  "fis140","fis130",
  "ili236","ili239",
  "ili243","ili245",
  "ili270","ili260",
  "ili270","iwn261",
  "ili280","mat024",
  "ici344","ili243",
  "ili263","ili260",
  "ili263","iwn261",
  "ili332","ili236",
  "ili355","ili153",
  "ili355","ili280",
  "ili362","ili263",
  0,0
  };

  /// Courses for third example
  const Course courses12[] = {
  {"dew100",1},
  {"fis100",3},
  {"hcw310",1},
  {"iwg101",2},
  {"mat111",4},
  {"mat121",4},
  {"dew101",1},
  {"fis110",5},
  {"iwi131",3},
  {"mat112",4},
  {"mat122",4},
  {"dewxx0",1},
  {"fis120",4},
  {"hcw311",1},
  {"hxwxx1",1},
  {"ili142",4},
  {"mat113",4},
  {"mat123",4},
  {"fis130",4},
  {"ili134",4},
  {"ili151",3},
  {"iwm185",3},
  {"mat124",4},
  {"fis140",4},
  {"hxwxx2",1},
  {"ile260",3},
  {"ili260",3},
  {"iwn170",3},
  {"qui104",3},
  {"ili231",3},
  {"ili243",4},
  {"ili252",4},
  {"ili273",3},
  {"mat210",4},
  {"mat260",4},
  {"ild208",3},
  {"ili221",4},
  {"ili274",3},
  {"ili281",3},
  {"iwn270",3},
  {"mat270",4},
  {"hrw150",2},
  {"ili238",4},
  {"ili242",3},
  {"ili275",3},
  {"ili355",4},
  {"hrw110",2},
  {"ici393",4},
  {"ili237",4},
  {"ili334",4},
  {"ili363",3},
  {"iwn261",3},
  {"hrw100",2},
  {"ici382",4},
  {"ili331",4},
  {"ili362",3},
  {"ili381",3},
  {"iln230",3},
  {"ici313",2},
  {"ici315",2},
  {"ici332",3},
  {"ici344",4},
  {"icn336",3},
  {"iwi365",3},
  {"ici314",2},
  {"ici367",2},
  {0,0}
  };

  /// Prerequisites for third example
  const char* prereqs12[] = {
  "dew101","dew100",
  "fis110","fis100",
  "fis110","mat121",
  "mat112","mat111",
  "mat122","mat111",
  "mat122","mat121",
  "dewxx0","dew101",
  "fis120","fis110",
  "fis120","mat122",
  "hcw311","hcw310",
  "ili142","iwi131",
  "mat113","mat112",
  "mat113","mat122",
  "mat123","mat112",
  "mat123","mat122",
  "fis130","fis110",
  "fis130","mat122",
  "ili134","iwi131",
  "ili151","mat112",
  "mat124","mat113",
  "mat124","mat123",
  "fis140","fis120",
  "fis140","fis130",
  "ile260","fis120",
  "ile260","mat124",
  "ili231","iwi131",
  "ili252","iwi131",
  "ili273","ili260",
  "mat210","mat113",
  "mat260","iwi131",
  "mat260","mat113",
  "mat260","mat123",
  "ili221","ili134",
  "ili221","ili231",
  "ili221","mat260",
  "ili274","ili273",
  "ili281","mat260",
  "mat270","iwi131",
  "mat270","mat113",
  "mat270","mat123",
  "ili238","ili134",
  "ili242","ili142",
  "ili275","ili274",
  "ili355","ili221",
  "hrw110","hrw150",
  "ici393","mat210",
  "ici393","mat260",
  "ili237","ili231",
  "ili237","ili252",
  "ili334","ili238",
  "ili363","ili273",
  "hrw100","hrw110",
  "ici382","ili334",
  "ili331","ili238",
  "ili331","ili274",
  "ili362","ili363",
  "ili381","ili281",
  "ili381","mat210",
  "iln230","iwn170",
  "ici313","ili331",
  "ici332","ici393",
  "ici332","ili331",
  "ici344","ili243",
  "icn336","ici393",
  "ici314","ici313",
  0,0
  };

  /// The example specifications
  const Curriculum curriculum[]=
    { { 8, 10, 24, 2, 10,
        courses8, prereqs8
      },
      { 10, 10, 24, 2, 10,
        courses10, prereqs10
      },
      { 12, 10, 24, 2, 10,
        courses12, prereqs12
      }
    };

  /// The number of examples
  const unsigned int n_examples = sizeof(curriculum) / sizeof(Curriculum);

  //@}
}

// STATISTICS: example-any

