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

#ifndef GECODE_INSTANCE_EVAL_HPP
#define GECODE_INSTANCE_EVAL_HPP

#include <gecode/instance/impl.hpp>

namespace Gecode { namespace Instance {

  /// Return \a s in single quotes, with newlines and tabs escaped
  std::string quote(const std::string& s);
  /// Store \a x + \a y in \a r unless it overflows
  bool add(long long int x, long long int y, long long int& r);
  /// Store \a x - \a y in \a r unless it overflows
  bool sub(long long int x, long long int y, long long int& r);

  /**
   * \brief Records or a list of records of a named repeat
   *
   * The indices \a path select them in the record starts of the repeat
   * (see record_starts), and in each name defined in the repeat.
   */
  class RecordRef {
  public:
    /// The repeat
    const Stmt* repeat;
    /// The indices
    Seq<int> path;
    /// Initialize without a repeat
    RecordRef(void)
      : repeat(nullptr) {}
  };

  /// A part of the explanation of a failed check, such as "record 3 of
  /// 'jobs'", with the position of the record in the input if known
  class Witness {
  public:
    /// The text
    std::string text;
    /// The position, or npos
    size_t pos;
  };

  /// A name bound by a comprehension, visible while the binding exists
  class Binding {
  private:
    /// The innermost binding, which this binding is while it exists
    const Binding*& innermost;
  public:
    /// The name
    const std::string& name;
    /// The value, unless a record
    const Value* value;
    /// The record, if a record of a named repeat
    const RecordRef* record;
    /// The binding of the enclosing comprehension, or null
    const Binding* outer;
    /// Bind \a n inside the bindings starting at \a b
    Binding(const Binding*& b, const std::string& n)
      : innermost(b), name(n), value(nullptr), record(nullptr), outer(b) {
      innermost = this;
    }
    /// Remove the binding
    ~Binding(void) {
      innermost = outer;
    }
  };

  /// A position in the input, with its line
  class Place {
  public:
    /// Position
    size_t pos;
    /// Line (1-based)
    int line;
    /// Start of the line
    size_t start;
  };

  /// A statement being executed, for errors
  class Context {
  public:
    /// What is executed, such as a name
    std::string text;
    /// The iteration of a repeat or instances statement, from 1, or 0
    long long int iteration;
    /// The number of iterations, or -1 if unknown
    long long int iterations;
    /// Initialize for \a t in iteration \a i of \a n
    explicit Context(const std::string& t, long long int i=0, long long int n=-1)
      : text(t), iteration(i), iterations(n) {}
  };

  /// Instance text with a position and line tracking
  class Input {
  private:
    /// What a character is in tokens
    enum class CharKind : unsigned char {
      PLAIN,     ///< Part of a token
      SEPARATOR, ///< Separates tokens, like white space
      SPLIT      ///< A token by itself
    };
    /// The kind of each character
    CharKind kind[256];
    /// Return the kind of \a c
    CharKind kind_of(char c) const {
      return kind[static_cast<unsigned char>(c)];
    }
  public:
    /// The text
    const std::string& text;
    /// Where the text comes from
    std::string origin;
    /// Comment line prefixes
    Seq<std::string> comments;
    /// Prefixes of lines that end the data
    Seq<std::string> stops;
    /// Current position
    size_t pos;
    /// Current line (1-based)
    int line;
    /// Start of current line
    size_t start;

    /// Initialize for \a t from \a o
    Input(const std::string& t, const std::string& o)
      : text(t), origin(o), pos(0), line(1), start(0) {
      for (int c=0; c<256; c++)
        kind[c] = is_space(static_cast<char>(c)) ? CharKind::SEPARATOR : CharKind::PLAIN;
    }

    /// Make the characters \a cs tokens by themselves, unless they separate tokens
    void add_split(const std::string& cs) {
      for (char c : cs)
        if (kind_of(c) == CharKind::PLAIN)
          kind[static_cast<unsigned char>(c)] = CharKind::SPLIT;
    }
    /// Make the characters \a cs separate tokens like white space
    void add_separators(const std::string& cs) {
      for (char c : cs)
        kind[static_cast<unsigned char>(c)] = CharKind::SEPARATOR;
    }

