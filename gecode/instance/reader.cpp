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
#include <unordered_map>

namespace Gecode { namespace Instance {

  namespace {

    /**
     * \brief Return the first column of row \a i of a stored triangle
     *
     * The triangle is upper if \a upper and includes the diagonal if
     * \a diag.
     */
    int triangle_from(bool upper, bool diag, int i) {
      return upper ? (diag ? i : i+1) : 0;
    }
    /// Return the column after the last of row \a i of a stored triangle of an
    /// \a n by \a n matrix (see triangle_from)
    int triangle_to(bool upper, bool diag, int n, int i) {
      return upper ? n : (diag ? i+1 : i);
    }

    /// Return a description of what read type \a t reads, for messages
    const char* describe(ReadType t) {
      switch (t) {
      case ReadType::INT: return "int";
      case ReadType::FLOAT: return "float";
      case ReadType::NUM: return "a number";
      case ReadType::WORD: return "a word";
      case ReadType::REST: return "the rest of the line";
      default: return "a symbol";
      }
    }

    /// The bounds of a range type, evaluated
    class Range {
    public:
      /// Whether there is a lower bound
      bool low;
      /// Whether there is an upper bound
      bool high;
      /// Lower bound
      long long int lo;
      /// Upper bound, included
      long long int hi;
      /// Initialize without bounds
      Range(void)
        : low(false), high(false), lo(0), hi(0) {}
      /// Test whether \a v is in the range
      bool contains(long long int v) const {
        return (!low || (v >= lo)) && (!high || (v <= hi));
      }
      /// Test whether there are bounds
      bool bounded(void) const {
        return low || high;
      }
    };

    /// A type with its range evaluated
    class Domain {
    public:
      /// The type
      const Type& type;
      /// The range
      Range range;
      /// Initialize for \a t with range \a r
      Domain(const Type& t, const Range& r)
        : type(t), range(r) {}
      /// Return a description of the values, for messages
      std::string describe(void) const {
        std::string d = Instance::describe(type.read);
        if (range.bounded())
          d = "an int in " + (range.low ? std::to_string(range.lo) : "") + ".." +
            (range.high ? std::to_string(range.hi) : "");
        return type.name.empty() ? d : type.name + " (" + d + ")";
      }
    };

    /// Interpreter running a spec on an input
    class Interpreter : public Evaluator {
    public:
      /// Initialize
      Interpreter(const SpecImpl& s, const std::string& text, const std::string& origin, const Params& p, bool c)
        : Evaluator(s, text, origin), params(p), trailing(false), catalog(c), instance_count(-1), select(false),
          number(-1), current(0), instance_frame(0), selected(false), idle(0), token_start(0) {}

      /// Add the names of the param statements in \a b to \a out
      static void declared_params(const Block& b, Seq<std::string>& out) {
        for (const std::unique_ptr<Stmt>& s : b) {
          if (s->kind == Stmt::Kind::PARAM)
            out.push(s->name);
          if (s->kind == Stmt::Kind::INSTANCES)
            out.push("instance");
          declared_params(s->body, out);
          declared_params(s->orelse, out);
          for (const Arm& a : s->arms)
            declared_params(a.body, out);
        }
      }

      /// Run the spec and return the data
      void run(DataImpl& d) {
        // A misspelt parameter would otherwise be ignored silently
        Seq<std::string> known;
        declared_params(spec.body, known);
        for (int i=0; i<params.size(); i++) {
          bool found = false;
          for (const std::string& k : known)
            found = found || (k == params.name(i));
          if (!found)
            throw Error(spec.origin + ": the spec has no parameter '" + params.name(i) + "'");
        }
        exec(spec.body);
        spec_line = 0;
        // Reading stops after the selected instance
        if (!trailing && !selected && in.skip_to_data())
          fail("unexpected input after the end of the spec: " + quote(in.rest()));
        if (catalog) {
          // Only the number of instances and their names
          Seq<Entry> c;
          Entry n;
          n.name = "instances";
          n.value = Value::integer(instance_count < 0 ? 1 : instance_count);
          c.push(std::move(n));
          if (names.leaves() > 0) {
            Entry e;
            e.name = "name";
            e.value = names;
            c.push(std::move(e));
          }
          if (failures.leaves() > 0) {
            Entry e;
            e.name = "failed";
            e.value = failures;
            c.push(std::move(e));
          }
          frames[0] = std::move(c);
        }
        for (Entry& e : frames[0])
          if (!is_internal(e.name)) {
            e.value.index();
            d.entries.push(std::move(e));
          }
      }

