/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
 *
 *  Last modified:
 *     $Date: 2005-10-14 07:33:24 +0000 (Fri, 14 Oct 2005) $ by $Author: pekczynski $
 *     $Revision: 2339 $
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

#include "test/int.hh"
#include "test/log.hh"
#include "../../support/sort.hh"

static IntSet ds_12(1,2);
static IntSet ds_13(1,3);
static IntSet ds_14(1,4);
static IntSet ds_03(0,3);

class IntSortMin {
public:
  bool operator()(const int& x, const int& y) {
    return x < y;
  }
};

class SortPermAssignment : public Assignment{
  int problow;
  int probup;
  int permlow;
  int permup;
  int xsize;
public:
  SortPermAssignment(int xlow, int xup, 
		     int plow, int pup,
		     int xs,
		     int n0, const IntSet& d0) 
    : Assignment(n0, d0), 
      problow(xlow), probup(xup), 
      permlow(plow), permup(pup), 
      xsize(xs) {
    reset();
  }
  void SortPermAssignment::reset(void) {
    done = false;
    IntSet perm_dom(permlow, permup);
    IntSet var_dom(problow, probup);
    for (int i = 2*xsize; i < n; i++) {
      dsv[i].init(perm_dom);
    }
    for (int i=0; i < 2*xsize; i++)
      dsv[i].init(var_dom);
  }
  void SortPermAssignment::operator++(void) {
    IntSet perm_dom(permlow, permup);
    IntSet var_dom(problow, probup);
    int i = n-1;
    while (true) {
      ++dsv[i];
      if (dsv[i]())
	return;
      dsv[i].init(d);
      if (i >= 2*xsize && i < n) {
	dsv[i].init(perm_dom);
      } else {
	dsv[i].init(var_dom);
      }
      --i;
      if (i<0) {
	done = true;
	return;
      }
    }
  }
};

class Sortedness_NoVar : public IntTest {
private:
  IntConLevel icl;
  static const int xs = 6;
  static const int ve = xs /2;

public:
  Sortedness_NoVar(const char* t, IntConLevel icl0) 
    : IntTest(t, xs, ds_13, false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    GECODE_AUTOARRAY(int, sortx, ve);
    for (int i = 0; i < ve; i++) {
      sortx[i] = x[i];
    }
    IntSortMin min_inc;
    Support::quicksort<int, IntSortMin> (&sortx[0], ve, min_inc);

//     std::cout << "defined sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       if (i < ve) {
// 	std::cout << sortx[i]<<" ";
//       } else {
// 	std::cout <<x[i] << " ";
//       }
//     }
//     std::cout <<"...";
    

    for (int i = ve; i < xs - 1; i++) {
      if ( !(x[i] <= x[i + 1]) ) {
	// std::cout << "y not sorted\n";
	return false;
      }
    }

    for (int i = ve; i < xs; i++) {
      if (sortx[i - ve] != x[i]) {
	// std::cout << "no sorting poss\n";
	return false;
      }
    }
    
    // std::cout << "valid\n";
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs z(ve);
    for (int i = 0; i < ve; i++) {
      z[i] = x[i]; 
    }

    IntVarArgs y(ve);
    for (int i = ve; i < xs; i++) {
      y[i - ve] = x[i]; 
    }
    
    sortedness(home, z, y, icl);
  }
};


class Sortedness_PermVar : public IntTest {
private:
  IntConLevel icl;
  static const int xs = 9;
  static const int ve1 = xs / 3;
  static const int ve2 = 2 * ve1;

  static const int xlow  = 1;
  static const int xup   = 3;
  static const int plow  = 0;
  static const int pup   = 2;
  static const int xsize = 3;
  
