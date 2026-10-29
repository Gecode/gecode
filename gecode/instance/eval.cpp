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

#include <gecode/instance/eval.hpp>

#include <climits>
#include <iomanip>
#include <sstream>

namespace Gecode { namespace Instance {

  std::string
  quote(const std::string& s) {
    std::string r = "'";
    for (char c : s) {
      if (c == '\n') r += "\\n";
      else if (c == '\t') r += "\\t";
      else r += c;
    }
    return r + "'";
  }

  bool
  add(long long int x, long long int y, long long int& r) {
    if (((y > 0) && (x > LLONG_MAX - y)) || ((y < 0) && (x < LLONG_MIN - y)))
      return false;
    r = x + y;
    return true;
  }

  bool
  sub(long long int x, long long int y, long long int& r) {
    if (((y < 0) && (x > LLONG_MAX + y)) || ((y > 0) && (x < LLONG_MIN + y)))
      return false;
    r = x - y;
    return true;
  }

  namespace {

    /// Store \a x * \a y in \a r unless it overflows
    bool mul(long long int x, long long int y, long long int& r) {
      // Sign-aware bounds that never negate LLONG_MIN
      bool ok;
      if (x > 0)
        ok = (y > 0) ? (x <= LLONG_MAX / y) : (y >= LLONG_MIN / x);
      else if (x < 0)
        ok = (y > 0) ? (x >= LLONG_MIN / y) : ((y == 0) || (y >= LLONG_MAX / x));
      else
        ok = true;
      if (ok)
        r = x * y;
      return ok;
    }

    /// Store \a x to the power of \a y >= 0 in \a r unless it overflows
    bool pow(long long int x, long long int y, long long int& r) {
      // Squaring the base only overflows if the result does, as the base
      // is squared only while bits of the exponent remain
      r = 1;
      while (y > 0) {
        if (((y & 1) != 0) && !mul(r, x, r))
          return false;
        y >>= 1;
        if ((y > 0) && !mul(x, x, x))
          return false;
      }
      return true;
    }

    /**
     * \brief Find the part of array \a v that the indices \a path select
     *
     * Return whether the indices are in range, and then the level of the
     * part (the number of indices) and its position among the lists or
     * leaves of that level. Needs the starts of \a v (see Value::index).
     */
    bool locate(const Value& v, const Seq<int>& path, int& level, int& pos) {
      assert(v.starts.size() == v.depth);
      level = 0;
      pos = 0;
      for (int i : path) {
        if ((level >= v.depth) || (i < 0) || (i >= v.lengths[level][pos]))
          return false;
        pos = v.starts[level][pos] + i;
        level++;
      }
      return true;
    }

    /// Return the part of array \a v at position \a pos of level \a level
    /// (see locate)
    Value part(const Value& v, int level, int pos) {
      Value r(v.type);
      r.depth = v.depth-level;
      // The lists of the part at each level, then its leaves
      int from = pos, to = pos+1;
      for (int k=level; k<v.depth; k++) {
        r.lengths.push(Seq<int>());
        for (int j=from; j<to; j++)
          r.lengths.back().push(v.lengths[k][j]);
        from = v.starts[k][from];
        to = v.starts[k][to];
      }
      for (int j=from; j<to; j++)
        switch (v.type) {
        case ValueType::INT: r.ints.push(v.ints[j]); break;
        case ValueType::FLOAT: r.reals.push(v.reals[j]); break;
        default: r.strs.push(v.strs[j]); break;
        }
      r.index();
      return r;
    }

    /**
     * \brief Return element \a i of the outermost list of array \a v
     *
     * The element is a scalar for a one-dimensional array and a row
     * otherwise. Needs the starts of \a v (see Value::index).
     */
    Value element(const Value& v, int i) {
      assert((v.depth > 0) && (v.starts.size() == v.depth) && (i >= 0) && (i < v.size()));
      return part(v, 1, i);
    }

  }

  Evaluator::Evaluator(const SpecImpl& s, const std::string& text, const std::string& origin)
    : spec(s), in(text, origin), spec_line(0), bindings(nullptr), checked(nullptr), record_at(nullptr), effort(0),
      max_effort(base_effort + 16 * static_cast<long long int>(text.size())) {
    frames.push(Seq<Entry>());
  }

  std::string
  Evaluator::located(const std::string& msg) {
    return located(msg, in.here());
  }

