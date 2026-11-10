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
#include <gecode/minimodel/word-post.hh>
#include <cstdint>

namespace Gecode {

  class WordExpr::Node {
  public:
    typedef MiniModel::WordPostContext LoweringCache;

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
    WordVar post(Home home, WordDomainType domain_type=WDT_CUBE) const;
    WordVar post(Home& home, WordDomainType domain_type,
                 LoweringCache& cache) const;
    static void* operator new(size_t size);
    static void operator delete(void* p, size_t size);
  private:
    /// Lowered dependencies for one operation
    struct Operands {
      WordVar left, right, control;
      BoolVar bool_control;
    };
    Operands lower_operands(Home& home, WordDomainType domain_type,
                            LoweringCache& cache) const;
    BoolVar lower_boolean(Home& home, LoweringCache& cache) const;
    WordVar post_operation(Home& home, WordDomainType domain_type,
                           const Operands& operands) const;
    void post_unary(Home& home, WordVar operand, WordVar result) const;
    void post_binary(Home& home, const Operands& operands,
                     WordVar result) const;
  };

  WordExpr::Node::Node(void)
    : use(1), t(NT_VAR), l(nullptr), r(nullptr), c(nullptr),
      width(0), parameter(0), extent(0), value(0),
      bool_control(nullptr) {}

  WordExpr::Node::~Node(void) {
    delete bool_control;
  }

