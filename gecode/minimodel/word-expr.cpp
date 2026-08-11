/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

#include <gecode/minimodel.hh>

#ifdef GECODE_HAS_WORD_VARS

namespace Gecode {

  class WordExpr::Node {
  public:
    unsigned int use;
    NodeType t;
    Node *l, *r, *c;
    WordVar x;
    unsigned int width;
    unsigned int parameter;
    unsigned int extent;
    WordValue value;
    BoolExpr* bool_control;

    Node(void);
    ~Node(void);
    bool decrement(void);
    WordVar post(Home home) const;
    static void* operator new(size_t size);
    static void operator delete(void* p, size_t size);
  };

  WordExpr::Node::Node(void)
    : use(1), t(NT_VAR), l(nullptr), r(nullptr), c(nullptr),
      width(0), parameter(0), extent(0), value(0),
      bool_control(nullptr) {}

  WordExpr::Node::~Node(void) {
    delete bool_control;
  }

  void*
  WordExpr::Node::operator new(size_t size) {
#ifdef GECODE_HAS_FAULT_INJECTION
    Support::FailPoint::check(Support::FailPoint::Phase::MiniModel);
#endif
    return heap.ralloc(size);
  }

  void
  WordExpr::Node::operator delete(void* p, size_t) {
    heap.rfree(p);
  }

  bool
  WordExpr::Node::decrement(void) {
    if (--use == 0) {
      if ((l != nullptr) && l->decrement())
        delete l;
      if ((r != nullptr) && r->decrement())
        delete r;
      if ((c != nullptr) && c->decrement())
        delete c;
      return true;
    }
    return false;
  }

  namespace {
    void
    check_width(unsigned int left, unsigned int right) {
      if (left != right)
        throw Word::WidthMismatch("MiniModel::WordExpr");
    }

    WordOpType
    operation(WordExpr::NodeType t) {
      switch (t) {
      case WordExpr::NT_AND: return WOT_AND;
      case WordExpr::NT_OR: return WOT_OR;
      case WordExpr::NT_XOR: return WOT_XOR;
      case WordExpr::NT_NAND: return WOT_NAND;
      case WordExpr::NT_NOR: return WOT_NOR;
      case WordExpr::NT_XNOR: return WOT_XNOR;
      default: throw Word::UnknownOperation("MiniModel::WordExpr");
      }
    }

    WordRelType
    negated(WordRelType wrt) {
      switch (wrt) {
      case WRT_EQ: return WRT_NQ;
      case WRT_NQ: return WRT_EQ;
      case WRT_ULQ: return WRT_UGR;
      case WRT_ULE: return WRT_UGQ;
      case WRT_UGQ: return WRT_ULE;
      case WRT_UGR: return WRT_ULQ;
      case WRT_SLQ: return WRT_SGR;
      case WRT_SLE: return WRT_SGQ;
      case WRT_SGQ: return WRT_SLE;
      case WRT_SGR: return WRT_SLQ;
      default: throw Word::UnknownRelation("MiniModel::WordExpr");
      }
    }

    class WordRelation : public BoolExpr::Misc {
    private:
      WordExpr left;
      WordExpr right;
      WordRelType wrt;
    public:
      WordRelation(const WordExpr& left0, WordRelType wrt0,
                   const WordExpr& right0)
        : left(left0), right(right0), wrt(wrt0) {
        check_width(left.width(),right.width());
      }
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&) {
        WordVar x = left.post(home);
        WordVar y = right.post(home);
        Gecode::rel(home,x,neg ? negated(wrt) : wrt,y,
                    Reify(b,RM_EQV));
      }
    };