    private:
      /// The parameters
      const Params& params;
      /// Row numbers of the active repeats
      Seq<long long int> rows;
      /// Whether input after the spec is allowed
      bool trailing;
      /// Whether only the instances are listed
      bool catalog;
      /// Number of instances in an instances statement (-1 if none)
      int instance_count;
      /// Names of the instances, for a catalog
      Value names;
      /// The first rejection of each instance, for a catalog
      Value failures;
      /// Whether the parameter instance selects an instance
      bool select;
      /// The value of the parameter instance
      std::string want;
      /// The number of the instance selected (-1 if selected by name)
      long long int number;
      /// Number of the instance being read (0 if none)
      int current;
      /// Frame of the instance being read
      int instance_frame;
      /// Error of the first rejection of the instance being read
      std::string deferred_error;
      /// Summary of that error
      std::string deferred_message;
      /// Whether the selected instance has been read
      bool selected;
      /// Repeat iterations and empty rows created without reading input
      long long int idle;
      /// Limit for idle: such work could otherwise be unbounded or take
      /// time and memory far beyond the size of the input
      static const long long int max_idle = 1LL << 20;
      /// The ranges of the named types, by type statement
      std::unordered_map<const Stmt*, Range> type_ranges;
      /// Where the last token read as a value starts
      size_t token_start;
      /// Lists of finished repeat iterations, kept for their memory
      Seq<Value> spare;
      /// Most lists kept in spare
      static const int max_spare = 8;

      /// Account for \a k units of work that read no input
      void work(long long int k) {
        idle += k;
        if (idle > max_idle)
          fail("too many repetitions or empty rows without reading input");
      }


      /*
       * Reading
       */
      /// Store \a t read as \a type in \a v, and return whether it fits
      static bool convert(ReadType type, const std::string& t, Value& v) {
        long long int iv;
        double fv;
        if ((type == ReadType::WORD) || (type == ReadType::REST)) {
          v = Value::string(t);
          return !t.empty();
        }
        if ((type != ReadType::FLOAT) && parse_int(t, iv)) {
          v = Value::integer(iv);
          return true;
        }
        if ((type != ReadType::INT) && parse_float(t, fv)) {
          v = Value::real(fv);
          return true;
        }
        return false;
      }

      /// Fail for want of \a what at the end of the input or the token there
      [[noreturn]] void expected(const std::string& what) {
        if (!in.skip_to_data())
          fail("expected " + what + ", found end of input");
        fail("expected " + what + ", found " + quote(in.peek()));
      }
      /**
       * \brief Read one value of \a type into \a v, and return whether
       * there is one
       *
       * At the end of the input or a token that is not of the type, the
       * position stays before it.
       */
      bool scan(ReadType type, Value& v) {
        if (!in.skip_to_data())
          return false;
        token_start = in.pos;
        if (type == ReadType::REST) {
          v = Value::string(in.rest());
          in.to_eol();
          return true;
        }
        std::string t = in.token();
        if (convert(type, t, v))
          return true;
        in.pos = token_start;
        return false;
      }
      /// Read one value of \a type, described by \a what
      Value scalar(ReadType type, const std::string& what) {
        Value v(ValueType::INT);
        if (!scan(type, v))
          expected(what);
        return v;
      }
      /// Read one value of domain \a d, without checking its range
      Value scalar(const Domain& d) {
        Value v(ValueType::INT);
        if (!scan(d.type.read, v))
          expected(d.describe());
        return v;
      }
      /// Read an integer of domain \a d, which reads integers, without
      /// checking its range
      long long int int_scalar(const Domain& d) {
        long long int x;
        if (!in.skip_to_data())
          expected(d.describe());
        token_start = in.pos;
        if (!in.int_token(x))
          expected(d.describe());
        return x;
      }
      /// Test whether value \a v is in the range of domain \a d
      static bool fits(const Domain& d, const Value& v) {
        return (v.type != ValueType::INT) || d.range.contains(v.ints[0]);
      }
      /// Fail for the value from position \a from to \a to on the current
      /// line, which is not in domain \a d
      [[noreturn]] void outside(const Domain& d, size_t from, size_t to) {
        std::string t = in.text.substr(from, to-from);
        in.pos = from;
        fail("expected " + d.describe() + ", found " + quote(t));
      }
      /// Read one value of domain \a d
      Value value(const Domain& d) {
        Value v = scalar(d);
        if (!fits(d, v))
          outside(d, token_start, in.pos);
        return v;
      }
      /// Read one value of domain \a d and append it to the list \a v
      void push(const Domain& d, Value& v) {
        if ((d.type.read == ReadType::INT) && (v.type == ValueType::INT)) {
          long long int x = int_scalar(d);
          if (!d.range.contains(x))
            outside(d, token_start, in.pos);
          v.ints.push(x);
        } else {
          v.push(value(d));
        }
      }

      /// Return the range of type \a t
      Range range(const Type& t) {
        if (t.named != nullptr) {
          auto r = type_ranges.find(t.named);
          if (r == type_ranges.end())
            fail("the type '" + t.name + "' is not defined yet");
          return r->second;
        }
        Range r;
        if (t.lo) {
          r.low = true;
          r.lo = bound(*t.lo);
        }
        if (t.hi) {
          r.high = true;
          r.hi = bound(*t.hi);
          if (t.exclusive && !sub(r.hi, 1, r.hi))
            fail("integer overflow in the upper bound of a range");
        }
        return r;
      }
      /// Return the value of the bound \a e of a range
      long long int bound(const Expr& e) {
        // Ranges are evaluated for every value read, so common bounds
        // such as 0 and -n avoid making a value
        if (e.kind == Expr::Kind::INT)
          return e.i;
        if ((e.kind == Expr::Kind::UNARY) && (e.op == Expr::Op::NEG)) {
          long long int x = bound(*e.args[0]);
          if (x == LLONG_MIN)
            fail("integer overflow in '-'");
          return -x;
        }
        Value tmp(ValueType::INT);
        const Value& v = ref(e, tmp);
        if ((v.depth > 0) || (v.type != ValueType::INT))
          fail("the bounds of a range must be integers");
        return v.ints[0];
      }

