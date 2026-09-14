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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <gecode/word.hh>

#include "smt2-zero-functions.hpp"
#include "smt2-cardinality.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace Gecode;

namespace {

  // This benchmark reader uses recursive parsing, typing, and MiniModel
  // lowering. Bound both source nesting and the chains produced by n-ary
  // operators (distinct lowers to a conjunction of all operand pairs).
  const std::size_t max_input_bytes=16U*1024U*1024U;
  const unsigned int max_nesting=128;
  const unsigned int max_expansion_depth=1024;
  const unsigned int max_syntax_nodes=100000;

  struct Error : public std::runtime_error {
    explicit Error(const std::string& message) : std::runtime_error(message) {}
  };

  enum SortKind { SORT_BOOL, SORT_WORD, SORT_INT };

  struct Sort {
    SortKind kind;
    unsigned int width;
    Sort(SortKind kind0=SORT_INT, unsigned int width0=0)
      : kind(kind0), width(width0) {}
  };

  struct SExpr {
    enum Token { SIMPLE, QUOTED, STRING };
    bool atom;
    Token token;
    std::string text;
    std::vector<SExpr> items;
    unsigned int line;
    unsigned int expansion_depth=0;
    // Filled once during type checking; lexical scope is fixed per occurrence.
    mutable Sort sort;

    SExpr(std::string text0, unsigned int line0, Token token0=SIMPLE)
      : atom(true), token(token0), text(std::move(text0)), line(line0) {}
    SExpr(std::vector<SExpr> items0, unsigned int line0)
      : atom(false), token(SIMPLE), items(std::move(items0)), line(line0) {}
  };

  class Parser {
  private:
    const std::string& input;
    std::size_t position;
    unsigned int line;
    unsigned int nodes=0;

    void whitespace(void) {
      while (position < input.size()) {
        if (input[position] == ';') {
          while ((position < input.size()) && (input[position] != '\n'))
            position++;
        } else if (std::isspace(static_cast<unsigned char>(input[position]))) {
          if (input[position] == '\n') line++;
          position++;
        } else {
          break;
        }
      }
    }

    SExpr expression(unsigned int depth=0) {
      whitespace();
      if (depth > max_nesting)
        throw Error("reader nesting limit is 128 on line " +
                    std::to_string(line));
      if (++nodes > max_syntax_nodes)
        throw Error("reader syntax-node limit is 100000");
      if (position == input.size())
        throw Error("unexpected end of input");
      const unsigned int start_line=line;
      if (input[position] == '(') {
        position++;
        std::vector<SExpr> items;
        unsigned int child_depth=0;
        for (;;) {
          whitespace();
          if (position == input.size())
            throw Error("unterminated list starting on line " +
                        std::to_string(start_line));
          if (input[position] == ')') {
            position++;
            const std::size_t n=items.empty() ? 0 : items.size()-1;
            const bool distinct=!items.empty() && items[0].atom &&
              (items[0].text == "distinct");
            const std::size_t expansion=child_depth+
              (distinct && (n > 1) ? n*(n-1)/2 : items.size());
            if (expansion > max_expansion_depth)
              throw Error("reader expression expansion-depth limit is 1024 "
                          "on line " + std::to_string(start_line));
            SExpr result(std::move(items),start_line);
            result.expansion_depth=static_cast<unsigned int>(expansion);
            return result;
          }
          if (items.size() >= max_expansion_depth)
            throw Error("reader list-length limit is 1024 on line " +
                        std::to_string(start_line));
          items.push_back(expression(depth+1));
          if (items.back().expansion_depth > child_depth)
            child_depth=items.back().expansion_depth;
        }
      }
      if (input[position] == ')')
        throw Error("unexpected ')' on line " + std::to_string(line));
      std::string token;
      SExpr::Token kind=SExpr::SIMPLE;
      if (input[position] == '"') {
        kind=SExpr::STRING;
        token += input[position++];
        bool closed=false;
        while (position < input.size()) {
          char c=input[position++];
          token += c;
          if (c == '\n') line++;
          if (c == '"') {
            if ((position < input.size()) && (input[position] == '"'))
              token += input[position++]; // SMT-LIB escapes quotes by doubling.
            else { closed=true; break; }
          }
        }
        if (!closed)
          throw Error("unterminated string on line " +
                      std::to_string(start_line));
      } else if (input[position] == '|') {
        kind=SExpr::QUOTED;
        position++;
        while ((position < input.size()) && (input[position] != '|')) {
          if (input[position] == '\\')
            throw Error("backslash in quoted symbol on line " +
                        std::to_string(line));
          if (input[position] == '\n') line++;
          token += input[position++];
        }
        if (position == input.size())
          throw Error("unterminated quoted symbol on line " +
                      std::to_string(start_line));
        position++;
      } else {
        while ((position < input.size()) &&
               !std::isspace(static_cast<unsigned char>(input[position])) &&
               (input[position] != '(') && (input[position] != ')') &&
               (input[position] != ';'))
          token += input[position++];
      }
      if (token.empty() && (kind != SExpr::QUOTED))
        throw Error("empty token on line " + std::to_string(start_line));
      return SExpr(std::move(token),start_line,kind);
    }

  public:
    explicit Parser(const std::string& input0)
      : input(input0), position(0), line(1) {}

    std::vector<SExpr> parse(void) {
      std::vector<SExpr> result;
      for (;;) {
        whitespace();
        if (position == input.size()) return result;
        result.push_back(expression());
      }
    }
  };

  bool same_sort(const Sort& a, const Sort& b) {
    return (a.kind == b.kind) &&
      ((a.kind != SORT_WORD) || (a.width == b.width));
  }

  unsigned int unsigned_atom(const SExpr& e, const char* context) {
    if (!e.atom || (e.token != SExpr::SIMPLE) || e.text.empty())
      throw Error(std::string(context) +
                  " expects an unsigned integer on line " +
                  std::to_string(e.line));
    unsigned int value=0;
    for (char c : e.text) {
      if ((c < '0') || (c > '9') ||
          (value > (std::numeric_limits<unsigned int>::max() - (c-'0'))/10U))
        throw Error(std::string(context) + " has an invalid integer on line " +
                    std::to_string(e.line));
      value=value*10U+static_cast<unsigned int>(c-'0');
    }
    return value;
  }

  std::string head(const SExpr& e) {
    if (e.atom || e.items.empty() || !e.items[0].atom)
      throw Error("expected a command or application on line " +
                  std::to_string(e.line));
    return e.items[0].text;
  }

  Sort parse_sort(const SExpr& e) {
    if (e.atom) {
      if (e.text == "Bool") return Sort(SORT_BOOL);
      if (e.text == "Int") return Sort(SORT_INT);
    } else if ((e.items.size() == 3) && e.items[0].atom &&
               (e.items[0].text == "_") && e.items[1].atom &&
               (e.items[1].text == "BitVec")) {
      unsigned int width=unsigned_atom(e.items[2],"BitVec sort");
      if ((width == 0U) || (width > 64U))
        throw Error("WordVar reader supports bit-vector widths 1..64 on line " +
                    std::to_string(e.line));
      return Sort(SORT_WORD,width);
    }
    throw Error("unsupported sort on line " + std::to_string(e.line));
  }

  struct Decl {
    std::string name;
    Sort sort;
  };

  struct Script {
    enum Objective { SATISFY, MAXIMIZE, MINIMIZE };
    std::vector<Decl> declarations;
    std::vector<std::pair<std::string,SExpr> > definitions;
    std::vector<SExpr> assertions;
    std::unordered_map<std::string,Sort> globals;
    Objective objective=SATISFY;
    std::unique_ptr<SExpr> objective_term;
  };

  bool is_atom(const SExpr& e, const char* value) {
    return e.atom && (e.text == value);
  }

  unsigned int rotation_amount(const SExpr& e, unsigned int width) {
    if (!e.atom || (e.token != SExpr::SIMPLE) || e.text.empty())
      throw Error("rotation expects an unsigned numeral");
    unsigned int value=0;
    for (char c : e.text) {
      if ((c < '0') || (c > '9'))
        throw Error("rotation expects an unsigned numeral");
      value=(value*10U+static_cast<unsigned int>(c-'0'))%width;
    }
    return value;
  }

  bool literal_sort(const std::string& token, Sort& sort) {
    if ((token.size() > 2) && (token[0] == '#') && (token[1] == 'b')) {
      sort=Sort(SORT_WORD,static_cast<unsigned int>(token.size()-2));
      return true;
    }
    if ((token.size() > 2) && (token[0] == '#') && (token[1] == 'x')) {
      sort=Sort(SORT_WORD,static_cast<unsigned int>(4*(token.size()-2)));
      return true;
    }
    return false;
  }

  WordValue word_literal(const std::string& token);

  bool integer_magnitude(const SExpr& e, WordValue& value) {
    if (!e.atom || (e.token != SExpr::SIMPLE) || e.text.empty())
      return false;
    value=0;
    const WordValue maximum=WordValue(1) << 63;
    for (char c : e.text) {
      if ((c < '0') || (c > '9'))
        return false;
      const unsigned int digit=static_cast<unsigned int>(c-'0');
      if (value > (maximum-digit)/10U)
        throw Error("integer literal is outside the supported 64-bit range on "
                    "line " + std::to_string(e.line));
      value=value*10U+digit;
    }
    return true;
  }

  bool integer_literal(const SExpr& e, WordValue& value) {
    if (!integer_magnitude(e,value)) return false;
    if (value >= (WordValue(1) << 63))
      throw Error("nonnegative integer literal is outside signed 64-bit range "
                  "on line " + std::to_string(e.line));
    return true;
  }

  bool signed_integer_literal(const SExpr& e, WordValue& value) {
    if (e.atom) return integer_literal(e,value);
    if ((e.items.size() != 2) || !is_atom(e.items[0],"-")) return false;
    WordValue magnitude;
    if (!integer_magnitude(e.items[1],magnitude)) return false;
    value=WordValue(0)-magnitude;
    return true;
  }

