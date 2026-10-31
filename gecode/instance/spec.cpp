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

#include <sys/stat.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <locale>
#include <sstream>

namespace Gecode { namespace Instance {

  std::string
  read_file(const std::string& path) {
    struct stat st;
    if ((stat(path.c_str(), &st) == 0) && ((st.st_mode & S_IFMT) == S_IFDIR))
      throw Error(path + ": is a directory");
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f)
      throw Error(path + ": cannot open file");
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
  }

  bool
  parse_int(const std::string& t, long long int& v) {
    if (t.empty() || is_space(t[0]))
      return false;
    errno = 0;
    char* end = nullptr;
    v = std::strtoll(t.c_str(), &end, 10);
    return (errno == 0) && (end == t.c_str() + t.size());
  }

  bool
  parse_float(const std::string& t, double& v) {
    // Only decimal notation: no hexadecimal, infinity, or NaN
    if (t.empty() || (number_end(t, 0, true, true) != t.size()))
      return false;
    // Out-of-range values fail
    std::istringstream in(t);
    in.imbue(std::locale::classic());
    in >> v;
    return !in.fail() && (in.peek() == EOF);
  }

  void
  fail_at(const std::string& origin, int line, const std::string& m) {
    throw Error(origin + ":" + std::to_string(line) + ": " + m);
  }

  namespace {

    /// Token of the spec language
    class Token {
    public:
      /// Kind of token
      enum class Kind { IDENT, INT, FLOAT, STRING, PUNCT, END } kind;
      /// Text
      std::string text;
      /// Integer value
      long long int i;
      /// Float value
      double f;
      /// Line in the spec
      int line;
      /// Whether no white space precedes the token
      bool glued;
      /// Initialize
      Token(void)
        : kind(Kind::END), i(0), f(0.0), line(0), glued(false) {}
    };

    /**
     * \brief Return the character that the escape \a e stands for
     *
     * The escape is the character after a backslash in a string on line
     * \a line of the spec from \a origin.
     */
    char
    unescape(char e, const std::string& origin, int line) {
      switch (e) {
      case 'n': return '\n';
      case 't': return '\t';
      case '\\': case '"': return e;
      case '\n': fail_at(origin, line, "unterminated string");
      default:
        fail_at(origin, line, std::string("unknown escape '\\") + e + "' in string (use \\n, \\t, \\\\, or \\\")");
      }
    }

    /// Return where the name starting at position \a p of \a src ends
    size_t
    name_end(const std::string& src, size_t p) {
      while ((p < src.size()) && is_alnum(src[p]))
        p++;
      return p;
    }

    /// Prefix of the names of Gecode constants
    const std::string gecode_prefix = "gecode.";

    /// Test whether the name from \a p to \a q in \a src is followed by
    /// a dot and a name that make it the name of a Gecode constant
    bool
    at_gecode_name(const std::string& src, size_t p, size_t q) {
      return (src.compare(p, q-p, "gecode") == 0) && (q+1 < src.size()) && (src[q] == '.') && is_alnum(src[q+1]);
    }

    /// Test whether \a n is the name of a Gecode constant
    bool
    is_gecode_name(const std::string& n) {
      return n.compare(0, gecode_prefix.size(), gecode_prefix) == 0;
    }

    /// Split spec \a src, which starts on line \a line, into tokens
    void
    lex(const std::string& src, const std::string& origin, Seq<Token>& toks, int line = 1) {
      size_t p = 0;
      bool space = true;
      while (p < src.size()) {
        char c = src[p];
        if (c == '\n') { line++; p++; space = true; continue; }
        if (is_space(c)) { p++; space = true; continue; }
        if (c == '#') {
          while ((p < src.size()) && (src[p] != '\n')) p++;
          space = true;
          continue;
        }
        Token t;
        t.line = line;
        t.glued = !space;
        space = false;
        if (c == '"') {
          p++;
          // A string ends on its line: a newline, even after a backslash,
          // leaves it unterminated
          while ((p < src.size()) && (src[p] != '"')) {
            if (src[p] == '\n')
              fail_at(origin, line, "unterminated string");
            if (src[p] == '\\') {
              if (p+1 >= src.size())
                break;
              t.text += unescape(src[p+1], origin, line);
              p += 2;
            } else {
              t.text += src[p++];
            }
          }
          if (p >= src.size())
            fail_at(origin, line, "unterminated string");
          p++;
          t.kind = Token::Kind::STRING;
        } else if (is_digit(c)) {
          // A minus sign before a number is an operator, and a range such
          // as 0..9 is two numbers
          size_t q = number_end(src, p, false, true);
          for (size_t k=p; k+1<q; k++)
            if (src.compare(k, 2, "..") == 0) {
              q = k;
              break;
            }
          t.text = src.substr(p, q-p);
          p = q;
          bool flt = (t.text.find_first_not_of("0123456789") != std::string::npos);
          t.kind = flt ? Token::Kind::FLOAT : Token::Kind::INT;
          if (flt ? !parse_float(t.text, t.f) : !parse_int(t.text, t.i))
            fail_at(origin, line, "bad number " + t.text);
        } else if (is_alnum(c)) {
          size_t q = name_end(src, p);
          // Gecode constants such as gecode.int_max are single names
          if (at_gecode_name(src, p, q))
            q = name_end(src, q+1);
          t.text = src.substr(p, q-p);
          p = q;
          t.kind = Token::Kind::IDENT;
        } else {
          // The longest operator that comes next
          static const char* ops[] = {"..<", "..", "==", "!=", "<=", ">=", "&&", "||"};
          t.kind = Token::Kind::PUNCT;
          t.text = std::string(1, c);
          for (const char* o : ops)
            if ((t.text.size() == 1) && (src.compare(p, std::strlen(o), o) == 0))
              t.text = o;
          if ((t.text.size() == 1) && (std::strchr("{}[]()=<>+-*/%^!,.:", c) == nullptr))
            fail_at(origin, line, std::string("unexpected character '") + c + "'");
          p += t.text.size();
        }
        toks.push(t);
      }
      Token e;
      e.line = line;
      toks.push(e);
    }