      /// Read an array of domain \a d with dimensions \a dims
      Value array(const Domain& d, const Seq<long long int>& dims) {
        // Each factor is at most INT_MAX, so the product cannot overflow
        long long int size = 1;
        for (long long int n : dims) {
          size *= n;
          if (size > INT_MAX)
            fail("array is too large");
        }
        // Rows before an empty dimension cost memory without reading any input
        if (size == 0) {
          long long int rows = 1;
          for (int k=0; (k < dims.size()) && (dims[k] > 0); k++) {
            rows *= dims[k];
            work(rows);
          }
        }
        Value v;
        if (d.type.read == ReadType::FLOAT)
          v.type = ValueType::FLOAT;
        else if (d.type.read == ReadType::WORD)
          v.type = ValueType::STRING;
        // Read the values before allocating the structure, so that a wrong
        // count fails at the end of the input rather than on allocation
        for (long long int i=0; i<size; i++)
          try {
            push(d, v);
          } catch (const ShapeError& e) {
            fail(e, context.back().text);
          }
        v.depth = dims.size();
        v.lengths.clear();
        long long int lists = 1;
        for (long long int n : dims) {
          v.lengths.push(Seq<int>());
          v.lengths.back().reserve(static_cast<int>(lists));
          for (long long int i=0; i<lists; i++)
            v.lengths.back().push(static_cast<int>(n));
          lists *= n;
        }
        return v;
      }

      /**
       * \brief Read a symmetric \a n by \a n matrix of domain \a d stored
       * in TSPLIB layout \a f with offset \a o
       *
       * The offset applies to the numbers read, not to the cells that only
       * exist after mirroring.
       */
      Value layout(const Domain& d, long long int n, long long int m, const std::string& f, long long int o) {
        if (f == "FULL_MATRIX") {
          Seq<long long int> dims;
          dims.push(n); dims.push(m);
          Value v = array(d, dims);
          offset(v, o);
          return v;
        }
        if (n != m)
          fail("layout " + f + " needs a square matrix");
        // Column-wise upper equals row-wise lower for symmetric matrices
        bool upper, diag;
        if ((f == "UPPER_ROW") || (f == "LOWER_COL")) {
          upper = true; diag = false;
        } else if ((f == "LOWER_ROW") || (f == "UPPER_COL")) {
          upper = false; diag = false;
        } else if ((f == "UPPER_DIAG_ROW") || (f == "LOWER_DIAG_COL")) {
          upper = true; diag = true;
        } else if ((f == "LOWER_DIAG_ROW") || (f == "UPPER_DIAG_COL")) {
          upper = false; diag = true;
        } else {
          fail("unknown matrix layout " + quote(f));
        }
        if (n*n > INT_MAX)
          fail("matrix is too large");
        int nn = static_cast<int>(n);
        // Read the stored triangle, then mirror it into a full matrix
        Value t;
        if (d.type.read == ReadType::FLOAT)
          t.type = ValueType::FLOAT;
        for (int i=0; i<nn; i++)
          for (int j=triangle_from(upper, diag, i); j<triangle_to(upper, diag, nn, i); j++)
            push(d, t);
        offset(t, o);
        Value v;
        v.type = t.type;
        v.depth = 2;
        v.lengths.clear();
        v.lengths.push(Seq<int>());
        v.lengths[0].push(nn);
        v.lengths.push(Seq<int>());
        for (int i=0; i<nn; i++)
          v.lengths[1].push(nn);
        for (int k=0; k<nn*nn; k++)
          if (v.type == ValueType::INT) v.ints.push(0); else v.reals.push(0.0);
        int k = 0;
        for (int i=0; i<nn; i++)
          for (int j=triangle_from(upper, diag, i); j<triangle_to(upper, diag, nn, i); j++, k++)
            if (v.type == ValueType::INT) {
              v.ints[i*nn+j] = t.ints[k]; v.ints[j*nn+i] = t.ints[k];
            } else {
              v.reals[i*nn+j] = t.reals[k]; v.reals[j*nn+i] = t.reals[k];
            }
        return v;
      }

      /// Add \a o to every number in \a v
      void offset(Value& v, long long int o) {
        for (long long int& x : v.ints)
          if (!add(x, o, x))
            fail("integer overflow when applying offset");
        for (double& x : v.reals)
          x += static_cast<double>(o);
      }

      /// Return the offset of statement \a s (0 if none)
      long long int offset(const Stmt& s) {
        if (!s.offset)
          return 0;
        Value o = eval(*s.offset);
        if ((o.depth > 0) || (o.type != ValueType::INT))
          fail("offset must be an integer");
        return o.ints[0];
      }

