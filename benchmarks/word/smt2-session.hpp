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

// Internal to smt2-reader.cpp; included after the typed single-query reader.
#ifndef GECODE_BENCHMARK_WORD_SMT2_SESSION_HPP
#define GECODE_BENCHMARK_WORD_SMT2_SESSION_HPP

struct ReaderOptions {
  unsigned int timeout_ms=0;
  bool produce_models=false;
  std::vector<std::pair<std::string,std::string> > ignored;
};

struct ReaderQuery {
  Script script;
  ReaderOptions options;
  bool model=false;
  ReaderQuery(Script&& script0, const ReaderOptions& options0)
    : script(std::move(script0)), options(options0) {}
};

std::size_t syntax_weight(const SExpr& term) {
  std::size_t size=sizeof(SExpr)+term.text.size();
  for (const SExpr& child : term.items)
    size+=syntax_weight(child);
  return size;
}

bool option_boolean(const SExpr& value) {
  if (value.atom && value.token == SExpr::SIMPLE) {
    if (value.text == "true") return true;
    if (value.text == "false") return false;
  }
  throw Error("option expects true or false on line "+
              std::to_string(value.line));
}

void read_option(const SExpr& form, ReaderOptions& options) {
  require_arity(form,2);
  if (!form.items[1].atom || form.items[1].token != SExpr::SIMPLE)
    throw Error("option name must be a keyword");
  const std::string& name=form.items[1].text;
  const SExpr& value=form.items[2];
  if (name == ":timeout") {
    options.timeout_ms=unsigned_atom(value,"timeout");
  } else if (name == ":produce-models") {
    options.produce_models=option_boolean(value);
  } else if (name == ":global-declarations") {
    if (option_boolean(value))
      throw Error("global-declarations=true is unsupported");
  } else if ((name == ":rlimit") || (name == ":opt.rlimit") ||
             (name == ":opt.arith.solver") ||
             (name == ":smt.arith.solver") ||
             (name == ":smt.arith.nl.nra")) {
    // Z3 work counters and arithmetic backend selectors have no Gecode
    // equivalent. Accept these capture settings only with explicit reporting.
    if (name == ":smt.arith.nl.nra")
      option_boolean(value);
    else
      unsigned_atom(value,"solver-specific option");
    for (auto& entry : options.ignored)
      if (entry.first == name) {
        entry.second=value.text;
        return;
      }
    options.ignored.emplace_back(name,value.text);
  } else {
    throw Error("unsupported option '"+name+"'");
  }
}

std::vector<ReaderQuery> read_session(std::vector<SExpr> forms) {
  std::vector<ReaderQuery> queries;
  std::vector<SExpr> active;
  std::vector<std::pair<std::size_t,std::size_t> > scopes;
  ReaderOptions options;
  std::size_t active_weight=0, expanded_weight=0;
  bool result_available=false, unvalidated=false;
  const std::size_t max_session_bytes=64U*1024U*1024U;
  auto snapshot=[&](const SExpr& check) {
    if (expanded_weight+active_weight > max_session_bytes)
      throw Error("reader expanded-query limit is 64 MiB");
    expanded_weight+=active_weight;
    active.push_back(check);
    Script result=read_script(active);
    active.pop_back();
    unvalidated=false;
    return result;
  };
  auto validate=[&]() {
    if (unvalidated)
      snapshot(SExpr(std::vector<SExpr>{SExpr("check-sat",1)},1));
  };
  for (SExpr& form : forms) {
    const std::string command=head(form);
    if (command == "set-option") {
      read_option(form,options);
    } else if ((command == "push") || (command == "pop")) {
      if (form.items.size() > 2)
        throw Error("push/pop expects one scope count");
      const unsigned int count=form.items.size() == 1 ? 1U :
        unsigned_atom(form.items[1],"scope count");
      if (command == "push") {
        if (count > max_nesting-scopes.size())
          throw Error("reader scope limit is 128");
        for (unsigned int i=0; i<count; i++)
          scopes.emplace_back(active.size(),active_weight);
      } else {
        if (count > scopes.size()) throw Error("pop exceeds active scopes");
        // A scope without check-sat still has to contain valid commands and
        // well-typed terms. Validate before those commands are discarded.
        if (count != 0) validate();
        for (unsigned int i=0; i<count; i++) {
          active.erase(active.begin()+scopes.back().first,active.end());
          active_weight=scopes.back().second;
          scopes.pop_back();
        }
      }
      if (count != 0) result_available=false;
    } else if (command == "check-sat") {
      require_arity(form,0);
      if (queries.size() == 128)
        throw Error("reader query limit is 128");
      queries.emplace_back(snapshot(form),options);
      result_available=true;
    } else if ((command == "get-model") || (command == "get-objectives")) {
      require_arity(form,0);
      if (!result_available)
        throw Error(command+" requires a preceding check-sat with no state changes");
      if (command == "get-model") {
        if (!queries.back().options.produce_models)
          throw Error("get-model requires produce-models=true before check-sat");
        queries.back().model=true;
      }
    } else if (command == "exit") {
      require_arity(form,0);
      break;
    } else {
      active_weight+=syntax_weight(form);
      active.push_back(std::move(form));
      unvalidated=true;
      if ((command != "set-info") && (command != "set-logic"))
        result_available=false;
    }
  }
  if (queries.empty()) throw Error("script has no check-sat command");
  if (unvalidated) {
    // Validate commands after the last query as well. This extra snapshot is
    // not solved and does not produce a result record.
    validate();
  }
  return queries;
}

#endif