    /// Store the base type named \a t in \a r, and return whether there is one
    bool read_type(const std::string& t, ReadType& r) {
      if (t == "int") r = ReadType::INT;
      else if (t == "float") r = ReadType::FLOAT;
      else if (t == "num") r = ReadType::NUM;
      else if (t == "word") r = ReadType::WORD;
      else if (t == "rest") r = ReadType::REST;
      else return false;
      return true;
    }

    /// Test whether \a s is a name: a letter or underscore, then letters,
    /// digits, and underscores
    bool is_name(const std::string& s) {
      if (s.empty() || is_digit(s[0]))
        return false;
      for (char c : s)
        if (!is_alnum(c))
          return false;
      return true;
    }

    /// Add the literal text \a lit, if any, as an item to \a pat and clear it
    void
    flush(Pattern& pat, std::string& lit) {
      if (!lit.empty()) {
        Pattern::Item it(Pattern::Item::Kind::LIT);
        it.text = lit;
        pat.items.push(std::move(it));
        lit.clear();
      }
    }

    /// Test whether pattern item \a it ends with a word character
    bool
    ends_word(const Pattern::Item& it) {
      typedef Pattern::Item::Kind Kind;
      return (it.kind == Kind::CAP) || (it.kind == Kind::VAR) ||
        ((it.kind == Kind::LIT) && is_alnum(it.text[it.text.size()-1]));
    }

    /// Test whether pattern item \a it starts with a word character
    bool
    starts_word(const Pattern::Item& it) {
      typedef Pattern::Item::Kind Kind;
      return (it.kind == Kind::CAP) || (it.kind == Kind::VAR) || ((it.kind == Kind::LIT) && is_alnum(it.text[0]));
    }

    /// Compile line pattern \a src, adding the text of the type of each
    /// capture to \a types
    Pattern
    compile(const std::string& src, Seq<std::string>& types) {
      typedef Pattern::Item Item;
      Pattern pat;
      pat.source = src;
      size_t p = 0;
      std::string lit;
      while (p < src.size()) {
        char c = src[p];
        if (is_blank(c)) {
          flush(pat, lit);
          while ((p < src.size()) && is_blank(src[p])) p++;
          pat.items.push(Item(Item::Kind::SPACE));
        } else if (c == '{') {
          flush(pat, lit);
          size_t q = src.find('}', p);
          if (q == std::string::npos)
            throw Error("unterminated capture in pattern \"" + src + "\"");
          std::string cap = src.substr(p+1, q-p-1);
          p = q+1;
          if (!cap.empty() && (cap[0] == '$')) {
            Item it(Item::Kind::VAR);
            it.text = cap.substr(1);
            if (!is_name(it.text))
              throw Error("missing or bad name in pattern \"" + src + "\"");
            pat.items.push(std::move(it));
            continue;
          }
          Item it(Item::Kind::CAP);
          size_t colon = cap.find(':');
          it.text = cap.substr(0, colon);
          if (!is_name(it.text) && !it.text.empty() && (it.text != "_"))
            throw Error("capture name '" + it.text + "' in pattern \"" + src + "\" is not a name");
          types.push((colon == std::string::npos) ? "word" : cap.substr(colon+1));
          pat.items.push(std::move(it));
        } else if (src.compare(p, 3, "...") == 0) {
          flush(pat, lit);
          if (src.find_first_not_of(" \t", p+3) != std::string::npos)
            throw Error("'...' must end the pattern \"" + src + "\"");
          pat.items.push(Item(Item::Kind::ANY));
          p += 3;
        } else {
          lit += c;
          p++;
        }
      }
      flush(pat, lit);
      // A space is required between two items that would otherwise run
      // together, such as "p cnf" or "{a:int} {b:int}".
      for (int k=1; k+1<pat.items.size(); k++)
        if (pat.items[k].kind == Item::Kind::SPACE)
          pat.items[k].required = ends_word(pat.items[k-1]) && starts_word(pat.items[k+1]);
      return pat;
    }

    /// A name bound by a comprehension, at a line of the spec
    class BoundName {
    public:
      /// The name
      std::string name;
      /// Line in the spec
      int line;
    };

    /// A named type, defined by a type statement
    class TypeName {
    public:
      /// The name
      std::string name;
      /// The type statement
      const Stmt* def;
    };