      /// Store value \a v read by statement \a s, unless its name is _
      void store(const Stmt& s, Value&& v) {
        if (s.name != "_")
          set(s.name, std::move(v));
      }

      /// A value captured from a line
      class Capture {
      public:
        /// The capture
        const Pattern::Item* item;
        /// The value
        Value value;
        /// Where its text starts in the line
        size_t from;
        /// Where its text ends in the line
        size_t to;
      };

      /// Match pattern \a p against the rest of the current line, consuming it
      bool match(const Pattern& p) {
        Seq<Capture> caps;
        if (!matches(p, caps))
          return false;
        take(caps);
        return true;
      }

      /// Test whether \a p matches the rest of the current line, giving the captures \a caps
      bool matches(const Pattern& p, Seq<Capture>& caps) {
        in.skip();
        if (in.pos >= in.text.size())
          return false;
        std::string l = in.rest();
        size_t c = 0;
        for (int k=0; k<p.items.size(); k++)
          if (!match_item(p, k, l, c, caps))
            return false;
        while ((c < l.size()) && is_blank(l[c]))
          c++;
        return c == l.size();
      }

      /// Define the values \a caps captured from the current line, and consume it
      void take(const Seq<Capture>& caps) {
        // The captures can use each other in their ranges
        for (const Capture& x : caps)
          if (!x.item->text.empty() && (x.item->text != "_"))
            set(x.item->text, Value(x.value));
        for (const Capture& x : caps)
          if (x.item->type.ranged()) {
            Domain d(x.item->type, range(x.item->type));
            if (!fits(d, x.value))
              outside(d, in.pos + x.from, in.pos + x.to);
          }
        in.to_eol();
      }

      /// Match item \a k of \a p at position \a c of line \a l
      bool match_item(const Pattern& p, int k, const std::string& l, size_t& c, Seq<Capture>& caps) {
        typedef Pattern::Item Item;
        const Item& it = p.items[k];
        switch (it.kind) {
        case Item::Kind::SPACE: {
          size_t c0 = c;
          while ((c < l.size()) && is_blank(l[c]))
            c++;
          return !it.required || (c > c0) || (c == l.size());
        }
        case Item::Kind::LIT:
          if (l.compare(c, it.text.size(), it.text) != 0)
            return false;
          c += it.text.size();
          return true;
        case Item::Kind::ANY:
          c = l.size();
          return true;
        case Item::Kind::VAR: {
          std::string want = variable(it.text);
          if (want.empty() || (l.compare(c, want.size(), want) != 0))
            return false;
          c += want.size();
          return true;
        }
        case Item::Kind::CAP: {
          // A word ends before a literal that follows it directly
          char stop = 0;
          if ((k+1 < p.items.size()) && (p.items[k+1].kind == Item::Kind::LIT))
            stop = p.items[k+1].text[0];
          size_t c0 = c;
          c = capture_end(it.type.read, l, c, stop);
          std::string t = l.substr(c0, c-c0);
          while (!t.empty() && is_blank(t[t.size()-1]))
            t.erase(t.size()-1);
          Capture x;
          if (!convert(it.type.read, t, x.value))
            return false;
          x.item = &it;
          x.from = c0;
          x.to = c0 + t.size();
          caps.push(std::move(x));
          return true;
        }
        }
        return false;
      }

      /// Return the value of variable \a n as text, for {$n} in a pattern
      std::string variable(const std::string& n) {
        const Value* v = lookup(n);
        if (v == nullptr)
          fail("'" + n + "' is not defined yet");
        if ((v->depth == 0) && (v->type == ValueType::STRING))
          return v->strs[0];
        if ((v->depth == 0) && (v->type == ValueType::INT))
          return std::to_string(v->ints[0]);
        fail("'" + n + "' must be a string or an integer");
      }

      /// Return where a capture of \a type starting at \a c of \a l ends
      static size_t capture_end(ReadType type, const std::string& l, size_t c, char stop) {
        switch (type) {
        case ReadType::REST:
          return l.size();
        case ReadType::WORD:
          while ((c < l.size()) && !is_blank(l[c]) && (l[c] != stop))
            c++;
          return c;
        default:
          // Numbers end where a following literal starts
          return number_end(l, c, true, type != ReadType::INT);
        }
      }

      void exec(const Block& b) {
        for (const std::unique_ptr<Stmt>& s : b)
          exec(*s);
      }