  bool decimal_literal(const SExpr& e, Sort& sort, WordValue& value) {
    if (e.atom || e.items.empty() || !is_atom(e.items[0],"_"))
      return false;
    if ((e.items.size() != 3) || !e.items[1].atom ||
        (e.items[1].text.size() <= 2) ||
        (e.items[1].text.compare(0,2,"bv") != 0))
      throw Error("malformed decimal bit-vector literal");
    unsigned int width=unsigned_atom(e.items[2],"bit-vector literal width");
    if ((width == 0) || (width > 64))
      throw Error("bit-vector literal width is outside 1..64");
    const WordValue maximum=(width == 64) ?
      std::numeric_limits<WordValue>::max() : (WordValue(1) << width)-1;
    value=0;
    for (std::size_t i=2; i<e.items[1].text.size(); i++) {
      char c=e.items[1].text[i];
      if ((c < '0') || (c > '9'))
        throw Error("invalid decimal bit-vector literal");
      const unsigned int digit=static_cast<unsigned int>(c-'0');
      if ((digit > maximum) || (value > (maximum-digit)/10U))
        throw Error("decimal bit-vector literal exceeds its width");
      value=value*10U+digit;
    }
    sort=Sort(SORT_WORD,width);
    return true;
  }

  struct TypeEnv {
    const TypeEnv* parent;
    std::unordered_map<std::string,Sort> local;
    explicit TypeEnv(const TypeEnv* parent0=nullptr) : parent(parent0) {}
    bool find(const std::string& name, Sort& result) const {
      auto i=local.find(name);
      if (i != local.end()) { result=i->second; return true; }
      return (parent != nullptr) && parent->find(name,result);
    }
  };

  Sort infer(const SExpr& e, const TypeEnv& env);

  /// Reject bindings that would shadow a supported theory function symbol.
  void require_binding_symbol(const SExpr& e) {
    if (!e.atom || (e.token == SExpr::STRING))
      throw Error("binding expects a symbol on line " +
                  std::to_string(e.line));
    // Quoting does not change symbol identity: |true| and true both name
    // the Core theory constant. User names and theory names cannot overlap.
    static const std::unordered_set<std::string> theory_symbols={
      "true", "false", "not", "and", "or", "xor", "=>", "=", "distinct",
      "ite", "+", "-", "*", "div", "mod", "<", "<=", ">", ">=",
      "bvule", "bvult", "bvuge", "bvugt", "bvsle", "bvslt", "bvsge",
      "bvsgt", "bvneg", "bvnot", "concat", "bvadd", "bvmul", "bvand",
      "bvor", "bvxor", "bvsub", "bvsdiv", "bvsmod", "bvudiv", "bvurem",
      "bvshl", "bvlshr", "bvashr"
    };
    if (theory_symbols.find(e.text) != theory_symbols.end())
      throw Error("binding shadows theory symbol '" + e.text + "' on line " +
                  std::to_string(e.line));
  }

  std::vector<std::pair<std::string,const SExpr*> > let_bindings(
    const SExpr& e) {
    if ((e.items.size() != 3) || e.items[1].atom)
      throw Error("malformed let on line " + std::to_string(e.line));
    std::vector<std::pair<std::string,const SExpr*> > result;
    for (const SExpr& binding : e.items[1].items) {
      if (binding.atom || (binding.items.size() != 2) ||
          !binding.items[0].atom)
        throw Error("malformed let binding on line " +
                    std::to_string(binding.line));
      result.push_back(std::make_pair(binding.items[0].text,
                                      &binding.items[1]));
    }
    return result;
  }

  void require_arity(const SExpr& e, std::size_t arity) {
    if (e.items.size() != arity+1)
      throw Error(head(e) + " expects " + std::to_string(arity) +
                  " arguments on line " + std::to_string(e.line));
  }

  Sort infer_term(const SExpr& e, const TypeEnv& env) {
    WordValue integer;
    if (signed_integer_literal(e,integer)) return Sort(SORT_INT);
    if (e.atom) {
      if (e.token == SExpr::STRING)
        throw Error("string is not a Bool or bit-vector term");
      if ((e.text == "true") || (e.text == "false")) return Sort(SORT_BOOL);
      Sort result;
      if ((e.token == SExpr::SIMPLE) && literal_sort(e.text,result)) {
        if ((result.width == 0U) || (result.width > 64U))
          throw Error("bit-vector literal width is outside 1..64 on line " +
                      std::to_string(e.line));
        word_literal(e.text);
        return result;
      }
      if (env.find(e.text,result)) return result;
      throw Error("unknown symbol '" + e.text + "' on line " +
                  std::to_string(e.line));
    }
    if (e.items.empty())
      throw Error("empty application on line " + std::to_string(e.line));
    Sort literal;
    WordValue value;
    if (decimal_literal(e,literal,value)) return literal;

    if (!e.items[0].atom) {
      const SExpr& indexed=e.items[0];
      if ((indexed.items.size() < 2) || !is_atom(indexed.items[0],"_") ||
          !indexed.items[1].atom)
        throw Error("unsupported indexed operator on line " +
                    std::to_string(e.line));
      require_arity(e,1);
      Sort source=infer(e.items[1],env);
      if (source.kind != SORT_WORD)
        throw Error("indexed word operator has a non-word operand on line " +
                    std::to_string(e.line));
      if ((indexed.items[1].text == "rotate_left") ||
          (indexed.items[1].text == "rotate_right")) {
        if (indexed.items.size() != 3)
          throw Error("rotation expects one index");
        rotation_amount(indexed.items[2],source.width);
        return source;
      }
      if (indexed.items[1].text == "extract") {
        if (indexed.items.size() != 4)
          throw Error("extract expects high and low indices on line " +
                      std::to_string(e.line));
        unsigned int high=unsigned_atom(indexed.items[2],"extract");
        unsigned int low=unsigned_atom(indexed.items[3],"extract");
        if ((low > high) || (high >= source.width))
          throw Error("extract indices are outside the operand on line " +
                      std::to_string(e.line));
        return Sort(SORT_WORD,high-low+1U);
      }
      if ((indexed.items[1].text == "zero_extend") ||
          (indexed.items[1].text == "sign_extend")) {
        const std::string& op=indexed.items[1].text;
        if (indexed.items.size() != 3)
          throw Error(op + " expects one index on line " +
                      std::to_string(e.line));
        unsigned int extra=unsigned_atom(indexed.items[2],op.c_str());
        if ((extra > 64U) || (source.width+extra > 64U))
          throw Error(op + " result exceeds 64 bits on line " +
                      std::to_string(e.line));
        return Sort(SORT_WORD,source.width+extra);
      }
      throw Error("unsupported indexed operator '" + indexed.items[1].text +
                  "' on line " + std::to_string(e.line));
    }

    const std::string op=e.items[0].text;
    if (op == "let") {
      TypeEnv nested(&env);
      const auto bindings=let_bindings(e);
      for (const SExpr& binding : e.items[1].items)
        require_binding_symbol(binding.items[0]);
      for (const auto& binding : bindings) {
        Sort value=infer(*binding.second,env); // SMT-LIB let is simultaneous.
        if (!nested.local.emplace(binding.first,value).second)
          throw Error("duplicate let binding '" + binding.first + "' on line " +
                      std::to_string(e.line));
      }
      return infer(e.items[2],nested);
    }
    if (op == "not") { require_arity(e,1); Sort s=infer(e.items[1],env);
      if (s.kind != SORT_BOOL) throw Error("not expects Bool"); return s; }
    if ((op == "and") || (op == "or") || (op == "xor") || (op == "=>")) {
      if (((op == "xor") || (op == "=>")) && (e.items.size() < 3))
        throw Error(op + " expects at least two arguments");
      for (std::size_t i=1; i<e.items.size(); i++)
        if (infer(e.items[i],env).kind != SORT_BOOL)
          throw Error(op + " expects Bool arguments on line " +
                      std::to_string(e.line));
      return Sort(SORT_BOOL);
    }
    if ((op == "=") || (op == "distinct")) {
      if (e.items.size() < 3)
        throw Error(op + " expects at least two arguments on line " +
                    std::to_string(e.line));
      Sort first=infer(e.items[1],env);
      for (std::size_t i=2; i<e.items.size(); i++)
        if (!same_sort(first,infer(e.items[i],env)))
          throw Error(op + " has operands of different sorts on line " +
                      std::to_string(e.line));
      return Sort(SORT_BOOL);
    }
    if (op == "ite") {
      require_arity(e,3);
      if (infer(e.items[1],env).kind != SORT_BOOL)
        throw Error("ite condition is not Bool on line " +
                    std::to_string(e.line));
      Sort then_sort=infer(e.items[2],env), else_sort=infer(e.items[3],env);
      if (!same_sort(then_sort,else_sort))
        throw Error("ite branches have unsupported or different sorts on "
                    "line " + std::to_string(e.line));
      return then_sort;
    }
    if ((op == "<") || (op == "<=") || (op == ">") || (op == ">=")) {
      require_arity(e,2);
      Sort a=infer(e.items[1],env), b=infer(e.items[2],env);
      if ((a.kind != SORT_INT) || !same_sort(a,b))
        throw Error(op + " expects integer operands on line " +
                    std::to_string(e.line));
      return Sort(SORT_BOOL);
    }
    if ((op == "bvule") || (op == "bvult") || (op == "bvuge") ||
        (op == "bvugt") || (op == "bvsle") || (op == "bvslt") ||
        (op == "bvsge") || (op == "bvsgt")) {
      require_arity(e,2);
      Sort a=infer(e.items[1],env), b=infer(e.items[2],env);
      if ((a.kind != SORT_WORD) || !same_sort(a,b))
        throw Error(op + " expects equal-width words on line " +
                    std::to_string(e.line));
      return Sort(SORT_BOOL);
    }
    if ((op == "bvneg") || (op == "bvnot")) {
      require_arity(e,1); Sort s=infer(e.items[1],env);
      if (s.kind != SORT_WORD) throw Error(op + " expects a word"); return s;
    }
    if (op == "concat") {
      require_arity(e,2);
      Sort a=infer(e.items[1],env), b=infer(e.items[2],env);
      if ((a.kind != SORT_WORD) || (b.kind != SORT_WORD) ||
          (a.width+b.width > 64U))
        throw Error("concat expects words with total width at most 64");
      return Sort(SORT_WORD,a.width+b.width);
    }
    if ((op == "+") || (op == "*")) {
      if (e.items.size() < 3)
        throw Error(op + " expects at least two arguments on line " +
                    std::to_string(e.line));
      for (std::size_t i=1; i<e.items.size(); i++)
        if (infer(e.items[i],env).kind != SORT_INT)
          throw Error(op + " expects integer operands on line " +
                      std::to_string(e.line));
      return Sort(SORT_INT);
    }
    if (op == "-") {
      if ((e.items.size() != 2) && (e.items.size() != 3))
        throw Error("- expects one or two arguments on line " +
                    std::to_string(e.line));
      for (std::size_t i=1; i<e.items.size(); i++)
        if (infer(e.items[i],env).kind != SORT_INT)
          throw Error("- expects integer operands on line " +
                      std::to_string(e.line));
      return Sort(SORT_INT);
    }
    if ((op == "div") || (op == "mod")) {
      require_arity(e,2);
      Sort a=infer(e.items[1],env), b=infer(e.items[2],env);
      if ((a.kind != SORT_INT) || !same_sort(a,b))
        throw Error(op + " expects integer operands on line " +
                    std::to_string(e.line));
      return Sort(SORT_INT);
    }
    if ((op == "bvadd") || (op == "bvmul") || (op == "bvand") ||
        (op == "bvor") || (op == "bvxor")) {
      if (e.items.size() < 3)
        throw Error(op + " expects at least two arguments on line " +
                    std::to_string(e.line));
      Sort first=infer(e.items[1],env);
      for (std::size_t i=2; i<e.items.size(); i++)
        if ((first.kind != SORT_WORD) ||
            !same_sort(first,infer(e.items[i],env)))
          throw Error(op + " expects equal-width words on line " +
                      std::to_string(e.line));
      return first;
    }
    if ((op == "bvsub") || (op == "bvsdiv") || (op == "bvsmod") ||
        (op == "bvsmod_i") || (op == "bvudiv") || (op == "bvurem") ||
        (op == "bvshl") || (op == "bvlshr") || (op == "bvashr")) {
      require_arity(e,2);
      Sort a=infer(e.items[1],env), b=infer(e.items[2],env);
      if ((a.kind != SORT_WORD) || !same_sort(a,b))
        throw Error(op + " expects equal-width words on line " +
                    std::to_string(e.line));
      return a;
    }
    throw Error("unsupported operator '" + op + "' on line " +
                std::to_string(e.line));
  }