  std::string
  Evaluator::located(const std::string& msg, const Place& at) {
    std::ostringstream os;
    size_t col = at.pos - at.start + 1;
    os << in.origin << ":" << at.line << ":" << col << ": " << msg << "\n";
    std::string l = in.line_at(at.start);
    if (l.size() > 100) l = l.substr(0, 100) + "...";
    os << std::setw(6) << at.line << " | " << l << "\n"
       << "       | " << std::string(col > 1 ? col-1 : 0, ' ') << "^";
    if (!context.empty()) {
      os << "\n  while reading: ";
      for (int k=0; k<context.size(); k++) {
        const Context& x = context[k];
        os << (k > 0 ? " > " : "") << x.text;
        if (x.iteration > 0) {
          os << " [" << x.iteration;
          if (x.iterations >= 0)
            os << " of " << x.iterations;
          os << "]";
        }
      }
    }
    if (spec_line > 0)
      os << "\n  spec: " << spec.origin << ":" << spec_line;
    return os.str();
  }

  void
  Evaluator::fail(const std::string& msg) {
    throw Error(located(msg));
  }

  void
  Evaluator::fail(const ShapeError& e, const std::string& n) {
    fail("'" + n + "': " + e.msg);
  }

  void
  Evaluator::visit(long long int k) {
    effort += k;
    if (effort > max_effort)
      fail("the expressions do too much work for the size of the input");
  }

  void
  Evaluator::set(const std::string& n, Value&& v) {
    // Expressions access the rows of arrays through the starts
    v.index();
    Seq<Entry>& f = frames.back();
    for (Entry& e : f)
      if (e.name == n) {
        e.value = std::move(v);
        return;
      }
    f.push(Entry(n, std::move(v)));
  }

  const Value*
  Evaluator::lookup(const std::string& n) const {
    for (const Binding* b = bindings; b != nullptr; b = b->outer)
      if (b->name == n)
        return b->value;
    for (int k=frames.size(); k--; )
      for (const Entry& e : frames[k])
        if (e.name == n)
          return &e.value;
    return nullptr;
  }

  bool
  Evaluator::truth(const Value& v) {
    if (v.depth > 0) return v.size() > 0;
    switch (v.type) {
    case ValueType::INT: return v.ints[0] != 0;
    case ValueType::FLOAT: return v.reals[0] != 0.0;
    default: return !v.strs[0].empty();
    }
  }

  Value
  Evaluator::boolean(bool b) {
    return Value::integer(b ? 1 : 0);
  }

  const Value&
  Evaluator::ref(const Expr& e, Value& tmp) {
    assert(e.records == nullptr);
    if (e.kind == Expr::Kind::NAME) {
      const Value* v = (e.record >= 0) ? records[e.record] : lookup(e.s);
      if (v == nullptr)
        fail("'" + e.s + "' is not defined yet");
      return *v;
    }
    if (e.kind == Expr::Kind::FIELD)
      return field(e, tmp);
    tmp = eval(e);
    return tmp;
  }

  Seq<int>
  Evaluator::record_path(int i) const {
    Seq<int> p;
    for (int k=0; k<checked->records[i].repeats.size(); k++)
      p.push((*record_at)[k]);
    return p;
  }

  const Value&
  Evaluator::starts(const Stmt* r, const std::string& n) {
    const Value* v = lookup(record_starts(r->id));
    if (v == nullptr)
      fail("'" + n + "' is not defined yet");
    return *v;
  }

  int
  Evaluator::count_records(const RecordRef& l) {
    int level, pos;
    const Value& v = starts(l.repeat, l.repeat->list);
    if (!locate(v, l.path, level, pos) || (level >= v.depth))
      return 0;
    return v.lengths[level][pos];
  }

  size_t
  Evaluator::record_start(const RecordRef& l, int i) {
    const Value* v = lookup(record_starts(l.repeat->id));
    Seq<int> p(l.path);
    p.push(i);
    int level, pos;
    if ((v == nullptr) || !locate(*v, p, level, pos) || (level != v->depth))
      return std::string::npos;
    return static_cast<size_t>(v->ints[pos]);
  }