  Assignment* make_assignment() {
    return new SortPermAssignment(1, 3, 0, 2, 3, 9, dom);
  }

public:
  Sortedness_PermVar(const char* t, IntConLevel icl0) 
    : IntTest(t, xs, ds_03, false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {


    GECODE_AUTOARRAY(int, sortx, ve1);
    for (int i = 0; i < ve1; i++) {
      sortx[i] = x[i];
    }

    IntSortMin imin;
    Support::quicksort<int, IntSortMin> (&sortx[0], ve1, imin);

//     std::cout << "perm issol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve1 || i == ve2) std::cout << "||";
//       std::cout <<x[i] << " ";
// 	}
//     std::cout <<"...";
    

    // assert correct domains
    for (int i = 0; i < ve2; i++) {
      if (!(x[i] > 0)) {
	// std::cout << "false domain\n";
	return false;
      }
    }

    for (int i = ve2; i < xs; i++) {
      if (! (x[i] < 3)) {
	// std::cout << "false domain 2\n";
	return false;
      }
    }

    // perm index
    for (int i = 0; i < ve1; i++) {
      if (x[i] != x[ve1 + x[i + ve2]]) {
	// std::cout << "wrong index\n";
	return false;
      }
    }

    // perm distinct
    for (int i = ve2; i < xs - 1; i++) {
      for (int j = i + 1; j < xs; j++) {
	if (x[i] == x[j]) {
	  // std::cout << "no perm\n";
	  return false;
	}
      }
    }

    // ys have to be sorted
    for (int i = ve1; i < ve2 - 1; i++) {
      if ( !(x[i] <= x[i + 1]) ) {
	// std::cout << "y not sorted\n";
	return false;
      }
    }

    for (int i = ve1; i < ve2; i++) {
      if (sortx[i - ve1] != x[i]) {
	// std::cout << "no sorting poss\n";
	return false;
      }
    }

    // std::cout << "valid\n";
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs z(ve1);
    for (int i = 0; i < ve1; i++) {
      z[i] = x[i]; 
    }

    IntVarArgs y(ve1);
    for (int i = ve1; i < ve2; i++) {
      y[i - ve1] = x[i]; 
    }

    IntVarArgs p(ve1);
    for (int i = ve2; i < xs; i++) {
      p[i - ve2] = x[i]; 
    }
    
    for (int i = 0; i < ve1; i++) {
      rel(home, z[i], IRT_GQ, 1);
      rel(home, y[i], IRT_GQ, 1);
      rel(home, p[i], IRT_LQ, 2);
    }
    // std::cout << "post sort constraint\n";
    sortedness(home, z, y, p, icl);
  }
};



class Sortedness_NoVar_Shared : public IntTest {
private:
  IntConLevel icl;
  static const int xs = 4;
  static const int ve = xs /2;
  
public:
  Sortedness_NoVar_Shared(const char* t, IntConLevel icl0) 
    : IntTest(t, xs, ds_13, false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    GECODE_AUTOARRAY(int, sortx, ve);
    for (int i = 0; i < ve; i++) {
      sortx[i] = x[i];
    }
    IntSortMin min_inc;
    Support::quicksort<int, IntSortMin> (&sortx[0], ve, min_inc);

//     std::cout << "defined sol: ";
//     for (int i = 0; i < xs; i++) {
//       if (i == ve) std::cout << "||";
//       if (i < ve) {
// 	std::cout << sortx[i]<<" ";
//       } else {
// 	std::cout <<x[i] << " ";
//       }
//     }
//     std::cout <<"...";
    

    for (int i = ve; i < xs - 1; i++) {
      if ( !(x[i] <= x[i + 1]) ) {
	// std::cout << "y not sorted\n";
	return false;
      }
    }
    
    for (int i = ve; i < xs; i++) {
      if (sortx[i - ve] != x[i]) {
	// std::cout << "no sorting poss\n";
	return false;
      }
    }
    
    // std::cout << "valid\n";
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs z(ve);
    for (int i = 0; i < ve; i++) {
      if (i < ve /2) {
	z[i] = x[0]; 
      } else {
	z[i] = x[1]; 
      }
    }
    
    IntVarArgs y(ve);
    for (int i = ve; i < xs; i++) {
      if (i < ve + ((xs - ve) /2)) {
	y[i - ve] = x[2]; 
      } else {
	y[i - ve] = x[3]; 
      }
    }
    sortedness(home, z, y, icl);
  }
};


class Sortedness_PermVar_Shared_Perm : public IntTest {
private:
  IntConLevel icl;
  static const int xs = 7;
  static const int ve1 = xs / 2;
  static const int ve2 = 2 * ve1;