  Sort infer(const SExpr& e, const TypeEnv& env) {
    e.sort=infer_term(e,env);
    return e.sort;
  }

  Script read_script(const std::vector<SExpr>& forms) {
    Script script;
    TypeEnv env;
    bool checked=false;
    bool pushed=false;
    for (const SExpr& form : forms) {
      const std::string command=head(form);
      if ((command == "declare-fun") || (command == "declare-const")) {
        const bool constant=(command == "declare-const");
        if ((form.items.size() != (constant ? 3U : 4U)) ||
            !form.items[1].atom || (!constant &&
            (form.items[2].atom || !form.items[2].items.empty())))
          throw Error("only nullary declare-fun is supported on line " +
                      std::to_string(form.line));
        if (checked) throw Error("declaration after check-sat is unsupported");
        require_binding_symbol(form.items[1]);
        Decl d={form.items[1].text,parse_sort(form.items[constant ? 2 : 3])};
        if (!script.globals.emplace(d.name,d.sort).second)
          throw Error("duplicate declaration '" + d.name + "'");
        script.declarations.push_back(d);
        env.local.emplace(d.name,d.sort);
      } else if (command == "define-fun") {
        if ((form.items.size() != 5) || !form.items[1].atom ||
            form.items[2].atom || !form.items[2].items.empty())
          throw Error("only nullary define-fun is supported");
        if (checked) throw Error("definition after check-sat is unsupported");
        require_binding_symbol(form.items[1]);
        const std::string& name=form.items[1].text;
        Sort declared=parse_sort(form.items[3]);
        // Check before extending the environment: no forward references or
        // recursion. All global names remain unique for the entire script.
        Sort actual=infer(form.items[4],env);
        if (!same_sort(declared,actual))
          throw Error("define-fun body has an unsupported or different sort");
        if (!script.globals.emplace(name,declared).second)
          throw Error("duplicate declaration '" + name + "'");
        env.local.emplace(name,declared);
        script.definitions.emplace_back(name,form.items[4]);
      } else if (command == "assert") {
        require_arity(form,1);
        if (checked) throw Error("assert after check-sat is unsupported");
        if (infer(form.items[1],env).kind != SORT_BOOL)
          throw Error("assert expects Bool on line " +
                      std::to_string(form.line));
        script.assertions.push_back(form.items[1]);
      } else if ((command == "maximize") || (command == "minimize")) {
        require_arity(form,1);
        if (checked) throw Error("objective after check-sat is unsupported");
        if (script.objective != Script::SATISFY)
          throw Error("multiple optimization objectives are unsupported");
        if (infer(form.items[1],env).kind != SORT_INT)
          throw Error("optimization objective must have sort Int on line " +
                      std::to_string(form.line));
        script.objective=(command == "maximize") ?
          Script::MAXIMIZE : Script::MINIMIZE;
        script.objective_term.reset(new SExpr(form.items[1]));
      } else if (command == "push") {
        if (pushed || checked || (form.items.size() > 2) ||
            ((form.items.size() == 2) && !is_atom(form.items[1],"1")))
          throw Error("only one scope-neutral top-level push is supported");
        pushed=true;
      } else if (command == "check-sat") {
        require_arity(form,0);
        if (checked) throw Error("multiple check-sat commands are unsupported");
        checked=true;
      } else if (command == "get-objectives") {
        require_arity(form,0);
        if (!checked)
          throw Error("get-objectives before check-sat is unsupported");
      } else if ((command == "set-logic") || (command == "set-info")) {
        // Metadata does not change the asserted formula.
      } else if (command == "exit") {
        require_arity(form,0);
        break;
      } else {
        throw Error("unsupported command '" + command + "' on line " +
                    std::to_string(form.line));
      }
    }
    if (!checked) throw Error("script has no check-sat command");
    return script;
  }

#include "smt2-session.hpp"

  // Conservative, finite Int interval analysis. This pass adds redundant
  // bounds; it does not translate BitVec arithmetic or change its wraparound.
  struct IntegerRange {
    using Value=std::int64_t;
    Value lo=std::numeric_limits<Value>::min();
    Value hi=std::numeric_limits<Value>::max();
    IntegerRange(void) = default;
    IntegerRange(Value lo0, Value hi0) : lo(lo0), hi(hi0) {}
    static bool add(Value a, Value b, Value& result) {
      if ((b > 0 && a > std::numeric_limits<Value>::max()-b) ||
          (b < 0 && a < std::numeric_limits<Value>::min()-b)) return false;
      result=a+b; return true;
    }
    static bool multiply(Value a, Value b, Value& result) {
      const bool negative=(a < 0) != (b < 0);
      const WordValue aa=a < 0 ? WordValue(0)-WordValue(a) : WordValue(a);
      const WordValue bb=b < 0 ? WordValue(0)-WordValue(b) : WordValue(b);
      const WordValue limit=negative ? WordValue(1)<<63 :
        WordValue(std::numeric_limits<Value>::max());
      if (bb != 0 && aa > limit/bb) return false;
      const WordValue product=aa*bb;
      // Subtract one before conversion to handle the magnitude of INT64_MIN.
      result=product == 0 ? 0 : negative ? -Value(product-1)-1 : Value(product);
      return true;
    }
    static bool divide(Value a, Value b, Value& result) {
      if (b == 0 || (a == std::numeric_limits<Value>::min() && b == -1))
        return false;
      result=a/b;
      if (a%b < 0) result += b < 0 ? 1 : -1;
      return true;
    }
    static Value remainder(Value a, Value positive) {
      Value r=a%positive;
      return r < 0 ? r+positive : r;
    }
    static IntegerRange hull(IntegerRange a, IntegerRange b) {
      return IntegerRange(std::min(a.lo,b.lo),std::max(a.hi,b.hi));
    }
  };

  class IntegerBounds {
    using Value=IntegerRange::Value;
    struct Node;
    using Ref=std::shared_ptr<Node>;
    struct Node {
      std::size_t id;
      std::string op;
      Sort sort;
      Value value;
      std::vector<Ref> args;
    };
    using Environment=std::unordered_map<std::string,Ref>;
    using Bounds=std::unordered_map<std::size_t,IntegerRange>;
    struct Assignment { Ref variable, value, guard; };
    std::unordered_map<std::string,Ref> interned;
    Environment globals;
    Bounds bounds;
    std::vector<Ref> assertions;
    std::vector<Assignment> assignments;
    std::unordered_set<std::string> collected;
    std::vector<std::vector<Assignment>> alternatives;
    std::size_t steps=0;
    static constexpr std::size_t step_limit=200000;

    Ref make(std::string op, Sort sort, std::vector<Ref> args={}, Value value=0) {
      if (op == "distinct" && args.size() == 2)
        return make("not",Sort(SORT_BOOL),{make("=",Sort(SORT_BOOL),args)});
      if (op == "not" && args[0]->op == "not") return args[0]->args[0];
      if ((op == "=" || op == "and" || op == "or") && args.size() == 2 &&
          args[1]->id < args[0]->id) std::swap(args[0],args[1]);
      // n - d*div(n,d) is the Euclidean remainder. Resolve aliases first so
      // guarded ceil-division encodings share this expression across asserts.
      if (sort.kind == SORT_INT && op == "-" && args.size() == 2 &&
          args[1]->op == "*" && args[1]->args.size() == 2) {
        for (int i=0; i<2; i++) {
          Ref d=args[1]->args[i], q=args[1]->args[1-i];
          if (d->op == "integer" && d->value > 0 && q->op == "div" &&
              q->args[0] == args[0] && q->args[1] == d)
            return make("mod",sort,{args[0],d});
        }
      }
      std::string key=std::to_string(op.size())+":"+op+":"+
        std::to_string(sort.kind)+":"+std::to_string(sort.width)+":"+
        std::to_string(value);
      for (const Ref& a:args) key+=":"+std::to_string(a->id);
      auto found=interned.find(key);
      if (found != interned.end()) return found->second;
      Ref result=std::make_shared<Node>(Node{interned.size(),std::move(op),sort,
                                           value,std::move(args)});
      interned.emplace(std::move(key),result);
      return result;
    }

