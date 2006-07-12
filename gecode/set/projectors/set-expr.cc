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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set/projectors.hh"

using namespace Gecode::Set;

namespace Gecode {

  SetExpr::proj_scope
  SetExpr::combineScopes(const SetExpr::proj_scope& s1,
			 const SetExpr::proj_scope& s2) {
    SetExpr::proj_scope s = s1;

    SetExpr::proj_scope::const_iterator iter;
    for (iter=s2.begin(); iter != s2.end(); iter++) {
      if ( s.find(iter->first) == s.end()) {
	// variable in s2 is not in s1, add it
	s[iter->first] = iter->second;
      } else if (iter->second != s[iter->first]) {
	// variables are in both scopes, but with different PropCond
	s[iter->first] = PC_SET_ANY;
      } else {
	// variables are in both scopes with the same PropCond
      }
    }

    return s;
  }

  /**
   * \brief Nodes used to construct set expressions
   */
  class SetExpr::Node {
    friend class SetExpr::NodeIter;
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
    
    /// String representation
    std::string toString(bool monotone) const;
    /// Returns the scope of the node
    proj_scope scope(int sign) const;
    /// Returns the arity of the node
    int arity(void) const;

    /// Returns an iterator for this node
    Iter::Ranges::Virt::Iterator* iter(void);
    
    /// Memory management
    static void* operator new(size_t size);
    /// Memory management
    static void  operator delete(void* p,size_t size);
  };

  /**
   * \brief Node iterator
   */
  class SetExpr::NodeIter : public Iter::Ranges::Virt::Iterator {
  private:
    /// An iterator with virtual member functions for the node
    Iterator* i;
    /// Copy constructor not needed
    NodeIter(const NodeIter&);
  public:
    /// \name Constructor and destructor
    //@{
    /// Construct iterator
    NodeIter(const ViewArray<SetView>& x, SetExpr::Node* n,
	     int sign, bool monotone);
    /// Destructor
    virtual ~NodeIter(void);
    ///@}
    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    virtual void operator++(void);
    /// Test whether iterator is still at a range or done
    virtual bool operator()(void);
    //@}
    /// \name Range access
    //@{
    /// Return smallest value of range
    virtual int min(void) const;
    /// Return largest value of range
    virtual int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    virtual unsigned int width(void) const;
    ///@}
  };

  /*
   * Operations for nodes
   *
   */

  forceinline void*
  SetExpr::Node::operator new(size_t size) {
    return Memory::malloc(size);
  }