    /// Move forward to \a p
    void advance(size_t p) {
      while (pos < p) {
        if (text[pos] == '\n') {
          line++;
          start = pos+1;
        }
        pos++;
      }
    }
    /// Test whether only blanks precede the position on its line
    bool line_start(void) const {
      for (size_t k=start; k<pos; k++)
        if (!is_blank(text[k]))
          return false;
      return true;
    }
    /// Test whether the text at the position starts with \a prefix
    bool starts_with(const std::string& prefix) const {
      if (prefix.empty() || (text.compare(pos, prefix.size(), prefix) != 0))
        return false;
      size_t after = pos + prefix.size();
      return !is_alnum(prefix[prefix.size()-1]) || (after >= text.size()) || !is_alnum(text[after]);
    }
    /// Move to the end of the current line
    void to_eol(void) {
      size_t p = text.find('\n', pos);
      advance(p == std::string::npos ? text.size() : p);
    }
    /// Test whether \a c separates tokens
    bool is_separator(char c) const {
      return kind_of(c) == CharKind::SEPARATOR;
    }
    /// Test whether \a c forms a token by itself
    bool is_split(char c) const {
      return kind_of(c) == CharKind::SPLIT;
    }
    /// Test whether position \a q is within the current line
    bool in_line(size_t q) const {
      return (q < text.size()) && (text[q] != '\n');
    }
    /// Skip white space, comment lines, and everything after a stop line
    void skip(void) {
      for (;;) {
        const char* s = text.data();
        size_t n = text.size();
        while ((pos < n) && is_separator(s[pos])) {
          if (s[pos] == '\n') {
            line++;
            start = pos+1;
          }
          pos++;
        }
        if ((pos >= text.size()) || !line_start())
          return;
        bool comment = false, stop = false;
        for (const std::string& c : comments)
          comment = comment || starts_with(c);
        for (const std::string& s : stops)
          stop = stop || starts_with(s);
        if (comment) {
          to_eol();
        } else if (stop) {
          advance(text.size());
          return;
        } else {
          return;
        }
      }
    }
    /// Skip to the next data (see skip), and return whether there is any
    bool skip_to_data(void) {
      skip();
      return pos < text.size();
    }
    /// Skip blanks and separators, and return whether data follows on the line
    bool skip_to_line_data(void) {
      while (in_line(pos) && is_separator(text[pos]))
        pos++;
      return in_line(pos);
    }
    /// Return where the token at the position ends
    size_t token_end(void) const {
      size_t q = pos;
      if ((q < text.size()) && is_split(text[q]))
        return q+1;
      while ((q < text.size()) && (kind_of(text[q]) == CharKind::PLAIN))
        q++;
      return q;
    }
    /// Read the token at the position
    std::string token(void) {
      size_t q = token_end();
      std::string t = text.substr(pos, q-pos);
      advance(q);
      return t;
    }
    /**
     * \brief Read the token at the position as an integer \a v, and
     * return whether it is one
     *
     * Accepts what parse_int accepts, without making a string.
     */
    bool int_token(long long int& v) {
      const char* s = text.data();
      size_t n = text.size(), p = pos;
      if ((p < n) && is_split(s[p])) {
        if (!is_digit(s[p]))
          return false;
        v = s[p] - '0';
        pos = p+1;
        return true;
      }
      bool negative = (p < n) && (s[p] == '-');
      if ((p < n) && is_sign(s[p]))
        p++;
      size_t digits = p;
      // The magnitude, which may be one more than LLONG_MAX if negative
      unsigned long long int limit = static_cast<unsigned long long int>(LLONG_MAX) + (negative ? 1 : 0);
      unsigned long long int m = 0;
      for (; (p < n) && is_digit(s[p]) && (kind_of(s[p]) == CharKind::PLAIN); p++) {
        unsigned int d = static_cast<unsigned int>(s[p] - '0');
        if (m > (limit - d) / 10)
          return false;
        m = 10*m + d;
      }
      // The token must end after the digits
      if ((p == digits) || ((p < n) && (kind_of(s[p]) == CharKind::PLAIN)))
        return false;
      v = negative ? static_cast<long long int>(0ULL - m) : static_cast<long long int>(m);
      // A token holds no newline
      pos = p;
      return true;
    }
    /// Return the next token without consuming it
    std::string peek(void) {
      size_t p0 = pos, s0 = start;
      int l0 = line;
      std::string t = skip_to_data() ? token() : std::string();
      pos = p0; start = s0; line = l0;
      return t;
    }
    /// Return the rest of the current line without trailing blanks
    std::string rest(void) const {
      size_t q = text.find('\n', pos);
      if (q == std::string::npos) q = text.size();
      while ((q > pos) && is_blank(text[q-1])) q--;
      return text.substr(pos, q-pos);
    }
    /// Return the line starting at \a s without trailing blanks
    std::string line_at(size_t s) const {
      size_t q = text.find('\n', s);
      if (q == std::string::npos) q = text.size();
      while ((q > s) && is_blank(text[q-1])) q--;
      return text.substr(s, q-s);
    }
    /// Return the current place
    Place here(void) const {
      Place p;
      p.pos = pos; p.line = line; p.start = start;
      return p;
    }
    /// Return the place of position \a q, which is not after the current one
    Place place(size_t q) const {
      assert(q <= pos);
      // Counted back from the current place, as q is usually close to it
      Place p;
      p.pos = q; p.line = line; p.start = q;
      for (size_t k=q; k<pos; k++)
        if (text[k] == '\n')
          p.line--;
      while ((p.start > 0) && (text[p.start-1] != '\n'))
        p.start--;
      return p;
    }
    /// Return where the next data starts (see skip), without moving there
    size_t next_data(void) {
      size_t p0 = pos, s0 = start;
      int l0 = line;
      skip();
      size_t r = pos;
      pos = p0; start = s0; line = l0;
      return r;
    }
  };