    /// Parser for specs
    class Parser {
    public:
      /// Initialize for \a src from \a origin
      Parser(const std::string& src, const std::string& o)
        : origin(o), p(0), depth(0), nesting(0), repeats(0), repeat_start(false), each_ok(false) {
        lex(src, origin, toks);
      }
      /// Parse the whole spec into \a s
      void parse(SpecImpl& s) {
        while (peek().kind != Token::Kind::END) {
          if (keyword("format")) {
            next();
            if ((peek().kind != Token::Kind::IDENT) && (peek().kind != Token::Kind::STRING))
              fail("expected a format name, found " + describe(peek()));
            s.name = next().text;
            // Names such as dimacs-cnf: words joined by '-' or '.'
            while (at_name_joiner()) {
              s.name += next().text;
              s.name += next().text;
            }
          } else {
            if (!s.body.empty() && (s.body.back()->kind == Stmt::Kind::INSTANCES))
              fail("'instances' must be the last statement of the spec");
            s.body.push(statement());
          }
        }
        // A bound name that is also a spec name would hide it
        Seq<Decl> names;
        declared(s.body, names);
        for (const BoundName& b : bound_all)
          for (const Decl& d : names)
            if (d.name == b.name)
              fail_at(origin, b.line, "'" + b.name + "' is bound by a comprehension but is also a name in the spec");
        for (const TypeName& t : types_all)
          for (const Decl& d : names)
            if (d.name == t.name)
              fail_at(origin, t.def->line, "'" + t.name + "' names a type, so it cannot also name a value");
        resolve(s.body, origin);
      }
    private:
      /// Where the spec comes from
      std::string origin;
      /// Tokens
      Seq<Token> toks;
      /// Current position
      int p;
      /// Nesting depth of blocks
      int depth;
      /// Nesting depth of expressions
      int nesting;
      /// Number of repeats parsed
      int repeats;
      /// Whether the next statement is the first of the body of a repeat
      bool repeat_start;
      /// Whether the statement being parsed may be each
      bool each_ok;
      /// Names bound by the comprehensions being parsed
      Seq<std::string> bound;
      /// All names bound by comprehensions
      Seq<BoundName> bound_all;
      /// The named types visible to the statement being parsed
      Seq<TypeName> types;
      /// All named types
      Seq<TypeName> types_all;
      /// Maximal nesting of blocks and expressions
      static const int max_nesting = 256;

      const Token& peek(void) const {
        return toks[p];
      }
      const Token& next(void) {
        const Token& t = toks[p];
        if (p+1 < toks.size()) p++;
        return t;
      }
      [[noreturn]] void fail(const std::string& m) const {
        fail_at(origin, peek().line, m);
      }
      static std::string describe(const Token& t) {
        if (t.kind == Token::Kind::END) return "end of spec";
        if (t.kind == Token::Kind::STRING) return "\"" + t.text + "\"";
        return "'" + t.text + "'";
      }
      bool keyword(const char* k) const {
        return (peek().kind == Token::Kind::IDENT) && (peek().text == k);
      }
      bool punct(const char* k) const {
        return (peek().kind == Token::Kind::PUNCT) && (peek().text == k);
      }
      /// Test whether a '-' or '.' joining the words of a format name such as
      /// dimacs-cnf comes next
      bool at_name_joiner(void) const {
        if (!(punct("-") || punct(".")) || !peek().glued || (p+1 >= toks.size()))
          return false;
        const Token& t = toks[p+1];
        return t.glued && ((t.kind == Token::Kind::IDENT) || (t.kind == Token::Kind::INT));
      }
      void expect_punct(const char* k) {
        if (!punct(k))
          fail(std::string("expected '") + k + "', found " + describe(peek()));
        next();
      }
      void expect_keyword(const char* k) {
        if (!keyword(k))
          fail(std::string("expected '") + k + "', found " + describe(peek()));
        next();
      }
      std::string ident(const char* what) {
        if (peek().kind != Token::Kind::IDENT)
          fail(std::string("expected ") + what + ", found " + describe(peek()));
        return next().text;
      }
      /// A name that the spec defines, described by \a what
      std::string new_name(const char* what) {
        std::string n = ident(what);
        if (is_gecode_name(n))
          fail("'" + n + "' cannot be defined: names starting with '" + gecode_prefix + "' are Gecode constants");
        return n;
      }
      std::string string(const char* what) {
        if (peek().kind != Token::Kind::STRING)
          fail(std::string("expected ") + what + " (a string), found " + describe(peek()));
        return next().text;
      }
      Pattern pattern(void) {
        int line = peek().line;
        std::string s = string("a line pattern");
        Pattern pat;
        Seq<std::string> texts;
        try {
          pat = compile(s, texts);
        } catch (const Error& e) {
          fail_at(origin, line, e.what());
        }
        int k = 0;
        for (Pattern::Item& it : pat.items)
          if (it.kind == Pattern::Item::Kind::CAP) {
            it.type = capture_type(texts[k++], line);
            if (it.type.read == ReadType::SYMBOL)
              fail_at(origin, line, "a capture cannot be of an alphabet type");
          }
        return pat;
      }
      /// Return the type \a text of a capture in a pattern on line \a line
      Type capture_type(const std::string& text, int line) {
        // Parse the text in place of the tokens of the spec
        Seq<Token> sub;
        lex(text, origin, sub, line);
        if (sub.size() == 1)
          fail_at(origin, line, "a capture needs a type after ':'");
        std::swap(toks, sub);
        int at = p;
        p = 0;
        Type t = type();
        if (peek().kind != Token::Kind::END)
          fail("unexpected " + describe(peek()) + " in the capture type '" + text + "'");
        std::swap(toks, sub);
        p = at;
        return t;
      }
      /// Increment nesting counter \a n for \a what, failing beyond the limit
      void enter(int& n, const char* what) {
        if (++n > max_nesting)
          fail(std::string(what) + " are nested too deeply");
      }
      Block block(void) {
        expect_punct("{");
        enter(depth, "blocks");
        // The types defined in the block are only visible inside it
        int scope = types.size();
        Block b;
        while (!punct("}")) {
          if (peek().kind == Token::Kind::END)
            fail("missing '}'");
          b.push(statement());
        }
        next();
        while (types.size() > scope)
          types.pop();
        depth--;
        return b;
      }
      std::unique_ptr<Expr> integer(long long int v) {
        std::unique_ptr<Expr> e(new Expr(Expr::Kind::INT));
        e->i = v;
        return e;
      }

