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

/*
 * Internal declarations shared by the implementation files of the
 * instance module. Not part of the public interface.
 */

#ifndef GECODE_INSTANCE_IMPL_HPP
#define GECODE_INSTANCE_IMPL_HPP

#include <gecode/instance.hh>

#include <climits>
#include <new>
#include <utility>

namespace Gecode { namespace Instance {

  /*
   * Character classes and number syntax, ASCII only and independent of the
   * global locale
   */

  /// Test whether \a c is a digit
  inline bool is_digit(char c) {
    return (c >= '0') && (c <= '9');
  }
  /// Test whether \a c is a letter
  inline bool is_letter(char c) {
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
  }
  /// Test whether \a c can occur in a name
  inline bool is_alnum(char c) {
    return is_digit(c) || is_letter(c) || (c == '_');
  }
  /// Test whether \a c is white space
  inline bool is_space(char c) {
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f');
  }
  /// Test whether \a c is white space within a line
  inline bool is_blank(char c) {
    return (c == ' ') || (c == '\t') || (c == '\r');
  }
  /// Test whether \a c is a sign
  inline bool is_sign(char c) {
    return (c == '+') || (c == '-');
  }
  /// Test whether \a c starts the exponent of a float
  inline bool is_exponent(char c) {
    return (c == 'e') || (c == 'E');
  }
  /**
   * \brief Return where the number starting at position \a p of \a s ends
   *
   * A number has digits, a sign first if \a sign, and, if \a real, also
   * '.', 'e', 'E', and a sign directly after 'e' or 'E'. Only the
   * characters are scanned: parse_int and parse_float check the syntax.
   */
  inline size_t number_end(const std::string& s, size_t p, bool sign, bool real) {
    size_t q = p;
    for (; q < s.size(); q++) {
      char c = s[q];
      if (is_digit(c))
        continue;
      if (sign && (q == p) && is_sign(c))
        continue;
      if (real && ((c == '.') || is_exponent(c)))
        continue;
      if (real && (q > p) && is_sign(c) && is_exponent(s[q-1]))
        continue;
      break;
    }
    return q;
  }
  /// Parse decimal integer \a t into \a v, and return whether it is one
  bool parse_int(const std::string& t, long long int& v);
  /// Parse decimal float \a t into \a v, and return whether it is one
  bool parse_float(const std::string& t, double& v);

  /**
   * \brief Growable array on the Gecode heap
   *
   * Supports move-only element types. Elements are moved when the array
   * grows.
   */
  template<class T>
  class Seq {
  private:
    /// Elements
    T* x;
    /// Number of elements
    int n;
    /// Capacity
    int cap;
    /// Grow to capacity at least \a m
    void grow(int m) {
      // Doubling, computed without overflow
      long long int d = std::min(2LL*cap, static_cast<long long int>(INT_MAX));
      int c = std::max(std::max(m, static_cast<int>(d)), 2);
      T* y = static_cast<T*>(heap.ralloc(sizeof(T)*static_cast<size_t>(c)));
      for (int i=0; i<n; i++) {
        new (&y[i]) T(std::move(x[i]));
        x[i].~T();
      }
      if (x != nullptr)
        heap.rfree(x);
      x = y; cap = c;
    }
  public:
    /// Empty array
    Seq(void)
      : x(nullptr), n(0), cap(0) {}
    /// Copy elements of \a s
    Seq(const Seq& s)
      : Seq() {
      // Delegating, so that the destructor runs if a copy throws
      reserve(s.n);
      for (int i=0; i<s.n; i++)
        push(s.x[i]);
    }
    /// Take elements of \a s
    Seq(Seq&& s) noexcept : x(s.x), n(s.n), cap(s.cap) {
      s.x = nullptr; s.n = 0; s.cap = 0;
    }
    /// Assign elements of \a s
    Seq& operator =(Seq s) noexcept {
      std::swap(x,s.x); std::swap(n,s.n); std::swap(cap,s.cap);
      return *this;
    }
    /// Release memory
    ~Seq(void) {
      clear();
      if (x != nullptr)
        heap.rfree(x);
    }
    /// Return number of elements
    int size(void) const { return n; }
    /// Test whether empty
    bool empty(void) const { return n == 0; }
    /// Make room for \a m elements
    void reserve(int m) {
      if (m > cap) grow(m);
    }
    /// Append a copy of \a v
    void push(const T& v) {
      if (n == cap) {
        T t(v);
        grow(n+1);
        new (&x[n]) T(std::move(t));
      } else {
        new (&x[n]) T(v);
      }
      n++;
    }
    /// Append \a v
    void push(T&& v) {
      if (n == cap) {
        T t(std::move(v));
        grow(n+1);
        new (&x[n]) T(std::move(t));
      } else {
        new (&x[n]) T(std::move(v));
      }
      n++;
    }
    /// Remove last element
    void pop(void) {
      assert(n > 0);
      x[--n].~T();
    }
    /// Remove all elements
    void clear(void) {
      while (n > 0)
        x[--n].~T();
    }
    /// Return element \a i
    T& operator [](int i) {
      assert((i >= 0) && (i < n));
      return x[i];
    }
    /// Return element \a i
    const T& operator [](int i) const {
      assert((i >= 0) && (i < n));
      return x[i];
    }
    /// Return last element
    T& back(void) {
      assert(n > 0);
      return x[n-1];
    }
    /// Return last element
    const T& back(void) const {
      assert(n > 0);
      return x[n-1];
    }
    /// Iteration support
    T* begin(void) { return x; }
    /// Iteration support
    T* end(void) { return x+n; }
    /// Iteration support
    const T* begin(void) const { return x; }
    /// Iteration support
    const T* end(void) const { return x+n; }
  };