  /**
   * \brief Evaluator of the expressions of a spec on an input
   *
   * Holds the values defined so far and evaluates expressions and checks
   * with them. Errors point at the current position in the input.
   */
  class Evaluator {
  public:
    /// Initialize for spec \a s and input \a text from \a origin
    Evaluator(const SpecImpl& s, const std::string& text, const std::string& origin);
  protected:
    /// The spec
    const SpecImpl& spec;
    /// The input
    Input in;
    /// Values: the outermost frame and one per active repeat iteration
    Seq<Seq<Entry>> frames;
    /// Description of the statements being executed, for errors
    Seq<Context> context;
    /// Spec line of the current statement
    int spec_line;
    /// Names bound by the comprehensions being evaluated
    const Binding* bindings;
    /// The check being evaluated, or null
    const Stmt* checked;
    /// Values of its record names (see Record)
    Seq<const Value*> records;
    /// The records of the deepest of them, one index per repeat level
    const Seq<int>* record_at;
    /// Why the last condition evaluated is false, for messages: the
    /// elements of the all-comprehensions that failed, innermost first
    Seq<Witness> why;
    /// Values visited by expressions
    long long int effort;
    /// Limit for effort, which grows with the input, so that repeated or
    /// nested expressions cannot take time far beyond the input's size
    const long long int max_effort;
    /// Base of the limit for effort
    static const long long int base_effort = 1LL << 20;

    /// Return the error message \a msg with the location and context
    std::string located(const std::string& msg);
    /// Return the error message \a msg at \a at with the context
    std::string located(const std::string& msg, const Place& at);
    /// Throw an error with message \a msg at the current position
    [[noreturn]] void fail(const std::string& msg);
    /// Throw an error for the shape error \a e of the value of \a n
    [[noreturn]] void fail(const ShapeError& e, const std::string& n);
    /// Account for visiting \a k values in expressions
    void visit(long long int k);
    /// Define \a n as \a v in the innermost frame
    void set(const std::string& n, Value&& v);
    /// Return the value of \a n, or null if it is not defined
    const Value* lookup(const std::string& n) const;

