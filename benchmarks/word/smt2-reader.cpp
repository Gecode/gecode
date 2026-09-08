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
      for (const auto& binding : let_bindings(e)) {
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

  class SMT2Space : public Space {
  private:
    WordDomainType policy;
    Script::Objective objective_kind;
    WordVar objective_word;
    bool tables;
    unsigned int table_count=0;
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

    WordExpr integer_div(const WordExpr& a, const WordExpr& b,
                         const SExpr& divisor) {
      const WordExpr zero(64,0), one(64,1);
      const WordDomainType integer_domain=expression_policy(Sort(SORT_INT));
      WordValue constant;
      if (integer_literal(divisor,constant)) {
        if (constant == 0) return zero;
        if (constant == 1) return a;
        const WordExpr quotient=signed_div(a,b,WS_SMTLIB);
        const WordExpr remainder=signed_rem(a,b,WS_SMTLIB);
        const BoolExpr remainder_negative=
          word_rel(remainder,WRT_SLE,zero,integer_domain);
        return Gecode::ite(remainder_negative,quotient-one,quotient);
      }
      const BoolExpr b_zero=word_rel(b,WRT_EQ,zero,integer_domain);
      const BoolExpr b_negative=word_rel(b,WRT_SLE,zero,integer_domain);
      const WordExpr quotient=signed_div(a,b,WS_SMTLIB);
      const WordExpr remainder=signed_rem(a,b,WS_SMTLIB);
      const BoolExpr remainder_negative=
        word_rel(remainder,WRT_SLE,zero,integer_domain);
      const WordExpr adjusted=Gecode::ite(
        remainder_negative,
        Gecode::ite(b_negative,quotient+one,quotient-one),quotient);
      // SMT-LIB leaves division by zero underspecified. Zero matches the
      // conventional concrete interpretation used by this reader.
      return Gecode::ite(b_zero,zero,adjusted);
    }

    WordExpr integer_mod(const WordExpr& a, const WordExpr& b,
                         const SExpr& divisor) {
      const WordExpr zero(64,0);
      const WordDomainType integer_domain=expression_policy(Sort(SORT_INT));
      WordValue constant;
      if (integer_literal(divisor,constant)) {
        if (constant == 0) return zero;
        if (constant == 1) return zero;
        // SMT Int mod has a nonnegative result. With a positive divisor this
        // is exactly the native signed-mod operation, including its optimized
        // constant and power-of-two propagators.
        return signed_mod(a,b,WS_SMTLIB);
      }
      const BoolExpr b_zero=word_rel(b,WRT_EQ,zero,integer_domain);
      const BoolExpr b_negative=word_rel(b,WRT_SLE,zero,integer_domain);
      const WordExpr magnitude=Gecode::ite(b_negative,-b,b);
      const WordExpr remainder=signed_rem(a,magnitude,WS_SMTLIB);
      const BoolExpr remainder_negative=
        word_rel(remainder,WRT_SLE,zero,integer_domain);
      const WordExpr adjusted=Gecode::ite(
        remainder_negative,remainder+magnitude,remainder);
      return Gecode::ite(b_zero,zero,adjusted);
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
      int wi=0, bi=0;
      for (const Decl& d : script.declarations) {
        if (d.sort.kind != SORT_BOOL) {
          if (d.sort.kind == SORT_INT)
            words[wi]=(policy == WDT_CUBE) ? WordVar(*this,64) :
              WordVar(*this,64,WDT_SIGNED);
          else
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
      WordVarArgs active_cube, active_bounded;
      for (int i=0; i<words.size(); i++)
        if (words[i].degree() != 0) {
          if (words[i].domain_type() == WDT_CUBE)
            active_cube << words[i];
          else
            active_bounded << words[i];
        }
      if ((objective_kind != Script::SATISFY) && !objective_word.assigned()) {
        int objective_declaration=-1;
        for (int i=0; i<words.size(); i++)
          if (words[i].varimp() == objective_word.varimp()) {
            objective_declaration=i;
            break;
          }
        if ((objective_declaration >= 0) &&
            (words[objective_declaration].degree() == 0)) {
          if (objective_word.domain_type() == WDT_CUBE)
            active_cube << objective_word;
          else
            active_bounded << objective_word;
        }
      }
      if (active_bounded.size() != 0)
        branch(*this,active_bounded,WORD_VAR_SIZE_MIN(),
               objective_kind == Script::MAXIMIZE ?
               WORD_VAL_SPLIT_MAX() : WORD_VAL_SPLIT_MIN());
      if (active_cube.size() != 0)
        branch(*this,active_cube,WORD_VAR_SIZE_MIN(),WORD_VAL_MSB());
      BoolVarArgs active_bools;
      for (int i=0; i<bools.size(); i++)
        if (bools[i].degree() != 0)
          active_bools << bools[i];
      if (active_bools.size() != 0)
        branch(*this,active_bools,BOOL_VAR_NONE(),BOOL_VAL_MIN());
    }

    SMT2Space(SMT2Space& other)
      : Space(other), policy(other.policy),
        objective_kind(other.objective_kind), tables(other.tables),
        table_count(other.table_count) {
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
    virtual Space* copy(void) { return new SMT2Space(*this); }
  };

  std::string json_escape(const std::string& text) {
    std::ostringstream out;
    for (char c : text) {
      if ((c == '\\') || (c == '"')) out << '\\' << c;
      else if (c == '\n') out << "\\n";
      else if (static_cast<unsigned char>(c) < 0x20)
        out << '?';
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

  WordDomainType policy(const std::string& name) {
    if (name == "cube") return WDT_CUBE;
    if (name == "unsigned") return WDT_UNSIGNED;
    if (name == "signed") return WDT_SIGNED;
    throw Error("variant must be cube, unsigned, or signed");
  }

}

int main(int argc, char* argv[]) {
  if ((argc < 3) || (argc > 5)) {
    std::cerr << "usage: word-smt2-reader VARIANT FILE "
                 "[--parse-only|--model-only] [--tables]\n";
    return 2;
  }
  bool parse_only=false, model_only=false, tables=false;
  for (int i=3; i<argc; i++) {
    std::string flag=argv[i];
    if (flag=="--parse-only") parse_only=true;
    else if (flag=="--model-only") model_only=true;
    else if (flag=="--tables") tables=true;
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
    auto start=std::chrono::steady_clock::now();
    Parser parser(source);
    Script script=read_script(parser.parse());
    const auto parsed=std::chrono::steady_clock::now();
    const auto parse_us=std::chrono::duration_cast<std::chrono::microseconds>(
      parsed-start).count();
    if (parse_only) {
      std::cout << "{\"status\":\"ok\",\"declarations\":"
                << script.declarations.size() << ",\"assertions\":"
                << script.assertions.size() << ",\"parse_us\":" << parse_us
                << "}\n";
      return 0;
    }
    std::unique_ptr<SMT2Space> root(new SMT2Space(script,domain_policy,tables));
    const auto modeled=std::chrono::steady_clock::now();
    const auto model_us=std::chrono::duration_cast<std::chrono::microseconds>(
      modeled-parsed).count();
    if (model_only) {
      root.reset();
      std::cout << "{\"status\":\"ok\",\"declarations\":"
                << script.declarations.size() << ",\"assertions\":"
                << script.assertions.size() << ",\"parse_us\":" << parse_us
                << ",\"model_us\":" << model_us << "}\n";
      return 0;
    }
    const unsigned int replacements=root->replacements();
    StatusStatistics root_stats;
    SpaceStatus root_status=root->status(root_stats);
    const bool optimizing=root->optimizing();
    bool sat=false;
    WordValue objective_value=0;
    Search::Statistics stats;
    if (optimizing) {
      BAB<SMT2Space> search(root_status == SS_FAILED ? nullptr : root.get());
      root.reset();
      while (std::unique_ptr<SMT2Space> solution=std::unique_ptr<SMT2Space>(
               search.next())) {
        sat=true;
        objective_value=solution->objective();
      }
      stats=search.statistics();
    } else {
      DFS<SMT2Space> search(root_status == SS_FAILED ? nullptr : root.get());
      root.reset();
      std::unique_ptr<SMT2Space> solution(search.next());
      sat=solution != nullptr;
      stats=search.statistics();
    }
    const auto finished=std::chrono::steady_clock::now();
    const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(
      finished-start).count();
    const auto solve_us=std::chrono::duration_cast<std::chrono::microseconds>(
      finished-modeled).count();
    std::cout << "{\"status\":\"ok\",\"result\":\""
              << (sat ? "sat" : "unsat") << "\",\"variant\":\""
              << argv[1] << "\",\"elapsed_us\":" << elapsed
              << ",\"parse_us\":" << parse_us << ",\"model_us\":" << model_us
              << ",\"table_replacements\":" << replacements
              << ",\"solve_us\":" << solve_us
              << (optimizing && sat ? ",\"objective\":"+
                  integer_text(objective_value)+",\"optimal\":true" : "")
              << ",\"nodes\":" << stats.node << ",\"failures\":"
              << stats.fail << ",\"propagations\":"
              << (root_stats.propagate+stats.propagate) << "}\n";
    return 0;
  } catch (const std::exception& e) {
    std::cout << "{\"status\":\"error\",\"message\":\""
              << json_escape(e.what()) << "\"}\n";
    return 1;
  }
}
