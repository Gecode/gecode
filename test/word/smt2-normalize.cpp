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

#include "test/word.hh"

// Exercise the actual typed syntax, lexical environments and inferred ranges.
// The factor helper depends on these reader internals. This scoped adapter
// avoids a second implementation of them and excludes the CLI entry point.
#define GECODE_WORD_SMT2_NO_MAIN
#include "benchmarks/word/smt2-reader.cpp"
#undef GECODE_WORD_SMT2_NO_MAIN

namespace Test { namespace Word { namespace SMT2Normalization {

  using Value=std::int64_t;
  using Values=std::unordered_map<std::string,Value>;

  /// Evaluate only the small arithmetic oracle inputs, without range analysis.
  bool evaluate(const SExpr& e, const EvalEnv& env, const Values& values,
                Value& result) {
    WordValue literal;
    if (signed_integer_literal(e,literal)) {
      result=literal >> 63 ? -Value(~literal)-1 : Value(literal);
      return true;
    }
    if (e.atom) {
      if (const EvalEnv::Binding* binding=env.find(e.text))
        return evaluate(*binding->term,*binding->outer,values,result);
      const auto found=values.find(e.text);
      if (found == values.end()) {
        olog << "Unknown oracle variable: " << e.text << '\n';
        return false;
      }
      result=found->second;
      return true;
    }
    if (is_atom(e.items[0],"let")) {
      EvalEnv nested(e,env);
      return evaluate(e.items[2],nested,values,result);
    }
    const std::string& op=e.items[0].text;
    if (!evaluate(e.items[1],env,values,result))
      return false;
    if (op == "-" && e.items.size() == 2) {
      result=-result;
      return true;
    }
    for (std::size_t i=2; i<e.items.size(); i++) {
      Value next;
      if (!evaluate(e.items[i],env,values,next))
        return false;
      if (op == "+") {
        result+=next;
      } else if (op == "-") {
        result-=next;
      } else if (op == "*") {
        result*=next;
      } else {
        olog << "Unsupported oracle operator: " << op << '\n';
        return false;
      }
    }
    return true;
  }

  struct Context {
    std::vector<ReaderQuery> queries;
    std::unique_ptr<IntegerBounds> bounds;
    EvalEnv env;
    std::unique_ptr<IntegerNormalization> normalizer;

    explicit Context(const std::string& text) {
      Parser parser(text);
      queries=read_session(parser.parse());
      bounds.reset(new IntegerBounds(queries[0].script));
      for (const auto& definition : queries[0].script.definitions)
        env.bindings.emplace(definition.first,
          EvalEnv::Binding(&definition.second,&env));
      normalizer.reset(new IntegerNormalization(*bounds));
    }
    const SExpr& expression(void) const {
      return queries[0].script.assertions.back().items[1];
    }
  };

  std::string script(const std::string& expression, const std::string& bounds,
                     const std::string& definitions="") {
    return "(declare-const x Int)(declare-const y Int)(declare-const z Int)"+
      definitions+bounds+"(assert (= "+expression+" 0))(check-sat)";
  }

  const std::string small=
    "(assert (<= (- 3) x))(assert (<= x 3))"
    "(assert (<= (- 3) y))(assert (<= y 3))"
    "(assert (<= (- 3) z))(assert (<= z 3))";

  /// Compare the accepted factor plan at every small input assignment.
  bool exhaustive(Context& context, const SExpr& expression,
                  const EvalEnv& env) {
    IntegerNormalization::Factor plan;
    if (!context.normalizer->factor(expression,env,plan)) {
      olog << "Expected the factor rule to accept this expression\n";
      return false;
    }
    for (int x=-3; x<=3; x++)
      for (int y=-3; y<=3; y++)
        for (int z=-3; z<=3; z++) {
          const Values values={{"x",x},{"y",y},{"z",z}};
          const auto product=[&](const std::vector<const SExpr*>& factors,
                                  Value& result) {
            result=1;
            for (const SExpr* factor : factors) {
              Value value;
              if (!evaluate(*factor,env,values,value))
                return false;
              result*=value;
            }
            return true;
          };
          Value left, right, common, original;
          if (!product(plan.left,left) || !product(plan.right,right) ||
              !evaluate(*plan.common,env,values,common) ||
              !evaluate(expression,env,values,original))
            return false;
          const Value normalized=common*(plan.subtract ? left-right : left+right);
          if (normalized != original) {
            olog << "x=" << x << " y=" << y << " z=" << z
                 << " original=" << original
                 << " normalized=" << normalized << '\n';
            return false;
          }
        }
    return true;
  }