  /// Error about the shape or type of a value, located by the caller
  class ShapeError {
  public:
    /// Message
    std::string msg;
    /// Initialize with message \a m
    explicit ShapeError(const std::string& m) : msg(m) {}
  };

  /**
   * \brief A value: scalar or nested (possibly ragged) array
   *
   * The leaves are stored in a flat array of the value's type. The
   * structure of a value of depth \a d is given by \a d levels of list
   * lengths: level 0 has a single entry (the length of the outermost
   * list), level \a k has one entry per list at depth \a k. The lengths at
   * the last level sum to the number of leaves.
   */
  class Value {
  public:
    /// Type of the leaves
    ValueType type;
    /// Nesting depth (0 for scalars)
    int depth;
    /// List lengths per level
    Seq<Seq<int>> lengths;
    /// Start of each list among the lists or leaves one level down (see index)
    Seq<Seq<int>> starts;
    /// Integer leaves
    Seq<long long int> ints;
    /// Float leaves
    Seq<double> reals;
    /// String leaves
    Seq<std::string> strs;

    /// The empty integer list
    Value(void)
      : type(ValueType::INT), depth(1) {
      lengths.push(Seq<int>());
      lengths[0].push(0);
    }
    /// Scalar of type \a t whose leaf is still to be added
    explicit Value(ValueType t)
      : type(t), depth(0) {}
    /// Integer scalar
    static Value integer(long long int v) {
      Value x(ValueType::INT); x.ints.push(v);
      return x;
    }
    /// Float scalar
    static Value real(double v) {
      Value x(ValueType::FLOAT); x.reals.push(v);
      return x;
    }
    /// String scalar
    static Value string(const std::string& v) {
      Value x(ValueType::STRING); x.strs.push(v);
      return x;
    }