      /// Consume keyword \a k if it comes next, and return whether it did
      bool accept(const char* k) {
        if (!keyword(k))
          return false;
        next();
        return true;
      }

      /*
       * Statements: one function per keyword, called after the keyword
       * with a new statement of the keyword's kind
       */

      /// A function completing statement \a s, which starts with \a kw
      typedef void (Parser::*Parse)(Stmt& s, const std::string& kw);

      /// A statement that starts with a keyword
      class Keyword {
      public:
        /// The keyword
        const char* keyword;
        /// Kind of statement
        Stmt::Kind kind;
        /// Function parsing the rest of the statement
        Parse parse;
        /// Whether the statement is a setting, which changes how the whole
        /// input is read, so it is at the top level, where it runs once
        bool setting;
      };

      /// Return the statement that starts with keyword \a k, or null
      static const Keyword* statement_keyword(const std::string& k) {
        static const Keyword statements[] = {
          {"comment",    Stmt::Kind::COMMENT,    &Parser::words,        true},
          {"stop",       Stmt::Kind::STOP,       &Parser::words,        true},
          {"expect",     Stmt::Kind::EXPECT,     &Parser::words,        false},
          {"tokens",     Stmt::Kind::TOKENS,     &Parser::chars,        true},
          {"separators", Stmt::Kind::SEPARATORS, &Parser::chars,        true},
          {"ignore",     Stmt::Kind::TRAILING,   &Parser::trailing,     true},
          {"param",      Stmt::Kind::PARAM,      &Parser::param,        false},
          {"list",       Stmt::Kind::LIST,       &Parser::list,         false},
          {"line",       Stmt::Kind::LINE,       &Parser::line_pattern, false},
          {"find",       Stmt::Kind::FIND,       &Parser::line_pattern, false},
          {"skip",       Stmt::Kind::SKIP,       &Parser::skip,         false},
          {"index",      Stmt::Kind::INDEX,      &Parser::index,        false},
          {"repeat",     Stmt::Kind::REPEAT,     &Parser::repeat,       false},
          {"each",       Stmt::Kind::EACH,       &Parser::each,         false},
          {"instances",  Stmt::Kind::INSTANCES,  &Parser::instances,    false},
          {"sections",   Stmt::Kind::SECTIONS,   &Parser::sections,     false},
          {"if",         Stmt::Kind::IF,         &Parser::conditional,  false},
          {"let",        Stmt::Kind::LET,        &Parser::let,          false},
          {"check",      Stmt::Kind::CHECK,      &Parser::check,        false},
          {"type",       Stmt::Kind::TYPE,       &Parser::type_def,     false}
        };
        for (const Keyword& e : statements)
          if (k == e.keyword)
            return &e;
        return nullptr;
      }

      /**
       * \brief Test whether a declaration starts at token \a k
       *
       * A declaration is a name, any dimensions, and a colon, as in
       * <tt>d[n][n]: int</tt>. Its name can be a keyword.
       */
      bool at_declaration(int k) const {
        if (toks[k].kind != Token::Kind::IDENT)
          return false;
        k++;
        while ((toks[k].kind == Token::Kind::PUNCT) && (toks[k].text == "[")) {
          int open = 0;
          do {
            if (toks[k].kind == Token::Kind::END)
              return false;
            if (toks[k].kind == Token::Kind::PUNCT)
              open += (toks[k].text == "[") ? 1 : ((toks[k].text == "]") ? -1 : 0);
            k++;
          } while (open > 0);
        }
        return (toks[k].kind == Token::Kind::PUNCT) && (toks[k].text == ":");
      }

      std::unique_ptr<Stmt> statement(void) {
        const Token& t = peek();
        each_ok = repeat_start;
        repeat_start = false;
        if (t.kind != Token::Kind::IDENT)
          fail("expected a statement, found " + describe(t));
        if (at_declaration(p)) {
          std::unique_ptr<Stmt> s(new Stmt(Stmt::Kind::READ, t.line));
          declaration(*s);
          return s;
        }
        const Keyword* e = statement_keyword(t.text);
        if (e == nullptr)
          fail("unknown statement '" + t.text + "'");
        if (e->setting && (depth > 0))
          fail("the setting '" + t.text + "' must be at the top level of the spec");
        std::unique_ptr<Stmt> s(new Stmt(e->kind, t.line));
        std::string kw = next().text;
        (this->*e->parse)(*s, kw);
        return s;
      }

