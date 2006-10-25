/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"
#include "test/log.hh"

class ElementInt : public IntTest {
private:
  const int* c;
  const int n;
public:
  ElementInt(const char* t, const IntSet& is, const int* c0, int n0)
    : IntTest(t,2,is), c(c0), n(n0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]>= 0) && (x[0]<n) && c[x[0]]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ia(n,c);
    element(home, ia, x[0], x[1]);
  }
};
static IntSet is1(-4,8);
static const int c1[5] = {-1,1,-3,3,-4};
static ElementInt _elinta("Element::Int::A",is1,&c1[0],5);
static IntSet is2(-4,8);
static const int c2[8] = {-1,1,-1,1,-1,1,0,0};
static ElementInt _elintb("Element::Int::B",is2,&c2[0],8);
static IntSet is3(-4,8);
static const int c3[1] = {-1};
static ElementInt _elintc("Element::Int::C",is3,&c3[0],1);

class ElementShareInt : public IntTest {
private:
  const int* c;
  const int n;
public:
  ElementShareInt(const char* t, const IntSet& is, const int* c0, int n0)
    : IntTest(t,1,is), c(c0), n(n0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]>= 0) && (x[0]<n) && c[x[0]]==x[0];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ia(n,c);
    element(home, ia, x[0], x[0]);
  }
};

static IntSet is4(-4,8);
static const int c4[7] = {0,-1,2,-2,4,-3,6};
static ElementShareInt _elintd("Element::Int::D",is3,&c4[0],7);

class ElementVar : public IntTest {
private:
  const int n;
public:
  ElementVar(const char* t, const IntSet& is, int n0, IntConLevel icl)
    : IntTest(t,n0+2,is,false,icl), n(n0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]>= 0) && (x[0]<n) && x[2+x[0]]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("", icl == ICL_BND
             ? "\tIntConLevel icl = ICL_BND;"
             : "\tIntConLevel icl = ICL_DOM;");
    Log::log("post element",
             "\tint n = x.size()-2;\n"
             "\tIntVarArgs ia(n);\n"
             "\tfor (int i=0;i<n;i++)\n"
             "\t  ia[i]=x[2+i];\n"
             "\telement(this, ia, x[0], x[1], icl);");
    IntVarArgs ia(n);
    for (int i=0;i<n;i++)
      ia[i]=x[2+i];
    element(home, ia, x[0], x[1], icl);
  }
};

class ElementShareVar : public IntTest {
private:
  const int n;
  IntConLevel icl;
public:
  // This is actually not domain consistent!
  ElementShareVar(const char* t, const IntSet& is, int n0, IntConLevel icl0)
    : IntTest(t,n0+1,is,false), n(n0), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]>= 0) && (x[0]<n) && x[1+x[0]]==x[0];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("", icl == ICL_BND
             ? "\tIntConLevel icl = ICL_BND;"
             : "\tIntConLevel icl = ICL_DOM;");
    Log::log("post element",
             "\tint n = x.size()-1;\n"
             "\tIntVarArgs ia(n);\n"
             "\tfor (int i=0;i<n;i++)\n"
             "\t  ia[i]=x[1+i];\n"
             "\telement(this, ia, x[0], x[0], icl);");
    IntVarArgs ia(n);
    for (int i=0;i<n;i++)
      ia[i]=x[1+i];
    element(home, ia, x[0], x[0], icl);
  }
};
static IntSet iv1(-1,3);
static ElementVar _elvarbnda("Element::Var::Bnd::A",iv1,3,ICL_BND);
static ElementVar _elvardoma("Element::Var::Dom::A",iv1,3,ICL_DOM);
static ElementShareVar _elsvarbnda("Element::Var::Bnd::B",iv1,3,ICL_BND);
static ElementShareVar _elsvardoma("Element::Var::Dom::B",iv1,3,ICL_DOM);



// STATISTICS: test-int

