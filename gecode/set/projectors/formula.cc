/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#include "gecode/set/projectors.hh"

#include "gecode/support/buddy/bdd.h"
#include <set>

using namespace Gecode::Set;

namespace Gecode {

  /**
   * \brief Nodes used to construct formulas
   */
  class Formula::Node {
  private:
    /// Nodes are reference counted
    unsigned int use;
    /// Left and right subtrees
    Node *left, *right;
    /// Left and right signs for entire subtress
    int signLeft, signRight;
    /// Variable
    int x;
    /// Relation
    Operator r;
    /// Collect free variables of the formula
    void fv(std::set<int>& s);
  public:
    /// Construct node for \a x
    Node(const int x);
    /// Construct node from nodes \a n0 and \a n1 with signs \a s0 and \a s1
    Node(Node* n0, int s0, Operator r, Node* n1, int s1);
    
    /// Increment reference count
    void increment(void);
    /// Decrement reference count and possibly free memory
    bool decrement(void);

    /// Return free variables of the formula
    std::set<int> fv(void);
    /// Transform formula into bdd
    bdd toBdd(void);
    
    /// Memory management
    static void* operator new(size_t size);
    /// Memory management
    static void  operator delete(void* p,size_t size);
  };

  /*
   * Operations for nodes
   *
   */

  forceinline void*
  Formula::Node::operator new(size_t size) {
    return heap.malloc(size);
  }

  forceinline void
  Formula::Node::operator delete(void* p, size_t) {
    heap.free(p);
  }


  forceinline void
  Formula::Node::increment(void) { 
    ++use; 
  }

  forceinline
  Formula::Node::Node(const int x) :
    use(1),
    left(NULL), right(NULL), signLeft(0), signRight(0), x(x) {}

  forceinline
  Formula::Node::Node(Node* l, int sL, Operator _r, Node* r, int sR) :
    use(1),
    left(l), right(r), signLeft(sL), signRight(sR), r(_r) {
    if (left != NULL)
      left->increment();
    if (right != NULL)
      right->increment();
  }

  bool
  Formula::Node::decrement(void) {
    if (--use == 0) {
      if (left != NULL && left->decrement())
	      delete left;
      if (right != NULL && right->decrement())
	      delete right;
      return true;
    }
    return false;
  }

  void
  Formula::Node::fv(std::set<int>& s) {
    if (left==NULL)
      s.insert(x);
    else {
      left->fv(s);
      right->fv(s);
    }
  }
  
  inline
  std::set<int>
  Formula::Node::fv(void) {
    std::set<int> s;
    fv(s);
    return s;
  }

  bdd
  Formula::Node::toBdd(void) {
    if (left==NULL) {
      return bdd_ithvar(x);
    } else {
      bdd bl = left->toBdd(); bdd br = right->toBdd();
      if (signLeft == -1)
	bl = !bl;
      if (signRight == -1)
	br = !br;
      
      switch (r) {
      case OR:
        return bl | br;
      case AND:
        return bl & br;
      case IMPL:
        return bl >> br;
      case EQUIV:
        return (bl >> br) & (br >> bl);
      default:
        assert(false);
        return bdd_false();
      }
    }
  }

  /*
   * Operations for formulas
   *
   */
  
  Formula::Formula(int v) : ax(new Node(v)), sign(1) {}

  Formula::Formula(const Formula& f, int sf,
                   Operator r,
                   const Formula& g, int sg)
    : ax(new Node(f.ax,f.sign*sf,r,g.ax,g.sign*sg)), sign(1) {}

  Formula::Formula(const Formula& f, int sign)
    : ax(f.ax), sign(f.sign*sign) {
    if (ax != NULL)
      ax->increment();
  }

  Formula::Formula(const Formula& f)
    : ax(f.ax), sign(f.sign) {
    if (ax != NULL)
      ax->increment();
  }

  const Formula&
  Formula::operator=(const Formula& f) {
    if (this != &f) {
      if ((ax != NULL) && ax->decrement())
        delete ax;
      ax = f.ax;
      sign = f.sign;
      if (ax != NULL)
        ax->increment();
    }
    return *this;
  }

  Formula::~Formula(void) {
    if ((ax != NULL) && ax->decrement())
      delete ax;
  }

  namespace {

    /// Translate \a b into a SetExpr
    SetExpr bddToSetExpr(const bdd& b) {
      if (b == bdd_true())
        return SetExpr(SetExpr(), -1);
      else if (b == bdd_false())
        return SetExpr();

      SetExpr right = bddToSetExpr(bdd_high(b));
      SetExpr left = bddToSetExpr(bdd_low(b));

      SetExpr selfRight, selfLeft;

      int var = bdd_var(b);

      if (bdd_high(b) == bdd_true()) {
        selfRight = SetExpr(var);
        selfLeft = left;
      } else if (bdd_low(b) == bdd_true()) {
        selfLeft = SetExpr(SetExpr(var), -1);
        selfRight = right;
      } else {
        selfRight = SetExpr(var) && right;
        selfLeft = SetExpr(SetExpr(var), -1) && left;
      }

      if (bdd_high(b) == bdd_false())
        return selfLeft;
      if (bdd_low(b) == bdd_false())
        return selfRight;
    
      return selfLeft || selfRight;
    }

  }

  ProjectorSet
  Formula::projectors(void) {

    // Empty formla, empty projector set
    if (ax == NULL)
      return ProjectorSet(0);

    // Get free variables from formula
    std::set<int> s = ax->fv();
    int maxVar=0;
    for (std::set<int>::const_iterator i=s.begin(); i != s.end(); ++i) {
      maxVar = std::max(maxVar, *i);
    }

    bool bddWasRunning = true;
    // Initialize buddy
    if (!bdd_isrunning()) {
      bddWasRunning = false;
      bdd_init(1000, 1000);
    }
    if (bdd_varnum() < maxVar+1)
      bdd_setvarnum(maxVar+1);

    // First, transform the formula into a bdd
    bdd f = ax->toBdd();

    ProjectorSet p(s.size());

    // Now extract the projectors for each variable
    for (std::set<int>::const_iterator i=s.begin(); i != s.end(); ++i) {
      bdd x = bdd_ithvar(*i);

      // Compute bdd for psi -> x
      bdd psi1 = bdd_exist((! bdd_exist( ((!x) & f), x)) & f, x);
      //      bdd_reorder(BDD_REORDER_WIN3ITE);
      SetExpr glbExpr = bddToSetExpr(psi1);

      // Compute bdd for x -> psi
      bdd psi2 = bdd_exist( x & f, x);
      //      bdd_reorder(BDD_REORDER_WIN3ITE);
      SetExpr lubExpr = bddToSetExpr(psi2);

      Projector proj(*i, glbExpr, lubExpr);
      p.add(proj);
    }

    if (!bddWasRunning)
      bdd_done();

    return p;
  }

}

// STATISTICS: set-prop