    /// Test whether scalar
    bool is_scalar(void) const { return depth == 0; }
    /// Test whether numeric scalar
    bool is_number(void) const { return (depth == 0) && (type != ValueType::STRING); }
    /// Return number of leaves
    int leaves(void) const {
      switch (type) {
      case ValueType::INT: return ints.size();
      case ValueType::FLOAT: return reals.size();
      default: return strs.size();
      }
    }
    /// Return length of outermost list
    int size(void) const { return depth == 0 ? 0 : lengths[0][0]; }
    /// Return value of numeric scalar or leaf \a i as a double
    double as_double(int i=0) const {
      return type == ValueType::INT ? static_cast<double>(ints[i]) : reals[i];
    }
    /// Convert integer leaves to floats
    void to_float(void) {
      if (type != ValueType::INT) return;
      for (long long int v : ints)
        reals.push(static_cast<double>(v));
      ints.clear();
      type = ValueType::FLOAT;
    }
    /// Append the value of scalar \a s, adapting the type
    void push(const Value& s) {
      if (leaves() == 0) {
        type = s.type;
      } else if ((type == ValueType::STRING) != (s.type == ValueType::STRING)) {
        throw ShapeError("strings and numbers are mixed");
      } else if ((type == ValueType::INT) && (s.type == ValueType::FLOAT)) {
        to_float();
      }
      switch (s.type) {
      case ValueType::INT:
        if (type == ValueType::INT) ints.push(s.ints[0]);
        else reals.push(static_cast<double>(s.ints[0]));
        break;
      case ValueType::FLOAT: reals.push(s.reals[0]); break;
      default: strs.push(s.strs[0]); break;
      }
    }
    /// Test whether this is an empty list
    bool is_empty_list(void) const { return (depth > 0) && (lengths[0][0] == 0); }
    /// Make this the empty list of type \a t, keeping the memory for reuse
    void clear_list(ValueType t) {
      type = t;
      depth = 1;
      ints.clear(); reals.clear(); strs.clear();
      while (lengths.size() > 1)
        lengths.pop();
      if (lengths.empty())
        lengths.push(Seq<int>());
      lengths[0].clear();
      lengths[0].push(0);
    }

    /// Compute \a starts, needed by range
    void index(void);
    /// Find leaf range of list \a pos at level \a level
    void range(int level, int pos, int& from, int& to) const;
  };

  /**
   * \brief Array built row by row from the values of a repeated statement
   *
   * Row \a r is the value the name had in iteration \a r. For example,
   * the rows [1,2], [], and [3] give a value of depth 2 with lengths
   * {3} and {2,0,1} and leaves 1,2,3. A missing row (from an iteration
   * that did not define the name) becomes an empty list, which needs
   * rows that are lists.
   *
   * Depth and type come from the rows added. A column without rows takes
   * them from the statement defining the name.
   */
  class Column {
  private:
    /// The array
    Value c;
    /// Number of rows
    int rows;
    /// Missing rows before the first row added
    int pending;
    /// Whether a row has been added
    bool started;
    /// Whether a row has been missing
    bool missing;
    /// Depth of the rows
    int d;
  public:
    /// Empty column for rows of type \a t and depth \a depth
    Column(ValueType t, int depth);
    /// Add a row \a v; throws ShapeError with a phrase completing the name
    void add(Value& v);
    /// Add a missing row
    void skip(void);
    /// Return the array
    Value finish(void);
  };

  /// A named value
  class Entry {
  public:
    /// Name
    std::string name;
    /// Value
    Value value;
    /// Initialize as the empty integer list without a name
    Entry(void) {}
    /// Initialize as \a v named \a n
    Entry(const std::string& n, Value&& v)
      : name(n), value(std::move(v)) {}
  };

  /// Implementation of Data
  class DataImpl {
  public:
    /// The entries in definition order
    Seq<Entry> entries;
    /// Return entry \a n or null
    const Value* find(const std::string& n) const {
      for (const Entry& e : entries)
        if (e.name == n)
          return &e.value;
      return nullptr;
    }
  };

  /// Implementation of Params
  class ParamsImpl {
  public:
    /// Names
    Seq<std::string> names;
    /// Values
    Seq<std::string> values;
  };

  /*
   * Spec syntax tree
   */

  class Stmt;