  RecordRef
  Evaluator::records_of(const Expr& e) {
    assert(e.records != nullptr);
    RecordRef r;
    r.repeat = e.records;
    if (e.record >= 0) {
      // A list of records that a check ranges over
      if (records[e.record] == nullptr)
        fail("'" + checked->records[e.record].label + "' is not defined yet");
      r.path = record_path(e.record);
      return r;
    }
    switch (e.kind) {
    case Expr::Kind::NAME:
      for (const Binding* b = bindings; b != nullptr; b = b->outer)
        if (b->name == e.s)
          return *b->record;
      // A record name outside a check is the current record
      if (e.open > 0)
        (void) starts(e.records, e.s);
      return r;
    case Expr::Kind::FIELD:
      // A list of records in a record
      r.path = records_of(*e.args[0]).path;
      return r;
    default: {
      r = records_of(*e.args[0]);
      long long int k = index(*e.args[1]);
      int n = count_records(r);
      if ((k < 0) || (k >= n))
        fail("index " + std::to_string(k) + " is out of range for '" + r.repeat->list + "' (size " +
             std::to_string(n) + ", indices start at 0)");
      r.path.push(static_cast<int>(k));
      visit(1);
      return r;
    }
    }
  }

  const Value&
  Evaluator::field(const Expr& e, Value& tmp) {
    if (e.record >= 0) {
      if (records[e.record] == nullptr)
        fail("'" + e.s + "' is not defined yet");
      return *records[e.record];
    }
    RecordRef r = records_of(*e.args[0]);
    const Value* v = lookup(e.s);
    if (v == nullptr)
      fail("'" + e.s + "' is not defined yet");
    if (r.path.empty())
      return *v;
    int level, pos;
    if (!locate(*v, r.path, level, pos))
      fail("'" + e.s + "' is not defined in this record of '" + r.repeat->list + "'");
    tmp = part(*v, level, pos);
    visit(1 + tmp.leaves());
    return tmp;
  }

  long long int
  Evaluator::integer(const Value& v, const std::string& op) {
    if ((v.depth > 0) || (v.type != ValueType::INT))
      fail("'" + op + "' needs integers");
    return v.ints[0];
  }

  bool
  Evaluator::explains(const Expr& e) {
    if (e.kind == Expr::Kind::BINARY)
      return (e.op == Expr::Op::OR) || (e.op == Expr::Op::AND);
    return (e.kind == Expr::Kind::COMPREHENSION) && (e.s == "all");
  }

  Value
  Evaluator::eval(const Expr& e) {
    Value r = evaluate(e);
    if (!explains(e))
      why.clear();
    return r;
  }

  Value
  Evaluator::evaluate(const Expr& e) {
    switch (e.kind) {
    case Expr::Kind::INT: return Value::integer(e.i);
    case Expr::Kind::FLOAT: return Value::real(e.f);
    case Expr::Kind::STR: return Value::string(e.s);
    case Expr::Kind::NAME: case Expr::Kind::FIELD: {
      Value tmp(ValueType::INT);
      return ref(e, tmp);
    }
    case Expr::Kind::UNARY: return unary(e);
    case Expr::Kind::BINARY: return binary(e);
    case Expr::Kind::INDEX: return subscript(e);
    case Expr::Kind::COMPREHENSION: return comprehension(e);
    default: return call(e);
    }
  }

  bool
  Evaluator::test(const Expr& e) {
    Value tmp(ValueType::INT);
    return truth(ref(e, tmp));
  }

  bool
  Evaluator::explained(const Expr& e) {
    why.clear();
    return test(e);
  }

  Value
  Evaluator::unary(const Expr& e) {
    if (e.op == Expr::Op::NOT)
      return boolean(!test(*e.args[0]));
    Value tmp(ValueType::INT);
    const Value& a = ref(*e.args[0], tmp);
    if (a.is_number() && (a.type == ValueType::FLOAT))
      return Value::real(-a.reals[0]);
    long long int x = integer(a, e.s);
    if (x == LLONG_MIN)
      fail("integer overflow in '-'");
    return Value::integer(-x);
  }