  forceinline void
  SetExpr::Node::operator delete(void* p, size_t) {
    Memory::free(p);
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

  std::string
  SetExpr::Node::toString(bool monotone) const {
    if (left==NULL && right==NULL) {
      char cidx[256];
      sprintf(cidx, "%d", x);
      std::string ret = "x[";
      ret += cidx;
      ret += "].";
      if (monotone)
	ret += "lub()";
      else
	ret += "glb()";
      return ret;
    }

    std::string ret = "";
    if (signLeft==-1)
      ret += "-(";
    if (left==NULL) {
      ret += (signLeft==-1) ? "e" : "u";
    } else {
      ret += left->toString( (signLeft==1) ? monotone : !monotone);
    }
    if (signLeft==-1)
      ret += ")";
    switch (rel) {
    case REL_INTER: ret += " && "; break;
    case REL_UNION: ret += " || "; break;
    default:
      assert(false);
      exit(2);
    }
    if (signRight==-1)
      ret += "-(";
    if (right==NULL) {
      ret += (signRight==1) ? "e" : "u";
    } else {
      ret += right->toString((signRight==1) ? monotone : !monotone);
    }
    if (signRight==-1)
      ret += ")";
    return ret;
  }

  SetExpr::proj_scope
  SetExpr::Node::scope(int sign) const {
    if (left==NULL && right==NULL) {
      proj_scope ps;
      ps[x] = (sign==1) ? PC_SET_CLUB : PC_SET_CGLB;
      return ps;
    }
    proj_scope ls =
      (left==NULL) ? proj_scope::map() : left->scope(sign*signLeft);
    proj_scope rs =
      (right==NULL) ? proj_scope::map() : right->scope(sign*signRight);
    return combineScopes(ls, rs);
  }

  int
  SetExpr::Node::arity(void) const {
    if (left==NULL && right==NULL)
      return x;
    return std::max( left==NULL ? 0 : left->arity(),
		     right==NULL ? 0 : right->arity() );
  }


  SetExpr::NodeIter::NodeIter(const ViewArray<SetView>& x, SetExpr::Node* n,
			      int sign, bool monotone) {
    if (n==NULL) {
      int mi = sign==1 ? 1 : Limits::Set::int_min;
      int ma = sign==1 ? 0 : Limits::Set::int_max;
      Iter::Ranges::Singleton s(mi, ma);
      i =
	new Iter::Ranges::Virt::RangesTemplate<Iter::Ranges::Singleton>(s);
      return;
    }
    if (n->left==NULL && n->right==NULL) {
      if (monotone) {
	if (sign==1) {
	  LubRanges<SetView> r(x[n->x]);
	  i = new Iter::Ranges::Virt::RangesTemplate<LubRanges<SetView> >(r);
	  return;
	} else {
	  LubRanges<SetView> r(x[n->x]);
	  RangesCompl<LubRanges<SetView> > c(r);
	  i = new Iter::Ranges::Virt::RangesTemplate<
	           RangesCompl<LubRanges<SetView> > >(c);
	  return;	  
	}
      } else {
	if (sign==1) {
	  GlbRanges<SetView> r(x[n->x]);
	  i = new Iter::Ranges::Virt::RangesTemplate<GlbRanges<SetView> >(r);
	  return;
	} else {
	  GlbRanges<SetView> r(x[n->x]);
	  RangesCompl<GlbRanges<SetView> > c(r);
	  i = new Iter::Ranges::Virt::RangesTemplate<
	           RangesCompl<GlbRanges<SetView> > >(c);
	  return;	  
	}
      }
      return;
    }
    NodeIter* li = new NodeIter(x, n->left, n->signLeft,
				n->signLeft==1 ? monotone : !monotone);
    NodeIter* ri = new NodeIter(x, n->right, n->signRight,
				n->signRight==1 ? monotone : !monotone);
    switch (n->rel) {
    case SetExpr::REL_INTER:
      i = new Iter::Ranges::Virt::Inter(li, ri);
      break;
    case SetExpr::REL_UNION:
      i = new Iter::Ranges::Virt::Union(li, ri);
      break;
    default:
      assert(false);
    }
    if (sign!=1) {
      i = new Iter::Ranges::Virt::Compl<Limits::Set::int_min,
	                                Limits::Set::int_max>(i);
    }
  }

  SetExpr::NodeIter::~NodeIter(void) { delete i; }

  forceinline void
  SetExpr::NodeIter::operator++(void) { ++(*i); }

  forceinline bool
  SetExpr::NodeIter::operator()(void) { return (*i)(); }

  forceinline int
  SetExpr::NodeIter::min(void) const { return i->min(); }

  forceinline int
  SetExpr::NodeIter::max(void) const { return i->max(); }

  forceinline unsigned int
  SetExpr::NodeIter::width(void) const { return i->width(); }

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

  std::string
  SetExpr::toString(bool monotone) const {
    if (ax==NULL)
      return (sign==1) ? "e" : "u";
    if (sign==-1) {
      std::string ret = "-(";
      ret += ax->toString(!monotone);
      ret += ")";
      return ret;
    }
    return ax->toString(monotone);
  }

  SetExpr::proj_scope
  SetExpr::scope(int sign) const {
    return (ax==NULL) ? proj_scope::map() : ax->scope(sign*this->sign);
  }

  int
  SetExpr::arity(void) const {
    return (ax==NULL) ? 0 : ax->arity();
  }

  SetExprRanges::SetExprRanges(const ViewArray<Set::SetView>& x,
			       SetExpr& s,
			       bool monotone) {
    i = new Iter(new SetExpr::NodeIter(x, s.ax, s.sign,
				       s.sign==1 ? monotone : !monotone));
  }

}

// STATISTICS: set-prop
