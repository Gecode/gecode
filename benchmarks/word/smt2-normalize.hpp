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
 *
 */

#ifndef GECODE_WORD_SMT2_NORMALIZE_HPP
#define GECODE_WORD_SMT2_NORMALIZE_HPP

// Included after the reader's typed syntax, finite intervals, and EvalEnv.
// This optional lowering rule does not mutate syntax or introduce symbols.
/// Construction-time factoring with checked finite-integer range proofs.
class IntegerNormalization {
public:
  /// A certified replacement whose syntax pointers borrow the input tree.
  struct Factor {
    const SExpr* common=nullptr;
    std::vector<const SExpr*> left, right;
    bool subtract=false;
  };
private:
  using Value=IntegerRange::Value;
  const IntegerBounds& bounds;
  std::size_t steps=0, attempt_steps=0;
  static constexpr std::size_t work_limit=100000;
  static constexpr std::size_t attempt_limit=2048;
  static constexpr unsigned int depth_limit=64;
  static constexpr std::size_t factor_limit=16;

  bool visit(unsigned int depth) {
    if (depth >= depth_limit || steps >= work_limit ||
        attempt_steps >= attempt_limit) return false;
    ++steps; ++attempt_steps;
    return true;
  }

  static bool combine(const std::string& op, IntegerRange a,
                      IntegerRange b, IntegerRange& result) {
    Value lo,hi;
    if (op == "*") {
      Value values[4];
      if (!IntegerRange::multiply(a.lo,b.lo,values[0]) ||
          !IntegerRange::multiply(a.lo,b.hi,values[1]) ||
          !IntegerRange::multiply(a.hi,b.lo,values[2]) ||
          !IntegerRange::multiply(a.hi,b.hi,values[3])) return false;
      result=IntegerRange(*std::min_element(values,values+4),
                          *std::max_element(values,values+4));
      return true;
    }
    if (op == "-") {
      if (b.lo == std::numeric_limits<Value>::min()) return false;
      b=IntegerRange(-b.hi,-b.lo);
    }
    if (!IntegerRange::add(a.lo,b.lo,lo) ||
        !IntegerRange::add(a.hi,b.hi,hi)) return false;
    result=IntegerRange(lo,hi);
    return true;
  }

  bool range(const SExpr& e, const EvalEnv& env, IntegerRange& result,
             unsigned int depth=0) {
    if (!visit(depth) || e.sort.kind != SORT_INT ||
        (e.atom && e.text.size() > 256)) return false;
    WordValue literal;
    if (signed_integer_literal(e,literal)) {
      const Value value=(literal >> 63) != 0 ? -Value(~literal)-1 : Value(literal);
      result=IntegerRange(value,value);
      return true;
    }
    if (e.atom) {
      if (const EvalEnv::Binding* binding=env.find(e.text))
        return range(*binding->term,*binding->outer,result,depth+1);
      result=bounds.range(e.text);
      return true;
    }
    if (is_atom(e.items[0],"let")) {
      // Bound environment construction as well as expression traversal.
      if (e.items[1].items.size() > factor_limit) return false;
      for (const SExpr& binding:e.items[1].items)
        if (!visit(depth) || binding.items[0].text.size() > 256) return false;
      // SMT let right-hand sides all use the outer environment.
      EvalEnv nested(e,env);
      return range(e.items[2],nested,result,depth+1);
    }
    if (!e.items[0].atom) return false;
    const std::string& op=e.items[0].text;
    if (op != "+" && op != "-" && op != "*") return false;
    if (!range(e.items[1],env,result,depth+1)) return false;
    if (op == "-" && e.items.size() == 2) {
      if (result.lo == std::numeric_limits<Value>::min()) return false;
      result=IntegerRange(-result.hi,-result.lo);
      return true;
    }
    for (std::size_t i=2; i<e.items.size(); i++) {
      IntegerRange next;
      if (!range(e.items[i],env,next,depth+1) ||
          !combine(op,result,next,result)) return false;
    }
    return true;
  }

  bool flatten(const SExpr& e, std::vector<const SExpr*>& factors,
               unsigned int depth=0) {
    if (!visit(depth)) return false;
    if (!e.atom && is_atom(e.items[0],"*")) {
      for (std::size_t i=1; i<e.items.size(); i++)
        if (!flatten(e.items[i],factors,depth+1)) return false;
      return true;
    }
    if (factors.size() == factor_limit) return false;
    factors.push_back(&e);
    return true;
  }

  bool product(const std::vector<const SExpr*>& factors, const EvalEnv& env,
               IntegerRange& result) {
    result=IntegerRange(1,1);
    for (const SExpr* factor:factors) {
      IntegerRange next;
      if (!range(*factor,env,next) || !combine("*",result,next,result)) return false;
    }
    return true;
  }

public:
  explicit IntegerNormalization(const IntegerBounds& bounds0) : bounds(bounds0) {}

  /// Populate a replacement on success; ignore result when returning false.
  /// The input must already have passed the reader's sort and arity checks.
  bool factor(const SExpr& e, const EvalEnv& env, Factor& result) {
    if (e.atom || e.sort.kind != SORT_INT || e.items.size() != 3 ||
        (!is_atom(e.items[0],"+") && !is_atom(e.items[0],"-")) ||
        steps >= work_limit) return false;
    // A bare repeated atom is deliberately outside this factor rule.
    if ((e.items[1].atom || !is_atom(e.items[1].items[0],"*")) &&
        (e.items[2].atom || !is_atom(e.items[2].items[0],"*"))) return false;
    attempt_steps=0;
    std::vector<const SExpr*> left,right;
    if (!flatten(e.items[1],left) || !flatten(e.items[2],right)) return false;
    // All candidates occur in this one lexical environment. Do not match
    // expressions through let boundaries or use names from another scope.
    std::size_t li=left.size(),ri=right.size();
    for (std::size_t i=0; i<left.size() && li==left.size(); i++) {
      WordValue literal;
      if (!left[i]->atom || left[i]->text.size() > 256 ||
          signed_integer_literal(*left[i],literal)) continue;
      for (std::size_t j=0; j<right.size(); j++)
        if (right[j]->atom && left[i]->text == right[j]->text &&
            !signed_integer_literal(*right[j],literal)) {
          li=i; ri=j; break;
        }
    }
    if (li == left.size()) return false;
    IntegerRange original,common,a,b,residual,replacement;
    // Certify the original evaluation order before reassociating anything.
    if (!range(e,env,original) || !range(*left[li],env,common)) return false;
    result.common=left[li];
    left.erase(left.begin()+li); right.erase(right.begin()+ri);
    result.subtract=is_atom(e.items[0],"-");
    if (!product(left,env,a) || !product(right,env,b) ||
        !combine(result.subtract ? "-" : "+",a,b,residual) ||
        !combine("*",common,residual,replacement)) return false;
    result.left=std::move(left); result.right=std::move(right);
    return true;
  }
};

#endif