  Value
  Evaluator::binary(const Expr& e) {
    // Only evaluate the second operand if needed
    if (e.op == Expr::Op::OR)
      return boolean(explained(*e.args[0]) || explained(*e.args[1]));
    if (e.op == Expr::Op::AND)
      return boolean(explained(*e.args[0]) && explained(*e.args[1]));
    Value ta(ValueType::INT), tb(ValueType::INT);
    const Value& a = ref(*e.args[0], ta);
    const Value& b = ref(*e.args[1], tb);
    switch (e.op) {
    case Expr::Op::EQ: case Expr::Op::NE: case Expr::Op::LT: case Expr::Op::GT:
    case Expr::Op::LE: case Expr::Op::GE:
      return boolean(compare(a, b, e));
    default:
      break;
    }
    // Arithmetic is on integers: specs compute sizes and counts
    long long int x = integer(a, e.s), y = integer(b, e.s), r = 0;
    bool ok;
    switch (e.op) {
    case Expr::Op::ADD: ok = add(x, y, r); break;
    case Expr::Op::SUB: ok = sub(x, y, r); break;
    case Expr::Op::MUL: ok = mul(x, y, r); break;
    case Expr::Op::POW:
      if (y < 0)
        fail("negative exponent in '^'");
      ok = pow(x, y, r);
      break;
    default:
      // Division and remainder truncate towards zero
      if (y == 0)
        fail("division by zero");
      ok = (x != LLONG_MIN) || (y != -1);
      if (ok)
        r = (e.op == Expr::Op::DIV) ? (x / y) : (x % y);
      break;
    }
    if (!ok)
      fail("integer overflow in '" + e.s + "'");
    return Value::integer(r);
  }

  bool
  Evaluator::compare(const Value& a, const Value& b, const Expr& e) {
    if ((a.depth > 0) || (b.depth > 0))
      fail("'" + e.s + "' compares single values, not arrays");
    int c;
    if ((a.type == ValueType::STRING) || (b.type == ValueType::STRING)) {
      if ((e.op != Expr::Op::EQ) && (e.op != Expr::Op::NE))
        fail("'" + e.s + "' compares numbers");
      c = ((a.type == b.type) && (a.strs[0] == b.strs[0])) ? 0 : 1;
    } else if ((a.type == ValueType::INT) && (b.type == ValueType::INT)) {
      c = (a.ints[0] < b.ints[0]) ? -1 : (a.ints[0] > b.ints[0] ? 1 : 0);
    } else {
      // An integer compared with a float is converted to a float
      c = (a.as_double() < b.as_double()) ? -1 : (a.as_double() > b.as_double() ? 1 : 0);
    }
    switch (e.op) {
    case Expr::Op::EQ: return c == 0;
    case Expr::Op::NE: return c != 0;
    case Expr::Op::LT: return c < 0;
    case Expr::Op::GT: return c > 0;
    case Expr::Op::LE: return c <= 0;
    default: return c >= 0;
    }
  }

  Value
  Evaluator::call(const Expr& e) {
    const std::string& f = e.s;
    if (f == "defined")
      return boolean(defined(*e.args[0]));
    if (((f == "count") || (f == "size")) && (e.args[0]->records != nullptr))
      return Value::integer(count_records(records_of(*e.args[0])));
    if ((f == "count") || (f == "size")) {
      Value tmp(ValueType::INT);
      const Value& v = ref(*e.args[0], tmp);
      if (v.depth == 0)
        fail("'" + f + "' takes an array");
      return Value::integer(v.size());
    }
    if (f == "distinct")
      return boolean(distinct(e));
    // min, max, and sum over all integers in all arguments
    bool any = false;
    long long int r = 0;
    for (const std::unique_ptr<Expr>& a : e.args) {
      Value tmp(ValueType::INT);
      const Value& v = ref(*a, tmp);
      if ((v.type != ValueType::INT) && (v.leaves() > 0))
        fail("'" + f + "' takes integers");
      visit(v.leaves());
      for (long long int x : v.ints) {
        if (f == "sum") {
          if (!add(r, x, r))
            fail("integer overflow in 'sum'");
        } else if (!any || ((f == "min") ? (x < r) : (x > r))) {
          r = x;
        }
        any = true;
      }
    }
    if (!any && (f != "sum"))
      fail("'" + f + "' of no integers");
    return Value::integer(r);
  }

  bool
  Evaluator::defined(const Expr& e) const {
    for (const Binding* b = bindings; b != nullptr; b = b->outer)
      if (b->name == e.s)
        return true;
    if (e.records != nullptr)
      return lookup(record_starts(e.records->id)) != nullptr;
    return lookup(e.s) != nullptr;
  }

  bool
  Evaluator::distinct(const Expr& e) {
    Seq<long long int> x;
    for (const std::unique_ptr<Expr>& a : e.args) {
      Value tmp(ValueType::INT);
      const Value& v = ref(*a, tmp);
      if ((v.type != ValueType::INT) && (v.leaves() > 0))
        fail("'distinct' takes integers");
      visit(v.leaves());
      for (long long int i : v.ints)
        x.push(i);
    }
    Support::quicksort(x.begin(), x.size());
    for (int k=1; k<x.size(); k++)
      if (x[k-1] == x[k])
        return false;
    return true;
  }