  /// One algebraic shape, including lexical definitions or simultaneous let.
  class Identity : public Base {
  private:
    std::string expression, definitions;
    bool let_body;
  public:
    Identity(const std::string& name, const std::string& expression0,
             const std::string& definitions0="", bool let_body0=false)
      : Base("Word::SMT2::Normalize::Identity::"+name),
        expression(expression0), definitions(definitions0),
        let_body(let_body0) {}
    virtual bool run(void) {
      try {
        Context context(script(expression,small,definitions));
        if (let_body) {
          const SExpr& term=context.expression();
          EvalEnv nested(term,context.env);
          if (exhaustive(context,term.items[2],nested))
            return true;
        } else if (exhaustive(context,context.expression(),context.env)) {
          return true;
        }
        olog << "expression=" << expression << '\n';
        return false;
      } catch (const std::exception& error) {
        olog << "Unexpected fixture error: " << error.what() << '\n';
        return false;
      }
    }
  };

  /// One range-safety or construction-budget guard decision.
  class Guard : public Base {
  private:
    std::string expression, domain;
    bool expected;
  public:
    Guard(const std::string& name, const std::string& expression0,
          const std::string& domain0, bool expected0)
      : Base("Word::SMT2::Normalize::Guard::"+name),
        expression(expression0), domain(domain0), expected(expected0) {}
    virtual bool run(void) {
      try {
        Context context(script(expression,domain));
        IntegerNormalization::Factor plan;
        const bool actual=context.normalizer->factor(
          context.expression(),context.env,plan);
        if (actual != expected) {
          olog << "expression=" << expression << " domain=" << domain
               << " expected acceptance=" << expected
               << " actual acceptance=" << actual << '\n';
          return false;
        }
        return true;
      } catch (const std::exception& error) {
        olog << "Unexpected fixture error for " << expression
             << ": " << error.what() << '\n';
        return false;
      }
    }
  };

  /// A model's exhausted work budget must persist across later attempts.
  class Budget : public Base {
  public:
    Budget(void) : Base("Word::SMT2::Normalize::Budget") {}
    virtual bool run(void) {
      try {
        Context context(script("(- (* x y) (* x z))",small));
        IntegerNormalization::Factor plan;
        unsigned int attempts=0;
        while (attempts < 100000 && context.normalizer->factor(
                 context.expression(),context.env,plan))
          attempts++;
        if (attempts == 0 || attempts == 100000) {
          olog << "Unexpected attempts before budget exhaustion: "
               << attempts << '\n';
          return false;
        }
        if (context.normalizer->factor(context.expression(),context.env,plan)) {
          olog << "An exhausted model budget accepted another factor rule\n";
          return false;
        }
        return true;
      } catch (const std::exception& error) {
        olog << "Unexpected budget fixture error: " << error.what() << '\n';
        return false;
      }
    }
  } budget;

  class Create {
  public:
    Create(void) {
      const char* names[]={"CommonFirst","Reordered","Nested",
                           "BareRight","BareLeft","Repeated"};
      const std::pair<std::string,std::string> sides[]={
        {"(* x y)","(* x z)"}, {"(* y x)","(* z x)"},
        {"(* 4 (* x y))","(* 4 x)"}, {"(* x y)","x"},
        {"x","(* z x)"}, {"(* x x y)","(* x z)"}
      };
      for (const char* op : {"+","-"})
        for (unsigned int i=0; i<6; i++)
          (void) new Identity(std::string(op[0] == '+' ? "Add::" : "Sub::")+
            names[i],"("+std::string(op)+" "+sides[i].first+" "+
            sides[i].second+")");
      (void) new Identity("Definition","(- (* k y) (* k z))",
                         "(define-fun k () Int (+ x 1))");
      (void) new Identity("SimultaneousLet",
        "(let ((x y) (k x)) (- (* x z) (* z x)))","",true);
      (void) new Guard("UnknownRange","(- (* x y) (* x z))","",false);
      (void) new Guard("OriginalOverflow","(+ (* x 2) (* x 3))",
                       "(assert (= x 9223372036854775807))",false);
      (void) new Guard("ResidualOverflow","(- (* x y) (* x z))",
        "(assert (= x 0))(assert (= y (- 9223372036854775808)))"
        "(assert (= z 1))",false);
      (void) new Guard("SafeMinimum","(+ (* x 0) (* x 1))",
                       "(assert (= x (- 9223372036854775808)))",true);
      (void) new Guard("DivisionFallback","(- (* x (div y 0)) (* x z))",
                       small,false);
      std::string deep="y";
      for (int i=0; i<70; i++)
        deep="(- "+deep+")";
      (void) new Guard("DepthLimit","(- (* x "+deep+") (* x z))",small,false);
      std::string wide="(* x";
      for (int i=0; i<17; i++)
        wide+=" 1";
      wide+=")";
      (void) new Guard("FactorLimit","(- "+wide+" (* x y))",small,false);
    }
  } create;

}}}

// STATISTICS: test-word