    Ref build(const SExpr& e, const Environment& env) {
      WordValue value;
      if (signed_integer_literal(e,value)) {
        const Value integer=(value >> 63) != 0 ?
          -Value(~value)-1 : Value(value);
        return make("integer",Sort(SORT_INT),{},integer);
      }
      if (e.atom) {
        if (e.text == "true" || e.text == "false")
          return make(e.text,Sort(SORT_BOOL));
        auto found=env.find(e.text);
        if (found != env.end()) return found->second;
        return make("literal:"+e.text,e.sort);
      }
      if (is_atom(e.items[0],"let")) {
        Environment nested=env;
        for (const auto& binding:let_bindings(e))
          nested[binding.first]=build(*binding.second,env);
        return build(e.items[2],nested);
      }
      std::string op=e.items[0].atom ? e.items[0].text : "indexed";
      std::vector<Ref> args;
      if (!e.items[0].atom)
        for (const SExpr& index:e.items[0].items)
          op+=":"+std::to_string(index.text.size())+":"+index.text;
      for (std::size_t i=1; i<e.items.size(); i++) args.push_back(build(e.items[i],env));
      return make(std::move(op),e.sort,std::move(args));
    }

    struct Evaluation {
      IntegerBounds& owner;
      const Bounds& bounds;
      std::unordered_map<std::size_t,IntegerRange> ranges;
      std::unordered_map<std::size_t,int> truths;
      unsigned int depth=0;
      Evaluation(IntegerBounds& owner0, const Bounds& bounds0)
        : owner(owner0), bounds(bounds0) {}
      IntegerRange range(const Ref& n) {
        auto found=ranges.find(n->id);
        if (found != ranges.end()) return found->second;
        if (++owner.steps > step_limit || depth >= max_nesting) return IntegerRange();
        ++depth;
        IntegerRange result=compute(n);
        --depth;
        auto bound=bounds.find(n->id);
        if (bound != bounds.end()) {
          result.lo=std::max(result.lo,bound->second.lo);
          result.hi=std::min(result.hi,bound->second.hi);
          if (result.lo > result.hi) result=IntegerRange();
        }
        ranges.emplace(n->id,result);
        return result;
      }
      IntegerRange compute(const Ref& n) {
        if (n->op == "integer") return IntegerRange(n->value,n->value);
        if (n->sort.kind != SORT_INT || n->args.empty()) return IntegerRange();
        if (n->op == "ite") {
          int c=truth(n->args[0]);
          if (c >= 0) return range(n->args[c ? 1 : 2]);
          return IntegerRange::hull(range(n->args[1]),range(n->args[2]));
        }
        IntegerRange a=range(n->args[0]);
        if (n->op == "-" && n->args.size() == 1) {
          if (a.lo == std::numeric_limits<Value>::min()) return IntegerRange();
          return IntegerRange(-a.hi,-a.lo);
        }
        if (n->op == "+" || n->op == "-" || n->op == "*") {
          for (std::size_t i=1; i<n->args.size(); i++) {
            IntegerRange b=range(n->args[i]);
            Value lo,hi;
            if (n->op == "-") {
              if (b.lo == std::numeric_limits<Value>::min()) return IntegerRange();
              b=IntegerRange(-b.hi,-b.lo);
            }
            if (n->op != "*") {
              if (!IntegerRange::add(a.lo,b.lo,lo) ||
                  !IntegerRange::add(a.hi,b.hi,hi)) return IntegerRange();
            } else {
              Value candidates[4];
              if (!IntegerRange::multiply(a.lo,b.lo,candidates[0]) ||
                  !IntegerRange::multiply(a.lo,b.hi,candidates[1]) ||
                  !IntegerRange::multiply(a.hi,b.lo,candidates[2]) ||
                  !IntegerRange::multiply(a.hi,b.hi,candidates[3])) return IntegerRange();
              lo=*std::min_element(candidates,candidates+4);
              hi=*std::max_element(candidates,candidates+4);
            }
            a=IntegerRange(lo,hi);
          }
          return a;
        }
        if ((n->op == "div" || n->op == "mod") && n->args.size() == 2) {
          IntegerRange b=range(n->args[1]);
          if (b.lo <= 0 && b.hi >= 0) return IntegerRange();
          if (n->op == "mod") {
            WordValue lo=b.lo < 0 ? WordValue(0)-WordValue(b.lo) : WordValue(b.lo);
            WordValue hi=b.hi < 0 ? WordValue(0)-WordValue(b.hi) : WordValue(b.hi);
            return IntegerRange(0,Value(std::max(lo,hi)-1));
          }
          Value candidates[4];
          if (!IntegerRange::divide(a.lo,b.lo,candidates[0]) ||
              !IntegerRange::divide(a.lo,b.hi,candidates[1]) ||
              !IntegerRange::divide(a.hi,b.lo,candidates[2]) ||
              !IntegerRange::divide(a.hi,b.hi,candidates[3])) return IntegerRange();
          return IntegerRange(*std::min_element(candidates,candidates+4),
                              *std::max_element(candidates,candidates+4));
        }
        return IntegerRange();
      }
      int truth(const Ref& n) {
        auto found=truths.find(n->id);
        if (found != truths.end()) return found->second;
        if (++owner.steps > step_limit || depth >= max_nesting) return -1;
        ++depth;
        int result=boolean(n);
        --depth;
        truths.emplace(n->id,result);
        return result;
      }
      int boolean(const Ref& n) {
        const std::string& op=n->op;
        if (op == "true") return 1;
        if (op == "false") return 0;
        if (op == "not") { int a=truth(n->args[0]); return a < 0 ? -1 : 1-a; }
        if (op == "and" || op == "or") {
          bool unknown=false;
          for (const Ref& a:n->args) {
            int t=truth(a);
            if (t == (op == "and" ? 0 : 1)) return t;
            unknown |= t < 0;
          }
          return unknown ? -1 : op == "and";
        }
        if (op == "=>" && n->args.size() == 2) {
          int a=truth(n->args[0]), b=truth(n->args[1]);
          if (a == 0 || b == 1) return 1;
          return a == 1 ? b : -1;
        }
        if (op == "ite" && n->sort.kind == SORT_BOOL) {
          int c=truth(n->args[0]);
          if (c >= 0) return truth(n->args[c ? 1 : 2]);
          int a=truth(n->args[1]), b=truth(n->args[2]);
          return a == b ? a : -1;
        }
        if (n->args.size() != 2) return -1;
        if (op == "=" && n->args[0] == n->args[1]) return 1;
        if (op == "=" && n->args[0]->sort.kind == SORT_BOOL) {
          int a=truth(n->args[0]),b=truth(n->args[1]);
          return a < 0 || b < 0 ? -1 : a == b;
        }
        if (n->args[0]->sort.kind != SORT_INT) return -1;
        IntegerRange a=range(n->args[0]),b=range(n->args[1]);
        if (op == "=") {
          if (a.hi < b.lo || b.hi < a.lo) return 0;
          return a.lo == a.hi && b.lo == b.hi ? a.lo == b.lo : -1;
        }
        if (op == ">" || op == ">=") std::swap(a,b);
        if (op == "<" || op == ">")
          return a.hi < b.lo ? 1 : a.lo >= b.hi ? 0 : -1;
        if (op == "<=" || op == ">=")
          return a.hi <= b.lo ? 1 : a.lo > b.hi ? 0 : -1;
        return -1;
      }
    };

    static void restrict(Bounds& state, const Ref& n, IntegerRange r) {
      if (r.lo > r.hi) return;
      auto found=state.find(n->id);
      if (found != state.end()) {
        r.lo=std::max(r.lo,found->second.lo);
        r.hi=std::min(r.hi,found->second.hi);
        if (r.lo > r.hi) return;
      }
      state[n->id]=r;
    }
    void remainder_guard(Bounds& state, const Ref& n, bool equal) {
      if (n->op != "mod" || n->args[1]->op != "integer" ||
          n->args[1]->value <= 0) return;
      const Value divisor=n->args[1]->value;
      IntegerRange a=Evaluation(*this,state).range(n->args[0]);
      const Value lo_rem=IntegerRange::remainder(a.lo,divisor);
      const Value hi_rem=IntegerRange::remainder(a.hi,divisor);
      Value lo=a.lo,hi=a.hi;
      if (equal) {
        if (!IntegerRange::add(lo,lo_rem == 0 ? 0 : divisor-lo_rem,lo) ||
            !IntegerRange::add(hi,-hi_rem,hi)) return;
      } else {
        if ((lo_rem == 0 && !IntegerRange::add(lo,1,lo)) ||
            (hi_rem == 0 && !IntegerRange::add(hi,-1,hi))) return;
      }
      restrict(state,n->args[0],IntegerRange(lo,hi));
    }
    void assume(Bounds& state, const Ref& n, bool positive=true, unsigned int depth=0) {
      if (++steps > step_limit || depth >= max_nesting) return;
      if (n->op == "not") { assume(state,n->args[0],!positive,depth+1); return; }
      if ((n->op == "and" && positive) || (n->op == "or" && !positive)) {
        for (const Ref& a:n->args) assume(state,a,positive,depth+1);
        return;
      }
      if (n->op == "=>" && positive && n->args.size() == 2) {
        if (Evaluation(*this,state).truth(n->args[0]) == 1)
          assume(state,n->args[1],true,depth+1);
        return;
      }
      if (n->args.size() != 2 || n->args[0]->sort.kind != SORT_INT) return;
      Ref x=n->args[0], y=n->args[1];
      IntegerRange a=Evaluation(*this,state).range(x),b=Evaluation(*this,state).range(y);
      std::string op=n->op;
      if (op == "=") {
        if (positive) {
          IntegerRange common(std::max(a.lo,b.lo),std::min(a.hi,b.hi));
          restrict(state,x,common); restrict(state,y,common);
        }
        if (b.lo == 0 && b.hi == 0) remainder_guard(state,x,positive);
        if (a.lo == 0 && a.hi == 0) remainder_guard(state,y,positive);
        return;
      }
      if (!positive) {
        if (op == "<") op=">="; else if (op == "<=") op=">";
        else if (op == ">") op="<="; else if (op == ">=") op="<";
      }
      if (op == ">" || op == ">=") { std::swap(x,y); std::swap(a,b); }
      if (op == "<=" || op == ">=") {
        restrict(state,x,IntegerRange(a.lo,b.hi));
        restrict(state,y,IntegerRange(a.lo,b.hi));
      } else if (op == "<" || op == ">") {
        Value hi,lo;
        if (IntegerRange::add(b.hi,-1,hi)) restrict(state,x,IntegerRange(a.lo,hi));
        if (IntegerRange::add(a.lo,1,lo)) restrict(state,y,IntegerRange(lo,b.hi));
      }
    }
    bool assignment(const Ref& n, Ref guard, Assignment& result) {
      if (n->op != "=" || n->args.size() != 2 ||
          n->args[0]->sort.kind != SORT_INT) return false;
      for (int i=0; i<2; i++)
        if (n->args[i]->op.compare(0,9,"variable:") == 0) {
          result=Assignment{n->args[i],n->args[1-i],guard}; return true;
        }
      return false;
    }
    void collect(const Ref& n, Ref guard, unsigned int depth=0) {
      if (++steps > step_limit || depth >= max_nesting || !collected.emplace(
            std::to_string(n->id)+":"+std::to_string(guard->id)).second) return;
      if (n->op == "and") {
        for (const Ref& a:n->args) collect(a,guard,depth+1);
      } else if (n->op == "=>" && n->args.size() == 2) {
        collect(n->args[1],guard->op == "true" ? n->args[0] :
                make("and",Sort(SORT_BOOL),{guard,n->args[0]}),depth+1);
      } else {
        Assignment a;
        if (assignment(n,guard,a)) assignments.push_back(a);
        // Every branch of an asserted disjunction must bind a variable before
        // its alternatives can yield an unconditional range for that variable.
        if (n->op == "or" && guard->op == "true" && !n->args.empty()) {
          std::unordered_map<std::size_t,std::vector<Assignment>> candidates;
          for (const Ref& branch:n->args) {
            std::vector<Ref> terms=branch->op == "and" ? branch->args : std::vector<Ref>{branch};
            std::unordered_map<std::size_t,Assignment> branch_assignments;
            for (const Ref& term:terms) {
              Assignment candidate;
              if (assignment(term,branch,candidate))
                branch_assignments.emplace(candidate.variable->id,candidate);
            }
            for (const auto& candidate:branch_assignments)
              candidates[candidate.first].push_back(candidate.second);
          }
          for (const auto& candidate:candidates)
            if (candidate.second.size() == n->args.size()) alternatives.push_back(candidate.second);
        }
      }
    }
    IntegerRange guarded(const Assignment& a) {
      // Copying the environment is work too. Exhausting the analysis budget
      // must only discard deductions, never truncate a union of alternatives.
      if (steps >= step_limit || bounds.size() >= step_limit-steps) {
        steps=step_limit;
        return IntegerRange();
      }
      steps+=bounds.size()+1;
      Bounds local=bounds;
      assume(local,a.guard);
      return Evaluation(*this,local).range(a.value);
    }
    static bool complementary(const Ref& a, const Ref& b) {
      return (a->op == "not" && a->args[0] == b) ||
        (b->op == "not" && b->args[0] == a);
    }