  int
  MiniModel::WordPostContext::slot(const Key& key) const {
    const unsigned int slot_count = 2U*static_cast<unsigned int>(entry_capacity);
    // Fibonacci hashing spreads aligned node addresses over the table.
    const std::uint64_t hash =
      (static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(key.node)) ^
       static_cast<std::uint64_t>(key.domain_type)) * 0x9e3779b97f4a7c15ULL;
    unsigned int i = static_cast<unsigned int>(hash >> 32) & (slot_count-1U);
    while ((slots[i] >= 0) && !(entry_array[slots[i]].first == key))
      i = (i+1U) & (slot_count-1U);
    return static_cast<int>(i);
  }

  void
  MiniModel::WordPostContext::grow(void) {
    const int capacity = (entry_capacity == 0) ? 16 : 2*entry_capacity;
    Entry* grown_entries = heap.alloc<Entry>(capacity);
    int* grown_slots;
    try {
      grown_slots = heap.alloc<int>(2*capacity);
    } catch (...) {
      heap.free<Entry>(grown_entries,capacity);
      throw;
    }
    for (int i=0; i<entry_count; i++)
      grown_entries[i] = entry_array[i];
    heap.free<Entry>(entry_array,entry_capacity);
    heap.free<int>(slots,2*entry_capacity);
    entry_array = grown_entries;
    entry_capacity = capacity;
    slots = grown_slots;
    for (int i=0; i<2*capacity; i++)
      slots[i] = -1;
    for (int i=0; i<entry_count; i++)
      slots[slot(entry_array[i].first)] = i;
  }

  const WordVar*
  MiniModel::WordPostContext::find(const Key& key) const {
    if (entry_count == 0)
      return nullptr;
    const int index = slots[slot(key)];
    return (index < 0) ? nullptr : &entry_array[index].second;
  }

  void
  MiniModel::WordPostContext::insert(const Key& key, WordVar value) {
    if (find(key) != nullptr)
      return;
    if (entry_count == entry_capacity)
      grow();
    slots[slot(key)] = entry_count;
    entry_array[entry_count++] = Entry(key,value);
    const_cast<WordExpr::Node*>(key.node)->use++;
  }

  MiniModel::WordPostContext::~WordPostContext(void) {
    for (const Entry& entry : entries()) {
      WordExpr::Node* node=const_cast<WordExpr::Node*>(entry.first.node);
      if (node->decrement())
        delete node;
    }
    heap.free<Entry>(entry_array,entry_capacity);
    heap.free<int>(slots,2*entry_capacity);
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

    /** \brief Return \a value as a constant of \a width bits
     *
     * Accepts a value that fits \a width as an unsigned value or whose 64-bit
     * two's-complement reading fits it as a signed value, and encodes it
     * modulo 2 to the \a width. Throws Word::OutOfLimits otherwise.
     */
    WordExpr
    constant_of_width(unsigned int width, WordValue value) {
      return WordExpr(width,Word::fit_to_width(width,value,"MiniModel::WordExpr"));
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

    using MiniModel::WordMisc;

    class WordRelation : public WordMisc {
    private:
      WordExpr left;
      WordExpr right;
      WordRelType wrt;
      WordDomainType domain_type;
    public:
      WordRelation(const WordExpr& left0, WordRelType wrt0,
                   const WordExpr& right0, WordDomainType domain_type0)
        : left(left0), right(right0), wrt(wrt0), domain_type(domain_type0) {
        check_width(left.width(),right.width());
      }
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&,
                        MiniModel::WordPostContext* context) {
        WordVar x = MiniModel::WordPostAccess::post(left,home,domain_type,context);
        WordVar y = MiniModel::WordPostAccess::post(right,home,domain_type,context);
        Gecode::rel(home,x,neg ? negated(wrt) : wrt,y,
                    Reify(b,RM_EQV));
      }
    };

    /// Post a positive Boolean result, then apply the requested polarity.
    template<class Post>
    void post_boolean(Home home, BoolVar result, bool is_negated,
                      const Post& post) {
      BoolVar actual = is_negated ? BoolVar(home,0,1) : result;
      post(actual);
      if (is_negated)
        Gecode::rel(home,result,IRT_NQ,actual);
    }

    class WordBit : public WordMisc {
    private:
      WordExpr word;
      unsigned int bit_index;
      WordDomainType domain_type;
    public:
      WordBit(const WordExpr& word0, unsigned int bit_index0,
              WordDomainType domain_type0)
        : word(word0), bit_index(bit_index0), domain_type(domain_type0) {
        if (bit_index >= word.width())
          throw Word::OutOfLimits("MiniModel::bit");
      }
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&,
                        MiniModel::WordPostContext* context) {
        WordVar x = MiniModel::WordPostAccess::post(word,home,domain_type,context);
        post_channel(home,x,b,neg);
      }
    private:
      // Keep channel-posting state off the mixed Word/Boolean recursive path.
      void post_channel(Home& home, WordVar x, BoolVar b, bool neg) const {
        post_boolean(home,b,neg,[&](BoolVar actual) {
          channel(home,x,bit_index,actual);
        });
      }
    };

    enum class WordReductionType { AND, OR, XOR };

    class WordReduction : public WordMisc {
    private:
      WordExpr word;
      WordReductionType reduction;
      WordDomainType domain_type;
    public:
      WordReduction(const WordExpr& word0, WordReductionType reduction0,
                    WordDomainType domain_type0)
        : word(word0), reduction(reduction0), domain_type(domain_type0) {}
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&,
                        MiniModel::WordPostContext* context) {
        WordVar x = MiniModel::WordPostAccess::post(word,home,domain_type,context);
        post_boolean(home,b,neg,[&](BoolVar actual) {
          switch (reduction) {
          case WordReductionType::AND: Gecode::reduce_and(home,x,actual); break;
          case WordReductionType::OR:  Gecode::reduce_or(home,x,actual); break;
          case WordReductionType::XOR: Gecode::reduce_xor(home,x,actual); break;
          default: GECODE_NEVER;
          }
        });
      }
    };

    class UnaryWordOverflow : public WordMisc {
    private:
      WordExpr word;
      WordOverflowType operation;
      WordDomainType domain_type;
    public:
      UnaryWordOverflow(const WordExpr& word0, WordOverflowType operation0,
                        WordDomainType domain_type0)
        : word(word0), operation(operation0), domain_type(domain_type0) {}
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&,
                        MiniModel::WordPostContext* context) {
        WordVar x = MiniModel::WordPostAccess::post(word,home,domain_type,context);
        post_boolean(home,b,neg,[&](BoolVar actual) {
          Gecode::overflow(home,x,operation,actual);
        });
      }
    };

    class BinaryWordOverflow : public WordMisc {
    private:
      WordExpr left;
      WordExpr right;
      WordOverflowType operation;
      WordDomainType domain_type;
    public:
      BinaryWordOverflow(const WordExpr& left0, WordOverflowType operation0,
                         const WordExpr& right0, WordDomainType domain_type0)
        : left(left0), right(right0), operation(operation0),
          domain_type(domain_type0) {
        check_width(left.width(),right.width());
      }
      virtual void post(Home home, BoolVar b, bool neg,
                        const IntPropLevels&,
                        MiniModel::WordPostContext* context) {
        WordVar x = MiniModel::WordPostAccess::post(left,home,domain_type,context);
        WordVar y = MiniModel::WordPostAccess::post(right,home,domain_type,context);
        post_boolean(home,b,neg,[&](BoolVar actual) {
          Gecode::overflow(home,x,operation,y,actual);
        });
      }
    };

    BoolExpr
    relation(const WordExpr& left, WordRelType wrt,
             const WordExpr& right) {
      return BoolExpr(new WordRelation(left,wrt,right,WDT_CUBE));
    }

    WordVar
    temporary(Home home, unsigned int width, WordDomainType domain_type) {
      return WordVar(home,width,domain_type);
    }
  }

  WordVar
  WordExpr::Node::post(Home home, WordDomainType domain_type) const {
    LoweringCache cache(home);
    return post(home,domain_type,cache);
  }

  BoolVar
  WordExpr::Node::lower_boolean(Home& home, LoweringCache& cache) const {
    PostInfo pi(home);
    return home.failed() ? BoolVar(home,0,1) :
      MiniModel::WordPostAccess::expr(*bool_control,home,IntPropLevels::def,
                                       &cache);
  }

  WordExpr::Node::Operands
  WordExpr::Node::lower_operands(Home& home, WordDomainType domain_type,
                                 LoweringCache& cache) const {
    Operands operands;
    // Boolean posting has a large Debug frame, unnecessary for word recursion.
    if (t == NT_BOOL_ITE)
      operands.bool_control=lower_boolean(home,cache);
    if (c != nullptr)
      operands.control=c->post(home,domain_type,cache);
    if (l != nullptr)
      operands.left=l->post(home,domain_type,cache);
    if (r != nullptr)
      operands.right=r->post(home,domain_type,cache);
    return operands;
  }

  void
  WordExpr::Node::post_unary(Home& home, WordVar operand,
                             WordVar result) const {
    switch (t) {
    case NT_COMPL:
      complement(home,operand,result); break;
    case NT_EXTRACT:
      Gecode::extract(home,operand,parameter,extent,result); break;
    case NT_REPEAT:
      Gecode::repeat(home,operand,parameter,result); break;
    case NT_ZERO_EXTEND:
      Gecode::zero_extend(home,operand,width,result); break;
    case NT_SIGN_EXTEND:
      Gecode::sign_extend(home,operand,width,result); break;
    case NT_SHIFT_LEFT:
      Gecode::shift_left(home,operand,parameter,result); break;
    case NT_LOGICAL_SHIFT_RIGHT:
      Gecode::logical_shift_right(home,operand,parameter,result); break;
    case NT_ARITHMETIC_SHIFT_RIGHT:
      Gecode::arithmetic_shift_right(home,operand,parameter,result); break;
    case NT_ROTATE_LEFT:
      Gecode::rotate_left(home,operand,parameter,result); break;
    case NT_ROTATE_RIGHT:
      Gecode::rotate_right(home,operand,parameter,result); break;
    case NT_NEG:
      Gecode::neg(home,operand,result); break;
    default:
      GECODE_NEVER;
    }
  }

  void
  WordExpr::Node::post_binary(Home& home, const Operands& operands,
                              WordVar result) const {
    WordVar left=operands.left, right=operands.right;
    switch (t) {
    case NT_AND: case NT_OR: case NT_XOR:
    case NT_NAND: case NT_NOR: case NT_XNOR:
      rel(home,left,operation(t),right,result); break;
    case NT_CONCAT:
      Gecode::concat(home,left,right,result); break;
    case NT_VAR_SHIFT_LEFT:
      Gecode::shift_left(home,left,right,result); break;
    case NT_VAR_LOGICAL_SHIFT_RIGHT:
      Gecode::logical_shift_right(home,left,right,result); break;
    case NT_VAR_ARITHMETIC_SHIFT_RIGHT:
      Gecode::arithmetic_shift_right(home,left,right,result); break;
    case NT_VAR_ROTATE_LEFT:
      Gecode::rotate_left(home,left,right,result); break;
    case NT_VAR_ROTATE_RIGHT:
      Gecode::rotate_right(home,left,right,result); break;
    case NT_ADD:
      Gecode::add(home,left,right,result); break;
    case NT_SUB:
      Gecode::sub(home,left,right,result); break;
    case NT_MULT:
      Gecode::mult(home,left,right,result); break;
    case NT_DIV:
      Gecode::div(home,left,right,result); break;
    case NT_MOD:
      Gecode::mod(home,left,right,result); break;
    case NT_SIGNED_DIV:
      Gecode::signed_div(home,left,right,result); break;
    case NT_SIGNED_REM:
      Gecode::signed_rem(home,left,right,result); break;
    case NT_SIGNED_MOD:
      Gecode::signed_mod(home,left,right,result); break;
    default:
      GECODE_NEVER;
    }
  }

  WordVar
  WordExpr::Node::post_operation(Home& home, WordDomainType domain_type,
                                 const Operands& operands) const {
    if (t == NT_VAR)
      return x;
    if (t == NT_CONST) {
      return WordVar(home,width,{value},domain_type);
    }

    WordVar result=temporary(home,width,domain_type);
    switch (t) {
    case NT_BOOL_ITE:
      ite(home,operands.bool_control,operands.left,operands.right,result);
      break;
    case NT_WORD_ITE:
      ite(home,operands.control,operands.left,operands.right,result);
      break;
    case NT_COMPL: case NT_EXTRACT: case NT_REPEAT:
    case NT_ZERO_EXTEND: case NT_SIGN_EXTEND:
    case NT_SHIFT_LEFT: case NT_LOGICAL_SHIFT_RIGHT:
    case NT_ARITHMETIC_SHIFT_RIGHT: case NT_ROTATE_LEFT:
    case NT_ROTATE_RIGHT: case NT_NEG:
      post_unary(home,operands.left,result); break;
    case NT_AND: case NT_OR: case NT_XOR:
    case NT_NAND: case NT_NOR: case NT_XNOR: case NT_CONCAT:
    case NT_VAR_SHIFT_LEFT: case NT_VAR_LOGICAL_SHIFT_RIGHT:
    case NT_VAR_ARITHMETIC_SHIFT_RIGHT:
    case NT_VAR_ROTATE_LEFT: case NT_VAR_ROTATE_RIGHT:
    case NT_ADD: case NT_SUB:
    case NT_MULT: case NT_DIV: case NT_MOD: case NT_SIGNED_DIV:
    case NT_SIGNED_REM: case NT_SIGNED_MOD:
      post_binary(home,operands,result); break;
    default:
      throw Word::UnknownOperation("MiniModel::WordExpr");
    }
    return result;
  }

  WordVar
  WordExpr::Node::post(Home& home, WordDomainType domain_type,
                       LoweringCache& cache) const {
    const LoweringCache::Key key = {this,domain_type};
    if (const WordVar* found = cache.find(key))
      return *found;

    // Operation posting must not lower expressions: its large Debug frame
    // must return before another dependency can recurse.
    const Operands operands=lower_operands(home,domain_type,cache);
    WordVar result=post_operation(home,domain_type,operands);
    cache.insert(key,result);
    return result;
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

  WordVar
  WordExpr::post(Home home, WordDomainType domain_type) const {
    return post(home,domain_type,nullptr);
  }

  WordVar
  WordExpr::post(Home home, WordDomainType domain_type,
                 MiniModel::WordPostContext* context) const {
    // Temporaries of this type carry integer reasoning, which cannot read a
    // non-integer rank order.
    switch (Word::integer_rank_order(domain_type)) {
    case Word::IntegerRankOrder::NONE:
    case Word::IntegerRankOrder::UNSIGNED:
    case Word::IntegerRankOrder::SIGNED:
      break;
    case Word::IntegerRankOrder::NON_INTEGER:
      throw Word::OutOfLimits("MiniModel::WordExpr::post");
    }
    if ((context != nullptr) && context->matches(home))
      return n->post(home,domain_type,*context);
    return n->post(home,domain_type);
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
  WordExpr operator &(const WordExpr& l, WordValue r) {
    return l & constant_of_width(l.width(),r);
  }
  WordExpr operator &(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) & r;
  }
  WordExpr operator |(const WordExpr& l, WordValue r) {
    return l | constant_of_width(l.width(),r);
  }
  WordExpr operator |(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) | r;
  }
  WordExpr operator ^(const WordExpr& l, WordValue r) {
    return l ^ constant_of_width(l.width(),r);
  }
  WordExpr operator ^(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) ^ r;
  }
  WordExpr nand(const WordExpr& l, WordValue r) {
    return nand(l,constant_of_width(l.width(),r));
  }
  WordExpr nand(WordValue l, const WordExpr& r) {
    return nand(constant_of_width(r.width(),l),r);
  }
  WordExpr nor(const WordExpr& l, WordValue r) {
    return nor(l,constant_of_width(l.width(),r));
  }
  WordExpr nor(WordValue l, const WordExpr& r) {
    return nor(constant_of_width(r.width(),l),r);
  }
  WordExpr xnor(const WordExpr& l, WordValue r) {
    return xnor(l,constant_of_width(l.width(),r));
  }
  WordExpr xnor(WordValue l, const WordExpr& r) {
    return xnor(constant_of_width(r.width(),l),r);
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
  BoolExpr operator ==(const WordExpr& l, WordValue r) {
    return l == constant_of_width(l.width(),r);
  }
  BoolExpr operator ==(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) == r;
  }
  BoolExpr operator !=(const WordExpr& l, WordValue r) {
    return l != constant_of_width(l.width(),r);
  }
  BoolExpr operator !=(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) != r;
  }
  BoolExpr operator <(const WordExpr& l, WordValue r) {
    return l < constant_of_width(l.width(),r);
  }
  BoolExpr operator <(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) < r;
  }
  BoolExpr operator <=(const WordExpr& l, WordValue r) {
    return l <= constant_of_width(l.width(),r);
  }
  BoolExpr operator <=(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) <= r;
  }
  BoolExpr operator >(const WordExpr& l, WordValue r) {
    return l > constant_of_width(l.width(),r);
  }
  BoolExpr operator >(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) > r;
  }
  BoolExpr operator >=(const WordExpr& l, WordValue r) {
    return l >= constant_of_width(l.width(),r);
  }
  BoolExpr operator >=(WordValue l, const WordExpr& r) {
    return constant_of_width(r.width(),l) >= r;
  }
  BoolExpr signed_less(const WordExpr& l, WordValue r) {
    return signed_less(l,constant_of_width(l.width(),r));
  }
  BoolExpr signed_less(WordValue l, const WordExpr& r) {
    return signed_less(constant_of_width(r.width(),l),r);
  }
  BoolExpr signed_less_equal(const WordExpr& l, WordValue r) {
    return signed_less_equal(l,constant_of_width(l.width(),r));
  }
  BoolExpr signed_less_equal(WordValue l, const WordExpr& r) {
    return signed_less_equal(constant_of_width(r.width(),l),r);
  }
  BoolExpr signed_greater(const WordExpr& l, WordValue r) {
    return signed_greater(l,constant_of_width(l.width(),r));
  }
  BoolExpr signed_greater(WordValue l, const WordExpr& r) {
    return signed_greater(constant_of_width(r.width(),l),r);
  }
  BoolExpr signed_greater_equal(const WordExpr& l, WordValue r) {
    return signed_greater_equal(l,constant_of_width(l.width(),r));
  }
  BoolExpr signed_greater_equal(WordValue l, const WordExpr& r) {
    return signed_greater_equal(constant_of_width(r.width(),l),r);
  }

  BoolExpr word_rel(const WordExpr& l, WordRelType wrt, const WordExpr& r,
                    WordDomainType domain_type) {
    return BoolExpr(new WordRelation(l,wrt,r,domain_type));
  }
  BoolExpr word_rel(const WordExpr& l, WordRelType wrt, WordValue r,
                    WordDomainType domain_type) {
    return word_rel(l,wrt,constant_of_width(l.width(),r),domain_type);
  }
  BoolExpr word_rel(WordValue l, WordRelType wrt, const WordExpr& r,
                    WordDomainType domain_type) {
    return word_rel(constant_of_width(r.width(),l),wrt,r,domain_type);
  }

  BoolExpr bit(const WordExpr& e, unsigned int bit_index) {
    return bit(e,bit_index,WDT_CUBE);
  }
  BoolExpr bit(const WordExpr& e, unsigned int bit_index,
               WordDomainType domain_type) {
    return BoolExpr(new WordBit(e,bit_index,domain_type));
  }

  BoolExpr reduce_and(const WordExpr& e) {
    return reduce_and(e,WDT_CUBE);
  }
  BoolExpr reduce_and(const WordExpr& e, WordDomainType domain_type) {
    return BoolExpr(new WordReduction(e,WordReductionType::AND,domain_type));
  }

  BoolExpr reduce_or(const WordExpr& e) {
    return reduce_or(e,WDT_CUBE);
  }
  BoolExpr reduce_or(const WordExpr& e, WordDomainType domain_type) {
    return BoolExpr(new WordReduction(e,WordReductionType::OR,domain_type));
  }

  BoolExpr reduce_xor(const WordExpr& e) {
    return reduce_xor(e,WDT_CUBE);
  }
  BoolExpr reduce_xor(const WordExpr& e, WordDomainType domain_type) {
    return BoolExpr(new WordReduction(e,WordReductionType::XOR,domain_type));
  }

  BoolExpr overflow(const WordExpr& e, WordOverflowType operation) {
    return overflow(e,operation,WDT_CUBE);
  }
  BoolExpr overflow(const WordExpr& e, WordOverflowType operation,
                    WordDomainType domain_type) {
    return BoolExpr(new UnaryWordOverflow(e,operation,domain_type));
  }

  BoolExpr overflow(const WordExpr& l, WordOverflowType operation,
                    const WordExpr& r) {
    return overflow(l,operation,r,WDT_CUBE);
  }
  BoolExpr overflow(const WordExpr& l, WordOverflowType operation,
                    const WordExpr& r, WordDomainType domain_type) {
    return BoolExpr(new BinaryWordOverflow(l,operation,r,domain_type));
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
  ite(const BoolExpr& control, const WordExpr& then_word, WordValue else_value) {
    return ite(control,then_word,constant_of_width(then_word.width(),else_value));
  }

  WordExpr
  ite(const BoolExpr& control, WordValue then_value, const WordExpr& else_word) {
    return ite(control,constant_of_width(else_word.width(),then_value),else_word);
  }

  WordExpr
  ite(const WordExpr& control, const WordExpr& then_word, WordValue else_value) {
    return ite(control,then_word,constant_of_width(then_word.width(),else_value));
  }

  WordExpr
  ite(const WordExpr& control, WordValue then_value, const WordExpr& else_word) {
    return ite(control,constant_of_width(else_word.width(),then_value),else_word);
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
  rotate_left(const WordExpr& e, const WordExpr& count) {
    check_width(e.width(),count.width());
    return WordExpr(e,WordExpr::NT_VAR_ROTATE_LEFT,count,e.width());
  }

  WordExpr
  rotate_left(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_ROTATE_LEFT,amount,0,e.width());
  }

  WordExpr
  rotate_right(const WordExpr& e, const WordExpr& count) {
    check_width(e.width(),count.width());
    return WordExpr(e,WordExpr::NT_VAR_ROTATE_RIGHT,count,e.width());
  }

  WordExpr
  rotate_right(const WordExpr& e, unsigned int amount) {
    return WordExpr(e,WordExpr::NT_ROTATE_RIGHT,amount,0,e.width());
  }

  WordExpr
  operator +(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_ADD,right,left.width());
  }

  WordExpr
  operator +(const WordExpr& left, WordValue right) {
    return left + constant_of_width(left.width(),right);
  }

  WordExpr
  operator +(WordValue left, const WordExpr& right) {
    return constant_of_width(right.width(),left) + right;
  }

  WordExpr
  operator -(const WordExpr& e) {
    return WordExpr(e,WordExpr::NT_NEG,0,0,e.width());
  }

  WordExpr
  operator -(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_SUB,right,left.width());
  }

  WordExpr
  operator -(const WordExpr& left, WordValue right) {
    return left - constant_of_width(left.width(),right);
  }

  WordExpr
  operator -(WordValue left, const WordExpr& right) {
    return constant_of_width(right.width(),left) - right;
  }

  WordExpr
  operator *(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_MULT,right,left.width());
  }

  WordExpr
  operator *(const WordExpr& left, WordValue right) {
    return left * constant_of_width(left.width(),right);
  }

  WordExpr
  operator *(WordValue left, const WordExpr& right) {
    return constant_of_width(right.width(),left) * right;
  }

  WordExpr
  div(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_DIV,right,left.width());
  }

  WordExpr
  div(const WordExpr& left, WordValue right) {
    return div(left,constant_of_width(left.width(),right));
  }

  WordExpr
  div(WordValue left, const WordExpr& right) {
    return div(constant_of_width(right.width(),left),right);
  }

  WordExpr
  mod(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_MOD,right,left.width());
  }

  WordExpr
  mod(const WordExpr& left, WordValue right) {
    return mod(left,constant_of_width(left.width(),right));
  }

  WordExpr
  mod(WordValue left, const WordExpr& right) {
    return mod(constant_of_width(right.width(),left),right);
  }

  WordExpr
  signed_div(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_SIGNED_DIV,right,left.width());
  }

  WordExpr
  signed_div(const WordExpr& left, WordValue right) {
    return signed_div(left,constant_of_width(left.width(),right));
  }

  WordExpr
  signed_div(WordValue left, const WordExpr& right) {
    return signed_div(constant_of_width(right.width(),left),right);
  }

  WordExpr
  signed_rem(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_SIGNED_REM,right,left.width());
  }

  WordExpr
  signed_rem(const WordExpr& left, WordValue right) {
    return signed_rem(left,constant_of_width(left.width(),right));
  }

  WordExpr
  signed_rem(WordValue left, const WordExpr& right) {
    return signed_rem(constant_of_width(right.width(),left),right);
  }

  WordExpr
  signed_mod(const WordExpr& left, const WordExpr& right) {
    check_width(left.width(),right.width());
    return WordExpr(left,WordExpr::NT_SIGNED_MOD,right,left.width());
  }

  WordExpr
  signed_mod(const WordExpr& left, WordValue right) {
    return signed_mod(left,constant_of_width(left.width(),right));
  }

  WordExpr
  signed_mod(WordValue left, const WordExpr& right) {
    return signed_mod(constant_of_width(right.width(),left),right);
  }

  WordVar
  expr(Home home, const WordExpr& e) {
    PostInfo pi(home);
    if (!home.failed())
      return e.post(home);
    return WordVar(home,e.width(),{0});
  }

}

#endif

// STATISTICS: minimodel-any
