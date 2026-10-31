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

#include <gecode/instance/impl.hpp>

#include <climits>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <ostream>
#include <sstream>
#include <unordered_set>

namespace Gecode { namespace Instance {

  /*
   * Errors and parameters
   *
   */

  Error::Error(const std::string& m)
    : Exception("Instance", m.c_str()), msg(m) {}

  const char*
  Error::what(void) const noexcept {
    return msg.c_str();
  }

  Params::Params(void)
    : impl(std::make_shared<ParamsImpl>()) {}

  void
  Params::set(const std::string& name, const std::string& value) {
    if (impl.use_count() > 1)
      impl = std::make_shared<ParamsImpl>(*impl);
    for (int i=0; i<impl->names.size(); i++)
      if (impl->names[i] == name) {
        impl->values[i] = value;
        return;
      }
    impl->names.push(name);
    impl->values.push(value);
  }

  void
  Params::set(const std::string& assignments) {
    size_t b = 0;
    while (b <= assignments.size()) {
      size_t e = assignments.find(',', b);
      if (e == std::string::npos)
        e = assignments.size();
      std::string a = assignments.substr(b, e-b);
      b = e+1;
      if (a.empty())
        continue;
      size_t eq = a.find('=');
      if ((eq == std::string::npos) || (eq == 0))
        throw Error("parameter '" + a + "' must have the form name=value");
      set(a.substr(0,eq), a.substr(eq+1));
    }
  }

  bool
  Params::get(const std::string& name, std::string& value) const {
    for (int i=0; i<impl->names.size(); i++)
      if (impl->names[i] == name) {
        value = impl->values[i];
        return true;
      }
    return false;
  }

  int
  Params::size(void) const {
    return impl->names.size();
  }

  std::string
  Params::name(int i) const {
    if ((i < 0) || (i >= impl->names.size()))
      throw Error("parameter index out of range");
    return impl->names[i];
  }

  /*
   * Values
   *
   */

  Column::Column(ValueType t, int depth)
    : rows(0), pending(0), started(false), missing(false), d(depth) {
    c.type = t;
  }

  void
  Column::skip(void) {
    rows++;
    missing = true;
    if (!started)
      pending++;
    else if (d == 0)
      throw ShapeError("is defined in only some repetitions");
    else
      c.lengths[1].push(0);
  }

  void
  Column::add(Value& v) {
    rows++;
    if (!started) {
      if (missing && (v.depth == 0))
        throw ShapeError("is defined in only some repetitions");
      started = true;
      d = v.depth;
      c.type = v.type;
      c.depth = d+1;
      c.lengths.clear();
      for (int k=0; k<=d; k++)
        c.lengths.push(Seq<int>());
      for (int i=0; i<pending; i++)
        c.lengths[1].push(0);
    } else if (v.depth != d) {
      throw ShapeError("has rows of different depths");
    }
    if (v.leaves() > 0) {
      // The type follows the values; integers become floats if needed
      if (c.leaves() == 0)
        c.type = v.type;
      else if ((c.type == ValueType::STRING) != (v.type == ValueType::STRING))
        throw ShapeError("mixes strings and numbers");
      else if ((c.type == ValueType::INT) && (v.type == ValueType::FLOAT))
        c.to_float();
    }
    for (int k=0; k<d; k++)
      for (int l : v.lengths[k])
        c.lengths[k+1].push(l);
    switch (c.type) {
    case ValueType::INT:
      for (long long int x : v.ints) c.ints.push(x);
      break;
    case ValueType::FLOAT:
      for (long long int x : v.ints) c.reals.push(static_cast<double>(x));
      for (double x : v.reals) c.reals.push(x);
      break;
    default:
      for (std::string& x : v.strs) c.strs.push(std::move(x));
      break;
    }
  }

  Value
  Column::finish(void) {
    if (!started) {
      // No rows added: every row is missing, so empty if it is a list
      if ((d == 0) && (rows > 0))
        throw ShapeError("is defined in no repetition");
      c.depth = d+1;
      c.lengths.clear();
      for (int k=0; k<=d; k++)
        c.lengths.push(Seq<int>());
      for (int i=0; (d > 0) && (i<rows); i++)
        c.lengths[1].push(0);
    }
    c.lengths[0].clear();
    c.lengths[0].push(rows);
    return std::move(c);
  }