  /**
   * \brief Expression
   *
   * An index expression <tt>a[i]</tt> has the operands \a a and \a i, a
   * field <tt>r.f</tt> the operand \a r and the name \a f in \a s. A
   * comprehension such as <tt>all(x in a: e)</tt> has the function in
   * \a s, the bound name in \a var, and the operands \a a and \a e.
   *
   * The parser finds the expressions that are records or lists of records
   * of a repeat (see Stmt::list and Stmt::each): \a records is the repeat.
   */
  class Expr {
  public:
    /// Kind of expression
    enum class Kind { INT, FLOAT, STR, NAME, BINARY, UNARY, CALL, INDEX, FIELD, COMPREHENSION } kind;
    /// Operator of unary and binary expressions
    enum class Op {
      OR, AND, EQ, NE, LT, GT, LE, GE, ADD, SUB, MUL, DIV, MOD, POW, NEG, NOT
    } op;
    /// Integer literal
    long long int i;
    /// Float literal
    double f;
    /// String literal, name, operator, or function
    std::string s;
    /// Name bound by a comprehension
    std::string var;
    /// Index of a record name among the records of its check (-1 if none)
    int record;
    /// The repeat whose records or lists of records this is, or null
    const Stmt* records;
    /// 1 for a list of records of \a records, 0 for a record
    int open;
    /// For a comprehension, the repeat whose records are the elements of
    /// its array, or null
    const Stmt* elements;
    /// Operands or arguments
    Seq<std::unique_ptr<Expr>> args;
    /// Initialize with kind \a k
    explicit Expr(Kind k)
      : kind(k), op(Op::OR), i(0), f(0.0), record(-1), records(nullptr), open(0), elements(nullptr) {}
  };

  /**
   * \brief A record name of a check
   *
   * A name defined inside repeats that do not enclose the check, or the
   * name of each record of such a repeat (see Stmt::each), so that the
   * check holds for each of its records.
   */
  class Record {
  public:
    /// The name of the values
    std::string name;
    /// The name in messages: the name, or the list of records
    std::string label;
    /// The repeats between the check and the definition, outermost first
    Seq<const Stmt*> repeats;
    /// Name of the record starts of the innermost of these repeats
    std::string starts;
  };

  /**
   * \brief Return the name of the record starts of the repeat numbered \a id
   *
   * A repeat stores where each of its records starts in the input under
   * this name, which is not a spec name, so that a failed check can point
   * at a record. Such names are not part of the data read.
   */
  inline std::string record_starts(int id) {
    return "#" + std::to_string(id);
  }
  /// Test whether \a n is a name used internally (see record_starts)
  inline bool is_internal(const std::string& n) {
    return !n.empty() && (n[0] == '#');
  }

  /// What a declaration or a capture reads
  enum class ReadType {
    INT,    ///< An integer
    FLOAT,  ///< A float
    NUM,    ///< An integer if possible, otherwise a float
    WORD,   ///< A token as a string
    REST,   ///< The rest of the line as a string
    SYMBOL  ///< A character of an alphabet, as an integer
  };

  /**
   * \brief Type of the values that a declaration or a capture reads
   *
   * An integer type can be a range \c lo..hi, or \c lo..<hi without
   * \a hi, where one of the bounds can be left out. An alphabet type
   * reads one character for each value. A use of a named type (see
   * Stmt::Kind::TYPE) copies its definition except for the bounds, which
   * are evaluated where it is defined.
   */
  class Type {
  public:
    /// What the type reads
    ReadType read;
    /// Lower bound of a range, or null
    std::unique_ptr<Expr> lo;
    /// Upper bound of a range, or null
    std::unique_ptr<Expr> hi;
    /// Whether the upper bound is excluded
    bool exclusive;
    /// The type statement defining a named type, or null
    const Stmt* named;
    /// Name of a named type
    std::string name;
    /// Symbols of an alphabet type
    std::string alphabet;
    /// Symbols that stand for 0
    std::string blank;
    /// Symbols to skip
    std::string strip;
    /// Value of the first symbol
    long long int base;
    /// Initialize as \a r
    explicit Type(ReadType r = ReadType::INT)
      : read(r), exclusive(false), named(nullptr), base(1) {}
    /// Test whether the type has a range, possibly through its name
    bool ranged(void) const {
      return (lo != nullptr) || (hi != nullptr) || (named != nullptr);
    }
    /// Return a use of the type defined by type statement \a d
    Type use(const Stmt* d) const {
      Type t(read);
      t.named = d;
      t.alphabet = alphabet;
      t.blank = blank;
      t.strip = strip;
      t.base = base;
      return t;
    }
  };

