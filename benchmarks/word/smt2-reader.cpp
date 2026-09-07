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
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace Gecode;

namespace {

  struct Error : public std::runtime_error {
    explicit Error(const std::string& message) : std::runtime_error(message) {}
  };

  struct SExpr {
    bool atom;
    std::string text;
    std::vector<SExpr> items;
    unsigned int line;

    SExpr(std::string text0, unsigned int line0)
      : atom(true), text(std::move(text0)), line(line0) {}
    SExpr(std::vector<SExpr> items0, unsigned int line0)
      : atom(false), items(std::move(items0)), line(line0) {}
  };

  class Parser {
  private:
    const std::string& input;
    std::size_t position;
    unsigned int line;

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

    SExpr expression(void) {
      whitespace();
      if (position == input.size())
        throw Error("unexpected end of input");
      const unsigned int start_line=line;
      if (input[position] == '(') {
        position++;
        std::vector<SExpr> items;
        for (;;) {
          whitespace();
          if (position == input.size())
            throw Error("unterminated list starting on line " +
                        std::to_string(start_line));
          if (input[position] == ')') {
            position++;
            return SExpr(std::move(items),start_line);
          }
          items.push_back(expression());
        }
      }
      if (input[position] == ')')
        throw Error("unexpected ')' on line " + std::to_string(line));
      std::string token;
      if (input[position] == '|') {
        position++;
        while ((position < input.size()) && (input[position] != '|')) {
          if (input[position] == '\\') {
            position++;
            if (position == input.size())
              throw Error("unterminated quoted symbol on line " +
                          std::to_string(start_line));
          }
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
      if (token.empty())
        throw Error("empty token on line " + std::to_string(start_line));
      return SExpr(std::move(token),start_line);
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

  enum SortKind { SORT_BOOL, SORT_WORD, SORT_INT };

  struct Sort {
    SortKind kind;
    unsigned int width;
    Sort(SortKind kind0=SORT_INT, unsigned int width0=0)
      : kind(kind0), width(width0) {}
  };

  bool same_sort(const Sort& a, const Sort& b) {
    return (a.kind == b.kind) &&
      ((a.kind != SORT_WORD) || (a.width == b.width));
  }

  unsigned int unsigned_atom(const SExpr& e, const char* context) {
    if (!e.atom || e.text.empty())
      throw Error(std::string(context) +
                  " expects an unsigned integer on line " +
                  std::to_string(e.line));
    char* end=nullptr;
    unsigned long value=std::strtoul(e.text.c_str(),&end,10);
    if ((*end != '\0') || (value > 0xffffffffUL))
      throw Error(std::string(context) + " has an invalid integer on line " +
                  std::to_string(e.line));
    return static_cast<unsigned int>(value);
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
    std::vector<Decl> declarations;
    std::vector<SExpr> assertions;
    std::unordered_map<std::string,Sort> globals;
  };

  bool is_atom(const SExpr& e, const char* value) {
    return e.atom && (e.text == value);
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

  Sort infer(const SExpr& e, const TypeEnv& env) {
    if (e.atom) {
      if ((e.text == "true") || (e.text == "false")) return Sort(SORT_BOOL);
      Sort result;
      if (literal_sort(e.text,result)) {
        if ((result.width == 0U) || (result.width > 64U))
          throw Error("bit-vector literal width is outside 1..64 on line " +
                      std::to_string(e.line));
        return result;
      }
      if (env.find(e.text,result)) return result;
      throw Error("unknown symbol '" + e.text + "' on line " +
                  std::to_string(e.line));
    }
    if (e.items.empty())
      throw Error("empty application on line " + std::to_string(e.line));

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
      if (indexed.items[1].text == "zero_extend") {
        if (indexed.items.size() != 3)
          throw Error("zero_extend expects one index on line " +
                      std::to_string(e.line));
        unsigned int extra=unsigned_atom(indexed.items[2],"zero_extend");
        if ((extra > 64U) || (source.width+extra > 64U))
          throw Error("zero_extend result exceeds 64 bits on line " +
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
    if ((op == "and") || (op == "or")) {
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
      if (first.kind == SORT_INT)
        throw Error("integer relations are outside this WordVar reader on "
                    "line " + std::to_string(e.line));
      return Sort(SORT_BOOL);
    }
    if (op == "ite") {
      require_arity(e,3);
      if (infer(e.items[1],env).kind != SORT_BOOL)
        throw Error("ite condition is not Bool on line " +
                    std::to_string(e.line));
      Sort then_sort=infer(e.items[2],env), else_sort=infer(e.items[3],env);
      if (!same_sort(then_sort,else_sort) || (then_sort.kind == SORT_INT))
        throw Error("ite branches have unsupported or different sorts on "
                    "line " + std::to_string(e.line));
      return then_sort;
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
    if (op == "bvneg") {
      require_arity(e,1); Sort s=infer(e.items[1],env);
      if (s.kind != SORT_WORD) throw Error("bvneg expects a word"); return s;
    }
    if ((op == "bvadd") || (op == "bvmul")) {
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
        (op == "bvsmod_i")) {
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

  Script read_script(const std::vector<SExpr>& forms) {
    Script script;
    bool checked=false;
    bool pushed=false;
    for (const SExpr& form : forms) {
      const std::string command=head(form);
      if (command == "declare-fun") {
        if ((form.items.size() != 4) || !form.items[1].atom ||
            form.items[2].atom || !form.items[2].items.empty())
          throw Error("only nullary declare-fun is supported on line " +
                      std::to_string(form.line));
        Decl d={form.items[1].text,parse_sort(form.items[3])};
        if (!script.globals.emplace(d.name,d.sort).second)
          throw Error("duplicate declaration '" + d.name + "'");
        script.declarations.push_back(d);
      } else if (command == "assert") {
        require_arity(form,1);
        if (checked) throw Error("assert after check-sat is unsupported");
        script.assertions.push_back(form.items[1]);
      } else if (command == "push") {
        if (pushed || checked || (form.items.size() > 2) ||
            ((form.items.size() == 2) && !is_atom(form.items[1],"1")))
          throw Error("only one scope-neutral top-level push is supported");
        pushed=true;
      } else if (command == "check-sat") {
        require_arity(form,0);
        if (checked) throw Error("multiple check-sat commands are unsupported");
        checked=true;
      } else if ((command == "set-logic") || (command == "set-info")) {
        // Metadata does not change the asserted QF_BV formula.
      } else if (command == "exit") {
        require_arity(form,0);
      } else {
        throw Error("unsupported command '" + command + "' on line " +
                    std::to_string(form.line));
      }
    }
    if (!checked) throw Error("script has no check-sat command");
    TypeEnv env;
    env.local=script.globals;
    for (const SExpr& assertion : script.assertions)
      if (infer(assertion,env).kind != SORT_BOOL)
        throw Error("assert expects Bool on line " +
                    std::to_string(assertion.line));
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
    const EvalEnv* parent;
    std::unordered_map<std::string,std::unique_ptr<WordExpr> > words;
    std::unordered_map<std::string,std::unique_ptr<BoolExpr> > bools;
    std::unordered_map<std::string,Sort> sorts;
    explicit EvalEnv(const EvalEnv* parent0=nullptr) : parent(parent0) {}
    const WordExpr* word(const std::string& name) const {
      auto i=words.find(name);
      if (i != words.end()) return i->second.get();
      return parent == nullptr ? nullptr : parent->word(name);
    }
    const BoolExpr* boolean(const std::string& name) const {
      auto i=bools.find(name);
      if (i != bools.end()) return i->second.get();
      return parent == nullptr ? nullptr : parent->boolean(name);
    }
  };

  class SMT2Space : public Space {
  private:
    WordDomainType policy;
    WordVarArray words;
    BoolVarArray bools;
    std::unordered_map<std::string,int> word_index;
    std::unordered_map<std::string,int> bool_index;

    WordExpr eval_word(const SExpr& e, const EvalEnv& env) {
      if (e.atom) {
        Sort literal;
        if (literal_sort(e.text,literal))
          return WordExpr(literal.width,word_literal(e.text));
        if (const WordExpr* value=env.word(e.text)) return *value;
        auto i=word_index.find(e.text);
        if (i != word_index.end()) return WordExpr(words[i->second]);
        throw Error("word symbol '" + e.text + "' is unavailable");
      }
      if (!e.items[0].atom) {
        const SExpr& indexed=e.items[0];
        WordExpr source=eval_word(e.items[1],env);
        if (indexed.items[1].text == "extract") {
          unsigned int high=unsigned_atom(indexed.items[2],"extract");
          unsigned int low=unsigned_atom(indexed.items[3],"extract");
          return Gecode::extract(source,low,high-low+1U);
        }
        unsigned int extra=unsigned_atom(indexed.items[2],"zero_extend");
        return Gecode::zero_extend(source,source.width()+extra);
      }
      const std::string op=e.items[0].text;
      if (op == "let") {
        EvalEnv nested(&env);
        // Binding sorts were checked before Space construction.
        for (const auto& binding : let_bindings(e)) {
          Sort s=infer(*binding.second,types_for(env));
          nested.sorts.emplace(binding.first,s);
          if (s.kind == SORT_WORD)
            nested.words.emplace(binding.first,std::unique_ptr<WordExpr>(
              new WordExpr(eval_word(*binding.second,env))));
          else if (s.kind == SORT_BOOL)
            nested.bools.emplace(binding.first,std::unique_ptr<BoolExpr>(
              new BoolExpr(eval_bool(*binding.second,env))));
        }
        return eval_word(e.items[2],nested);
      }
      if (op == "ite")
        return Gecode::ite(eval_bool(e.items[1],env),
                           eval_word(e.items[2],env),eval_word(e.items[3],env));
      if (op == "bvneg") return -eval_word(e.items[1],env);
      WordExpr a=eval_word(e.items[1],env);
      if ((op == "bvadd") || (op == "bvmul")) {
        for (std::size_t i=2; i<e.items.size(); i++)
          a=(op == "bvadd") ? a+eval_word(e.items[i],env) :
            a*eval_word(e.items[i],env);
        return a;
      }
      WordExpr b=eval_word(e.items[2],env);
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
      return e.atom && literal_sort(e.text,sort) &&
        (word_literal(e.text) == 0);
    }

    BoolExpr eval_bool(const SExpr& e, const EvalEnv& env) {
      if (e.atom) {
        if (e.text == "true") return constant_bool(true);
        if (e.text == "false") return constant_bool(false);
        if (const BoolExpr* value=env.boolean(e.text)) return *value;
        auto i=bool_index.find(e.text);
        if (i != bool_index.end()) return BoolExpr(bools[i->second]);
        throw Error("Boolean symbol '" + e.text + "' is unavailable");
      }
      const std::string op=head(e);
      if (op == "let") {
        EvalEnv nested(&env);
        TypeEnv tenv=types_for(env);
        for (const auto& binding : let_bindings(e)) {
          Sort s=infer(*binding.second,tenv);
          nested.sorts.emplace(binding.first,s);
          if (s.kind == SORT_WORD)
            nested.words.emplace(binding.first,std::unique_ptr<WordExpr>(
              new WordExpr(eval_word(*binding.second,env))));
          else if (s.kind == SORT_BOOL)
            nested.bools.emplace(binding.first,std::unique_ptr<BoolExpr>(
              new BoolExpr(eval_bool(*binding.second,env))));
        }
        return eval_bool(e.items[2],nested);
      }
      if (op == "not") return !eval_bool(e.items[1],env);
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
        TypeEnv tenv=types_for(env);
        Sort s=infer(e.items[1],tenv);
        BoolExpr result;
        if (op == "=") {
          for (std::size_t i=2; i<e.items.size(); i++)
            result = result && (s.kind == SORT_BOOL ?
              eval_bool(e.items[i-1],env) == eval_bool(e.items[i],env) :
              word_rel(eval_word(e.items[i-1],env),WRT_EQ,
                       eval_word(e.items[i],env),policy));
        } else {
          for (std::size_t i=1; i<e.items.size(); i++)
            for (std::size_t j=i+1; j<e.items.size(); j++)
              result = result && (s.kind == SORT_BOOL ?
                eval_bool(e.items[i],env) != eval_bool(e.items[j],env) :
                word_rel(eval_word(e.items[i],env),WRT_NQ,
                         eval_word(e.items[j],env),policy));
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
      if (op == "bvule") relation=WRT_ULQ;
      else if (op == "bvult") relation=WRT_ULE;
      else if (op == "bvuge") relation=WRT_UGQ;
      else if (op == "bvugt") relation=WRT_UGR;
      else if (op == "bvsle") relation=WRT_SLQ;
      else if (op == "bvslt") relation=WRT_SLE;
      else if (op == "bvsge") relation=WRT_SGQ;
      else if (op == "bvsgt") relation=WRT_SGR;
      else throw Error("unsupported Boolean operator '" + op + "'");
      return word_rel(eval_word(e.items[1],env),relation,
                      eval_word(e.items[2],env),policy);
    }

    std::unordered_map<std::string,Sort> global_sorts;

    void add_types(const EvalEnv* env, TypeEnv& result) const {
      if (env == nullptr) return;
      add_types(env->parent,result);
      for (const auto& entry : env->sorts)
        result.local[entry.first]=entry.second;
    }

    TypeEnv types_for(const EvalEnv& env) const {
      TypeEnv result;
      result.local=global_sorts;
      add_types(&env,result);
      return result;
    }

  public:
    SMT2Space(const Script& script, WordDomainType policy0)
      : policy(policy0), words(*this,static_cast<int>(
          [&script](){ std::size_t n=0; for (const Decl& d:script.declarations)
            if (d.sort.kind==SORT_WORD) n++; return n; }())),
        bools(*this,static_cast<int>(
          [&script](){ std::size_t n=0; for (const Decl& d:script.declarations)
            if (d.sort.kind==SORT_BOOL) n++; return n; }())) {
      int wi=0, bi=0;
      for (const Decl& d : script.declarations) {
        global_sorts.emplace(d.name,d.sort);
        if (d.sort.kind == SORT_WORD) {
          words[wi]=(policy == WDT_CUBE) ? WordVar(*this,d.sort.width) :
            WordVar(*this,d.sort.width,policy);
          word_index.emplace(d.name,wi++);
        } else if (d.sort.kind == SORT_BOOL) {
          bools[bi]=BoolVar(*this,0,1);
          bool_index.emplace(d.name,bi++);
        }
      }
      EvalEnv env;
      for (const SExpr& assertion : script.assertions)
        Gecode::rel(*this,eval_bool(assertion,env));
      WordVarArgs active_words;
      for (int i=0; i<words.size(); i++)
        if (words[i].degree() != 0)
          active_words << words[i];
      if (active_words.size() != 0)
        branch(*this,active_words,WORD_VAR_SIZE_MIN(),
               policy == WDT_CUBE ? WORD_VAL_MSB() : WORD_VAL_SPLIT_MIN());
      BoolVarArgs active_bools;
      for (int i=0; i<bools.size(); i++)
        if (bools[i].degree() != 0)
          active_bools << bools[i];
      if (active_bools.size() != 0)
        branch(*this,active_bools,BOOL_VAR_NONE(),BOOL_VAL_MIN());
    }

    SMT2Space(SMT2Space& other)
      : Space(other), policy(other.policy) {
      words.update(*this,other.words);
      bools.update(*this,other.bools);
    }
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

  WordDomainType policy(const std::string& name) {
    if (name == "cube") return WDT_CUBE;
    if (name == "unsigned") return WDT_UNSIGNED;
    if (name == "signed") return WDT_SIGNED;
    throw Error("variant must be cube, unsigned, or signed");
  }

}

int main(int argc, char* argv[]) {
  if ((argc != 3) && (argc != 4)) {
    std::cerr << "usage: word-smt2-reader VARIANT FILE "
                 "[--parse-only|--model-only]\n";
    return 2;
  }
  const bool parse_only=(argc == 4) && (std::string(argv[3]) == "--parse-only");
  const bool model_only=(argc == 4) && (std::string(argv[3]) == "--model-only");
  if ((argc == 4) && !parse_only && !model_only) return 2;
  try {
    std::ifstream stream(argv[2]);
    if (!stream) throw Error("cannot open input file");
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    auto start=std::chrono::steady_clock::now();
    const std::string source=buffer.str();
    Parser parser(source);
    Script script=read_script(parser.parse());
    if (parse_only) {
      std::cout << "{\"status\":\"ok\",\"declarations\":"
                << script.declarations.size() << ",\"assertions\":"
                << script.assertions.size() << "}\n";
      return 0;
    }
    SMT2Space* root=new SMT2Space(script,policy(argv[1]));
    if (model_only) {
      delete root;
      std::cout << "{\"status\":\"ok\",\"declarations\":"
                << script.declarations.size() << ",\"assertions\":"
                << script.assertions.size() << "}\n";
      return 0;
    }
    StatusStatistics root_stats;
    SpaceStatus root_status=root->status(root_stats);
    DFS<SMT2Space> search(root_status == SS_FAILED ? nullptr : root);
    delete root;
    SMT2Space* solution=search.next();
    const bool sat=solution != nullptr;
    delete solution;
    Search::Statistics stats=search.statistics();
    auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now()-start).count();
    std::cout << "{\"status\":\"ok\",\"result\":\""
              << (sat ? "sat" : "unsat") << "\",\"variant\":\""
              << argv[1] << "\",\"elapsed_us\":" << elapsed
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