      void exec(const Stmt& s) {
        spec_line = s.line;
        switch (s.kind) {
        case Stmt::Kind::COMMENT:
          for (const std::string& w : s.words) in.comments.push(w);
          break;
        case Stmt::Kind::STOP:
          for (const std::string& w : s.words) in.stops.push(w);
          break;
        case Stmt::Kind::TOKENS:     in.add_split(s.chars); break;
        case Stmt::Kind::SEPARATORS: in.add_separators(s.chars); break;
        case Stmt::Kind::TRAILING:   trailing = true; break;
        case Stmt::Kind::PARAM:      param(s); break;
        case Stmt::Kind::READ:       read(s); break;
        case Stmt::Kind::LIST:       list(s); break;
        case Stmt::Kind::LINE:       line(s); break;
        case Stmt::Kind::FIND:       find(s); break;
        case Stmt::Kind::SKIP:       skip_lines(s); break;
        case Stmt::Kind::EXPECT:     expect(s); break;
        case Stmt::Kind::INDEX:      index(s); break;
        case Stmt::Kind::REPEAT:     repeat(s); break;
        case Stmt::Kind::SECTIONS:   sections(s); break;
        case Stmt::Kind::INSTANCES:  instances(s); break;
        case Stmt::Kind::TYPE:       type_ranges[&s] = range(s.type); break;
        case Stmt::Kind::LET:        set(s.name, eval(*s.expr)); break;
        case Stmt::Kind::IF:
          exec(truth(eval(*s.expr)) ? s.body : s.orelse);
          break;
        case Stmt::Kind::CHECK:      check(s); break;
        case Stmt::Kind::EACH:       break;
        }
      }

      /**
       * \brief param x [= default]
       *
       * A given value has the type of the default. Without a default, it
       * is an integer if possible, otherwise a float or a string.
       */
      void param(const Stmt& s) {
        std::string v;
        long long int iv;
        double fv;
        Value d;
        if (s.expr)
          d = eval(*s.expr);
        if (!params.get(s.name, v)) {
          if (!s.expr)
            fail("parameter '" + s.name + "' must be given");
          set(s.name, std::move(d));
        } else if (s.expr && (d.type == ValueType::STRING)) {
          set(s.name, Value::string(v));
        } else if (parse_int(v, iv) && (!s.expr || (d.type == ValueType::INT))) {
          set(s.name, Value::integer(iv));
        } else if (parse_float(v, fv) && (!s.expr || (d.type == ValueType::FLOAT))) {
          set(s.name, Value::real(fv));
        } else if (!s.expr) {
          set(s.name, Value::string(v));
        } else {
          fail("parameter '" + s.name + "' must be " + (d.type == ValueType::INT ? "an integer" : "a number") +
               ", not " + quote(v));
        }
      }

      /// x: type and x[d1][d2]...: type
      void read(const Stmt& s) {
        context.push(Context(s.name));
        long long int o = offset(s);
        Domain d(s.type, range(s.type));
        if (s.dims.empty()) {
          Value v = value(d);
          offset(v, o);
          store(s, std::move(v));
        } else {
          Seq<long long int> dims;
          for (const std::unique_ptr<Expr>& e : s.dims)
            dims.push(count(*e, "array size"));
          if (s.type.read == ReadType::SYMBOL) {
            store(s, symbols(s.type, dims[0], dims[1]));
          } else if (s.expr) {
            Value f = eval(*s.expr);
            if ((f.depth > 0) || (f.type != ValueType::STRING))
              fail("matrix layout must be a string");
            store(s, layout(d, dims[0], dims[1], f.strs[0], o));
          } else {
            Value v = array(d, dims);
            offset(v, o);
            store(s, std::move(v));
          }
        }
        context.pop();
      }

      /// list x: type until (eol | terminator)
      void list(const Stmt& s) {
        context.push(Context(s.name));
        Value v(ValueType::INT);
        if (!spare.empty()) {
          v = std::move(spare.back());
          spare.pop();
        }
        v.clear_list(s.type.read == ReadType::FLOAT ? ValueType::FLOAT :
                     s.type.read == ReadType::WORD ? ValueType::STRING : ValueType::INT);
        Domain d(s.type, range(s.type));
        try {
          if (s.until_eol)
            list_line(d, v);
          else
            list_until(s, d, v);
        } catch (const ShapeError& e) {
          fail(e, s.name);
        }
        v.lengths[0][0] = v.leaves();
        offset(v, offset(s));
        context.pop();
        store(s, std::move(v));
      }
      /// Read the values of domain \a d on the rest of the line into \a v
      void list_line(const Domain& d, Value& v) {
        // The rest of the current line, or the next line if the current
        // one is used up (skip_to_data() moves there)
        if (!in.skip_to_line_data() && !in.skip_to_data())
          fail("expected a line of values, found end of input");
        while (in.skip_to_line_data())
          push(d, v);
      }
      /// Test whether the scalars \a x and \a y are numbers that are equal
      static bool same_number(const Value& x, const Value& y) {
        if ((x.type == ValueType::INT) && (y.type == ValueType::INT))
          return x.ints[0] == y.ints[0];
        if ((x.type == ValueType::STRING) || (y.type == ValueType::STRING))
          return false;
        return x.as_double() == y.as_double();
      }
      /// Read integers of domain \a d into \a v up to the terminator \a stop
      void ints_until(const Domain& d, long long int stop, Value& v) {
        for (;;) {
          // The terminator need not be in the range
          long long int x = int_scalar(d);
          if (x == stop)
            return;
          if (!d.range.contains(x))
            outside(d, token_start, in.pos);
          v.ints.push(x);
        }
      }
      /// Read values of domain \a d into \a v up to the terminator of \a s
      void list_until(const Stmt& s, const Domain& d, Value& v) {
        if ((d.type.read == ReadType::INT) && (s.sentinel->kind == Expr::Kind::INT)) {
          ints_until(d, s.sentinel->i, v);
          return;
        }
        Value stop = eval(*s.sentinel);
        if (stop.depth > 0)
          fail("list terminator must be a single value");
        // Words end at the token spelling an integer terminator
        if ((s.type.read == ReadType::WORD) && (stop.type == ValueType::INT))
          stop = Value::string(std::to_string(stop.ints[0]));
        if ((s.type.read == ReadType::WORD) && (stop.type == ValueType::FLOAT))
          fail("a list of words needs a string or integer terminator");
        if ((d.type.read == ReadType::INT) && (stop.type == ValueType::INT)) {
          ints_until(d, stop.ints[0], v);
          return;
        }
        for (;;) {
          // A string terminator is compared before the token is read as
          // a value, so "EOF" can end a list of numbers
          if (stop.type == ValueType::STRING) {
            if (!in.skip_to_data())
              fail("expected " + quote(stop.strs[0]) + ", found end of input");
            if (in.peek() == stop.strs[0]) {
              (void) in.token();
              return;
            }
          }
          // The terminator need not be in the range
          Value x = scalar(d);
          if (same_number(x, stop))
            return;
          if (!fits(d, x))
            outside(d, token_start, in.pos);
          v.push(x);
        }
      }