    struct Affine {
      Ref variable;
      Value coefficient=0, offset=0;
      bool valid=true;
    };
    mutable std::unordered_map<std::size_t,Affine> affine_cache;
    mutable unsigned int affine_depth=0;
    Affine affine(const Ref& n) const {
      auto found=affine_cache.find(n->id);
      if (found != affine_cache.end()) return found->second;
      if (affine_depth >= max_nesting) { Affine result; result.valid=false; return result; }
      ++affine_depth;
      Affine result=affine_term(n);
      --affine_depth;
      affine_cache.emplace(n->id,result);
      return result;
    }
    Affine affine_term(const Ref& n) const {
      Affine result;
      if (n->op == "integer") { result.offset=n->value; return result; }
      if (n->op.compare(0,9,"variable:") == 0 && n->sort.kind == SORT_INT) {
        result.variable=n; result.coefficient=1; return result;
      }
      if (n->op != "+" && n->op != "-" && n->op != "*") {
        result.valid=false; return result;
      }
      result=affine(n->args[0]);
      for (std::size_t i=1; i<n->args.size() && result.valid; i++) {
        Affine b=affine(n->args[i]);
        if (!b.valid) { result.valid=false; break; }
        if (n->op == "*") {
          if (result.variable && b.variable) { result.valid=false; break; }
          Value coefficient,offset;
          if (!IntegerRange::multiply(result.variable ? result.coefficient : b.coefficient,
                                      result.variable ? b.offset : result.offset,coefficient) ||
              !IntegerRange::multiply(result.offset,b.offset,offset)) {
            result.valid=false; break;
          }
          if (!result.variable) result.variable=b.variable;
          result.coefficient=coefficient; result.offset=offset;
        } else {
          if (n->op == "-" &&
              (!IntegerRange::multiply(b.coefficient,-1,b.coefficient) ||
               !IntegerRange::multiply(b.offset,-1,b.offset))) {
            result.valid=false; break;
          }
          if (result.variable && b.variable && result.variable != b.variable) {
            result.valid=false; break;
          }
          if (!result.variable) result.variable=b.variable;
          result.valid=IntegerRange::add(result.coefficient,b.coefficient,result.coefficient) &&
            IntegerRange::add(result.offset,b.offset,result.offset);
        }
        if (result.coefficient == 0) result.variable.reset();
      }
      if (n->op == "-" && n->args.size() == 1)
        result.valid=result.valid &&
          IntegerRange::multiply(result.coefficient,-1,result.coefficient) &&
          IntegerRange::multiply(result.offset,-1,result.offset);
      return result;
    }

  public:
    explicit IntegerBounds(const Script& script) {
      for (const Decl& d:script.declarations)
        globals.emplace(d.name,make("variable:"+d.name,d.sort));
      for (const auto& definition:script.definitions)
        globals.emplace(definition.first,build(definition.second,globals));
      for (const SExpr& assertion:script.assertions) {
        Ref n=build(assertion,globals);
        assertions.push_back(n); collect(n,make("true",Sort(SORT_BOOL)));
      }
      // A bounded work budget and iteration count only lose deductions. The
      // original constraints remain in the model, even if analysis stops early.
      for (unsigned int iteration=0; iteration<32 && steps<step_limit; iteration++) {
        Bounds previous=bounds;
        for (const Ref& n:assertions) assume(bounds,n);
        for (std::size_t i=0; i<assignments.size() && steps<step_limit; i++) {
          const Assignment& a=assignments[i];
          if (Evaluation(*this,bounds).truth(a.guard) == 1)
            restrict(bounds,a.variable,guarded(a));
          for (std::size_t j=0; j<i && steps<step_limit; j++) {
            ++steps;
            const Assignment& b=assignments[j];
            if (a.variable == b.variable && complementary(a.guard,b.guard))
              restrict(bounds,a.variable,IntegerRange::hull(guarded(a),guarded(b)));
          }
        }
        for (const auto& choices:alternatives) {
          if (steps >= step_limit) break;
          IntegerRange range=guarded(choices[0]);
          std::size_t i=1;
          for (; i<choices.size() && steps<step_limit; i++)
            range=IntegerRange::hull(range,guarded(choices[i]));
          if (i == choices.size())
            restrict(bounds,choices[0].variable,range);
        }
        bool changed=previous.size() != bounds.size();
        if (!changed)
          for (const auto& entry:bounds) {
            auto old=previous.find(entry.first);
            if (old == previous.end() || old->second.lo != entry.second.lo ||
                old->second.hi != entry.second.hi) { changed=true; break; }
          }
        if (!changed) break;
      }
    }
    bool objective_source(const SExpr& objective, std::string& name,
                          bool& increasing) {
      Affine a=affine(build(objective,globals));
      if (!a.valid || !a.variable || a.coefficient == 0) return false;
      // Auxiliary values defined by constraints can have weak inverse
      // propagation, especially for variable division. Keep input-first
      // search for those sources even when the objective is affine in them.
      for (const Assignment& assignment:assignments)
        if (assignment.variable == a.variable && assignment.value != a.variable) return false;
      for (const auto& choices:alternatives)
        if (choices[0].variable == a.variable) return false;
      name=a.variable->op.substr(9);
      increasing=a.coefficient > 0;
      return true;
    }
    IntegerRange range(const std::string& name) const {
      auto variable=globals.find(name);
      if (variable == globals.end()) return IntegerRange();
      auto bound=bounds.find(variable->second->id);
      return bound == bounds.end() ? IntegerRange() : bound->second;
    }
  };

  WordValue word_literal(const std::string& token) {
    WordValue value=0;
    if (token[1] == 'x') {
      for (std::size_t i=2; i<token.size(); i++) {
        char c=token[i];
        unsigned int digit=(c >= '0' && c <= '9') ? c-'0' :
          (c >= 'a' && c <= 'f') ? c-'a'+10U :
          (c >= 'A' && c <= 'F') ? c-'A'+10U : 16U;
        if (digit >= 16U) throw Error("invalid hexadecimal bit-vector literal");
        value=(value << 4) | digit;
      }
    } else {
      for (std::size_t i=2; i<token.size(); i++) {
        if ((token[i] != '0') && (token[i] != '1'))
          throw Error("invalid binary bit-vector literal");
        value=(value << 1) | static_cast<unsigned int>(token[i]-'0');
      }
    }
    return value;
  }

  struct EvalEnv {
    struct Binding {
      const SExpr* term;
      const EvalEnv* outer;
      mutable std::unique_ptr<WordExpr> word;
      mutable std::unique_ptr<BoolExpr> boolean;
      Binding(const SExpr* term0, const EvalEnv* outer0)
        : term(term0), outer(outer0) {}
    };
    const EvalEnv* parent;
    std::unordered_map<std::string,Binding> bindings;
    explicit EvalEnv(const EvalEnv* parent0=nullptr) : parent(parent0) {}
    EvalEnv(const SExpr& e, const EvalEnv& outer) : parent(&outer) {
      for (const auto& binding : let_bindings(e))
        bindings.emplace(binding.first,Binding(binding.second,&outer));
    }
    const Binding* find(const std::string& name) const {
      auto i=bindings.find(name);
      if (i != bindings.end()) return &i->second;
      return parent == nullptr ? nullptr : parent->find(name);
    }
  };

#include "smt2-normalize.hpp"