  std::string
  Evaluator::indexed(const Expr& e) {
    const Expr* a = e.args[0].get();
    bool nested = false;
    while (a->kind == Expr::Kind::INDEX) {
      a = a->args[0].get();
      nested = true;
    }
    if ((a->kind != Expr::Kind::NAME) && (a->kind != Expr::Kind::FIELD))
      return "the array";
    return (nested ? "an element of '" : "'") + a->s + "'";
  }

  long long int
  Evaluator::index(const Expr& e) {
    Value tmp(ValueType::INT);
    const Value& i = ref(e, tmp);
    if ((i.depth > 0) || (i.type != ValueType::INT))
      fail("an index must be an integer");
    return i.ints[0];
  }

  std::string
  Evaluator::scalar_hint(const Expr& e) const {
    if (e.record < 0)
      return "";
    const Record& r = checked->records[e.record];
    const std::string& l = r.repeats.back()->list;
    std::string m = ", which is one value of each record here; to index the records, ";
    if (l.empty())
      return m + "name them, as in 'repeat n as nodes { each node ... }', and use 'nodes[0]." + e.s + "'";
    return m + "use '" + l + "[0]." + e.s + "'";
  }

  Value
  Evaluator::subscript(const Expr& e) {
    assert(e.records == nullptr);
    Value ta(ValueType::INT);
    const Value& a = ref(*e.args[0], ta);
    if (a.depth == 0)
      fail("only arrays can be indexed, not " + indexed(e) + scalar_hint(*e.args[0]));
    long long int k = index(*e.args[1]);
    if ((k < 0) || (k >= a.size()))
      fail("index " + std::to_string(k) + " is out of range for " + indexed(e) + " (size " +
           std::to_string(a.size()) + ", indices start at 0)");
    Value r = element(a, static_cast<int>(k));
    visit(1 + r.leaves());
    return r;
  }

  std::string
  Evaluator::array_name(const Expr& a) {
    if (a.kind == Expr::Kind::NAME)
      return "'" + a.s + "'";
    if ((a.kind == Expr::Kind::FIELD) && (a.args[0]->kind == Expr::Kind::NAME))
      return "'" + a.args[0]->s + "." + a.s + "'";
    return "the array";
  }

  RecordRef
  Evaluator::element_records(const Expr& e, const Expr& a) {
    RecordRef r;
    r.repeat = e.elements;
    if (a.kind == Expr::Kind::FIELD)
      r.path = records_of(*a.args[0]).path;
    return r;
  }

  void
  Evaluator::explain(const Expr& e, const RecordRef& r, int i) {
    Witness w;
    w.pos = std::string::npos;
    std::string n = std::to_string(i+1);
    if (e.elements == nullptr) {
      w.text = "element " + n + " of " + array_name(*e.args[0]);
    } else {
      const std::string& l = e.elements->list;
      w.text = "record " + n + " of " + (l.empty() ? array_name(*e.args[0]) : "'" + l + "'");
      w.pos = record_start(r, i);
    }
    why.push(std::move(w));
  }

  Value
  Evaluator::comprehension(const Expr& e) {
    const std::string& f = e.s;
    const Expr& source = *e.args[0];
    Value ta(ValueType::INT);
    const Value* a = nullptr;
    RecordRef list, x;
    int n;
    if (source.records != nullptr) {
      list = records_of(source);
      n = count_records(list);
    } else {
      a = &ref(source, ta);
      if (a->depth == 0)
        fail("'" + f + "(" + e.var + " in ...)' needs an array");
      n = a->size();
    }
    RecordRef elements;
    if ((f == "all") && (e.elements != nullptr))
      elements = (a == nullptr) ? list : element_records(e, source);
    Binding b(bindings, e.var);
    long long int r = 0;
    for (int i=0; i<n; i++) {
      Value v;
      if (a != nullptr) {
        v = element(*a, i);
        visit(1 + v.leaves());
        b.value = &v;
      } else {
        x = list;
        x.path.push(i);
        visit(1);
        b.record = &x;
      }
      if (f == "sum") {
        Value tv(ValueType::INT);
        if (!add(r, integer(ref(*e.args[1], tv), "sum"), r))
          fail("integer overflow in 'sum'");
      } else if (explained(*e.args[1])) {
        if (f == "any")
          return boolean(true);
        r++;
      } else if (f == "all") {
        explain(e, elements, i);
        return boolean(false);
      }
    }
    why.clear();
    if ((f == "all") || (f == "any"))
      return boolean(f == "all");
    return Value::integer(r);
  }

