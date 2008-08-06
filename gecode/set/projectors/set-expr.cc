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

using namespace Gecode::Set;

namespace Gecode {

  /**
   * \brief Nodes used to construct set expressions
   */
  class SetExpr::Node {
  private:
    /// Nodes are reference counted
    unsigned int use;
    /// Left and right subtrees
    Node *left, *right;
    /// Left and right signs for entire subtress
    int signLeft, signRight;
    /// Variable
    var_idx x;
    /// Relation
    RelType rel;
  public:
    /// Construct node for \a x
    Node(const var_idx x);
    /// Construct node from nodes \a n0 and \a n1 with signs \a s0 and \a s1
    Node(Node* n0, int s0, RelType r, Node* n1, int s1);
    
    /// Increment reference count
    void increment(void);
    /// Decrement reference count and possibly free memory
    GECODE_SET_EXPORT bool decrement(void);
    
    /// Adds code representation of the node to \a ret
    void encode(SetExprCode::Stream& ret, bool monotone) const;
    /// Returns the arity of the node
    int arity(void) const;

    /// Returns an iterator for this node
    Iter::Ranges::Virt::Iterator* iter(void);
    
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
  SetExpr::Node::operator new(size_t size) {
    return heap.alloc(size);
  }

  forceinline void
  SetExpr::Node::operator delete(void* p, size_t) {
    heap.free(p);
  }


  forceinline void
  SetExpr::Node::increment(void) { 
    ++use; 
  }

  forceinline
  SetExpr::Node::Node(const var_idx x) :
    use(1),
    left(NULL), right(NULL), signLeft(0), signRight(0), x(x) {}

  forceinline
  SetExpr::Node::Node(Node* l, int sL, RelType _rel, Node* r, int sR) :
    use(1),
    left(l), right(r), signLeft(sL), signRight(sR), rel(_rel) {
    if (left != NULL)
      left->increment();
    if (right != NULL)
      right->increment();
  }

  bool
  SetExpr::Node::decrement(void) {
    if (--use == 0) {
      if (left != NULL && left->decrement())
        delete left;
      if (right != NULL && right->decrement())
        delete right;
      return true;
    }
    return false;
  }

  int
  SetExpr::Node::arity(void) const {
    if (left==NULL && right==NULL)
      return x;
    return std::max( left==NULL ? 0 : left->arity(),
                     right==NULL ? 0 : right->arity() );
  }

  void
  SetExpr::Node::encode(SetExprCode::Stream& ret, bool monotone) const {
    if (left==NULL && right==NULL) {
      assert(x>=0);
      ret.add(SetExprCode::LAST+x);
      if (monotone)
        ret.add(SetExprCode::LUB);
      else
        ret.add(SetExprCode::GLB);
      return;
    }
    if (left==NULL) {
      ret.add(SetExprCode::UNIVERSE);
    } else {
      left->encode(ret, (signLeft==1) ? monotone : !monotone);
    }
    if (signLeft==-1)
      ret.add(SetExprCode::COMPLEMENT);
    if (right==NULL) {
      ret.add(SetExprCode::UNIVERSE);
    } else {
      right->encode(ret, (signRight==1) ? monotone : !monotone);
    }
    if (signRight==-1)
      ret.add(SetExprCode::COMPLEMENT);
    switch (rel) {
    case REL_INTER: ret.add(SetExprCode::INTER); break;
    case REL_UNION: ret.add(SetExprCode::UNION); break;
    default:
        GECODE_NEVER;
    }
  }


  /*
   * Operations for expressions
   *
   */
  
  SetExpr::SetExpr(var_idx v) : ax(new Node(v)), sign(1) {}

  SetExpr::SetExpr(const SetExpr& s, int ssign,
                   RelType r,
                   const SetExpr& t, int tsign)
    : ax(new Node(s.ax,s.sign*ssign,r,t.ax,t.sign*tsign)), sign(1) {}

  SetExpr::SetExpr(const SetExpr& s, int sign)
    : ax(s.ax), sign(s.sign*sign) {
    if (ax != NULL)
      ax->increment();
  }

  SetExpr::SetExpr(const SetExpr& s)
    : ax(s.ax), sign(s.sign) {
    if (ax != NULL)
      ax->increment();
  }

  const SetExpr&
  SetExpr::operator=(const SetExpr& s) {
    if (this != &s) {
      if ((ax != NULL) && ax->decrement())
        delete ax;
      ax = s.ax;
      sign = s.sign;
      if (ax != NULL)
        ax->increment();
    }
    return *this;
  }