  class SMT2Space : public Space {
  private:
    WordDomainType policy;
    Script::Objective objective_kind;
    WordVar objective_word;
    bool tables;
    unsigned int table_count=0;
    unsigned int normalization_count=0;
    IntegerNormalization* normalizer=nullptr;
    WordSmt2::IntegerZeroFunctions zero_functions;
    std::unordered_map<std::string,WordTupleSet> tuple_cache;

    WordExpr table_binary(const std::string& op, const WordExpr& a,
                          const WordExpr& b) {
      unsigned int width=a.width();
      std::string key=op+std::to_string(width);
      auto t=tuple_cache.find(key);
      if (t==tuple_cache.end()) {
        std::vector<std::vector<WordValue>> rows;
        WordValue size=WordValue(1)<<width;
        rows.reserve(size*size);
        for (WordValue x=0; x<size; x++)
          for (WordValue y=0; y<size; y++)
            rows.push_back({x,y,(op=="bvmul" ? x*y : x+y)&(size-1)});
        t=tuple_cache.emplace(key,WordTupleSet({width,width,width},rows)).first;
      }
      WordVar x=a.post(*this,policy), y=b.post(*this,policy);
      WordVar z(*this,width,policy);
      extensional(*this,WordVarArgs({x,y,z}),t->second);
      table_count++;
      return WordExpr(z);
    }

    WordVarArray words;
    BoolVarArray bools;
    std::unordered_map<std::string,int> word_index;
    std::unordered_map<std::string,int> bool_index;
    unsigned int evaluation_depth=0;

    struct EvalGuard {
      unsigned int& depth;
      explicit EvalGuard(unsigned int& depth0) : depth(depth0) {
        if (depth >= max_nesting)
          throw Error("reader evaluation nesting limit is 128");
        depth++;
      }
      ~EvalGuard(void) { depth--; }
    };

    WordDomainType expression_policy(const Sort& sort) const {
      // Signedness is the interpretation of SMT Int operations, not a reason
      // to change a classic cube WordVar into a bounds-enabled variable.
      return sort.kind == SORT_INT && policy != WDT_CUBE ? WDT_SIGNED : policy;
    }

    static WordValBranch signed_values(bool high_first) {
      return WORD_VAL(
        [](const Space&, WordVar x, int) -> WordValue {
          unsigned int bit=x.width()-1;
          while ((x.unknown() & (WordValue(1)<<bit)) == 0) --bit;
          return bit;
        },
        [high_first](Space& home, unsigned int alternative,
                     WordVar x, int, WordValue bit) {
          const WordValue mask=WordValue(1)<<bit;
          const bool preferred=bit == x.width()-1 ? !high_first : high_first;
          const bool one=alternative == 0 ? preferred : !preferred;
          dom(home,x,one ? x.lo()|mask : x.lo(),one ? x.hi() : x.hi()&~mask);
        });
    }