    class WordBit : public BoolExpr::Misc {
    private:
      WordExpr word;
      unsigned int bit_index;
    public:
      WordBit(const WordExpr& word0, unsigned int bit_index0)
        : word(word0), bit_index(bit_index0) {
        if (bit_index >= word.width())
          throw Word::OutOfLimits("MiniModel::bit");
      }
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&) {
        WordVar x = word.post(home);
        if (!neg) {
          channel(home,x,bit_index,b);
        } else {
          BoolVar actual(home,0,1);
          channel(home,x,bit_index,actual);
          Gecode::rel(home,b,IRT_NQ,actual);
        }
      }
    };

    BoolExpr
    relation(const WordExpr& left, WordRelType wrt,
             const WordExpr& right) {
      return BoolExpr(new WordRelation(left,wrt,right));
    }
  }

  WordVar
  WordExpr::Node::post(Home home) const {
    switch (t) {
    case NT_VAR:
      return x;
    case NT_CONST:
      return WordVar(home,width,value,value);
    case NT_COMPL: {
      WordVar operand = l->post(home);
      WordVar result(home,width);
      complement(home,operand,result);
      return result;
    }
    case NT_AND: case NT_OR: case NT_XOR:
    case NT_NAND: case NT_NOR: case NT_XNOR: {
      WordVar left = l->post(home);
      WordVar right = r->post(home);
      WordVar result(home,width);
      rel(home,left,operation(t),right,result);
      return result;
    }
    case NT_BOOL_ITE: {
      BoolVar control = expr(home,*bool_control);
      WordVar then_word = l->post(home);
      WordVar else_word = r->post(home);
      WordVar result(home,width);
      ite(home,control,then_word,else_word,result);
      return result;
    }
    case NT_WORD_ITE: {
      WordVar control = c->post(home);
      WordVar then_word = l->post(home);
      WordVar else_word = r->post(home);
      WordVar result(home,width);
      ite(home,control,then_word,else_word,result);
      return result;
    }
    case NT_EXTRACT: {
      WordVar operand = l->post(home);
      WordVar result(home,width);
      Gecode::extract(home,operand,parameter,extent,result);
      return result;
    }
    case NT_CONCAT: {
      WordVar high = l->post(home);
      WordVar low = r->post(home);
      WordVar result(home,width);
      Gecode::concat(home,high,low,result);
      return result;
    }
    case NT_REPEAT: {
      WordVar operand = l->post(home);
      WordVar result(home,width);
      Gecode::repeat(home,operand,parameter,result);
      return result;
    }
    case NT_ZERO_EXTEND: case NT_SIGN_EXTEND: {
      WordVar operand = l->post(home);
      WordVar result(home,width);
      if (t == NT_ZERO_EXTEND)
        Gecode::zero_extend(home,operand,width,result);
      else
        Gecode::sign_extend(home,operand,width,result);
      return result;
    }
    case NT_SHIFT_LEFT: case NT_LOGICAL_SHIFT_RIGHT:
    case NT_ARITHMETIC_SHIFT_RIGHT: case NT_ROTATE_LEFT:
    case NT_ROTATE_RIGHT: {
      WordVar operand = l->post(home);
      WordVar result(home,width);
      switch (t) {
      case NT_SHIFT_LEFT:
        Gecode::shift_left(home,operand,parameter,result); break;
      case NT_LOGICAL_SHIFT_RIGHT:
        Gecode::logical_shift_right(home,operand,parameter,result); break;
      case NT_ARITHMETIC_SHIFT_RIGHT:
        Gecode::arithmetic_shift_right(home,operand,parameter,result); break;
      case NT_ROTATE_LEFT:
        Gecode::rotate_left(home,operand,parameter,result); break;
      default:
        Gecode::rotate_right(home,operand,parameter,result); break;
      }
      return result;
    }
    case NT_VAR_SHIFT_LEFT: case NT_VAR_LOGICAL_SHIFT_RIGHT:
    case NT_VAR_ARITHMETIC_SHIFT_RIGHT: {
      WordVar operand = l->post(home);
      WordVar amount = r->post(home);
      WordVar result(home,width);
      switch (t) {
      case NT_VAR_SHIFT_LEFT:
        Gecode::shift_left(home,operand,amount,result); break;
      case NT_VAR_LOGICAL_SHIFT_RIGHT:
        Gecode::logical_shift_right(home,operand,amount,result); break;
      default:
        Gecode::arithmetic_shift_right(home,operand,amount,result); break;
      }
      return result;
    }
    default:
      throw Word::UnknownOperation("MiniModel::WordExpr");
    }
  }

  WordExpr::WordExpr(const WordExpr& e) : n(e.n) {
    n->use++;
  }

  WordExpr::WordExpr(const WordVar& x) : n(new Node) {
    n->t = NT_VAR;
    n->x = x;
    n->width = x.width();
  }

  WordExpr::WordExpr(unsigned int width, WordValue value) : n(nullptr) {
    Word::ConstWordView constant(width,value);
    n = new Node;
    n->t = NT_CONST;
    n->width = width;
    n->value = constant.val();
  }

  WordExpr::WordExpr(const WordExpr& l, NodeType t, const WordExpr& r)
    : n(nullptr) {
    check_width(l.width(),r.width());
    (void) operation(t);
    n = new Node;
    n->t = t;
    n->width = l.width();
    n->l = l.n;
    n->l->use++;
    n->r = r.n;
    n->r->use++;
  }

  WordExpr::WordExpr(const WordExpr& e, NodeType t) : n(nullptr) {
    if (t != NT_COMPL)
      throw Word::UnknownOperation("MiniModel::WordExpr");
    n = new Node;
    n->t = t;
    n->width = e.width();
    n->l = e.n;
    n->l->use++;
  }

  WordExpr::WordExpr(const WordExpr& e, NodeType t,
                     unsigned int parameter, unsigned int extent,
                     unsigned int result_width) : n(new Node) {
    n->t = t;
    n->width = result_width;
    n->parameter = parameter;
    n->extent = extent;
    n->l = e.n;
    n->l->use++;
  }

  WordExpr::WordExpr(const WordExpr& l, NodeType t, const WordExpr& r,
                     unsigned int result_width) : n(new Node) {
    n->t = t;
    n->width = result_width;
    n->l = l.n;
    n->l->use++;
    n->r = r.n;
    n->r->use++;
  }

  WordExpr::WordExpr(const BoolExpr& control, const WordExpr& then_word,
                     const WordExpr& else_word) : n(nullptr) {
    check_width(then_word.width(),else_word.width());
    BoolExpr* copied_control = new BoolExpr(control);
    try {
      n = new Node;
    } catch (...) {
      delete copied_control;
      throw;
    }
    n->t = NT_BOOL_ITE;
    n->width = then_word.width();
    n->bool_control = copied_control;
    n->l = then_word.n;
    n->l->use++;
    n->r = else_word.n;
    n->r->use++;
  }

  WordExpr::WordExpr(const WordExpr& control, const WordExpr& then_word,
                     const WordExpr& else_word) : n(nullptr) {
    check_width(control.width(),then_word.width());
    check_width(then_word.width(),else_word.width());
    n = new Node;
    n->t = NT_WORD_ITE;
    n->width = then_word.width();
    n->c = control.n;
    n->c->use++;
    n->l = then_word.n;
    n->l->use++;
    n->r = else_word.n;
    n->r->use++;
  }

  unsigned int
  WordExpr::width(void) const {
    return n->width;
  }

  WordVar
  WordExpr::post(Home home) const {
    return n->post(home);
  }

  const WordExpr&
  WordExpr::operator =(const WordExpr& e) {
    if (this != &e) {
      if (n->decrement())
        delete n;
      n = e.n;
      n->use++;
    }
    return *this;
  }

  WordExpr::~WordExpr(void) {
    if (n->decrement())
      delete n;
  }

  WordExpr operator ~(const WordExpr& e) {
    return WordExpr(e,WordExpr::NT_COMPL);
  }
  WordExpr operator &(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_AND,r);
  }
  WordExpr operator |(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_OR,r);
  }
  WordExpr operator ^(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_XOR,r);
  }
  WordExpr nand(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_NAND,r);
  }
  WordExpr nor(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_NOR,r);
  }
  WordExpr xnor(const WordExpr& l, const WordExpr& r) {
    return WordExpr(l,WordExpr::NT_XNOR,r);
  }

  BoolExpr operator ==(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_EQ,r);
  }
  BoolExpr operator !=(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_NQ,r);
  }
  BoolExpr operator <(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_ULE,r);
  }
  BoolExpr operator <=(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_ULQ,r);
  }
  BoolExpr operator >(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_UGR,r);
  }
  BoolExpr operator >=(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_UGQ,r);
  }
  BoolExpr signed_less(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_SLE,r);
  }
  BoolExpr signed_less_equal(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_SLQ,r);
  }
  BoolExpr signed_greater(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_SGR,r);
  }
  BoolExpr signed_greater_equal(const WordExpr& l, const WordExpr& r) {
    return relation(l,WRT_SGQ,r);
  }

  BoolExpr bit(const WordExpr& e, unsigned int bit_index) {
    return BoolExpr(new WordBit(e,bit_index));
  }

  WordExpr
  ite(const BoolExpr& control, const WordExpr& then_word,
      const WordExpr& else_word) {
    return WordExpr(control,then_word,else_word);
  }

  WordExpr
  ite(const WordExpr& control, const WordExpr& then_word,
      const WordExpr& else_word) {
    return WordExpr(control,then_word,else_word);
  }

  WordExpr
  extract(const WordExpr& e, unsigned int first, unsigned int width) {
    if ((width == 0) || (first >= e.width()) ||
        (width > e.width() - first))
      throw Word::OutOfLimits("MiniModel::extract");
    return WordExpr(e,WordExpr::NT_EXTRACT,first,width,width);
  }

  WordExpr
  concat(const WordExpr& high, const WordExpr& low) {
    if (high.width() > 64U - low.width())
      throw Word::WidthMismatch("MiniModel::concat");
    return WordExpr(high,WordExpr::NT_CONCAT,low,
                    high.width()+low.width());
  }

  WordExpr
  repeat(const WordExpr& e, unsigned int count) {
    if ((count == 0) || (count > 64U / e.width()))
      throw Word::OutOfLimits("MiniModel::repeat");
    return WordExpr(e,WordExpr::NT_REPEAT,count,0,e.width()*count);
  }

  WordExpr
  zero_extend(const WordExpr& e, unsigned int result_width) {
    if ((result_width < e.width()) || (result_width > 64U))
      throw Word::OutOfLimits("MiniModel::zero_extend");
    return WordExpr(e,WordExpr::NT_ZERO_EXTEND,0,0,result_width);
  }

  WordExpr
  sign_extend(const WordExpr& e, unsigned int result_width) {
    if ((result_width < e.width()) || (result_width > 64U))
      throw Word::OutOfLimits("MiniModel::sign_extend");
    return WordExpr(e,WordExpr::NT_SIGN_EXTEND,0,0,result_width);
  }

  WordExpr
  operator <<(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_SHIFT_LEFT,amount,0,e.width());
  }

  WordExpr
  operator <<(const WordExpr& e, const WordExpr& amount) {
    check_width(e.width(),amount.width());
    return WordExpr(e,WordExpr::NT_VAR_SHIFT_LEFT,amount,e.width());
  }

  WordExpr
  logical_shift_right(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_LOGICAL_SHIFT_RIGHT,amount,0,e.width());
  }

  WordExpr
  logical_shift_right(const WordExpr& e, const WordExpr& amount) {
    check_width(e.width(),amount.width());
    return WordExpr(e,WordExpr::NT_VAR_LOGICAL_SHIFT_RIGHT,amount,e.width());
  }

  WordExpr
  arithmetic_shift_right(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_ARITHMETIC_SHIFT_RIGHT,amount,0,e.width());
  }

  WordExpr
  arithmetic_shift_right(const WordExpr& e, const WordExpr& amount) {
    check_width(e.width(),amount.width());
    return WordExpr(e,WordExpr::NT_VAR_ARITHMETIC_SHIFT_RIGHT,amount,
                    e.width());
  }

  WordExpr
  rotate_left(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_ROTATE_LEFT,amount,0,e.width());
  }

  WordExpr
  rotate_right(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_ROTATE_RIGHT,amount,0,e.width());
  }

  WordVar
  expr(Home home, const WordExpr& e) {
    PostInfo pi(home);
    if (!home.failed())
      return e.post(home);
    return WordVar(home,e.width(),0,0);
  }

}

#endif

// STATISTICS: minimodel-any