      /// line "pattern": the rest of the current line, or the next line if
      /// the current one is used up, must match
      void line(const Stmt& s) {
        if (match(s.pattern))
          return;
        fail("expected a line matching \"" + s.pattern.source + "\"" +
             (in.pos >= in.text.size() ? ", found end of input" : ""));
      }
      /// find "pattern": skip lines until one matches
      void find(const Stmt& s) {
        while (!match(s.pattern)) {
          if (!in.skip_to_data())
            fail("no line matching \"" + s.pattern.source + "\" found");
          in.to_eol();
        }
      }
      /// skip e lines
      void skip_lines(const Stmt& s) {
        long long int n = count(*s.expr, "number of lines");
        for (long long int k=0; k<n; k++) {
          if (!in.skip_to_data())
            fail("cannot skip a line at end of input");
          in.to_eol();
        }
      }
      /// expect "a" "b" ...: literal tokens
      void expect(const Stmt& s) {
        for (const std::string& w : s.words) {
          if (!in.skip_to_data())
            fail("expected " + quote(w) + ", found end of input");
          size_t p0 = in.pos;
          std::string t = in.token();
          if (t != w) {
            in.pos = p0;
            fail("expected " + quote(w) + ", found " + quote(t));
          }
        }
      }
      /// index b: the row number of the innermost repeat, counted from b
      void index(const Stmt& s) {
        if (rows.empty())
          fail("'index' must be inside a repeat");
        long long int want = rows.back() + count(*s.expr, "index base");
        in.skip();
        size_t p0 = in.pos;
        Value v = scalar(ReadType::INT, describe(ReadType::INT));
        if (v.ints[0] != want) {
          in.pos = p0;
          fail("expected index " + std::to_string(want) + ", found " + std::to_string(v.ints[0]));
        }
      }

      /// Test whether the next token ends statement \a s, and consume it
      bool until(const Stmt& s) {
        if (!s.has_until || !in.skip_to_data())
          return false;
        std::string t = in.peek();
        for (const std::string& w : s.words)
          if (t == w) {
            (void) in.token();
            if (!s.store.empty())
              set(s.store, Value::string(w));
            return true;
          }
        return false;
      }
      std::string until_text(const Stmt& s) const {
        std::string r;
        for (int k=0; k<s.words.size(); k++)
          r += (k > 0 ? " or " : "") + quote(s.words[k]);
        return r;
      }

      /**
       * \brief Test whether the instance being read may not be the
       * selected one
       *
       * When selecting by name, the instance is the selected one if it has
       * the name, and may not be while the name is not defined yet.
       */
      bool may_be_skipped(void) const {
        if (current == 0)
          return false;
        if (catalog)
          return true;
        if (!select)
          return current != 1;
        if (number > 0)
          return current != number;
        return instance_name(frames[instance_frame]) != want;
      }

      /**
       * \brief Reject the instance being read with error \a e, summed up by
       * \a message
       *
       * Reading stops, unless the instance may not be the selected one:
       * then the first rejection is recorded, and reading continues (see
       * instances).
       */
      void reject(const std::string& e, const std::string& message) {
        if (!may_be_skipped())
          throw Error(e);
        if (deferred_error.empty()) {
          deferred_error = e;
          deferred_message = message;
        }
      }

      /// check e "message"
      void check(const Stmt& s) {
        std::string e;
        if (!holds(s, e))
          reject(e, s.message.empty() ? "check on line " + std::to_string(s.line) + " of the spec" : s.message);
      }