  /// Line pattern such as "DIMENSION : {n:int}"
  class Pattern {
  public:
    /// Pattern item
    class Item {
    public:
      /// Kind of item
      enum class Kind { LIT, SPACE, CAP, ANY, VAR } kind;
      /// Literal text, capture name, or variable name
      std::string text;
      /// What a capture reads
      Type type;
      /// Whether a space item needs at least one blank
      bool required;
      /// Initialize with kind \a k
      explicit Item(Kind k) : kind(k), required(false) {}
    };
    /// Source text
    std::string source;
    /// Items
    Seq<Item> items;
  };

  /// Sequence of statements
  typedef Seq<std::unique_ptr<Stmt>> Block;

  /// Arm of a sections statement
  class Arm {
  public:
    /// Line pattern
    Pattern pattern;
    /// Statements run after the line matched
    Block body;
    /// Whether the input may lack a line matching the arm
    bool optional;
    /// Whether the sections statement ends after this arm
    bool last;
    /// Initialize
    Arm(void)
      : optional(false), last(false) {}
  };

  /// Statement
  class Stmt {
  public:
    /// Kind of statement
    enum class Kind {
      COMMENT, STOP, TOKENS, SEPARATORS, TRAILING, PARAM, READ, LIST,
      LINE, FIND, SKIP, EXPECT, INDEX, REPEAT, SECTIONS, IF, LET, CHECK,
      TYPE, INSTANCES, EACH
    } kind;
    /// Line in the spec
    int line;
    /// What read and list statements read, or the type a type statement
    /// defines
    Type type;
    /// Name defined
    std::string name;
    /// Characters for tokens and separators statements
    std::string chars;
    /// Message of a check statement
    std::string message;
    /// Name for the terminating token found
    std::string store;
    /// Name of the list of records of a repeat
    std::string list;
    /// Name of each record of a repeat, inside and after it
    std::string each;
    /// Array dimensions
    Seq<std::unique_ptr<Expr>> dims;
    /// Count, condition, layout, value, or default
    std::unique_ptr<Expr> expr;
    /// List terminator
    std::unique_ptr<Expr> sentinel;
    /// Added to every number read
    std::unique_ptr<Expr> offset;
    /// Prefixes, literal tokens, or terminating tokens
    Seq<std::string> words;
    /// List ends at end of line
    bool until_eol;
    /// Repeat until end of input
    bool until_eof;
    /// Repeat or sections end at a token from \a words
    bool has_until;
    /// Sections skip unknown lines
    bool skip_other;
    /// Pattern for line and find
    Pattern pattern;
    /// Body, then-branch
    Block body;
    /// Else-branch
    Block orelse;
    /// Arms of sections
    Seq<Arm> arms;
    /// Number of a repeat, in the order of the spec
    int id;
    /// Record names of a check
    Seq<Record> records;
    /// Initialize with kind \a k at spec line \a l
    Stmt(Kind k, int l)
      : kind(k), line(l), until_eol(false), until_eof(false), has_until(false), skip_other(false), id(0) {}
  };

  /// A name that a block may define, with the type and depth of its values
  class Decl {
  public:
    /// Name
    std::string name;
    /// Type of the leaves
    ValueType type;
    /// Depth
    int depth;
  };
  /// Add the names that block \a b may define to \a out
  void declared(const Block& b, Seq<Decl>& out);

  /// Throw an error with message \a m for line \a line of the spec from \a origin
  [[noreturn]] void fail_at(const std::string& origin, int line, const std::string& m);

  /**
   * \brief Resolve the names in the expressions of \a b, the body of a
   * spec from \a origin
   *
   * Finds the expressions that are records or lists of records of a named
   * repeat (see Expr) and the record names of the checks (see Record), and
   * reports their misuse.
   */
  void resolve(Block& b, const std::string& origin);

  /// Implementation of Spec
  class SpecImpl {
  public:
    /// Where the spec comes from
    std::string origin;
    /// Format name
    std::string name;
    /// Statements
    Block body;
    /// Parse \a text from \a origin
    SpecImpl(const std::string& text, const std::string& origin);
    /**
     * \brief Read \a input from \a origin
     *
     * With \a catalog, return the number of instances and their names
     * instead of the data of one instance.
     */
    Data read(const std::string& input, const std::string& origin, const Params& p, bool catalog) const;
  };

  /// Read file \a path into a string
  std::string read_file(const std::string& path);

}}

#endif

// STATISTICS: instance-any