  void
  Value::index(void) {
    // The memory of earlier starts is reused (see clear_list)
    while (starts.size() > depth)
      starts.pop();
    while (starts.size() < depth)
      starts.push(Seq<int>());
    for (int k=0; k<depth; k++) {
      Seq<int>& s = starts[k];
      s.clear();
      s.reserve(lengths[k].size()+1);
      int sum = 0;
      for (int l : lengths[k]) {
        s.push(sum);
        sum += l;
      }
      s.push(sum);
    }
  }

  void
  Value::range(int level, int pos, int& from, int& to) const {
    assert(starts.size() == depth);
    from = pos; to = pos+1;
    for (int k=level; k<depth; k++) {
      from = starts[k][from];
      to = starts[k][to];
    }
  }

  /*
   * Data access
   *
   */

  namespace {

    std::string
    where(const std::string& n, const IntArgs& path, int np) {
      std::string s = "'" + n;
      for (int i=0; i<np; i++)
        s += "[" + std::to_string(path[i]) + "]";
      return s + "'";
    }

    /// Return value \a n or throw
    const Value&
    lookup(const std::shared_ptr<const DataImpl>& impl, const std::string& n) {
      const Value* v = impl->find(n);
      if (v == nullptr)
        throw Error("no value named '" + n + "'");
      return *v;
    }

    /**
     * \brief Locate the list or leaf at \a path
     *
     * If \a leaf is true, the path must lead to a leaf and its index is
     * returned. Otherwise it must lead to a list, and the list's level and
     * position are stored in \a level and \a pos.
     */
    int
    locate(const Value& v, const std::string& n, const IntArgs& path, bool leaf, int& level, int& pos) {
      int np = path.size();
      if (leaf ? (np != v.depth) : (np >= v.depth))
        throw Error(where(n,path,np) + (leaf ? " is not a single value" : " is not an array"));
      level = 0; pos = 0;
      for (int i=0; i<np; i++) {
        int len = v.lengths[level][pos];
        if ((path[i] < 0) || (path[i] >= len))
          throw Error("index out of range in " + where(n,path,i+1) + " (size " + std::to_string(len) + ")");
        pos = v.starts[level][pos] + path[i];
        level++;
      }
      return pos;
    }

    int
    narrow(long long int x, const std::string& n) {
      if ((x < INT_MIN) || (x > INT_MAX))
        throw Error("value " + std::to_string(x) + " in '" + n + "' does not fit into an int");
      return static_cast<int>(x);
    }

    long long int
    integer_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      int i = locate(v,n,path,true,level,pos);
      if (v.type != ValueType::INT)
        throw Error(where(n,path,path.size()) + " is not an integer");
      return v.ints[i];
    }

    double
    number_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      int i = locate(v,n,path,true,level,pos);
      if (v.type == ValueType::STRING)
        throw Error(where(n,path,path.size()) + " is not a number");
      return v.as_double(i);
    }