    WordExpr integer_div(const WordExpr& a, const WordExpr& b,
                         const SExpr& divisor) {
      const WordExpr zero(64,0), one(64,1);
      const WordDomainType integer_domain=expression_policy(Sort(SORT_INT));
      WordValue constant;
      if (integer_literal(divisor,constant)) {
        if (constant == 0)
          return zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::DIVIDE,
                                      a,integer_domain);
        if (constant == 1) return a;
        const WordExpr quotient=signed_div(a,b,WS_SMTLIB);
        const WordExpr remainder=signed_rem(a,b,WS_SMTLIB);
        const BoolExpr remainder_negative=
          word_rel(remainder,WRT_SLE,zero,integer_domain);
        return Gecode::ite(remainder_negative,quotient-one,quotient);
      }
      WordVar divisor_word=b.post(*this,integer_domain);
      if (divisor_word.assigned() && divisor_word.val() == 0)
        return zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::DIVIDE,
                                    a,integer_domain);
      const WordExpr dividend(a.post(*this,integer_domain)), denominator(divisor_word);
      const BoolExpr b_negative=word_rel(denominator,WRT_SLE,zero,integer_domain);
      const WordExpr quotient=signed_div(dividend,denominator,WS_SMTLIB);
      const WordExpr remainder=signed_rem(dividend,denominator,WS_SMTLIB);
      const BoolExpr remainder_negative=
        word_rel(remainder,WRT_SLE,zero,integer_domain);
      const WordExpr adjusted=Gecode::ite(
        remainder_negative,
        Gecode::ite(b_negative,quotient+one,quotient-one),quotient);
      if (!divisor_word.in(0)) return adjusted;
      const BoolExpr b_zero=word_rel(denominator,WRT_EQ,zero,integer_domain);
      return Gecode::ite(b_zero,
        zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::DIVIDE,
                             dividend,integer_domain),adjusted);
    }

    WordExpr integer_mod(const WordExpr& a, const WordExpr& b,
                         const SExpr& divisor) {
      const WordExpr zero(64,0);
      const WordDomainType integer_domain=expression_policy(Sort(SORT_INT));
      WordValue constant;
      if (integer_literal(divisor,constant)) {
        if (constant == 0)
          return zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::MODULO,
                                      a,integer_domain);
        if (constant == 1) return zero;
        // SMT Int mod has a nonnegative result for a nonzero divisor.
        return signed_mod(a,b,WS_SMTLIB);
      }
      WordVar divisor_word=b.post(*this,integer_domain);
      if (divisor_word.assigned() && divisor_word.val() == 0)
        return zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::MODULO,
                                    a,integer_domain);
      const WordExpr dividend(a.post(*this,integer_domain)), denominator(divisor_word);
      const BoolExpr b_negative=word_rel(denominator,WRT_SLE,zero,integer_domain);
      const WordExpr magnitude=Gecode::ite(b_negative,-denominator,denominator);
      const WordExpr remainder=signed_rem(dividend,magnitude,WS_SMTLIB);
      const BoolExpr remainder_negative=
        word_rel(remainder,WRT_SLE,zero,integer_domain);
      const WordExpr adjusted=Gecode::ite(
        remainder_negative,remainder+magnitude,remainder);
      if (!divisor_word.in(0)) return adjusted;
      const BoolExpr b_zero=word_rel(denominator,WRT_EQ,zero,integer_domain);
      return Gecode::ite(b_zero,
        zero_functions.value(*this,WordSmt2::IntegerZeroFunctions::MODULO,
                             dividend,integer_domain),adjusted);
    }

    WordExpr eval_word(const SExpr& e, const EvalEnv& env) {
      EvalGuard guard(evaluation_depth);
      WordValue integer;
      if (signed_integer_literal(e,integer)) return WordExpr(64,integer);
      if (e.atom) {
        Sort literal;
        if ((e.token == SExpr::SIMPLE) && literal_sort(e.text,literal))
          return WordExpr(literal.width,word_literal(e.text));
        if (const EvalEnv::Binding* value=env.find(e.text)) {
          // Lower a demanded binding once. Keeping only an expression would
          // repost its DAG at every Boolean relation that uses it.
          if (!value->word)
            value->word.reset(new WordExpr(
              eval_word(*value->term,*value->outer).post(
                *this,expression_policy(value->term->sort))));
          return *value->word;
        }
        auto i=word_index.find(e.text);
        if (i != word_index.end()) return WordExpr(words[i->second]);
        throw Error("word symbol '" + e.text + "' is unavailable");
      }
      Sort literal;
      WordValue value;
      if (decimal_literal(e,literal,value)) return WordExpr(literal.width,value);
      if (!e.items[0].atom) {
        const SExpr& indexed=e.items[0];
        WordExpr source=eval_word(e.items[1],env);
        if ((indexed.items[1].text == "rotate_left") ||
            (indexed.items[1].text == "rotate_right")) {
          const unsigned int amount=rotation_amount(indexed.items[2],source.width());
          return (indexed.items[1].text == "rotate_left") ?
            Gecode::rotate_left(source,amount) : Gecode::rotate_right(source,amount);
        }
        if (indexed.items[1].text == "extract") {
          unsigned int high=unsigned_atom(indexed.items[2],"extract");
          unsigned int low=unsigned_atom(indexed.items[3],"extract");
          return Gecode::extract(source,low,high-low+1U);
        }
        unsigned int extra=unsigned_atom(indexed.items[2],"extension");
        return (indexed.items[1].text == "sign_extend") ?
          Gecode::sign_extend(source,source.width()+extra) :
          Gecode::zero_extend(source,source.width()+extra);
      }
      const std::string op=e.items[0].text;
      if (op == "let") {
        EvalEnv nested(e,env);
        return eval_word(e.items[2],nested);
      }
      if (op == "ite")
        return Gecode::ite(eval_bool(e.items[1],env),
                           eval_word(e.items[2],env),eval_word(e.items[3],env));
      if (op == "bvneg") return -eval_word(e.items[1],env);
      if (op == "bvnot") return ~eval_word(e.items[1],env);
      IntegerNormalization::Factor factored;
      if (normalizer && normalizer->factor(e,env,factored)) {
        ++normalization_count;
        const auto product=[&](const std::vector<const SExpr*>& factors) {
          WordExpr value(64,1);
          if (!factors.empty()) {
            value=eval_word(*factors[0],env);
            for (std::size_t i=1; i<factors.size(); i++)
              value=value*eval_word(*factors[i],env);
          }
          return value;
        };
        WordExpr left=product(factored.left), right=product(factored.right);
        return eval_word(*factored.common,env)*
          (factored.subtract ? left-right : left+right);
      }
      WordExpr a=eval_word(e.items[1],env);
      if ((op == "+") || (op == "*")) {
        for (std::size_t i=2; i<e.items.size(); i++)
          a=(op == "+") ? a+eval_word(e.items[i],env) :
            a*eval_word(e.items[i],env);
        return a;
      }
      if (op == "-")
        return e.items.size() == 2 ? -a : a-eval_word(e.items[2],env);
      if ((op == "bvadd") || (op == "bvmul") || (op == "bvand") ||
          (op == "bvor") || (op == "bvxor")) {
        for (std::size_t i=2; i<e.items.size(); i++) {
          WordExpr b=eval_word(e.items[i],env);
          if (tables && a.width()<=6 && ((op=="bvadd") || (op=="bvmul")))
            a=table_binary(op,a,b);
          else if (op == "bvadd") a=a+b;
          else if (op == "bvmul") a=a*b;
          else if (op == "bvand") a=a&b;
          else if (op == "bvor") a=a|b;
          else a=a^b;
        }
        return a;
      }
      WordExpr b=eval_word(e.items[2],env);
      if (op == "div") return integer_div(a,b,e.items[2]);
      if (op == "mod") return integer_mod(a,b,e.items[2]);
      if (op == "concat") return Gecode::concat(a,b);
      if (op == "bvudiv") return Gecode::div(a,b,WS_SMTLIB);
      if (op == "bvurem") return Gecode::mod(a,b,WS_SMTLIB);
      if (op == "bvshl") return a << b;
      if (op == "bvlshr") return logical_shift_right(a,b);
      if (op == "bvashr") return arithmetic_shift_right(a,b);
      if (op == "bvsub") return a-b;
      if (op == "bvsdiv") return signed_div(a,b,WS_SMTLIB);
      if ((op == "bvsmod") || (op == "bvsmod_i"))
        return signed_mod(a,b,WS_SMTLIB);
      throw Error("unsupported word operator '" + op + "'");
    }

    BoolExpr constant_bool(bool value) {
      return BoolExpr(BoolVar(*this,value ? 1 : 0,value ? 1 : 0));
    }

    bool zero_literal(const SExpr& e) {
      Sort sort;
      return e.atom && (e.token == SExpr::SIMPLE) && literal_sort(e.text,sort) &&
        (word_literal(e.text) == 0);
    }

    BoolExpr eval_bool(const SExpr& e, const EvalEnv& env) {
      EvalGuard guard(evaluation_depth);
      if (e.atom) {
        if (e.text == "true") return constant_bool(true);
        if (e.text == "false") return constant_bool(false);
        if (const EvalEnv::Binding* value=env.find(e.text)) {
          // NNF conversion expands shared BoolExpr trees: materialize each
          // demanded binding to preserve the input DAG through lowering.
          if (!value->boolean)
            value->boolean.reset(new BoolExpr(
              Gecode::expr(*this,eval_bool(*value->term,*value->outer))));
          return *value->boolean;
        }
        auto i=bool_index.find(e.text);
        if (i != bool_index.end()) return BoolExpr(bools[i->second]);
        throw Error("Boolean symbol '" + e.text + "' is unavailable");
      }
      const std::string op=head(e);
      if (op == "let") {
        EvalEnv nested(e,env);
        return eval_bool(e.items[2],nested);
      }
      if (op == "not") return !eval_bool(e.items[1],env);
      if (op == "=>") {
        // SMT-LIB implication is right associative.
        BoolExpr result=eval_bool(e.items.back(),env);
        for (std::size_t i=e.items.size()-2; i>0; i--)
          result=!eval_bool(e.items[i],env) || result;
        return result;
      }
      if (op == "xor") {
        BoolExpr result=eval_bool(e.items[1],env);
        for (std::size_t i=2; i<e.items.size(); i++)
          // Materialize parity steps to avoid exponential NNF expansion.
          result=BoolExpr(Gecode::expr(*this,result != eval_bool(e.items[i],env)));
        return result;
      }
      if ((op == "and") || (op == "or")) {
        if (e.items.size() == 1) return constant_bool(op == "and");
        BoolExpr result=eval_bool(e.items[1],env);
        for (std::size_t i=2; i<e.items.size(); i++)
          result=(op == "and") ? result && eval_bool(e.items[i],env) :
            result || eval_bool(e.items[i],env);
        return result;
      }
      if (op == "ite") {
        BoolExpr c=eval_bool(e.items[1],env);
        return (c && eval_bool(e.items[2],env)) ||
          (!c && eval_bool(e.items[3],env));
      }
      if ((op == "=") || (op == "distinct")) {
        Sort s=e.items[1].sort;
        const WordDomainType relation_policy=expression_policy(s);
        BoolExpr result;
        if (op == "=") {
          for (std::size_t i=2; i<e.items.size(); i++)
            result = result && (s.kind == SORT_BOOL ?
              eval_bool(e.items[i-1],env) == eval_bool(e.items[i],env) :
              word_rel(eval_word(e.items[i-1],env),WRT_EQ,
                       eval_word(e.items[i],env),relation_policy));
        } else {
          for (std::size_t i=1; i<e.items.size(); i++)
            for (std::size_t j=i+1; j<e.items.size(); j++)
              result = result && (s.kind == SORT_BOOL ?
                eval_bool(e.items[i],env) != eval_bool(e.items[j],env) :
                word_rel(eval_word(e.items[i],env),WRT_NQ,
                         eval_word(e.items[j],env),relation_policy));
        }
        return result;
      }
      // Unsigned zero is the least value.  Recognize these extrema before
      // lowering either operand: generated SMT often leaves a large, dead
      // arithmetic expression underneath such a tautology.
      if (((op == "bvuge") && zero_literal(e.items[2])) ||
          ((op == "bvule") && zero_literal(e.items[1])))
        return constant_bool(true);
      if (((op == "bvult") && zero_literal(e.items[2])) ||
          ((op == "bvugt") && zero_literal(e.items[1])))
        return constant_bool(false);
      WordRelType relation;
      WordDomainType relation_policy=policy;
      if (op == "<") {
        relation=WRT_SLE;
        relation_policy=expression_policy(e.items[1].sort);
      } else if (op == "<=") {
        relation=WRT_SLQ;
        relation_policy=expression_policy(e.items[1].sort);
      } else if (op == ">") {
        relation=WRT_SGR;
        relation_policy=expression_policy(e.items[1].sort);
      } else if (op == ">=") {
        relation=WRT_SGQ;
        relation_policy=expression_policy(e.items[1].sort);
      }
      else if (op == "bvule") relation=WRT_ULQ;
      else if (op == "bvult") relation=WRT_ULE;
      else if (op == "bvuge") relation=WRT_UGQ;
      else if (op == "bvugt") relation=WRT_UGR;
      else if (op == "bvsle") relation=WRT_SLQ;
      else if (op == "bvslt") relation=WRT_SLE;
      else if (op == "bvsge") relation=WRT_SGQ;
      else if (op == "bvsgt") relation=WRT_SGR;
      else throw Error("unsupported Boolean operator '" + op + "'");
      return word_rel(eval_word(e.items[1],env),relation,
                      eval_word(e.items[2],env),relation_policy);
    }

  public:
    SMT2Space(const Script& script, WordDomainType policy0, bool tables0=false)
      : policy(policy0), objective_kind(script.objective), tables(tables0),
        words(*this,static_cast<int>(
          [&script](){ std::size_t n=0; for (const Decl& d:script.declarations)
            if (d.sort.kind!=SORT_BOOL) n++; return n; }())),
        bools(*this,static_cast<int>(
          [&script](){ std::size_t n=0; for (const Decl& d:script.declarations)
            if (d.sort.kind==SORT_BOOL) n++; return n; }())) {
      IntegerBounds integer_bounds(script);
      IntegerNormalization normalization(integer_bounds);
      normalizer=&normalization;
      int wi=0, bi=0;
      for (const Decl& d : script.declarations) {
        if (d.sort.kind != SORT_BOOL) {
          if (d.sort.kind == SORT_INT) {
            const IntegerRange range=integer_bounds.range(d.name);
            if (policy == WDT_CUBE) {
              words[wi]=WordVar(*this,64);
              rel(*this,words[wi],WRT_SGQ,64,WordValue(range.lo));
              rel(*this,words[wi],WRT_SLQ,64,WordValue(range.hi));
            } else {
              words[wi]=WordVar(*this,64,WDT_SIGNED,
                                WordValue(range.lo),WordValue(range.hi));
            }
          } else
            words[wi]=(policy == WDT_CUBE) ? WordVar(*this,d.sort.width) :
              WordVar(*this,d.sort.width,policy);
          word_index.emplace(d.name,wi++);
        } else if (d.sort.kind == SORT_BOOL) {
          bools[bi]=BoolVar(*this,0,1);
          bool_index.emplace(d.name,bi++);
        }
      }
      EvalEnv env;
      // Bodies were checked at their declaration site. Evaluate them against
      // the global environment, never a let environment at the use site.
      for (const auto& definition : script.definitions)
        env.bindings.emplace(definition.first,
          EvalEnv::Binding(&definition.second,&env));
      for (const SExpr& assertion : script.assertions)
        Gecode::rel(*this,eval_bool(assertion,env));
      if (objective_kind != Script::SATISFY)
        objective_word=eval_word(*script.objective_term,env).post(
          *this,expression_policy(script.objective_term->sort));
      normalizer=nullptr;
      // An affine objective has a useful order on its source variable. Branch
      // there instead of on a weakly constrained arithmetic output. Preserve
      // input-first search for nonlinear or multi-variable objectives.
      int preferred=-1;
      if (objective_kind != Script::SATISFY) {
        std::string name;
        bool increasing;
        if (integer_bounds.objective_source(*script.objective_term,name,increasing)) {
          auto found=word_index.find(name);
          if (found != word_index.end()) {
            preferred=found->second;
            const bool high_first=(objective_kind == Script::MAXIMIZE) == increasing;
            branch(*this,words[preferred],words[preferred].domain_type() == WDT_CUBE ?
                   signed_values(high_first) : high_first ?
                   WORD_VAL_SPLIT_MAX() : WORD_VAL_SPLIT_MIN());
          }
        }
      }
      WordVarArgs active_cube, active_bounded;
      for (int i=0; i<words.size(); i++)
        if (i != preferred && (words[i].degree() != 0 ||
            (objective_kind != Script::SATISFY &&
             words[i].varimp() == objective_word.varimp()))) {
          if (words[i].domain_type() == WDT_CUBE)
            active_cube << words[i];
          else
            active_bounded << words[i];
        }
      if (active_bounded.size() != 0)
        branch(*this,active_bounded,WordSmt2::bounded_afc(*this,active_bounded),
               objective_kind == Script::MAXIMIZE ?
               WORD_VAL_SPLIT_MAX() : WORD_VAL_SPLIT_MIN());
      if (active_cube.size() != 0)
        branch(*this,active_cube,WORD_VAR_SIZE_MIN(),
               objective_kind == Script::SATISFY ? WORD_VAL_MSB() :
               signed_values(objective_kind == Script::MAXIMIZE));
      BoolVarArgs active_bools;
      for (int i=0; i<bools.size(); i++)
        if (bools[i].degree() != 0)
          active_bools << bools[i];
      if (active_bools.size() != 0)
        branch(*this,active_bools,BOOL_VAR_NONE(),BOOL_VAL_MIN());
      WordVarArgs zero_cube, zero_bounded;
      const WordVarArgs zero_results=zero_functions.variables();
      for (int i=0; i<zero_results.size(); i++)
        if (!zero_results[i].assigned() &&
            (zero_results[i].degree() != 0 ||
             (objective_kind != Script::SATISFY &&
              zero_results[i].varimp() == objective_word.varimp()))) {
          if (zero_results[i].domain_type() == WDT_CUBE)
            zero_cube << zero_results[i];
          else
            zero_bounded << zero_results[i];
        }
      if (zero_bounded.size() != 0)
        branch(*this,zero_bounded,WORD_VAR_SIZE_MIN(),
               objective_kind == Script::MAXIMIZE ?
               WORD_VAL_SPLIT_MAX() : WORD_VAL_SPLIT_MIN());
      if (zero_cube.size() != 0)
        branch(*this,zero_cube,WORD_VAR_SIZE_MIN(),
               signed_values(objective_kind == Script::MAXIMIZE));
    }

    SMT2Space(SMT2Space& other)
      : Space(other), policy(other.policy),
        objective_kind(other.objective_kind), tables(other.tables),
        table_count(other.table_count),
        normalization_count(other.normalization_count) {
      words.update(*this,other.words);
      bools.update(*this,other.bools);
      if (objective_kind != Script::SATISFY)
        objective_word.update(*this,other.objective_word);
    }
    virtual void constrain(const Space& best0) {
      const SMT2Space& best=static_cast<const SMT2Space&>(best0);
      const WordRelType relation=(objective_kind == Script::MAXIMIZE) ?
        WRT_SGR : WRT_SLE;
      rel(*this,objective_word,relation,64,best.objective_word.val());
    }
    bool optimizing(void) const { return objective_kind != Script::SATISFY; }
    WordValue objective(void) const { return objective_word.val(); }
    unsigned int replacements(void) const { return table_count; }
    unsigned int normalizations(void) const { return normalization_count; }
    WordValue word_value(int index) const {
      // Variables omitted from branching have no remaining propagators. Any
      // member of their domain is a valid completion of the public model.
      return words[index].domain_type() == WDT_CUBE ? words[index].lo() :
        words[index].minimum();
    }
    bool boolean_value(int index) const { return bools[index].min() != 0; }
    virtual Space* copy(void) { return new SMT2Space(*this); }
  };

  std::string json_escape(const std::string& text) {
    std::ostringstream out;
    for (char c : text) {
      if ((c == '\\') || (c == '"')) out << '\\' << c;
      else if (c == '\n') out << "\\n";
      else if (static_cast<unsigned char>(c) < 0x20) {
        static const char hex[]="0123456789abcdef";
        const unsigned char value=static_cast<unsigned char>(c);
        out << "\\u00" << hex[value >> 4] << hex[value & 15];
      }
      else out << c;
    }
    return out.str();
  }

  std::string integer_text(WordValue value) {
    const WordValue sign=WordValue(1) << 63;
    if ((value&sign) == 0)
      return std::to_string(value);
    const WordValue magnitude=(~value)+1U;
    return "-"+std::to_string(magnitude);
  }

  void print_options(std::ostream& out, const ReaderOptions& options) {
    out << ",\"timeout_ms\":" << options.timeout_ms
        << ",\"produce_models\":" << (options.produce_models ? "true" : "false")
        << ",\"ignored_options\":[";
    bool first=true;
    for (const auto& option : options.ignored) {
      if (!first) out << ',';
      first=false;
      out << "{\"name\":\"" << json_escape(option.first)
          << "\",\"value\":\"" << json_escape(option.second)
          << "\",\"reason\":\"solver-specific setting has no Gecode equivalent\"}";
    }
    out << ']';
  }

  void print_model(std::ostream& out, const Script& script,
                   const SMT2Space& solution) {
    out << ",\"model\":{";
    int wi=0, bi=0;
    bool first=true;
    for (const Decl& declaration : script.declarations) {
      if (!first) out << ',';
      first=false;
      out << '"' << json_escape(declaration.name) << "\":{\"sort\":\"";
      if (declaration.sort.kind == SORT_BOOL) {
        out << "Bool\",\"value\":" << (solution.boolean_value(bi++) ? "true" : "false");
      } else {
        const WordValue value=solution.word_value(wi++);
        if (declaration.sort.kind == SORT_INT)
          out << "Int\",\"value\":\"" << integer_text(value) << '"';
        else
          out << "BitVec\",\"width\":" << declaration.sort.width
              << ",\"value\":\"" << std::to_string(value) << '"';
      }
      out << '}';
    }
    out << '}';
  }

  WordDomainType policy(const std::string& name) {
    if (name == "cube") return WDT_CUBE;
    if (name == "unsigned") return WDT_UNSIGNED;
    if (name == "signed") return WDT_SIGNED;
    throw Error("variant must be cube, unsigned, or signed");
  }

}