  Assignment* make_assignment() {
    return new SortPermAssignment(1, 3, 0, 2, 3, arity, dom);
  }

public:
  Sortedness_PermVar_Shared_Perm(const char* t, IntConLevel icl0) 
    : IntTest(t, xs, ds_03, false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return false;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs z(ve1);
    for (int i = 0; i < ve1; i++) {
      z[i] = x[i]; 
    }

    IntVarArgs y(ve1);
    for (int i = ve1; i < ve2; i++) {
      y[i - ve1] = x[i]; 
    }

    IntVarArgs p(ve1);
    for (int i = ve2; i < 9; i++) {
      p[i - ve2] = x[6]; 
    }
    
    for (int i = 0; i < ve1; i++) {
      rel(home, z[i], IRT_GQ, 1);
      rel(home, y[i], IRT_GQ, 1);
      rel(home, p[i], IRT_LQ, 2);
    }
    // std::cout << "post sort constraint\n";
    sortedness(home, z, y, p, icl);
  }
};


class Sortedness_PermVar_Shared_Prob : public IntTest {
private:
  IntConLevel icl;
  static const int xs = 5;
  static const int ve1 = 1;
  static const int ve2 = 2;

  Assignment* make_assignment() {
    return new SortPermAssignment(1, 3, 0, 2, 1, arity, dom);
  }

public:
  Sortedness_PermVar_Shared_Prob(const char* t, IntConLevel icl0) 
    : IntTest(t, xs, ds_03, false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {

    for (int i = 0; i < ve2; i++) {
      if (!(x[i] > 0)) {
	// std::cout << "false domain\n";
	return false;
      }
    }

    for (int i = ve2; i < xs; i++) {
      if (! (x[i] < 3)) {
	// std::cout << "false domain 2\n";
	return false;
      }
    }

    // perm index
    if (x[0] != x[1]) {
      // std::cout << "wrong index\n";
      return false;
    }

    // perm distinct
    for (int i = ve2; i < xs - 1; i++) {
      for (int j = i + 1; j < xs; j++) {
	if (x[i] == x[j]) {
	  // std::cout << "no perm\n";
	  return false;
	}
      }
    }


    // std::cout << "valid\n";
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs z(3);
    IntVarArgs y(3);
    for (int i = 0; i < 3; i++) {
      z[i] = x[0]; 
      y[i] = x[1];
    }

    IntVarArgs p(3);
    for (int i = ve2; i < xs; i++) {
      p[i - ve2] = x[i]; 
    }
    
    for (int i = 0; i < ve1; i++) {
      rel(home, z[i], IRT_GQ, 1);
      rel(home, y[i], IRT_GQ, 1);
      rel(home, p[i], IRT_LQ, 2);
    }
    // std::cout << "post sort constraint\n";
    sortedness(home, z, y, p, icl);
  }
};


Sortedness_NoVar  _sort_novar("Sortedness::NoPermutationVariables::Bnd",ICL_BND);
Sortedness_NoVar_Shared  _sort_novar_shared("Sortedness::NoPermutationVariables::Shared::Bnd",ICL_BND);
Sortedness_PermVar  _sort_permvar("Sortedness::WithPermutationVariables::Bnd",ICL_BND);
Sortedness_PermVar_Shared_Prob  _sort_permvar_shareprob("Sortedness::WithPermutationVariables::SharedProblemVars::Bnd",ICL_BND);
Sortedness_PermVar_Shared_Perm  _sort_permvar_shareperm("Sortedness::WithPermutationVariables::SharedPermVars::Bnd",ICL_BND);

// STATISTICS: test-int