      /// comment, stop, and expect: one or more strings
      void words(Stmt& s, const std::string&) {
        s.words.push(string(s.kind == Stmt::Kind::EXPECT ? "a literal token" : "a line prefix"));
        while (peek().kind == Token::Kind::STRING)
          s.words.push(next().text);
      }
      /// tokens and separators: a string of characters
      void chars(Stmt& s, const std::string&) {
        s.chars = string(s.kind == Stmt::Kind::TOKENS ? "single-character tokens" : "separator characters");
      }
      /// ignore trailing
      void trailing(Stmt&, const std::string&) {
        expect_keyword("trailing");
      }
      /// A name for a value that is kept, so not _
      std::string defined(const char* what) {
        std::string n = new_name(what);
        if (n == "_")
          fail("'_' cannot be defined here");
        return n;
      }
      /// param x [= default]
      void param(Stmt& s, const std::string&) {
        s.name = defined("a parameter name");
        if (punct("=")) {
          next();
          s.expr = expr();
        }
      }
      /**
       * \brief Consume option \a k of a statement if it comes next, and
       * return whether it did
       *
       * A declaration of a value with the same name ends the statement.
       */
      bool option(const char* k) {
        return !at_declaration(p) && accept(k);
      }
      /// Test whether keyword \a k can end a range without an upper bound
      static bool ends_range(const std::string& k) {
        return (statement_keyword(k) != nullptr) || (k == "layout") || (k == "offset") || (k == "until");
      }
      /// Test whether the upper bound of a range comes next
      bool at_upper_bound(void) const {
        const Token& t = peek();
        switch (t.kind) {
        case Token::Kind::IDENT:
          return !ends_range(t.text) && !at_declaration(p);
        case Token::Kind::PUNCT:
          return punct("(") || punct("-") || punct("!");
        case Token::Kind::END:
          return false;
        default:
          return true;
        }
      }
      /// Return the type statement of the named type \a n, or null
      const Stmt* named_type(const std::string& n) const {
        for (int k=types.size(); k--; )
          if (types[k].name == n)
            return types[k].def;
        return nullptr;
      }
      /// Test whether \a n is the name of a base type
      static bool is_base_type(const std::string& n) {
        ReadType r;
        return read_type(n, r) || (n == "alphabet");
      }
      /**
       * \brief A type
       *
       * A base type, a named type, an alphabet type, or a range lo..hi or
       * lo..<hi, where one of the bounds can be left out (but not the upper
       * bound of lo..<hi).
       */
      Type type(void) {
        Type t;
        if (peek().kind == Token::Kind::IDENT) {
          const std::string& n = peek().text;
          if (read_type(n, t.read)) {
            next();
            return t;
          }
          if (n == "alphabet") {
            next();
            alphabet(t);
            return t;
          }
          if (const Stmt* d = named_type(n)) {
            next();
            Type u = d->type.use(d);
            u.name = d->name;
            return u;
          }
        }
        if (!punct("..") && !punct("..<")) {
          std::unique_ptr<Expr> lo = expr();
          if (!punct("..") && !punct("..<")) {
            if (lo->kind == Expr::Kind::NAME)
              fail("unknown type '" + lo->s + "'");
            fail("expected '..' or '..<' after the lower bound of a range, found " + describe(peek()));
          }
          t.lo = std::move(lo);
        }
        t.exclusive = punct("..<");
        next();
        if (at_upper_bound())
          t.hi = expr();
        if (t.exclusive && !t.hi)
          fail("'..<' needs an upper bound");
        if (!t.lo && !t.hi)
          fail("a range needs a lower or an upper bound");
        return t;
      }
      /// alphabet "..." [blank "..."] [strip "..."] [base n], after alphabet
      void alphabet(Type& t) {
        t.read = ReadType::SYMBOL;
        t.alphabet = string("an alphabet");
        for (;;) {
          if (option("blank")) {
            t.blank = string("blank symbols");
          } else if (option("strip")) {
            t.strip = string("symbols to strip");
          } else if (option("base")) {
            bool neg = punct("-");
            if (neg)
              next();
            if (peek().kind != Token::Kind::INT)
              fail("expected an integer, found " + describe(peek()));
            t.base = neg ? -next().i : next().i;
          } else {
            break;
          }
        }
      }
      /// Test whether type \a t reads numbers
      static bool numeric(const Type& t) {
        return (t.read == ReadType::INT) || (t.read == ReadType::FLOAT) || (t.read == ReadType::NUM);
      }
      /// Optional offset of a declaration or list statement
      void offset(Stmt& s) {
        if (!option("offset"))
          return;
        if (!numeric(s.type))
          fail("'offset' needs numbers");
        s.offset = expr();
      }
      /// x[d1][d2]...: type [layout e] [offset e]
      void declaration(Stmt& s) {
        s.name = new_name("a name");
        while (punct("[")) {
          next();
          s.dims.push(expr());
          expect_punct("]");
        }
        expect_punct(":");
        s.type = type();
        if ((s.type.read == ReadType::REST) && !s.dims.empty())
          fail("'rest' values cannot be arrays");
        if ((s.type.read == ReadType::SYMBOL) && (s.dims.size() != 2))
          fail("an alphabet type needs a two-dimensional array");
        if (option("layout")) {
          if ((s.dims.size() != 2) || !numeric(s.type))
            fail("'layout' needs a two-dimensional array of numbers");
          s.expr = expr();
        }
        offset(s);
      }
      /// list x: type until (e | eol) [offset e]
      void list(Stmt& s, const std::string&) {
        s.name = new_name("a name");
        expect_punct(":");
        s.type = type();
        if ((s.type.read == ReadType::REST) || (s.type.read == ReadType::SYMBOL))
          fail("a list cannot be of 'rest' or of an alphabet type");
        expect_keyword("until");
        if (accept("eol"))
          s.until_eol = true;
        else
          s.sentinel = expr();
        offset(s);
      }
      /// type t = type
      void type_def(Stmt& s, const std::string&) {
        s.name = defined("a type name");
        if (is_base_type(s.name))
          fail("'" + s.name + "' is a base type");
        if (named_type(s.name) != nullptr)
          fail("the type '" + s.name + "' is already defined");
        expect_punct("=");
        s.type = type();
        TypeName t;
        t.name = s.name;
        t.def = &s;
        types.push(t);
        types_all.push(t);
      }
      /// line and find: a pattern
      void line_pattern(Stmt& s, const std::string&) {
        s.pattern = pattern();
      }
      /// skip line, or skip e lines
      void skip(Stmt& s, const std::string&) {
        if (accept("line")) {
          s.expr = integer(1);
        } else {
          s.expr = expr();
          expect_keyword("lines");
        }
      }
      /// index e
      void index(Stmt& s, const std::string&) {
        s.expr = expr();
      }
      /// Terminating tokens: "a" "b" ... [store x]
      void until(Stmt& s) {
        s.has_until = true;
        s.words.push(string("a terminating token"));
        while (peek().kind == Token::Kind::STRING)
          s.words.push(next().text);
        if (accept("store"))
          s.store = new_name("a name");
      }
      /// repeat (e | until eof | until tokens) [as name] { ... }
      void repeat(Stmt& s, const std::string&) {
        s.id = repeats++;
        if (accept("until")) {
          if (accept("eof"))
            s.until_eof = true;
          else
            until(s);
        } else {
          s.expr = expr();
        }
        if (accept("as"))
          s.list = new_name("a name for the records");
        repeat_start = true;
        s.body = block();
        repeat_start = false;
        if (!s.body.empty() && (s.body[0]->kind == Stmt::Kind::EACH))
          s.each = s.body[0]->name;
      }
      /// each x, first in the body of a repeat
      void each(Stmt& s, const std::string&) {
        if (!each_ok)
          fail("'each' must be the first statement of a repeat, and comes at most once");
        s.name = new_name("a name for each record");
      }
      /// instances { ... }, at the top level only
      void instances(Stmt& s, const std::string&) {
        if (depth > 0)
          fail("'instances' must be at the top level of the spec");
        s.body = block();
      }
      /// sections [until tokens] [skip other] { "pattern" [optional] [last] [{...}] ... }
      void sections(Stmt& s, const std::string&) {
        for (;;) {
          if (accept("until")) {
            until(s);
          } else if (accept("skip")) {
            expect_keyword("other");
            s.skip_other = true;
          } else {
            break;
          }
        }
        expect_punct("{");
        while (!punct("}")) {
          if (peek().kind == Token::Kind::END)
            fail("missing '}'");
          Arm a;
          a.pattern = pattern();
          a.optional = accept("optional");
          a.last = accept("last");
          if (punct("{"))
            a.body = block();
          s.arms.push(std::move(a));
        }
        next();
      }
      /// if e { ... } [else (if ... | { ... })]
      void conditional(Stmt& s, const std::string&) {
        s.expr = expr();
        s.body = block();
        if (accept("else")) {
          if (keyword("if")) {
            // A chain of else if nests like blocks
            enter(depth, "blocks");
            s.orelse.push(statement());
            depth--;
          } else {
            s.orelse = block();
          }
        }
      }
      /// let x = e
      void let(Stmt& s, const std::string&) {
        s.name = defined("a name");
        expect_punct("=");
        s.expr = expr();
      }
      /// check e ["message"]
      void check(Stmt& s, const std::string&) {
        s.expr = expr();
        if (peek().kind == Token::Kind::STRING)
          s.message = next().text;
      }
      /*
       * Expressions, by precedence climbing
       */
      std::unique_ptr<Expr> expr(void) {
        return binary(0);
      }
      /// Store the binary operator \a t in \a op, and return its precedence
      /// (-1 if \a t is none)
      static int binary_op(const Token& t, Expr::Op& op) {
        static const struct {
          const char* text; Expr::Op op; int precedence;
        } ops[] = {
          {"||", Expr::Op::OR, 1}, {"&&", Expr::Op::AND, 2},
          {"==", Expr::Op::EQ, 3}, {"!=", Expr::Op::NE, 3},
          {"<", Expr::Op::LT, 4}, {">", Expr::Op::GT, 4},
          {"<=", Expr::Op::LE, 4}, {">=", Expr::Op::GE, 4},
          {"+", Expr::Op::ADD, 5}, {"-", Expr::Op::SUB, 5},
          {"*", Expr::Op::MUL, 6}, {"/", Expr::Op::DIV, 6}, {"%", Expr::Op::MOD, 6}
        };
        if (t.kind == Token::Kind::PUNCT)
          for (const auto& o : ops)
            if (t.text == o.text) {
              op = o.op;
              return o.precedence;
            }
        return -1;
      }
      /// Binary operators of at least precedence \a min, left-associative
      std::unique_ptr<Expr> binary(int min) {
        std::unique_ptr<Expr> lhs = unary();
        // Each operator adds a level above lhs
        int levels = 0;
        for (;;) {
          Expr::Op op;
          int prec = binary_op(peek(), op);
          if ((prec < 0) || (prec < min))
            break;
          enter(nesting, "expressions");
          levels++;
          std::unique_ptr<Expr> e(new Expr(Expr::Kind::BINARY));
          e->op = op;
          e->s = next().text;
          e->args.push(std::move(lhs));
          e->args.push(binary(prec+1));
          lhs = std::move(e);
        }
        nesting -= levels;
        return lhs;
      }
      std::unique_ptr<Expr> unary(void) {
        enter(nesting, "expressions");
        std::unique_ptr<Expr> e;
        if (punct("-") || punct("!")) {
          e.reset(new Expr(Expr::Kind::UNARY));
          e->op = punct("-") ? Expr::Op::NEG : Expr::Op::NOT;
          e->s = next().text;
          e->args.push(unary());
        } else {
          e = power();
        }
        nesting--;
        return e;
      }
      /// a ^ b, which binds tighter than unary operators and groups to the
      /// right; b is a unary expression, so 2^-1 is 2^(-1)
      std::unique_ptr<Expr> power(void) {
        std::unique_ptr<Expr> e = postfix();
        if (!punct("^"))
          return e;
        std::unique_ptr<Expr> x(new Expr(Expr::Kind::BINARY));
        x->op = Expr::Op::POW;
        x->s = next().text;
        x->args.push(std::move(e));
        x->args.push(unary());
        return x;
      }
      /// Indices and fields a[i][j].f...
      std::unique_ptr<Expr> postfix(void) {
        std::unique_ptr<Expr> e = primary();
        // Each index or field adds a level above e
        int levels = 0;
        while (punct("[") || punct(".")) {
          bool field = punct(".");
          next();
          enter(nesting, "expressions");
          levels++;
          std::unique_ptr<Expr> x(new Expr(field ? Expr::Kind::FIELD : Expr::Kind::INDEX));
          x->args.push(std::move(e));
          if (field) {
            x->s = ident("a field name");
          } else {
            x->args.push(expr());
            expect_punct("]");
          }
          e = std::move(x);
        }
        nesting -= levels;
        return e;
      }