  SetExpr::~SetExpr(void) {
    if ((ax != NULL) && ax->decrement())
      delete ax;
  }

  int
  SetExpr::arity(void) const {
    return (ax==NULL) ? 0 : ax->arity();
  }

  SetExprCode
  SetExpr::encode(void) const {
    SetExprCode::Stream s;
    if (ax==NULL) {
      s.add((sign==1) ? SetExprCode::EMPTY : SetExprCode::UNIVERSE);
    } else if (sign==-1) {
      ax->encode(s, false);
      s.add(SetExprCode::COMPLEMENT);
    } else {
      ax->encode(s, true);
    }
    SetExprCode ret(s);
    return ret;
  }

  Iter::Ranges::Virt::Iterator*
  codeToIterator(const ViewArray<Set::SetView>& x,
                 const SetExprCode& c, bool monotone) {

    typedef Iter::Ranges::Virt::Iterator Iterator;

    Support::DynamicStack<Iterator*> s;

    int tmp = 0;
    for (int i=0; i<c.size(); i++) {
      switch (c[i]) {
      case SetExprCode::COMPLEMENT:
        {
          Iterator* it = s.pop();
          s.push(new Iter::Ranges::Virt::Compl<Limits::min,
                                               Limits::max> (it));
        }
        break;
      case SetExprCode::INTER:
        {
          Iterator* ri = s.pop();
          Iterator* li = s.pop();
          s.push(new Iter::Ranges::Virt::Inter(li, ri));
        }
              break;
      case SetExprCode::UNION:
        {
          Iterator* ri = s.pop();
          Iterator* li = s.pop();
          s.push(new Iter::Ranges::Virt::Union(li, ri));
        }
        break;
      case SetExprCode::GLB:
        if (monotone) {
          GlbRanges<SetView> r(x[tmp]);
          s.push(new Iter::Ranges::Virt::RangesTemplate<GlbRanges<SetView> >(r));
        } else {
          LubRanges<SetView> r(x[tmp]);
          s.push(new Iter::Ranges::Virt::RangesTemplate<LubRanges<SetView> >(r));                
        }
        break;
      case SetExprCode::LUB:
        if (monotone) {
          LubRanges<SetView> r(x[tmp]);
          s.push(new Iter::Ranges::Virt::RangesTemplate<LubRanges<SetView> >(r));
        } else {
          GlbRanges<SetView> r(x[tmp]);
          s.push(new Iter::Ranges::Virt::RangesTemplate<GlbRanges<SetView> >(r));                
        }
        break;
      case SetExprCode::EMPTY:
        {
          Iter::Ranges::Singleton it(1,0);
          s.push(new Iter::Ranges::Virt::RangesTemplate<Iter::Ranges::Singleton> (it));
        }
        break;
      case SetExprCode::UNIVERSE:
        {
          Iter::Ranges::Singleton it(Limits::min,
                                     Limits::max);
          s.push(new Iter::Ranges::Virt::RangesTemplate<Iter::Ranges::Singleton> (it));
        }
        break;
      default:
        tmp = c[i]-SetExprCode::LAST;
        break;
      }
    }

    return s.top();
  }

  SetExprRanges::SetExprRanges(const ViewArray<Set::SetView>& x,
                               SetExpr& s,
                               bool monotone) {
    i = new Iter(codeToIterator(x, s.encode(), monotone));
  }

  SetExprRanges::SetExprRanges(const ViewArray<Set::SetView>& x,
                               const SetExprCode& c,
                               bool monotone) {
    i = new Iter(codeToIterator(x, c, monotone));
  }

}

std::ostream&
operator<<(std::ostream& os, const Gecode::SetExprCode& sec) {
  for (int i=0; i<sec.size(); i++) {
    switch (sec[i]) {
    case Gecode::SetExprCode::COMPLEMENT: os << " CMPL "; break;
    case Gecode::SetExprCode::INTER: os << " INTER "; break;
    case Gecode::SetExprCode::UNION: os << " UNION "; break;
    case Gecode::SetExprCode::GLB: os << " GLB "; break;
    case Gecode::SetExprCode::LUB: os << " LUB "; break;
    case Gecode::SetExprCode::EMPTY: os << " EMPTY "; break;
    case Gecode::SetExprCode::UNIVERSE: os << " UNIVERSE "; break;
    default: os << " x[" << sec[i]-Gecode::SetExprCode::LAST << "] "; break;
    }
  }
  return os;
}

// STATISTICS: set-prop
