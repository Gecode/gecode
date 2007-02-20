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
#include "gecode/minimodel.hh"

static IntSet is_01(0,1);

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
    element(home, ia, x[0], x[1], 0);
  }
};
static IntSet is1(-4,8);
static const int c1[5] = {-1,1,-3,3,-4};
static ElementInt _elinta("Element::Int::Int::A",is1,&c1[0],5);
static IntSet is2(-4,8);
static const int c2[8] = {-1,1,-1,1,-1,1,0,0};
static ElementInt _elintb("Element::Int::Int::B",is2,&c2[0],8);
static IntSet is3(-4,8);
static const int c3[1] = {-1};
static ElementInt _elintc("Element::Int::Int::C",is3,&c3[0],1);

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
    element(home, ia, x[0], x[0], 0);
  }
};

static IntSet is4(-4,8);
static const int c4[7] = {0,-1,2,-2,4,-3,6};
static ElementShareInt _elintd("Element::Int::Int::D",is3,&c4[0],7);


class ElementBool : public IntTest {
private:
  const int* c;
  const int n;
public:
  ElementBool(const char* t, const IntSet& is, const int* c0, int n0)
    : IntTest(t,2,is), c(c0), n(n0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]>= 0) && (x[0]<n) && c[x[0]]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ia(n,c);
    element(home, ia, x[0], channel(home,x[1]), 0);
  }
};
static const int bc1[5] = {0,1,1,0,1};
static ElementInt _elintba("Element::Int::Bool::A",is1,&bc1[0],5);
static const int bc2[8] = {-1,1,0,1,0,1,0,0};
static ElementInt _elintbb("Element::Int::Bool::B",is2,&bc2[0],8);
static const int bc3[1] = {1};
static ElementInt _elintbc("Element::Int::Bool::C",is3,&bc3[0],1);

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
    IntVarArgs ia(n);
    for (int i=0;i<n;i++)
      ia[i]=x[2+i];
    element(home, ia, x[0], x[1], 0, icl);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "post element" << std::endl;
    c << "IntConLevel icl = " << Log::iclc(icl) << ";\n"
      << "\tint n = x.size()-2;\n" 
      << "\tIntVarArgs ia(n);\n" 
      << "\tfor (int i = 0; i < n; ++i)\n"
      << "\t    ia[i] = x[2+i];\n"
      << "\telement(this, ia, x[0], x[1], icl);\n"
      << std::endl;
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
    IntVarArgs ia(n);
    for (int i=0;i<n;i++)
      ia[i]=x[1+i];
    element(home, ia, x[0], x[0], 0, icl);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "post element" << std::endl;
    c << "IntConLevel icl = " << Log::iclc(icl) << ";\n"
      << "\tint n = x.size()-1;\n" 
      << "\tIntVarArgs ia(n);\n" 
      << "\tfor (int i = 0; i < n; ++i)\n"
      << "\t    ia[i] = x[1+i];\n"
      << "\telement(this, ia, x[0], x[0], icl);\n"
      << std::endl;
  }
};
static IntSet iv1(-1,3);
static ElementVar _elvarbnda("Element::Var::Bnd::A",iv1,3,ICL_BND);
static ElementVar _elvardoma("Element::Var::Dom::A",iv1,3,ICL_DOM);
static ElementShareVar _elsvarbnda("Element::Var::Bnd::B",iv1,3,ICL_BND);
static ElementShareVar _elsvardoma("Element::Var::Dom::B",iv1,3,ICL_DOM);


class ElementVarBool : public IntTest {
private:
  const int n;
public:
  ElementVarBool(const char* t, const IntSet& is, int n0)
    : IntTest(t,n0+2,is,false), n(n0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0;i<n;i++)
      if ((x[2+i] < 0) || (x[2+i]>1))
        return false;
    return (x[0]>= 0) && (x[0]<n) && x[2+x[0]]==x[1] 
      && (x[1]>=0) && (x[1]<=1);
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs ia(n);
    for (int i=0;i<n;i++)
      ia[i]=channel(home,x[2+i]);
    element(home, ia, x[0], channel(home,x[1]), 0);
  }
};

static ElementVarBool _elvarbbnda("Element::Var::Bool",iv1,3);


// STATISTICS: test-int