      /// Check that call \a e on spec line \a line names a function with
      /// suitable arguments
      void check_call(const Expr& e, int line) {
        const std::string& f = e.s;
        int n = e.args.size();
        bool ok;
        if (f == "defined")
          ok = (n == 1) && (e.args[0]->kind == Expr::Kind::NAME);
        else if ((f == "count") || (f == "size"))
          ok = (n == 1);
        else if ((f == "min") || (f == "max") || (f == "sum") || (f == "distinct"))
          ok = (n >= 1);
        else if ((f == "all") || (f == "any"))
          fail_at(origin, line, "'" + f + "' needs the form " + f + "(x in a: e)");
        else
          fail_at(origin, line, "unknown function '" + f + "'");
        if (!ok)
          fail_at(origin, line, "wrong arguments for function '" + f + "'");
      }
      /// Return the value of the Gecode constant \a n on spec line \a line
      long long int gecode_constant(const std::string& n, int line) {
        static const struct {
          const char* name; long long int value;
        } constants[] = {
          {"gecode.int_max", Gecode::Int::Limits::max},
          {"gecode.int_min", Gecode::Int::Limits::min}
        };
        std::string known;
        for (const auto& c : constants) {
          if (n == c.name)
            return c.value;
          known += (known.empty() ? "" : ", ") + std::string(c.name);
        }
        fail_at(origin, line, "unknown Gecode constant '" + n + "' (known: " + known + ")");
      }
      /// Test whether a comprehension for function \a f comes next, after "f("
      bool at_comprehension(const std::string& f) const {
        if ((f != "all") && (f != "any") && (f != "sum") && (f != "count"))
          return false;
        if ((peek().kind != Token::Kind::IDENT) || (p+1 >= toks.size()))
          return false;
        const Token& t = toks[p+1];
        return (t.kind == Token::Kind::IDENT) && (t.text == "in");
      }
      /// f(x in a: e), after "f("
      std::unique_ptr<Expr> comprehension(const std::string& f) {
        std::unique_ptr<Expr> e(new Expr(Expr::Kind::COMPREHENSION));
        e->s = f;
        BoundName b;
        b.line = peek().line;
        b.name = new_name("a name");
        if (b.name == "_")
          fail("'_' cannot be bound");
        for (const std::string& o : bound)
          if (o == b.name)
            fail("'" + b.name + "' is already bound by an enclosing comprehension");
        e->var = b.name;
        expect_keyword("in");
        // The name is bound in the body only
        e->args.push(expr());
        expect_punct(":");
        bound.push(b.name);
        e->args.push(expr());
        bound.pop();
        expect_punct(")");
        bound_all.push(b);
        return e;
      }
      std::unique_ptr<Expr> primary(void) {
        const Token& t = peek();
        if (t.kind == Token::Kind::INT)
          return integer(next().i);
        if (t.kind == Token::Kind::FLOAT) {
          std::unique_ptr<Expr> e(new Expr(Expr::Kind::FLOAT));
          e->f = next().f;
          return e;
        }
        if (t.kind == Token::Kind::STRING) {
          std::unique_ptr<Expr> e(new Expr(Expr::Kind::STR));
          e->s = next().text;
          return e;
        }
        if (punct("(")) {
          next();
          std::unique_ptr<Expr> e = expr();
          expect_punct(")");
          return e;
        }
        if ((t.kind == Token::Kind::IDENT) && is_gecode_name(t.text)) {
          int line = t.line;
          return integer(gecode_constant(next().text, line));
        }
        if (t.kind == Token::Kind::IDENT) {
          int line = t.line;
          std::string n = next().text;
          if (punct("(")) {
            next();
            if (at_comprehension(n))
              return comprehension(n);
            std::unique_ptr<Expr> e(new Expr(Expr::Kind::CALL));
            e->s = n;
            if (!punct(")")) {
              e->args.push(expr());
              while (punct(",")) {
                next();
                e->args.push(expr());
              }
            }
            expect_punct(")");
            check_call(*e, line);
            return e;
          }
          std::unique_ptr<Expr> e(new Expr(Expr::Kind::NAME));
          e->s = n;
          return e;
        }
        fail("expected an expression, found " + describe(t));
      }
    };