    std::string
    text_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      int i = locate(v,n,path,true,level,pos);
      if (v.type != ValueType::STRING)
        throw Error(where(n,path,path.size()) + " is not a string");
      return v.strs[i];
    }

    int
    size_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      locate(v,n,path,false,level,pos);
      return v.lengths[level][pos];
    }

    IntArgs
    ints_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      locate(v,n,path,false,level,pos);
      if ((v.type != ValueType::INT) && (v.leaves() > 0))
        throw Error(where(n,path,path.size()) + " does not contain integers");
      int from, to;
      v.range(level,pos,from,to);
      IntArgs a(to-from);
      for (int i=from; i<to; i++)
        a[i-from] = narrow(v.ints[i],n);
      return a;
    }

    ArgArray<double>
    numbers_at(const std::shared_ptr<const DataImpl>& impl, const std::string& n, const IntArgs& path) {
      const Value& v = lookup(impl,n);
      int level, pos;
      locate(v,n,path,false,level,pos);
      if ((v.type == ValueType::STRING) && (v.leaves() > 0))
        throw Error(where(n,path,path.size()) + " does not contain numbers");
      int from, to;
      v.range(level,pos,from,to);
      ArgArray<double> a(to-from);
      for (int i=from; i<to; i++)
        a[i-from] = v.as_double(i);
      return a;
    }

  }

  Data::Data(void)
    : impl(std::make_shared<DataImpl>()) {}

  int
  Data::entries(void) const {
    return impl->entries.size();
  }

  std::string
  Data::name(int i) const {
    if ((i < 0) || (i >= impl->entries.size()))
      throw Error("entry index out of range");
    return impl->entries[i].name;
  }

  bool
  Data::has(const std::string& n) const {
    return impl->find(n) != nullptr;
  }

  ValueType
  Data::type(const std::string& n) const {
    return lookup(impl,n).type;
  }

  int
  Data::depth(const std::string& n) const {
    return lookup(impl,n).depth;
  }

  int
  Data::size(const std::string& n, const IntArgs& path) const {
    return size_at(impl,n,path);
  }

  long long int
  Data::integer(const std::string& n, const IntArgs& path) const {
    return integer_at(impl,n,path);
  }

  double
  Data::number(const std::string& n, const IntArgs& path) const {
    return number_at(impl,n,path);
  }

  std::string
  Data::text(const std::string& n, const IntArgs& path) const {
    return text_at(impl,n,path);
  }

  IntArgs
  Data::ints(const std::string& n, const IntArgs& path) const {
    return ints_at(impl,n,path);
  }

  ArgArray<double>
  Data::numbers(const std::string& n, const IntArgs& path) const {
    return numbers_at(impl,n,path);
  }

  /*
   * Output
   *
   */

  namespace {

    /**
     * \brief Print \a f in the shortest form that reads back as the same double
     *
     * Values are always finite: input and arithmetic reject infinities.
     */
    void
    print_real(std::ostream& os, double f) {
      // Start with all integer digits to avoid "2e+01" for 20
      int digits = (std::fabs(f) >= 1.0) && (std::fabs(f) < 1e17)
        ? static_cast<int>(std::floor(std::log10(std::fabs(f)))) + 1 : 1;
      for (int prec=std::max(digits,1); prec<=17; prec++) {
        // Independent of the global and the stream's locale
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::setprecision(prec) << f;
        std::istringstream back(ss.str());
        back.imbue(std::locale::classic());
        double g = 0.0;
        back >> g;
        if ((g == f) || (prec == 17)) {
          std::string t = ss.str();
          if (t.find_first_of(".e") == std::string::npos)
            t += ".0";
          os << t;
          return;
        }
      }
    }

    /// Print \a s as a MiniZinc string
    void
    print_quoted(std::ostream& os, const std::string& s) {
      os << '"';
      for (char c : s) {
        if ((c == '"') || (c == '\\')) {
          os << '\\' << c;
        } else if (c == '\n') {
          os << "\\n";
        } else if (c == '\t') {
          os << "\\t";
        } else if (static_cast<unsigned char>(c) < 0x20) {
          // Other control characters are written as \xXX
          static const char hex[] = "0123456789abcdef";
          os << "\\x" << hex[(c >> 4) & 0xf] << hex[c & 0xf];
        } else {
          os << c;
        }
      }
      os << '"';
    }

    /// Print leaf \a i of \a v as MiniZinc
    void
    print_leaf(std::ostream& os, const Value& v, int i) {
      switch (v.type) {
      case ValueType::INT: os << std::to_string(v.ints[i]); break;
      case ValueType::FLOAT: print_real(os, v.reals[i]); break;
      default: print_quoted(os, v.strs[i]); break;
      }
    }

    /// Print name \a n as a MiniZinc identifier, quoted if needed
    void
    print_name(std::ostream& os, const std::string& n) {
      static const std::unordered_set<std::string> keywords = {
        "ann", "annotation", "any", "array", "bool", "case", "constraint", "default", "diff", "div", "else", "elseif",
        "endif", "enum", "false", "float", "function", "if", "in", "include", "int", "intersect", "let", "list",
        "maximize", "minimize", "mod", "not", "of", "op", "opt", "output", "par", "predicate", "record", "satisfy",
        "set", "solve", "string", "subset", "superset", "symdiff", "test", "then", "true", "tuple", "type", "union",
        "var", "where", "xor"
      };
      // MiniZinc identifiers start with a letter
      bool plain = !n.empty() && is_letter(n[0]) && (keywords.count(n) == 0);
      if (plain)
        os << n;
      else
        os << '\'' << n << '\'';
    }

    /**
     * \brief Print the next list at \a level of \a v as a MiniZinc array of 1-tuples
     *
     * Entry \a k of \a next is the position of the next list at level \a k,
     * and the entry for the depth of \a v is the next leaf.
     */
    void
    print_list(std::ostream& os, const Value& v, Seq<int>& next, int level) {
      int pos = next[level]++;
      int len = v.lengths[level][pos];
      os << '[';
      for (int i=0; i<len; i++) {
        if (i > 0) os << ", ";
        if (level+1 == v.depth) {
          print_leaf(os, v, next[v.depth]++);
        } else {
          os << '(';
          print_list(os, v, next, level+1);
          os << ",)";
        }
      }
      os << ']';
    }

    /// Test whether all lists of each level have the same length
    bool
    rectangular(const Value& v) {
      for (int k=1; k<v.depth; k++)
        for (int l : v.lengths[k])
          if (l != v.lengths[k][0])
            return false;
      return true;
    }

  }

  void
  Data::dzn(std::ostream& os) const {
    for (const Entry& en : impl->entries) {
      const Value& v = en.value;
      const std::string& n = en.name;
      if (v.depth == 0) {
        print_name(os, n);
        os << " = ";
        print_leaf(os, v, 0);
        os << ";\n";
        continue;
      }
      int nl = v.leaves();
      // MiniZinc has array constructors up to array6d
      if (rectangular(v) && (v.depth <= 6)) {
        print_name(os, n);
        os << " = ";
        if ((v.depth == 2) && (v.size() > 0) && (v.lengths[1][0] > 0)) {
          int cols = v.lengths[1][0];
          os << "[|";
          for (int i=0; i<nl; i++) {
            if (i % cols == 0)
              os << (i > 0 ? "\n  | " : " ");
            else
              os << ", ";
            print_leaf(os, v, i);
          }
          os << " |];\n";
          continue;
        }
        // Arrays with an empty dimension need explicit index sets
        bool empty = false;
        for (int k=0; k<v.depth; k++)
          empty = empty || (v.lengths[k].size() == 0) || (v.lengths[k][0] == 0);
        bool nd = (v.depth > 2) || ((v.depth == 2) && empty);
        if (nd) {
          os << "array" << v.depth << "d(";
          for (int k=0; k<v.depth; k++)
            os << "1.." << (v.lengths[k].size() > 0 ? v.lengths[k][0] : 0) << ", ";
        }
        os << "[";
        for (int i=0; i<nl; i++) {
          if (i > 0) os << ", ";
          print_leaf(os, v, i);
        }
        os << "]" << (nd ? ")" : "") << ";\n";
        continue;
      }
      // Ragged or deeper: arrays of 1-tuples of arrays (MiniZinc 2.7 and later)
      std::string type = std::string("array[int] of ") +
        (v.type == ValueType::INT ? "int" : (v.type == ValueType::FLOAT ? "float" : "string"));
      for (int k=1; k<v.depth; k++)
        type = "array[int] of tuple(" + type + ")";
      os << "% " << n << ": " << type << "\n";
      print_name(os, n);
      os << " = ";
      Seq<int> next;
      for (int k=0; k<=v.depth; k++)
        next.push(0);
      print_list(os, v, next, 0);
      os << ";\n";
    }
  }

}}

// STATISTICS: instance-any