// The native normalization test includes the typed reader implementation.
#ifndef GECODE_WORD_SMT2_NO_MAIN
int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "usage: word-smt2-reader VARIANT FILE "
                 "[--parse-only|--model-only] [--tables] [--model]\n";
    return 2;
  }
  bool parse_only=false, model_only=false, tables=false, emit_model=false;
  for (int i=3; i<argc; i++) {
    std::string flag=argv[i];
    if (flag=="--parse-only") parse_only=true;
    else if (flag=="--model-only") model_only=true;
    else if (flag=="--tables") tables=true;
    else if (flag=="--model") emit_model=true;
    else return 2;
  }
  if (parse_only && model_only) return 2;
  try {
    const WordDomainType domain_policy=policy(argv[1]);
    std::ifstream stream(argv[2]);
    if (!stream) throw Error("cannot open input file");
    std::string source;
    char chunk[8192];
    while (stream.read(chunk,sizeof(chunk)) || stream.gcount() != 0) {
      if (source.size()+static_cast<std::size_t>(stream.gcount()) > max_input_bytes)
        throw Error("reader input-size limit is 16 MiB");
      source.append(chunk,static_cast<std::size_t>(stream.gcount()));
    }
    if (stream.bad()) throw Error("cannot read input file");
    const auto start=std::chrono::steady_clock::now();
    Parser parser(source);
    std::vector<ReaderQuery> queries=read_session(parser.parse());
    const auto parsed=std::chrono::steady_clock::now();
    const auto session_parse_us=std::chrono::duration_cast<std::chrono::microseconds>(
      parsed-start).count();
    for (std::size_t qi=0; qi<queries.size(); qi++) {
      const ReaderQuery& query=queries[qi];
      const Script& script=query.script;
      // Parse the complete session once; charge that work to its first query.
      const auto parse_us=qi == 0 ? session_parse_us : 0;
      std::ostringstream out;
      out << "{\"status\":\"ok\",\"query\":" << qi+1
          << ",\"query_count\":" << queries.size()
          << ",\"declarations\":" << script.declarations.size()
          << ",\"assertions\":" << script.assertions.size()
          << ",\"parse_us\":" << parse_us;
      print_options(out,query.options);
      if (parse_only) {
        std::cout << out.str() << "}\n";
        continue;
      }
      const auto model_start=std::chrono::steady_clock::now();
      std::unique_ptr<SMT2Space> root(new SMT2Space(script,domain_policy,tables));
      const auto modeled=std::chrono::steady_clock::now();
      const auto model_us=std::chrono::duration_cast<std::chrono::microseconds>(
        modeled-model_start).count();
      out << ",\"model_us\":" << model_us
          << ",\"integer_normalizations\":" << root->normalizations();
      if (model_only) {
        std::cout << out.str() << "}\n";
        continue;
      }
      const unsigned int replacements=root->replacements();
      Search::TimeStop stop(query.options.timeout_ms);
      Search::Options search_options;
      if (query.options.timeout_ms != 0) search_options.stop=&stop;
      StatusStatistics root_stats;
      const SpaceStatus root_status=root->status(root_stats);
      const bool optimizing=root->optimizing();
      const bool want_model=emit_model || query.model;
      bool sat=false, stopped=false;
      WordValue objective_value=0;
      std::unique_ptr<SMT2Space> witness;
      Search::Statistics stats;
      if (optimizing) {
        BAB<SMT2Space> search(root_status == SS_FAILED ? nullptr : root.get(),
                             search_options);
        root.reset();
        while (std::unique_ptr<SMT2Space> solution=std::unique_ptr<SMT2Space>(
                 search.next())) {
          sat=true;
          objective_value=solution->objective();
          if (want_model) witness=std::move(solution);
        }
        stats=search.statistics();
        stopped=search.stopped();
      } else {
        DFS<SMT2Space> search(root_status == SS_FAILED ? nullptr : root.get(),
                             search_options);
        root.reset();
        std::unique_ptr<SMT2Space> solution(search.next());
        sat=solution != nullptr;
        if (want_model) witness=std::move(solution);
        stats=search.statistics();
        stopped=search.stopped();
      }
      const auto finished=std::chrono::steady_clock::now();
      const auto solve_us=std::chrono::duration_cast<std::chrono::microseconds>(
        finished-modeled).count();
      out << ",\"result\":\"" << (stopped ? "unknown" : sat ? "sat" : "unsat")
          << "\",\"variant\":\"" << argv[1] << "\",\"elapsed_us\":"
          << parse_us+model_us+solve_us
          << ",\"table_replacements\":" << replacements
          << ",\"solve_us\":" << solve_us;
      if (stopped) out << ",\"reason_unknown\":\"timeout\"";
      if (optimizing && sat)
        out << ",\"objective\":" << integer_text(objective_value)
            << ",\"objective_decimal\":\"" << integer_text(objective_value)
            << "\",\"optimal\":" << (stopped ? "false" : "true");
      if (witness) print_model(out,script,*witness);
      else if (query.model)
        out << ",\"model_error\":\"model unavailable for "
            << (stopped ? "unknown" : "unsat") << " result\"";
      out << ",\"nodes\":" << stats.node << ",\"failures\":"
          << stats.fail << ",\"propagations\":"
          << root_stats.propagate+stats.propagate << "}\n";
      std::cout << out.str();
    }
    return 0;
  } catch (const std::exception& e) {
    std::cout << "{\"status\":\"error\",\"message\":\""
              << json_escape(e.what()) << "\"}\n";
    return 1;
  }
}

#endif