    /// Return the type of the values that \a t reads
    ValueType value_type(ReadType t) {
      switch (t) {
      case ReadType::FLOAT: return ValueType::FLOAT;
      case ReadType::WORD: case ReadType::REST: return ValueType::STRING;
      default: return ValueType::INT;
      }
    }

    /// Add name \a n with type \a t and depth \a d to \a out
    void declare(Seq<Decl>& out, const std::string& n, ValueType t, int d) {
      if (n.empty() || (n == "_"))
        return;
      for (const Decl& o : out)
        if (o.name == n)
          return;
      Decl x;
      x.name = n; x.type = t; x.depth = d;
      out.push(x);
    }

    /// Add the captures of \a p to \a out
    void declare(Seq<Decl>& out, const Pattern& p) {
      for (const Pattern::Item& it : p.items)
        if (it.kind == Pattern::Item::Kind::CAP)
          declare(out, it.text, value_type(it.type.read), 0);
    }

  }

  void
  declared(const Block& b, Seq<Decl>& out) {
    for (const std::unique_ptr<Stmt>& s : b) {
      switch (s->kind) {
      case Stmt::Kind::READ:
        declare(out, s->name, value_type(s->type.read), s->dims.size());
        break;
      case Stmt::Kind::LIST:
        declare(out, s->name, value_type(s->type.read), 1);
        break;
      case Stmt::Kind::LET: case Stmt::Kind::PARAM:
        // Usually a single value; the type is only known when running
        declare(out, s->name, ValueType::INT, 0);
        break;
      case Stmt::Kind::LINE: case Stmt::Kind::FIND:
        declare(out, s->pattern);
        break;
      case Stmt::Kind::REPEAT: {
        // Values of a nested repeat are one level deeper
        declare(out, s->store, ValueType::STRING, 0);
        declare(out, record_starts(s->id), ValueType::INT, 1);
        Seq<Decl> inner;
        declared(s->body, inner);
        for (const Decl& d : inner)
          declare(out, d.name, d.type, d.depth+1);
        break;
      }
      case Stmt::Kind::SECTIONS:
        declare(out, s->store, ValueType::STRING, 0);
        for (const Arm& a : s->arms) {
          declare(out, a.pattern);
          declared(a.body, out);
        }
        break;
      case Stmt::Kind::IF:
        declared(s->body, out);
        declared(s->orelse, out);
        break;
      case Stmt::Kind::INSTANCES:
        declared(s->body, out);
        break;
      default:
        break;
      }
    }
  }

  SpecImpl::SpecImpl(const std::string& t, const std::string& o)
    : origin(o) {
    Parser p(t, o);
    p.parse(*this);
  }

  /*
   * Specs
   *
   */

  Spec::Spec(void)
    : impl(std::make_shared<SpecImpl>("", "<empty>")) {}

  Spec
  Spec::parse(const std::string& text, const std::string& origin) {
    Spec s;
    s.impl = std::make_shared<SpecImpl>(text, origin);
    return s;
  }

  Spec
  Spec::load(const std::string& path) {
    return parse(read_file(path), path);
  }

  std::string
  Spec::name(void) const {
    return impl->name;
  }

  Data
  Spec::read(const std::string& path, const Params& p) const {
    return impl->read(read_file(path), path, p, false);
  }

  Data
  Spec::read_text(const std::string& text, const std::string& origin, const Params& p) const {
    return impl->read(text, origin, p, false);
  }

  Data
  Spec::catalog(const std::string& path, const Params& p) const {
    return impl->read(read_file(path), path, p, true);
  }

  Data
  Spec::catalog_text(const std::string& text, const std::string& origin, const Params& p) const {
    return impl->read(text, origin, p, true);
  }

}}

// STATISTICS: instance-any