      /// Read instance \a n with the body \a b into a frame, and return it
      Seq<Entry> instance(const Block& b, int n) {
        current = n;
        deferred_error.clear();
        deferred_message.clear();
        context.push(Context("instance", n));
        frames.push(Seq<Entry>());
        instance_frame = frames.size()-1;
        try {
          exec(b);
        } catch (const Error& e) {
          // The earlier rejection may explain the error
          if (deferred_error.empty())
            throw;
          throw Error(std::string(e.what()) + "\n  after an earlier error in the instance: " + deferred_message);
        }
        Seq<Entry> f = std::move(frames.back());
        frames.pop();
        context.pop();
        current = 0;
        return f;
      }

      /// Return the name of the instance in frame \a f, or empty
      static std::string instance_name(const Seq<Entry>& f) {
        for (const Entry& e : f)
          if ((e.name == "name") && (e.value.depth == 0)) {
            if (e.value.type == ValueType::STRING) return e.value.strs[0];
            if (e.value.type == ValueType::INT) return std::to_string(e.value.ints[0]);
          }
        return std::string();
      }

      /**
       * \brief Read the instances of a file and keep the selected one
       *
       * Each instance is read into a frame of its own, which is dropped
       * unless the instance is the one wanted. The parameter \c instance
       * selects an instance by number (from 1) if it is a number, and by
       * the value of \c name otherwise. Without it, the file must contain
       * a single instance. Reading stops after the selected instance. In a
       * catalog, all instances are read and only their names and
       * rejections are kept.
       *
       * A rejection (see reject) of an instance that is not the selected
       * one does not stop reading, so that an invalid instance does not
       * hide the ones after it. Other errors do, as the input cannot be
       * read on.
       */
      void instances(const Stmt& s) {
        select = params.get("instance", want);
        long long int k = 0;
        bool numbered = select && parse_int(want, k);
        if (numbered && (k < 1))
          fail("instance numbers start at 1");
        number = numbered ? k : -1;
        int n = 0;
        while (in.skip_to_data()) {
          n++;
          size_t start = in.pos;
          Seq<Entry> f = instance(s.body, n);
          spec_line = s.line;
          if (in.pos == start)
            fail("an instance must read some input");
          std::string name = instance_name(f);
          if (catalog) {
            names.push(Value::string(name));
            failures.push(Value::string(deferred_message));
          } else if (select ? ((number > 0) ? (number == n) : (name == want)) : (n == 1)) {
            // An instance selected by name may have been rejected before
            // its name was known
            if (!deferred_error.empty())
              throw Error(deferred_error);
            for (Entry& e : f)
              set(e.name, std::move(e.value));
            if (select) {
              selected = true;
              return;
            }
          }
        }
        instance_count = n;
        if (catalog) {
          // Keep the names only if every instance has one
          bool all = (n > 0);
          for (const std::string& v : names.strs)
            all = all && !v.empty();
          if (all)
            names.lengths[0][0] = n;
          else
            names = Value();
          bool failed = false;
          for (const std::string& m : failures.strs)
            failed = failed || !m.empty();
          if (failed)
            failures.lengths[0][0] = n;
          else
            failures = Value();
          return;
        }
        if (n == 0)
          fail("the file contains no instance");
        if (select)
          fail("there is no instance " + quote(want) + " (the file has " + std::to_string(n) + " instance" +
               (n == 1 ? "" : "s") + ")");
        if (n > 1)
          fail("the file has " + std::to_string(n) +
               " instances; select one with the parameter instance (a number from 1, or a name)");
      }

      /**
       * \brief Repeat the body of \a s
       *
       * Each iteration runs in a frame of its own. Afterwards, the value of
       * every name the body defines is added as a row to the name's column,
       * and the frame is dropped, so that only one iteration's values exist
       * at a time besides the columns.
       */
      void repeat(const Stmt& s) {
        Seq<Decl> names;
        declared(s.body, names);
        long long int n = s.expr ? count(*s.expr, "repeat count") : -1;
        Seq<Column> columns;
        for (const Decl& d : names)
          columns.push(Column(d.type, d.depth));
        // Where each record starts, for errors in checks
        Value starts;
        // The frame of an iteration, kept between iterations for its memory
        Seq<Entry> frame;
        for (long long int r=0; (n < 0) || (r < n); r++) {
          if (s.until_eof && !in.skip_to_data())
            break;
          if (until(s))
            break;
          if (s.has_until && !in.skip_to_data())
            fail("expected " + until_text(s) + " before end of input");
          context.push(Context("repeat", r+1, n));
          frames.push(std::move(frame));
          rows.push(r);
          starts.ints.push(static_cast<long long int>(in.next_data()));
          size_t start = in.pos;
          exec(s.body);
          if (in.pos == start) {
            if (n < 0)
              fail("the body of a repeat until read no input");
            work(1);
          }
          rows.pop();
          Seq<Entry>& f = frames.back();
          for (int i=0; i<names.size(); i++) {
            Entry* e = nullptr;
            for (Entry& x : f)
              if (x.name == names[i].name)
                e = &x;
            try {
              if (e == nullptr)
                columns[i].skip();
              else
                columns[i].add(e->value);
            } catch (const ShapeError& err) {
              fail("'" + names[i].name + "' " + err.msg);
            }
          }
          // A list read in each iteration then needs no new memory
          for (Entry& e : f)
            if ((e.value.depth > 0) && (spare.size() < max_spare))
              spare.push(std::move(e.value));
          frame = std::move(frames.back());
          frame.clear();
          frames.pop();
          context.pop();
          spec_line = s.line;
        }
        for (int i=0; i<names.size(); i++)
          try {
            set(names[i].name, columns[i].finish());
          } catch (const ShapeError& err) {
            fail("'" + names[i].name + "' " + err.msg);
          }
        starts.lengths[0][0] = starts.ints.size();
        set(record_starts(s.id), std::move(starts));
      }