    /*
     * Expressions
     */
    /// Test whether \a v is true
    static bool truth(const Value& v);
    /// Return \a b as an integer
    static Value boolean(bool b);
    /**
     * \brief Evaluate \a e, avoiding a copy for names
     *
     * The value is stored in \a tmp unless \a e is a name. Callers pass
     * an integer scalar without a leaf as \a tmp, which needs no memory.
     */
    const Value& ref(const Expr& e, Value& tmp);
    /// Return the indices of the record \a at of the check that select
    /// the records of its record name \a i
    Seq<int> record_path(int i) const;
    /// Return the record starts of repeat \a r, failing for \a n if the
    /// repeat has not run yet
    const Value& starts(const Stmt* r, const std::string& n);
    /// Return the number of records in list \a l
    int count_records(const RecordRef& l);
    /// Return where record \a i of list \a l starts in the input, or npos
    size_t record_start(const RecordRef& l, int i);
    /// Evaluate \a e, a record or a list of records
    RecordRef records_of(const Expr& e);
    /**
     * \brief Evaluate field \a e of a record or a list of records
     *
     * The field of a list is the array of the field of each record. The
     * value is stored in \a tmp unless it needs no copy.
     */
    const Value& field(const Expr& e, Value& tmp);
    /// Return the integer of single value \a v, or fail for \a op
    long long int integer(const Value& v, const std::string& op);
    /// Test whether \a e passes on why it is false (see why)
    static bool explains(const Expr& e);
    /// Evaluate \a e, keeping why only if \a e explains it
    Value eval(const Expr& e);
    /// Evaluate \a e
    Value evaluate(const Expr& e);
    /// Return whether \a e is true, avoiding a copy for names
    bool test(const Expr& e);
    /// Return whether \a e is true, with only its own explanation in why
    bool explained(const Expr& e);
    /// Evaluate unary expression \a e
    Value unary(const Expr& e);
    /// Evaluate binary expression \a e
    Value binary(const Expr& e);
    /// Compare single values \a a and \a b with the operator of \a e
    bool compare(const Value& a, const Value& b, const Expr& e);
    /// Call a function (names and arguments are checked by the parser)
    Value call(const Expr& e);
    /// Test whether name \a e is defined
    bool defined(const Expr& e) const;
    /// Test whether all integers in the arguments of call \a e differ
    bool distinct(const Expr& e);
    /// Describe the array of index expression \a e, for messages
    static std::string indexed(const Expr& e);
    /// Return the value of index \a e
    long long int index(const Expr& e);
    /**
     * \brief Return a hint for indexing \a e, a single value
     *
     * A record name in a check is one record, so that indexing the
     * records needs their name.
     */
    std::string scalar_hint(const Expr& e) const;
    /// a[i]: element i, counted from 0, of the outermost list of a
    Value subscript(const Expr& e);
    /// Describe array \a a of a comprehension, for messages
    static std::string array_name(const Expr& a);
    /// Return the records that are the elements of array \a a of
    /// comprehension \a e, an array of values (see Expr::elements)
    RecordRef element_records(const Expr& e, const Expr& a);
    /// Add why element \a i of the array of all-comprehension \a e,
    /// with records \a r, is false
    void explain(const Expr& e, const RecordRef& r, int i);
    /**
     * \brief all, any, count, and sum over the elements of an array
     *
     * The name of the comprehension \a e is bound to each element of
     * the outermost list of the array in turn, or to each record of a
     * list of records. \c all and \c any stop at the first element that
     * decides the result.
     */
    Value comprehension(const Expr& e);
    /// Return the value of \a e, a count described by \a what
    long long int count(const Expr& e, const char* what);
    /**
     * \brief Test whether check \a s holds, and otherwise store why it
     * fails in \a error
     *
     * The check holds for every record of its record names (see Record)
     * that are defined. The others are not defined when its expression
     * uses them.
     */
    bool holds(const Stmt& s, std::string& error);
    /// Test whether record name \a i of check \a s is defined and ranges
    /// over the repeat at \a level
    bool ranges(const Stmt& s, int i, int level) const;
    /**
     * \brief Evaluate check \a s for the records inside the records \a at
     * of its record name \a deepest, and return whether it holds
     *
     * \a at has a record number per repeat level, from the outermost.
     * Each record name that ranges over a level is set to its record.
     * If the check fails, \a error is why.
     */
    bool check_records(const Stmt& s, int deepest, Seq<int>& at, std::string& error);
    /// Return where the record \a at of record name \a r starts in the
    /// input, or npos if that is unknown
    size_t record_place(const Record& r, const Seq<int>& at);
    /// Return the place of position \a q in the input, or the current
    /// place if \a q is npos
    Place place(size_t q) const;
    /// Describe the records \a at of record name \a r, innermost first
    static std::string record_text(const Record& r, const Seq<int>& at);
    /// Return why check \a s fails for the records \a at of its record
    /// name \a deepest
    std::string failure(const Stmt& s, int deepest, const Seq<int>& at);
  };

}}

#endif

// STATISTICS: instance-any