  long long int
  Evaluator::count(const Expr& e, const char* what) {
    Value tmp(ValueType::INT);
    const Value& v = ref(e, tmp);
    if ((v.depth > 0) || (v.type != ValueType::INT))
      fail(std::string(what) + " must be an integer");
    if (v.ints[0] < 0)
      fail(std::string(what) + " is negative (" + std::to_string(v.ints[0]) + ")");
    if (v.ints[0] > INT_MAX)
      fail(std::string(what) + " is too large (" + std::to_string(v.ints[0]) + ")");
    return v.ints[0];
  }

  bool
  Evaluator::holds(const Stmt& s, std::string& error) {
    records.clear();
    int deepest = -1;
    for (int i=0; i<s.records.size(); i++) {
      records.push(lookup(s.records[i].name));
      if (ranges(s, i, (deepest < 0) ? 0 : s.records[deepest].repeats.size()))
        deepest = i;
    }
    Seq<int> at;
    checked = &s;
    record_at = &at;
    bool ok = check_records(s, deepest, at, error);
    checked = nullptr;
    record_at = nullptr;
    return ok;
  }

  bool
  Evaluator::ranges(const Stmt& s, int i, int level) const {
    return (records[i] != nullptr) && (s.records[i].repeats.size() > level);
  }

  bool
  Evaluator::check_records(const Stmt& s, int deepest, Seq<int>& at, std::string& error) {
    int level = at.size();
    if ((deepest < 0) || (level == s.records[deepest].repeats.size())) {
      if (explained(*s.expr))
        return true;
      error = failure(s, deepest, at);
      return false;
    }
    int n = -1, first = -1;
    for (int i=0; i<records.size(); i++)
      if (ranges(s, i, level)) {
        if (records[i]->depth == 0)
          fail("'" + s.records[i].label + "' has no records");
        if (first < 0) {
          first = i;
          n = records[i]->size();
        } else if (records[i]->size() != n) {
          fail("'" + s.records[first].label + "' and '" + s.records[i].label +
               "' have different numbers of records");
        }
      }
    Seq<const Value*> outer(records);
    Seq<Value> rows;
    // No reallocation, so the record names can point at the rows
    rows.reserve(records.size());
    for (int r=0; r<n; r++) {
      rows.clear();
      for (int i=0; i<records.size(); i++)
        if (ranges(s, i, level)) {
          rows.push(element(*outer[i], r));
          visit(1 + rows.back().leaves());
          records[i] = &rows.back();
        }
      at.push(r);
      bool ok = check_records(s, deepest, at, error);
      at.pop();
      if (!ok)
        return false;
    }
    records = outer;
    return true;
  }

  size_t
  Evaluator::record_place(const Record& r, const Seq<int>& at) {
    const Value* p = lookup(r.starts);
    int level, pos;
    if ((p == nullptr) || !locate(*p, at, level, pos) || (level != p->depth))
      return std::string::npos;
    return static_cast<size_t>(p->ints[pos]);
  }

  Place
  Evaluator::place(size_t q) const {
    return (q <= in.pos) ? in.place(q) : in.here();
  }

  std::string
  Evaluator::record_text(const Record& r, const Seq<int>& at) {
    bool named = true;
    for (const Stmt* x : r.repeats)
      named = named && !x->list.empty();
    std::string m;
    for (int k=at.size(); k--; ) {
      m += "record " + std::to_string(at[k]+1) + " of ";
      if (named)
        m += "'" + r.repeats[k]->list + "'" + (k > 0 ? " in " : "");
    }
    return named ? m : m + "'" + r.label + "'";
  }

  std::string
  Evaluator::failure(const Stmt& s, int deepest, const Seq<int>& at) {
    // The innermost record found is where the check failed
    std::string m;
    size_t q = std::string::npos;
    for (const Witness& w : why) {
      m += (m.empty() ? "" : " in ") + w.text;
      if (q == std::string::npos)
        q = w.pos;
    }
    if (!at.empty()) {
      m += (m.empty() ? "" : " in ") + record_text(s.records[deepest], at);
      if (q == std::string::npos)
        q = record_place(s.records[deepest], at);
    }
    m = "check failed" + (m.empty() ? "" : " for " + m);
    if (!s.message.empty())
      m += ": " + s.message;
    return located(m, place(q));
  }

}}

// STATISTICS: instance-any