      /**
       * \brief sections: read lines in any order, each by the first arm
       * that matches it
       *
       * An arm that is not optional must match a line, and no arm may
       * match two lines.
       */
      void sections(const Stmt& s) {
        // The line on which each arm matched, or 0
        Seq<int> seen;
        for (int k=0; k<s.arms.size(); k++)
          seen.push(0);
        std::string end = "the end of the input";
        Place end_at;
        for (;;) {
          spec_line = s.line;
          if (!in.skip_to_data()) {
            end_at = in.here();
            break;
          }
          Place at = in.here();
          if (until(s)) {
            end = until_text(s);
            end_at = at;
            break;
          }
          int found = -1;
          Seq<Capture> caps;
          for (int k=0; (found < 0) && (k < s.arms.size()); k++) {
            caps.clear();
            if (matches(s.arms[k].pattern, caps))
              found = k;
          }
          if (found < 0) {
            if (s.skip_other) {
              in.to_eol();
              continue;
            }
            std::string expected;
            for (int k=0; (k < s.arms.size()) && (k < 6); k++)
              expected += (k > 0 ? ", \"" : "\"") + s.arms[k].pattern.source + "\"";
            if (s.arms.size() > 6)
              expected += ", ...";
            fail("unexpected line; expected one of " + expected);
          }
          const Arm& a = s.arms[found];
          if (seen[found] > 0) {
            std::string m = "a second line matching \"" + a.pattern.source + "\"";
            reject(located(m + " (the first is on line " + std::to_string(seen[found]) + ")", at), m);
          } else {
            seen[found] = at.line;
          }
          take(caps);
          context.push(Context("section \"" + a.pattern.source + "\""));
          exec(a.body);
          context.pop();
          if (a.last) {
            end = "the line matching \"" + a.pattern.source + "\"";
            end_at = at;
            break;
          }
        }
        for (int k=0; k<s.arms.size(); k++)
          if (!s.arms[k].optional && (seen[k] == 0)) {
            std::string p = "\"" + s.arms[k].pattern.source + "\"";
            std::string e = located("expected a line matching " + p + " before " + end, end_at);
            // At the end of the input, no later instance can be hidden
            if (!in.skip_to_data())
              throw Error(e);
            reject(e, "no line matching " + p);
            return;
          }
      }

      /// Read an \a r by \a c array of the symbols of alphabet type \a t
      Value symbols(const Type& t, long long int r, long long int c) {
        if (r*c > INT_MAX)
          fail("array is too large");
        if (c == 0)
          work(r);
        Value g;
        for (long long int i=0; i<r; i++)
          for (long long int j=0; j<c; j++) {
            char ch;
            for (;;) {
              in.skip();
              if (in.pos >= in.text.size())
                fail("expected a symbol for row " + std::to_string(i+1) + ", column " + std::to_string(j+1) +
                     ", found end of input");
              ch = in.text[in.pos];
              if (t.strip.find(ch) == std::string::npos)
                break;
              in.advance(in.pos+1);
            }
            size_t k = t.alphabet.find(ch);
            if (t.blank.find(ch) != std::string::npos) {
              g.ints.push(0);
            } else if (k != std::string::npos) {
              long long int v;
              if (!add(static_cast<long long int>(k), t.base, v))
                fail("integer overflow in the value of a symbol");
              g.ints.push(v);
            } else {
              fail("unexpected symbol " + quote(std::string(1, ch)) + ", expected one of " + quote(t.alphabet + t.blank));
            }
            in.advance(in.pos+1);
          }
        // The structure is built after reading, as for arrays
        g.depth = 2;
        g.lengths.clear();
        g.lengths.push(Seq<int>());
        g.lengths[0].push(static_cast<int>(r));
        g.lengths.push(Seq<int>());
        for (long long int i=0; i<r; i++)
          g.lengths[1].push(static_cast<int>(c));
        return g;
      }
    };

  }

  Data
  SpecImpl::read(const std::string& input, const std::string& o, const Params& p, bool catalog) const {
    std::shared_ptr<DataImpl> d = std::make_shared<DataImpl>();
    Interpreter i(*this, input, o, p, catalog);
    i.run(*d);
    Data r;
    r.impl = d;
    return r;
  }

}}

// STATISTICS: instance-any
