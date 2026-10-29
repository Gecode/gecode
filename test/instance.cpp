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

#include <gecode/instance.hh>
#include <gecode/driver.hh>

#include "test/test.hh"

#include <chrono>
#include <climits>
#include <functional>
#include <sstream>

namespace Test { namespace Instance {

  using Gecode::IntArgs;
  using Gecode::Instance::Data;
  using Gecode::Instance::Error;
  using Gecode::Instance::Params;
  using Gecode::Instance::Spec;

  /**
   * \defgroup TaskTestInstance Instance reader tests
   * \ingroup TaskTest
   */

  /// Read \a text with the spec \a spec
  Data
  read(const char* spec, const char* text, const Params& p = Params()) {
    return Spec::parse(spec, "test.reader").read_text(text, "test.txt", p);
  }

  /// Check that \a f throws an Error whose message contains \a what
  bool
  fails(const std::function<void(void)>& f, const char* what) {
    try {
      f();
    } catch (const Error& e) {
      if (std::string(e.what()).find(what) != std::string::npos)
        return true;
      olog << "unexpected error message:\n" << e.what() << "\n";
      return false;
    }
    olog << "expected an error containing '" << what << "'\n";
    return false;
  }

  /// Check that reading \a text with \a spec fails with \a what
  bool
  fails(const char* spec, const char* text, const char* what) {
    return fails([&](void) { (void) read(spec, text); }, what);
  }

  /// Check that parsing \a spec fails with \a what
  bool
  rejects(const char* spec, const char* what) {
    return fails([&](void) { (void) Spec::parse(spec, "s"); }, what);
  }

  /// Compare integer array \a a with the values \a v
  bool
  equal(const IntArgs& a, std::initializer_list<int> v) {
    if (a.size() != static_cast<int>(v.size()))
      return false;
    int i = 0;
    for (int x : v)
      if (a[i++] != x)
        return false;
    return true;
  }

#define CHECK(c)                                                       \
  if (!(c)) {                                                          \
    olog << "check failed at line " << __LINE__ << ": " #c "\n";       \
    return false;                                                      \
  }

  /// %Test given by a function
  class Case : public Base {
  protected:
    /// The test
    std::function<bool(void)> f;
  public:
    /// Create test \a n running \a f0
    Case(const std::string& n, std::function<bool(void)> f0)
      : Base("Instance::" + n), f(f0) {}
    /// Run the test
    virtual bool run(void) {
      try {
        return f();
      } catch (const Error& e) {
        olog << "unexpected error:\n" << e.what() << "\n";
        return false;
      }
    }
  };

  Case scalars("Read::Scalars", [](void) {
    Data d = read("a: int  b: float  c: num  e: num  w: word  r: rest", "  -7 2.5\n 3 4.5e1 hello  the rest  \n");
    CHECK(d.integer("a") == -7);
    CHECK(d.number("b") == 2.5);
    CHECK(d.integer("c") == 3);
    CHECK(d.type("e") == Gecode::Instance::ValueType::FLOAT);
    CHECK(d.number("e") == 45.0);
    CHECK(d.text("w") == "hello");
    CHECK(d.text("r") == "the rest");
    CHECK(d.entries() == 6);
    CHECK(d.name(0) == "a");
    return true;
  });

  Case arrays("Read::Arrays", [](void) {
    Data d = read("n: int  m: int  a[n][m]: int  _: int  f[2]: float offset 1", "2 3\n1 2\n3 4 5 6\n99 1.5 2");
    CHECK(d.depth("a") == 2);
    CHECK(d.size("a") == 2);
    CHECK(d.size("a", {1}) == 3);
    CHECK(equal(d.ints("a"), {1,2,3,4,5,6}));
    CHECK(equal(d.ints("a", {1}), {4,5,6}));
    CHECK(d.integer("a", {1,2}) == 6);
    CHECK(!d.has("_"));
    CHECK(d.numbers("f")[0] == 2.5);
    CHECK(d.numbers("f")[1] == 3.0);
    return true;
  });

  Case layouts("Read::Layouts", [](void) {
    // The symmetric matrix 0 1 2 / 1 0 3 / 2 3 0 in all layouts
    const char* spec = "f: word  w[3][3]: int layout f";
    const char* inputs[] = {
      "FULL_MATRIX 0 1 2 1 0 3 2 3 0",
      "UPPER_ROW 1 2 3", "LOWER_COL 1 2 3",
      "LOWER_ROW 1 2 3", "UPPER_COL 1 2 3",
      "UPPER_DIAG_ROW 0 1 2 0 3 0", "LOWER_DIAG_COL 0 1 2 0 3 0",
      "LOWER_DIAG_ROW 0 1 0 2 3 0", "UPPER_DIAG_COL 0 1 0 2 3 0",
    };
    for (const char* in : inputs) {
      Data d = read(spec, in);
      if (!equal(d.ints("w"), {0,1,2,1,0,3,2,3,0})) {
        olog << "wrong matrix for " << in << "\n";
        return false;
      }
    }
    // An offset applies to the numbers read, not to the mirrored zeros
    CHECK(equal(read("w[3][3]: int layout \"UPPER_ROW\" offset 1", "1 2 3").ints("w"), {0,2,3,2,0,4,3,4,0}));
    return fails(spec, "DIAGONAL 1 2 3", "unknown matrix layout");
  });

  Case lists("Read::Lists", [](void) {
    Data d = read("list a: int until 0  list b: int until eol  list c: int until eol  list w: word until \"END\"",
                  "1 2\n 3 0 4 5\n6 7\nx y END");
    CHECK(equal(d.ints("a"), {1,2,3}));
    CHECK(equal(d.ints("b"), {4,5}));
    CHECK(equal(d.ints("c"), {6,7}));
    CHECK(d.size("w") == 2);
    CHECK(d.text("w", {1}) == "y");
    return true;
  });

  Case patterns("Read::Patterns", [](void) {
    Data d = read("line \"# size={n:int} box={w:int}x{h:int}\"\n"
                  "find \"NAME : {name:rest}\"\n"
                  "line \"p {_} {v:int} {c:num} ...\"",
                  "# size=6 box=3x2\nnoise\nNAME:  a b c \n"
                  "p cnf 10 2.5 trailing words\n");
    CHECK(d.integer("n") == 6);
    CHECK(d.integer("w") == 3);
    CHECK(d.integer("h") == 2);
    CHECK(d.text("name") == "a b c");
    CHECK(d.integer("v") == 10);
    CHECK(d.number("c") == 2.5);
    // A space between two words is required
    CHECK(fails("line \"p cnf {v:int}\"", "pcnf 3", "expected a line"));
    return true;
  });

  Case params("Read::Params", [](void) {
    const char* spec = "param which  param k = 2\n"
      "find \"instance {$which}\"  x[k]: int  ignore trailing";
    const char* text = "instance a\n1 2\ninstance b\n3 4\n";
    Params p;
    p.set("which=b");
    Data d = read(spec, text, p);
    CHECK(equal(d.ints("x"), {3,4}));
    CHECK(d.text("which") == "b");
    Params q;
    q.set("which", "a");
    q.set("k", "1");
    CHECK(equal(read(spec, text, q).ints("x"), {1}));
    Params r;
    r.set("which=a,,k=1,");
    CHECK(equal(read(spec, text, r).ints("x"), {1}));
    CHECK(fails([](void) { Params e; e.set("which=a,k"); }, "parameter 'k' must have the form name=value"));
    CHECK(fails([](void) { Params e; e.set("=a"); }, "parameter '=a' must have the form name=value"));
    return fails(spec, text, "parameter 'which' must be given");
  });

  Case repeats("Read::Repeat", [](void) {
    Data d = read("n: int\n"
                  "repeat n { index 1  k: int  s[k]: int offset -1 }\n"
                  "repeat until \"999\" { a: int }\n"
                  "repeat until \">=\" \"=\" store op { c: int  v: word }\n"
                  "rhs: int\n"
                  "repeat until eof { w: word }",
                  "3\n1 2 4 5\n2 0\n3 1 1\n7 8 999\n"
                  "2 x 3 y = 4\nfoo bar");
    CHECK(equal(d.ints("k"), {2,0,1}));
    CHECK(d.size("s") == 3);
    CHECK(d.size("s", {1}) == 0);
    CHECK(equal(d.ints("s"), {3,4,0}));
    CHECK(equal(d.ints("s", {2}), {0}));
    CHECK(equal(d.ints("a"), {7,8}));
    CHECK(d.text("op") == "=");
    CHECK(equal(d.ints("c"), {2,3}));
    CHECK(d.integer("rhs") == 4);
    CHECK(d.size("w") == 2);
    CHECK(fails("n: int repeat n { index 1 x: int }", "2\n1 5\n3 6", "expected index 2, found 3"));
    // Nested repeats give nested arrays
    Data e = read("repeat 2 { repeat 2 { x: int  y: int } }", "1 2 3 4 5 6 7 8");
    CHECK(e.depth("x") == 2);
    CHECK(equal(e.ints("x", {1}), {5,7}));
    CHECK(equal(e.ints("y"), {2,4,6,8}));
    return true;
  });

  Case sections("Read::Sections", [](void) {
    const char* spec = "sections until \"EOF\" {\n"
      "  \"N : {n:int}\"\n"
      "  \"DATA\" { x[n]: int }\n"
      "  \"NAME : {name:word}\" optional\n"
      "}\n";
    Data d = read(spec, "NAME : a\nN: 2\nDATA\n5 6\nEOF\n");
    CHECK(d.integer("n") == 2);
    CHECK(equal(d.ints("x"), {5,6}));
    CHECK(d.text("name") == "a");
    // No terminator needed at end of input
    CHECK(read(spec, "N : 1\nDATA\n4").ints("x").size() == 1);
    CHECK(fails(spec, "N : 1\nXYZZY_PLUGH\n", "expected one of"));
    CHECK(read("sections skip other { \"v: {v:int}\" }", "junk\nv: 3\nmore junk\n").integer("v") == 3);
    // An arm that is not optional matches exactly one line
    CHECK(fails(spec, "N : 1\nEOF\n", "test.txt:2:1: expected a line matching \"DATA\" before 'EOF'"));
    CHECK(fails(spec, "NAME : a\n", "expected a line matching \"N : {n:int}\" before the end of the input"));
    CHECK(fails(spec, "N : 1\nDATA\n4\nN : 2\n",
                "test.txt:4:1: a second line matching \"N : {n:int}\" (the first is on line 1)"));
    CHECK(fails(spec, "NAME : a\nN : 1\nDATA\n4\nNAME : b\n", "a second line matching \"NAME : {name:word}\""));
    // A last arm ends the statement, and may be optional
    const char* last = "sections { \"a {a:int}\"  \"end\" last }  y: int";
    CHECK(read(last, "a 1\nend\n5").integer("y") == 5);
    CHECK(fails(last, "a 1\n5", "expected one of"));
    CHECK(fails(last, "a 1", "expected a line matching \"end\" before the end of the input"));
    CHECK(fails(last, "end\n5",
                "test.txt:1:1: expected a line matching \"a {a:int}\" before the line matching \"end\""));
    const char* optional_last = "sections { \"a {a:int}\"  \"end\" optional last }";
    CHECK(read(optional_last, "a 1").integer("a") == 1);
    CHECK(read(optional_last, "a 1\nend").integer("a") == 1);
    return true;
  });

  Case alphabets("Read::Alphabets", [](void) {
    Data d = read("g[2][3]: alphabet \"123ABC\" blank \".\"", "1.A\nC 2 .");
    CHECK(equal(d.ints("g"), {1,0,4,6,2,0}));
    Data e = read("g[2][2]: alphabet \"ab\" base 0 strip \"|-\"", "a|b\n---\nb|a");
    CHECK(equal(e.ints("g"), {0,1,1,0}));
    // Alphabet options do not take the statements that follow
    CHECK(read("g[1][2]: alphabet \"ab\"  ignore trailing", "ab x").size("g") == 1);
    CHECK(read("g[1][1]: alphabet \"ab\"  base: int", "b 7").integer("base") == 7);
    CHECK(fails("g[1][2]: alphabet \"ab\"", "ax", "unexpected symbol 'x', expected one of 'ab'"));
    CHECK(fails("g[2][2]: alphabet \"ab\"", "ab\na", "expected a symbol for row 2, column 2"));
    CHECK(rejects("g[2]: alphabet \"ab\"", "an alphabet type needs a two-dimensional array"));
    CHECK(rejects("g[2][2]: alphabet \"ab\" offset 1", "'offset' needs numbers"));
    CHECK(rejects("list g: alphabet \"ab\" until eol", "a list cannot be of 'rest' or of an alphabet type"));
    CHECK(rejects("line \"{g:alphabet \\\"ab\\\"}\"", "a capture cannot be of an alphabet type"));
    return true;
  });

  Case ranges("Read::Ranges", [](void) {
    Data d = read("n: 1..  a[n]: 0..<n  b: ..0  c: -2..-1", "2 0 1 -5 -2");
    CHECK(equal(d.ints("a"), {0,1}));
    CHECK(d.integer("b") == -5);
    CHECK(fails("n: 1..  a[n]: 0..<n", "2 0 2", "test.txt:1:5: expected an int in 0..1, found '2'"));
    CHECK(fails("a: 0..9", "x", "expected an int in 0..9, found 'x'"));
    CHECK(fails("a: 0..", "-1", "expected an int in 0.., found '-1'"));
    CHECK(fails("a: ..<0", "0", "expected an int in ..-1, found '0'"));
    // Ranges are checked before the offset, and the terminator need not be
    // in the range
    CHECK(equal(read("a[2]: 1..2 offset -1  list l: 0..9 until -1", "1 2 3 -1").ints("a"), {0,1}));
    CHECK(fails("list l: 0..9 until -1", "3 10 -1", "found '10'"));
    CHECK(fails("w[3][3]: 0..1 layout \"UPPER_ROW\"", "1 2 1", "found '2'"));
    // Captures are checked after the line matched, so that their ranges
    // can use the captures before them
    Data l = read("line \"{n:int} {m:0..n}\"", "3 2");
    CHECK(l.integer("m") == 2);
    CHECK(fails("line \"{n:int} {m:0..n}\"", "3 4", "test.txt:1:3: expected an int in 0..3, found '4'"));
    CHECK(fails("a: 0..\"x\"", "1", "the bounds of a range must be integers"));
    CHECK(fails("a: 0..<-9223372036854775807-1", "1", "integer overflow"));
    return true;
  });

  Case types("Spec::Types", [](void) {
    Data d = read("type time = 0..gecode.int_max / 2\n"
                  "n: 1..\n"
                  "repeat n { ready: time  due: time }\n"
                  "type cell = alphabet \"ab\" blank \".\"\n"
                  "g[1][2]: cell",
                  "2 1 2 3 4 a.");
    CHECK(equal(d.ints("due"), {2,4}));
    CHECK(equal(d.ints("g"), {1,0}));
    CHECK(fails("n: int  type small = 0..n  a: small", "3 4", "expected small (an int in 0..3), found '4'"));
    CHECK(fails("type w = word  type v = w  line \"{x:v} {y:0..1}\"", "a 2", "expected an int in 0..1"));
    // The range is evaluated where the type is defined
    CHECK(fails("n: int  type t = 0..n  let n2 = n  repeat 2 { b: t }", "1 1 2", "expected t (an int in 0..1)"));
    // Names can be keywords when they are declared
    Data k = read("line: int  type: 0..  list: word  each: int  layout: int", "1 2 a 3 4");
    CHECK(k.integer("type") == 2);
    CHECK(k.integer("layout") == 4);
    // A range ends before a statement or a declaration
    CHECK(read("a: 0..\nb[2]: int  check a == 0", "0 1 2").size("b") == 2);
    CHECK(read("a: 0..  index: int", "0 1").integer("index") == 1);
    CHECK(rejects("a: 0..<", "'..<' needs an upper bound"));
    CHECK(rejects("a: ..", "a range needs a lower or an upper bound"));
    CHECK(rejects("a: foo", "unknown type 'foo'"));
    CHECK(rejects("a: 1 + 2", "expected '..' or '..<' after the lower bound of a range"));
    CHECK(rejects("type int = 0..", "'int' is a base type"));
    CHECK(rejects("type t = int  type t = word", "the type 't' is already defined"));
    CHECK(rejects("type t = int  t: int", "s:1: 't' names a type, so it cannot also name a value"));
    CHECK(rejects("if 1 > 0 { type t = int }  a: t", "unknown type 't'"));
    CHECK(rejects("a: word offset 1", "'offset' needs numbers"));
    CHECK(rejects("a[2]: int layout \"FULL_MATRIX\"", "'layout' needs a two-dimensional array of numbers"));
    CHECK(rejects("line \"{x:0..<}\"", "s:1: '..<' needs an upper bound"));
    CHECK(rejects("line \"{x:int int}\"", "unexpected 'int' in the capture type 'int int'"));
    CHECK(rejects("line \"{x:}\"", "a capture needs a type after ':'"));
    return true;
  });

  Case control("Read::Control", [](void) {
    Data d = read("n: int\n"
                  "if n > 1 { w: word } else { let w = \"none\" }\n"
                  "let m = n * 3 + 1\n"
                  "a[2]: int\n"
                  "let s = sum(a)\n"
                  "check max(a) - min(a) == 2 \"spread\"\n"
                  "check count(a) == 2 && defined(m) && !defined(zz)\n"
                  "check 7 / 2 == 3 && 7 % 2 == 1",
                  "2 x 1 3");
    CHECK(d.text("w") == "x");
    CHECK(d.integer("m") == 7);
    CHECK(d.integer("s") == 4);
    CHECK(read("n: int if n > 1 { w: word } else { let w = \"none\" }", "1").text("w") == "none");
    CHECK(fails("a: int check a == 2 \"must be two\"", "3", "check failed: must be two"));
    CHECK(fails("a: int let b = a / 0", "3", "division by zero"));
    CHECK(fails("a: int let b = c", "3", "'c' is not defined yet"));
    return true;
  });

  Case lexical("Read::Lexical", [](void) {
    Data d = read("comment \"c\" \"#\"\n"
                  "stop \"%\"\n"
                  "tokens \";\"\n"
                  "expect \"p\" \"cnf\"\n"
                  "repeat until eof { w: word  expect \";\" }",
                  "c comment\r\n# another\r\np cnf\r\na; b ;\r\nc x\r\n%\r\n0\r\n");
    CHECK(d.size("w") == 2);
    CHECK(d.text("w", {1}) == "b");
    CHECK(fails("a: int", "1 2", "unexpected input after the end"));
    CHECK(read("a: int ignore trailing", "1 2").integer("a") == 1);
    return true;
  });

  Case robustness("Read::Robustness", [](void) {
    // Separators, and a string terminator for a list of numbers
    Data d = read("separators \",\"  list a: int until eol  list b: int until \"EOF\"", "1,2, 3\n4 5 EOF");
    CHECK(equal(d.ints("a"), {1,2,3}));
    CHECK(equal(d.ints("b"), {4,5}));
    // Names with dashes only for formats; '-' is otherwise subtraction
    Spec f = Spec::parse("format dimacs-cnf.v2  n: int  x: int  let k = n-x", "s");
    CHECK(f.name() == "dimacs-cnf.v2");
    CHECK(f.read_text("5 2").integer("k") == 3);
    // Overflow and non-decimal numbers are errors
    CHECK(fails("a: int  let b = a * a", "9000000000", "integer overflow"));
    CHECK(fails("a: int  b: int offset 1", "1 9223372036854775807", "integer overflow"));
    CHECK(fails("f: float", "nan", "expected float"));
    // Signs come first or after an exponent, in input, captures, and specs
    Data n = read("line \"{a:float}x{b:int}-{c:num}\"  f: float\n"
                  "check 1.5e+1 == 15 && 5-2 == 3",
                  "-1.5e+2x+3-2.5\n+2.5E-1");
    CHECK(n.number("a") == -150.0);
    CHECK(n.integer("b") == 3);
    CHECK(n.number("c") == 2.5);
    CHECK(n.number("f") == 0.25);
    CHECK(fails("f: float", "1-2", "expected float"));
    CHECK(fails("f: num", "0x10", "expected a number"));
    CHECK(fails("n: int  a[n]: int", "2147483648", "too large"));
    // Integers in lists and arrays, which are read without making strings
    CHECK(equal(read("list a: int until 0", "+5 -7 05 -0").ints("a"), {5,-7,5}));
    CHECK(equal(read("tokens \";\"  a[3]: int  expect \";\"", "1 -2 3;").ints("a"), {1,-2,3}));
    CHECK(fails("list a: int until 0", "9223372036854775808 0", "expected int, found '9223372036854775808'"));
    CHECK(fails("list a: int until 0", "-9223372036854775809 0", "found '-9223372036854775809'"));
    CHECK(fails("list a: int until 0", "5x 0", "expected int, found '5x'"));
    CHECK(fails("list a: int until 0", "1 - 0", "expected int, found '-'"));
    CHECK(fails("a[2]: int", "1", "expected int, found end of input"));
    CHECK(fails([](void) { (void) Spec::parse("line \"a ... b\"", "s"); }, "'...' must end the pattern"));
    CHECK(fails("repeat 2 { k: int  if k > 0 { x: int } }", "1 5 0", "'x' is defined in only some repetitions"));
    // A wrong count fails at the end of the input, without allocating
    CHECK(fails("n: int  a[n][1]: int", "1000000000 1 2", "found end of input"));
    CHECK(fails("n: int  g[n][1]: alphabet \"ab\"", "1000000000 ab", "expected a symbol for row 3, column 1"));
    CHECK(fails("r: int  c: int  x[r][c]: int", "2000000000 0", "empty rows without reading input"));
    CHECK(fails("r: int  g[r][0]: alphabet \"ab\"", "2000000000", "empty rows without reading input"));
    CHECK(read("r: int  c: int  x[r][c]: int", "3 0").size("x") == 3);
    return true;
  });

  Case instances("Read::Instances", [](void) {
    const char* spec = "comment \"+++\"\n"
      "instances {\n"
      "  sections skip other {\n"
      "    \"instance {name:word}\" optional\n"
      "    \"{n:int}\" last { x[n]: int }\n"
      "  }\n"
      "}\n";
    const char* text = "header text\ninstance a\n2\n1 2\n+++\n"
      "instance b\ndescription\n1\n7\n+++ end\n";
    Params by_name, by_number, bad;
    by_name.set("instance", "b");
    by_number.set("instance", "1");
    bad.set("instance", "3");
    CHECK(equal(read(spec, text, by_name).ints("x"), {7}));
    CHECK(equal(read(spec, text, by_number).ints("x"), {1,2}));
    CHECK(read(spec, text, by_number).text("name") == "a");
    CHECK(fails([&](void) { (void) read(spec, text, bad); }, "there is no instance '3' (the file has 2 instances)"));
    CHECK(fails(spec, text, "the file has 2 instances; select one"));
    // A single instance needs no selection, and needs no name
    CHECK(equal(read(spec, "3\n4 5 6\n").ints("x"), {4,5,6}));
    // The catalog lists numbers and names
    Spec s = Spec::parse(spec, "s");
    Data c = s.catalog_text(text);
    CHECK(c.integer("instances") == 2);
    CHECK(c.text("name", {1}) == "b");
    Data c1 = s.catalog_text("3\n4 5 6\n");
    CHECK((c1.integer("instances") == 1) && !c1.has("name"));
    CHECK(Spec::parse("a: int", "s").catalog_text("5").integer("instances") == 1);
    // Trailing input that holds no instance is an error
    CHECK(fails(spec, "1\n2\njunk\n", "expected a line matching \"{n:int}\""));
    CHECK(fails("instances { a: int }", "", "the file contains no instance"));
    CHECK(fails([](void) {
                  (void) Spec::parse("instances { a: int } b: int", "s"); },
                "'instances' must be the last statement"));
    CHECK(fails([](void) {
                  (void) Spec::parse("repeat 2 { instances { a: int } }", "s"); },
                "'instances' must be at the top level"));
    CHECK(fails("instances { if 0 > 1 { a: int } }", "5", "an instance must read some input"));
    return true;
  });

  Case arithmetic("Read::Arithmetic", [](void) {
    Data d = read("a[3]: int  g: float  h: float\n"
                  "let s = sum(a)  let lo = min(a)  let hi = max(a, 7)\n"
                  "check g < 0 && -h < -3.5 \"floats compare\"",
                  "-5 2 1 -1.5 4");
    CHECK(d.integer("s") == -2);
    CHECK(d.integer("lo") == -5);
    CHECK(d.integer("hi") == 7);
    // Arithmetic is on integers
    CHECK(fails("f: float  let g = f * 2", "1.5", "'*' needs integers"));
    CHECK(fails("f[1]: float  let g = sum(f)", "1.5", "'sum' takes integers"));
    Data m = read("a: int  let p = a * 1  let q = a - a  let r = 1 * a", "-9223372036854775808");
    CHECK(m.integer("p") == m.integer("a"));
    CHECK(m.integer("q") == 0);
    CHECK(m.integer("r") == m.integer("a"));
    CHECK(fails("a: int  let b = 0 - a", "-9223372036854775808", "integer overflow"));
    CHECK(fails("g[1][1]: alphabet \"ab\" base 9223372036854775807", "b", "integer overflow"));
    return true;
  });

  Case progress("Read::Progress", [](void) {
    // Repeats whose body reads no input
    CHECK(fails("repeat until eof { let x = 1 }", "1", "the body of a repeat until read no input"));
    CHECK(read("repeat until \"END\" { if 1 > 2 { w: word } }", "END").size("w") == 0);
    CHECK(fails("repeat until \"END\" { if 1 > 2 { w: word } }", "y END", "the body of a repeat until read no input"));
    CHECK(fails("n: int  repeat n { a[0]: int }", "2000000000", "without reading input"));
    CHECK(read("n: int  repeat n { a[0]: int }", "3").size("a") == 3);
    // The limit is for the whole run, not each statement
    CHECK(fails("n: int  repeat n { repeat n { let x = 1 } }", "3000", "without reading input"));
    CHECK(fails("n: int  repeat n { a[n][0]: int }", "3000", "without reading input"));
    return true;
  });

  Case shapes("Read::Shapes", [](void) {
    // Empty values keep their declared depth and type
    Data d = read("repeat 2 { x[0][3]: float }", "");
    CHECK(d.depth("x") == 3);
    CHECK(d.type("x") == Gecode::Instance::ValueType::FLOAT);
    Data z = read("repeat 0 { x[2][3]: float  w: word  list l: int until 0 }", "");
    CHECK((z.depth("x") == 3) && (z.type("x") == Gecode::Instance::ValueType::FLOAT));
    CHECK((z.depth("w") == 1) && (z.type("w") == Gecode::Instance::ValueType::STRING));
    CHECK((z.depth("l") == 2) && (z.size("l") == 0));
    Data m = read("repeat 2 { k: int  if k > 5 { x[2][2]: int } }", "1 2");
    CHECK((m.depth("x") == 3) && (m.size("x") == 2) && (m.size("x", {1}) == 0));
    CHECK(fails("repeat 2 { k: int  if k > 5 { x: int } }", "1 2", "'x' is defined in no repetition"));
    // Paths reach values at any depth
    Data e = read("s[1][2][2]: word  v[2][1][2]: int", "a b c d 1 2 3 4");
    CHECK(e.text("s", {0,1,1}) == "d");
    CHECK(e.integer("v", {1,0,0}) == 3);
    CHECK(e.size("v", {1,0}) == 2);
    CHECK(equal(e.ints("v", {1,0}), {3,4}));
    CHECK(fails([&](void) { (void) e.text("s", {0,2,0}); }, "index out of range"));
    // Word lists end at an integer terminator
    CHECK(read("list w: word until 0", "a b 0").size("w") == 2);
    return true;
  });

  Case parameters("Read::Parameters", [](void) {
    Params p;
    p.set("k", "1.5");
    CHECK(read("param k  check k > 1 && k < 2", "", p).number("k") == 1.5);
    Params q;
    q.set("kk", "7");
    CHECK(fails([&](void) { (void) read("param k = 1", "", q); }, "the spec has no parameter 'kk'"));
    // A given value has the type of the default
    Params r;
    r.set("s", "3"); r.set("i", "4");
    Data d = read("param s = \"x\"  param i = 1  check s == \"3\" && i == 4", "", r);
    CHECK(d.text("s") == "3");
    Params t;
    t.set("i", "4.5");
    CHECK(fails([&](void) { (void) read("param i = 1", "", t); }, "parameter 'i' must be an integer"));
    return true;
  });

  Case checks("Spec::Checks", [](void) {
    // Errors in specs are found when they are parsed
    CHECK(fails([](void) {
                  (void) Spec::parse("n: int if n > 5 { let y = sqrt(n) }", "s");
                }, "unknown function 'sqrt'"));
    CHECK(fails([](void) { (void) Spec::parse("let y = count(1, 2)", "s"); }, "wrong arguments for function 'count'"));
    CHECK(fails([](void) { (void) Spec::parse("line \"{bad-name:int}\"", "s"); }, "is not a name"));
    CHECK(fails([](void) {
                  (void) Spec::parse("let y = " + std::string(300, '(') + "1" + std::string(300, ')'), "s");
                }, "nested too deeply"));
    // Long operator chains and else-if chains are deep trees as well
    std::string sum = "let y = 1", chain = "n: int  if n == 0 { }";
    for (int i=0; i<300; i++) {
      sum += " + 1";
      chain += " else if n == " + std::to_string(i+1) + " { }";
    }
    CHECK(fails([&](void) { (void) Spec::parse(sum, "s"); }, "nested too deeply"));
    CHECK(fails([&](void) { (void) Spec::parse(chain, "s"); }, "nested too deeply"));
    CHECK(fails([](void) { (void) Spec::parse("let _ = 1", "s"); }, "'_' cannot be defined here"));
    CHECK(fails([](void) { (void) Spec::parse("w: word offset 1", "s"); }, "'offset' needs numbers"));
    return true;
  });

  Case strings("Spec::Strings", [](void) {
    // The escapes \n, \t, \\, and \"
    Data d = read("let s = \"a\\nb\\tc\\\\d\\\"e\"", "");
    CHECK(d.text("s") == "a\nb\tc\\d\"e");
    // Other escapes and a backslash before a newline are errors on their line
    CHECK(fails([](void) { (void) Spec::parse("a: int\nlet s = \"a\\db\"", "s"); }, "s:2: unknown escape '\\d'"));
    CHECK(fails([](void) { (void) Spec::parse("a: int\nlet s = \"a\\\nb\"\nb: int", "s"); }, "s:2: unterminated string"));
    // Escapes do not disturb the line numbers of later errors
    CHECK(fails([](void) {
                  (void) Spec::parse("let s = \"a\\\\\"\nlet t = \"\\\"\"\nx: int untl", "s");
                }, "s:3: unknown statement 'untl'"));
    return true;
  });

  Case settings("Spec::Settings", [](void) {
    // Settings apply to the rest of the input, so they are not in blocks
    const char* nested[] = {
      "repeat 2 { comment \"c\"  a: int }",
      "if 1 > 0 { stop \"%\" }",
      "n: int  if n > 0 { } else if n < 0 { tokens \";\" }",
      "instances { separators \",\"  a: int }",
      "sections { \"x\" { ignore trailing } }"
    };
    for (const char* s : nested)
      CHECK(fails([&](void) { (void) Spec::parse(s, "s"); }, "must be at the top level of the spec"));
    CHECK(fails([](void) { (void) Spec::parse("a: int\nrepeat a {\n  comment \"c\"\n}", "s"); },
                "s:3: the setting 'comment' must be at the top level"));
    // Literal tokens are not a setting
    CHECK(read("repeat 2 { expect \"x\"  a: int }", "x 1 x 2").size("a") == 2);
    return true;
  });

  Case errors("Read::Errors", [](void) {
    CHECK(fails("n: int repeat n { x: int }", "3\n1\n2 z\n", "test.txt:3:3: expected int, found 'z'"));
    CHECK(fails("n: int repeat n { x: int }", "3\n1\n2\n", "while reading: repeat [3 of 3] > x"));
    CHECK(fails("n: int repeat n { x: int }", "3\n1\n2\n", "spec: test.reader:1"));
    CHECK(fails([](void) { (void) Spec::parse("x: int untl", "s"); }, "s:1: unknown statement 'untl'"));
    CHECK(fails([](void) { (void) Spec::parse("repeat 2 { x: int", "s"); }, "missing '}'"));
    CHECK(fails([](void) { (void) Spec::parse("line \"{x:bad}\"", "s"); }, "unknown type 'bad'"));
    CHECK(fails([](void) { (void) Spec::load("/nonexistent/x.reader"); }, "cannot open file"));
    CHECK(fails([](void) { (void) Spec::load("."); }, "is a directory"));
    return true;
  });

  Case access("Data::Access", [](void) {
    Data d = read("big: int  w: word  n: int  repeat n { k: int  x[k]: int }", "9999999999 hi 2 1 5 2 6 7");
    CHECK(d.integer("big") == 9999999999LL);
    CHECK(fails([&](void) { (void) d.ints("big"); }, "is not an array"));
    CHECK(fails([&](void) { (void) d.integer("w"); }, "is not an integer"));
    CHECK(fails([&](void) { (void) d.integer("nope"); }, "no value named"));
    CHECK(fails([&](void) { (void) d.size("x", {2}); }, "index out of range"));
    CHECK(fails([&](void) { (void) d.integer("x", {0}); }, "is not a single value"));
    Data e = read("n: int  repeat n { v: int }", "1 9999999999");
    CHECK(fails([&](void) { (void) e.ints("v"); }, "does not fit into an int"));
    return true;
  });

  Case output("Data::Output", [](void) {
    Data d = read("n: int  f: float  w: word  m[2][2]: int  repeat n { k: int  x[k]: int }", "2 20 a 1 2 3 4 1 5 2 6 7");
    std::ostringstream z;
    d.dzn(z);
    CHECK(z.str().find("f = 20.0;") != std::string::npos);
    CHECK(z.str().find("m = [| 1, 2\n  | 3, 4 |];") != std::string::npos);
    CHECK(z.str().find("% x: array[int] of tuple(array[int] of int)\n"
                       "x = [([5],), ([6, 7],)];") != std::string::npos);
    CHECK(z.str().find("w = \"a\";") != std::string::npos);
    // MiniZinc keywords are quoted, control characters escaped
    std::ostringstream zq;
    read("var: int  w: word", "3 a\033b").dzn(zq);
    CHECK(zq.str().find("'var' = 3;") != std::string::npos);
    CHECK(zq.str().find("w = \"a\\x1bb\";") != std::string::npos);
    // Empty dimensions and deeply ragged arrays
    Data e = read("e[2][0]: int  repeat 2 { k: int  repeat k { list y: int until 0 } }", "1 7 0 2 8 9 0 0");
    std::ostringstream ze;
    e.dzn(ze);
    CHECK(ze.str().find("e = array2d(1..2, 1..0, []);") != std::string::npos);
    CHECK(ze.str().find("y = [([([7],)],), ([([8, 9],), ([],)],)];") != std::string::npos);
    // MiniZinc has no constructor beyond array6d
    std::ostringstream z7;
    read("a[1][1][1][1][1][1][1]: int", "5").dzn(z7);
    CHECK(z7.str().find("a = [([([([([([([5],)],)],)],)],)],)];") != std::string::npos);
    return true;
  });

  Case labels("Read::Labels", [](void) {
    // The layout of PSPLIB single-mode RCPSP files
    const char* spec = "comment \"***\" \"---\"\n"
      "find \"jobs (incl. supersource/sink ): {jobs:int}\"\n"
      "find \"- renewable : {renewable:int} R\"\n"
      "find \"PRECEDENCE RELATIONS:\"\n"
      "line \"jobnr. #modes #successors successors\"\n"
      "repeat jobs { index 1  modes: int  nsucc: int  successors[nsucc]: int offset -1 }\n"
      "line \"REQUESTS/DURATIONS:\"\n"
      "line \"jobnr. mode duration ...\"\n"
      "repeat jobs { index 1  _: int  duration: int  request[renewable]: int }\n"
      "line \"RESOURCEAVAILABILITIES:\"\n"
      "line \"R 1 ...\"\n"
      "capacity[renewable]: int\n";
    Data d = read(spec,
      "****\n"
      "jobs (incl. supersource/sink ):  3\n"
      "horizon                       :  9\n"
      "RESOURCES\n"
      "  - renewable                 :  1   R\n"
      "  - nonrenewable              :  0   N\n"
      "  - doubly constrained        :  0   D\n"
      "****\n"
      "PRECEDENCE RELATIONS:\n"
      "jobnr.    #modes  #successors   successors\n"
      "   1        1          1           2\n"
      "   2        1          1           3\n"
      "   3        1          0\n"
      "****\n"
      "REQUESTS/DURATIONS:\n"
      "jobnr. mode duration  R 1\n"
      "------------------------\n"
      "  1      1     0       0\n"
      "  2      1     4       2\n"
      "  3      1     0       0\n"
      "****\n"
      "RESOURCEAVAILABILITIES:\n"
      "  R 1\n"
      "   3\n"
      "****\n");
    CHECK(equal(d.ints("successors"), {1,2}));
    CHECK(equal(d.ints("duration"), {0,4,0}));
    CHECK(equal(d.ints("capacity"), {3}));
    return true;
  });

  Case expressions("Read::Expressions", [](void) {
    // Indices start at 0, for numbers and for rows of ragged arrays
    const char* rows = "n: int  repeat n { list r: int until eol }  f[2]: float  w[2]: word\n";
    const char* text = "3\n1 2\n3 3 4\n5\n0.5 2.5 a b";
    Data d = read((std::string(rows) +
                   "let a = r[1]  let b = r[1][2] + r[2][0]  let g = f[1]  let v = w[0]\n"
                   "let s = sum(x in r: sum(x))  let c = count(x in r: count(x) > 1)\n"
                   "let t = sum(x in r: sum(y in x: y * y))  let k = count(x in f: x > 1)\n"
                   "check all(x in r: sum(x) + count(x) - 1 <= 12) && !all(x in r: count(x) == 2)\n"
                   "check any(x in r: !distinct(x)) && !any(x in w: x == \"c\") && any(x in w: x == \"b\")\n"
                   "let d = distinct(r[0], 3) && !distinct(r[0], r[2], 2) && distinct(r[1][1])  check d\n"
                   "check all(x in r: defined(x)) && !defined(x)").c_str(), text);
    CHECK(equal(d.ints("a"), {3,3,4}));
    CHECK(d.integer("b") == 9);
    CHECK(d.number("g") == 2.5);
    CHECK(d.text("v") == "a");
    CHECK(d.integer("s") == 18);
    CHECK(d.integer("c") == 2);
    CHECK(d.integer("t") == 64);
    CHECK(d.integer("k") == 1);
    // Empty arrays
    Data e = read("a[0]: int  let s = sum(x in a: x)  let c = count(x in a: 1)\n"
                  "check all(x in a: 0) && !any(x in a: 1) && distinct(a)", "");
    CHECK((e.integer("s") == 0) && (e.integer("c") == 0));
    // Errors
    std::string r(rows);
    CHECK(fails((r + "let z = r[3]").c_str(), text, "index 3 is out of range for 'r' (size 3, indices start at 0)"));
    CHECK(fails((r + "let z = r[0 - 1]").c_str(), text, "index -1 is out of range"));
    CHECK(fails((r + "let z = r[1][3]").c_str(), text, "index 3 is out of range for an element of 'r' (size 3"));
    CHECK(fails((r + "let z = n[0]").c_str(), text, "only arrays can be indexed, not 'n'"));
    CHECK(fails((r + "let z = r[f[0]]").c_str(), text, "an index must be an integer"));
    CHECK(fails((r + "let z = r[0][0][0]").c_str(), text, "only arrays can be indexed, not an element of 'r'"));
    CHECK(fails((r + "let z = all(x in n: 1)").c_str(), text, "'all(x in ...)' needs an array"));
    CHECK(fails((r + "let z = sum(x in r: x)").c_str(), text, "'sum' needs integers"));
    CHECK(fails((r + "let z = count(x in r: 1) + x").c_str(), text, "'x' is not defined yet"));
    CHECK(fails((r + "let z = all(x in r: q)").c_str(), text, "'q' is not defined yet"));
    CHECK(fails((r + "let z = distinct(w)").c_str(), text, "'distinct' takes integers"));
    CHECK(fails("a[2]: int  let z = sum(x in a: x)", "9223372036854775807 1", "integer overflow in 'sum'"));
    return true;
  });

  Case comprehensions("Spec::Comprehensions", [](void) {
    // The bound name is defined only in the body, and must be new
    CHECK(fails([](void) { (void) Spec::parse("n: int  a[n]: int  check all(n in a: n > 0)", "s"); },
                "s:1: 'n' is bound by a comprehension but is also a name in the spec"));
    CHECK(fails([](void) { (void) Spec::parse("a[2]: int\ncheck all(x in a: 1)\nrepeat 2 { x: int }", "s"); },
                "s:2: 'x' is bound by a comprehension but is also a name in the spec"));
    CHECK(fails([](void) { (void) Spec::parse("a[2][2]: int  check all(x in a: all(x in x: 1))", "s"); },
                "'x' is already bound by an enclosing comprehension"));
    CHECK(fails([](void) { (void) Spec::parse("a[2]: int  check all(_ in a: 1)", "s"); }, "'_' cannot be bound"));
    CHECK(fails([](void) { (void) Spec::parse("a[2]: int  check all(a)", "s"); },
                "'all' needs the form all(x in a: e)"));
    CHECK(fails([](void) { (void) Spec::parse("a[2]: int  check any(x in a)", "s"); }, "expected ':'"));
    CHECK(fails([](void) { (void) Spec::parse("a[2]: int  check a[0", "s"); }, "expected ']'"));
    CHECK(fails([](void) { (void) Spec::parse("let y = distinct()", "s"); },
                "wrong arguments for function 'distinct'"));
    // Comprehensions over the bound name of an enclosing one, and the same
    // name in comprehensions that do not nest
    Data d = read("a[2][2]: int  let s = sum(x in a: sum(y in x: y)) + sum(y in a: count(y))", "1 2 3 4");
    CHECK(d.integer("s") == 14);
    // Long index chains are deep trees as well
    std::string chain = "a[1]: int  let y = a";
    for (int i=0; i<300; i++)
      chain += "[0]";
    CHECK(fails([&](void) { (void) Spec::parse(chain, "s"); }, "nested too deeply"));
    return true;
  });

  Case effort("Read::Effort", [](void) {
    // Expressions visit values in proportion to the input
    std::string text = "2000\n";
    for (int i=0; i<2000; i++)
      text += "1 ";
    CHECK(read("n: int  a[n]: int  check all(x in a: x == 1) && sum(a) == n", text.c_str()).size("a") == 2000);
    // Comprehensions and repeated sums over the whole array
    CHECK(fails("n: int  a[n]: int  check all(x in a: sum(a) > 0)", text.c_str(),
                "the expressions do too much work for the size of the input"));
    // Nested comprehensions, which copy rows
    std::string rows = "100 500\n";
    for (int i=0; i<100*500; i++)
      rows += "1 ";
    CHECK(read("m: int  k: int  a[m][k]: int  check all(r in a: all(s in r: s == 1))", rows.c_str()).size("a") == 100);
    CHECK(fails("m: int  k: int  a[m][k]: int  check all(r in a: all(s in a: 1))", rows.c_str(), "too much work"));
    std::string more = text;
    for (int i=0; i<2000; i++)
      more += "1 ";
    CHECK(fails("n: int  a[n]: int  repeat n { k: int  check sum(a) > 0 }", more.c_str(), "too much work"));
    return true;
  });

  Case skipped("Read::SkippedInstances", [](void) {
    const char* spec = "instances {\n"
      "  sections skip other {\n"
      "    \"instance {name:word}\" optional\n"
      "    \"{n:int}\" last { check n <= 3 \"n is at most 3\"  x[n][n]: int }\n"
      "  }\n"
      "}\n";
    const char* text = "instance a\n4\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\ninstance b\n1\n7\n";
    Params first, second, a, b;
    first.set("instance", "1"); second.set("instance", "2");
    a.set("instance", "a"); b.set("instance", "b");
    // An invalid earlier instance does not hide a valid later one
    CHECK(equal(read(spec, text, second).ints("x"), {7}));
    CHECK(equal(read(spec, text, b).ints("x"), {7}));
    // An invalid selected instance is an error
    CHECK(fails([&](void) { (void) read(spec, text, first); }, "check failed: n is at most 3"));
    CHECK(fails([&](void) { (void) read(spec, text, a); }, "check failed: n is at most 3"));
    CHECK(fails(spec, "4\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n", "check failed: n is at most 3"));
    // Without a selection, a later invalid instance gives the usual error
    CHECK(fails(spec, "1\n7\n4\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n", "the file has 2 instances; select one"));
    // The catalog has the message of the first failed check of each instance
    Spec s = Spec::parse(spec, "s");
    Data c = s.catalog_text(text);
    CHECK(c.integer("instances") == 2);
    CHECK((c.text("failed", {0}) == "n is at most 3") && (c.text("failed", {1}) == ""));
    CHECK(!s.catalog_text("instance a\n1\n7\n").has("failed"));
    CHECK(Spec::parse("instances { n: int  check n > 5 }", "s").catalog_text("7 1").text("failed", {1}) ==
          "check on line 1 of the spec");
    // An instance selected by name may fail a check before its name is known
    const char* late = "instances { n: int  check n > 0 \"positive\"  name: word }";
    Params y;
    y.set("instance", "y");
    CHECK(read(late, "0 x 5 y", y).integer("n") == 5);
    CHECK(fails([&](void) { (void) read(late, "5 x 0 y", y); }, "check failed: positive"));
    // Other errors in a skipped instance stop reading, also when large sizes
    // follow a failed check
    CHECK(fails([&](void) { (void) read(spec, "instance a\n2\n1 2 3 x\ninstance b\n1\n7\n", b); }, "expected int"));
    auto start = std::chrono::steady_clock::now();
    CHECK(fails([&](void) { (void) read(spec, "instance a\n1000000000\n1 2\ninstance b\n1\n7\n", b); },
                "array is too large"));
    CHECK(fails([&](void) { (void) read(spec, "instance a\n40000\n1 2\ninstance b\n1\n7\n", b); },
                "after an earlier error in the instance: n is at most 3"));
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
    // A missing or repeated sections line in a skipped instance is deferred
    // as a failed check is
    const char* repeated = "instance a\ninstance x\n1\n7\ninstance b\n1\n8\n";
    CHECK(equal(read(spec, repeated, b).ints("x"), {8}));
    CHECK(fails([&](void) { (void) read(spec, repeated, a); },
                "test.txt:2:1: a second line matching \"instance {name:word}\""));
    CHECK(s.catalog_text(repeated).text("failed", {0}) == "a second line matching \"instance {name:word}\"");
    const char* stop = "instances { sections { \"n {n:int}\"  \"m {m:int}\"  \"end\" last } }";
    Params two;
    two.set("instance", "2");
    CHECK(read(stop, "n 1\nend\nn 2\nm 3\nend\n", two).integer("m") == 3);
    CHECK(Spec::parse(stop, "s").catalog_text("n 1\nend\nn 2\nm 3\nend\n").text("failed", {0}) ==
          "no line matching \"m {m:int}\"");
    // A selection that is not a number is a name
    Params named;
    named.set("instance", "3x");
    CHECK(read("instances { n: int  name: word }", "1 a 2 3x 3 c", named).integer("n") == 2);
    return true;
  });

  Case power("Read::Power", [](void) {
    // ^ binds tighter than * and unary -, and groups to the right
    Data d = read("let a = 2 * 3 ^ 2  let b = 2 ^ 3 ^ 2  let c = (2 ^ 3) ^ 2  let e = -2 ^ 2  let f = 1 + 2 ^ 2 * 3\n"
                  "let k = (-2) ^ 2  let m = -2 ^ 3 ^ 2\n"
                  "let g = 7 ^ 0  let h = (0 - 2) ^ 63  let i = 1 ^ 9223372036854775807  let j = 3 ^ 39", "");
    CHECK(d.integer("a") == 18);
    CHECK(d.integer("b") == 512);
    CHECK(d.integer("c") == 64);
    CHECK(d.integer("e") == -4);
    CHECK(d.integer("k") == 4);
    CHECK(d.integer("m") == -512);
    CHECK(d.integer("f") == 13);
    CHECK(d.integer("g") == 1);
    CHECK(d.integer("h") == LLONG_MIN);
    CHECK(d.integer("i") == 1);
    CHECK(d.integer("j") == 4052555153018976267LL);
    CHECK(fails("let x = 3 ^ 40", "", "integer overflow in '^'"));
    CHECK(fails("let x = 2 ^ 63", "", "integer overflow in '^'"));
    CHECK(fails("let x = 2 ^ -1", "", "negative exponent in '^'"));
    CHECK(fails("let x = 2 ^ -1 + 3", "", "negative exponent in '^'"));
    CHECK(fails("let x = -2 ^ 63", "", "integer overflow in '^'"));
    CHECK(fails("f: float  let x = f ^ 2", "1.5", "'^' needs integers"));
    return true;
  });

  Case constants("Spec::GecodeConstants", [](void) {
    Data d = read("let a = gecode.int_max  let b = gecode.int_min  let c = gecode.int_max / 2\n"
                  "gecode: int  check gecode == 3", "3");
    CHECK(d.integer("a") == Gecode::Int::Limits::max);
    CHECK(d.integer("b") == Gecode::Int::Limits::min);
    CHECK(d.integer("c") == 1073741823);
    CHECK(fails([](void) { (void) Spec::parse("let x = 1\nlet y = gecode.foo", "s"); },
                "s:2: unknown Gecode constant 'gecode.foo' (known: gecode.int_max, gecode.int_min)"));
    // Specs cannot define such names
    const char* defining[] = {
      "gecode.x: int", "let gecode.int_max = 3", "param gecode.p", "list gecode.l: int until 0",
      "gecode.g[1][1]: alphabet \"a\"", "repeat until \"x\" store gecode.t { a: int }",
      "a[2]: int  check all(gecode.v in a: 1)"
    };
    for (const char* spec : defining)
      if (!fails([&](void) { (void) Spec::parse(spec, "s"); }, "names starting with 'gecode.' are Gecode constants"))
        return false;
    return true;
  });

  Case records("Read::Records", [](void) {
    // A check after a repeat holds for every record, and a failure names
    // the record and points at its input
    const char* rows = "n: int  repeat n { list row: int until eol }  width: int\n"
      "check sum(row) + count(row) - 1 <= width \"the clues do not fit\"";
    CHECK(read(rows, "2\n1 1\n2\n3").size("row") == 2);
    CHECK(fails(rows, "2\n1 1\n  2 2\n3", "test.txt:3:3: check failed for record 2 of 'row': the clues do not fit"));
    // Internal values do not show in the data
    CHECK(read(rows, "2\n1 1\n2\n3").entries() == 3);
    // Names of one repeat are taken from the same record
    const char* windows = "n: int  repeat n { ready: int  due: int }  check ready <= due \"window\"";
    CHECK(read(windows, "2 3 5 0 1").size("ready") == 2);
    CHECK(fails(windows, "3 0 5 2 4 6 1", "check failed for record 3 of 'ready': window"));
    // Nested repeats: every inner record of every outer record
    const char* jobs = "j: int  m: int  repeat j { k: int  repeat m { x: int } }  check x < k \"x below k\"";
    CHECK(read(jobs, "2 2\n5 1 2\n3 1 2\n").size("x") == 2);
    CHECK(fails(jobs, "2 2\n5 1 2\n3 1 4\n", "test.txt:3:5: check failed for record 2 of record 2 of 'x': x below k"));
    // The whole array: size, the array of a comprehension, and defined
    Data d = read("n: int  repeat n { list row: int until eol }\n"
                  "check size(row) == 2 && count(row) == 3 && sum(r in row: sum(r)) == 21\n"
                  "check count(r in row: count(r) == 3) == size(row) && defined(row)", "2\n1 2 3\n4 5 6\n");
    CHECK(d.size("row") == 2);
    // Record names in the body of a comprehension range over records
    CHECK(fails("n: int  lim[2]: int  repeat n { x: int }  check all(l in lim: x < l) \"below\"", "2 5 3 1 4",
                "check failed for element 2 of 'lim' in record 2 of 'x': below"));
    // A name that is not defined does not range over records
    const char* optional =
      "sections { \"A\" optional { repeat 2 { x: int } }  \"B\" optional }  check !defined(x) || x > 0 \"positive\"";
    CHECK(!read(optional, "B").has("x"));
    CHECK(read(optional, "A\n1 2").size("x") == 2);
    CHECK(fails(optional, "A\n1 0", "check failed for record 2 of 'x': positive"));
    // Inside the repeat that defines it, a name is the current record
    CHECK(fails("n: int  repeat n { list row: int until eol  check count(row) == 2 \"two\" }", "2\n1 2\n3\n",
                "check failed: two"));
    CHECK(fails("n: int  repeat n { list row: int until eol  check count(row) == 2 \"two\" }", "2\n1 2\n3\n",
                "while reading: repeat [2 of 2]"));
    // Between the check and the definition, the repeats range over records
    CHECK(read("repeat 2 { repeat 2 { x: int }  check size(x) == 2 && x > 0 }", "1 2 3 4").size("x") == 2);
    CHECK(fails("repeat 2 { repeat 2 { x: int }  check x > 0 \"positive\" }", "1 2 3 0",
                "check failed for record 2 of 'x': positive"));
    // Only checks range over records
    Data l = read("n: int  repeat n { x: int }  let s = sum(x)  let f = x[0]  if count(x) == 2 { let t = 1 }", "2 3 4");
    CHECK(l.integer("s") == 7);
    CHECK(l.integer("f") == 3);
    CHECK(l.has("t"));
    // Without records, the check holds
    CHECK(read("n: int  repeat n { x: int }  check x > 0 && 1 > 2", "0").size("x") == 0);
    return true;
  });

  Case record_names("Spec::Records", [](void) {
    CHECK(fails([](void) { (void) Spec::parse("n: int  repeat n { a: int }\nrepeat n { b: int }\ncheck a < b", "s"); },
                "s:3: a check can only range over the records of one repeat, but 'a' and 'b' are defined in different "
                "repeats"));
    CHECK(fails([](void) {
                  (void) Spec::parse("repeat 2 { repeat 2 { a: int }  repeat 2 { b: int } }  check a < b", "s");
                }, "can only range over the records of one repeat"));
    CHECK(fails([](void) { (void) Spec::parse("n: int  repeat n { a: int }  let a = 3\ncheck a < 1", "s"); },
                "s:2: 'a' is defined in several repeats, or inside and outside a repeat"));
    CHECK(fails([](void) { (void) Spec::parse("repeat 2 { a: int }  repeat 2 { a: int }  check a < 1", "s"); },
                "'a' is defined in several repeats"));
    // Definitions that all enclose the check, or in one repeat
    (void) Spec::parse("n: int  repeat n { a: int  check a > 0 }  let a = 3", "s");
    (void) Spec::parse("repeat 2 { k: int  if k > 0 { a: int } else { let a = 0 } }  check a >= 0", "s");
    return true;
  });

  Case record_effort("Read::RecordEffort", [](void) {
    // Each record that a check visits counts
    std::string text = "2000\n";
    for (int i=0; i<4000; i++)
      text += "1 ";
    CHECK(read("n: int  repeat n { x: int }  a[n]: int  check x == 1 && a[0] == 1", text.c_str()).size("x") == 2000);
    CHECK(fails("n: int  repeat n { x: int }  a[n]: int  check x <= sum(a)", text.c_str(),
                "the expressions do too much work for the size of the input"));
    return true;
  });

  Case skipped_records("Read::SkippedInstanceRecords", [](void) {
    // A check over records in an instance before the selected one does not
    // stop reading
    const char* spec = "instances { n: int  repeat n { x: int }  check x > 0 \"positive\"  name: word }";
    const char* text = "2 1 0 a\n1 5 b\n";
    Params a, b;
    a.set("instance", "a"); b.set("instance", "b");
    CHECK(equal(read(spec, text, b).ints("x"), {5}));
    CHECK(fails([&](void) { (void) read(spec, text, a); }, "test.txt:1:5: check failed for record 2 of 'x': positive"));
    Data c = Spec::parse(spec, "s").catalog_text(text);
    CHECK((c.text("failed", {0}) == "positive") && (c.text("failed", {1}) == ""));
    return true;
  });

  Case record_lists("Read::RecordLists", [](void) {
    // The list of records of a repeat, and each of its records
    const char* nodes = "n: int  repeat n as nodes { each node  ready: int  due: int }\n"
      "check node.ready <= node.due \"window\"\n"
      "check nodes[0].ready == 0 \"depot\"\n"
      "let k = size(nodes)  let d = nodes[1].due  let c = count(x in nodes: x.due > 4)";
    Data d = read(nodes, "3\n0 5\n2 4\n6 7\n");
    CHECK((d.integer("k") == 3) && (d.integer("d") == 4) && (d.integer("c") == 2));
    // The names are not in the data
    CHECK(!d.has("nodes") && !d.has("node") && (d.entries() == 6));
    CHECK(fails(nodes, "3\n0 5\n2 4\n6 1\n", "test.txt:4:1: check failed for record 3 of 'nodes': window"));
    CHECK(fails(nodes, "3\n1 5\n2 4\n6 7\n", "check failed: depot"));
    CHECK(fails("n: int  repeat n as nodes { a: int }  let x = nodes[3].a", "2 1 2",
                "index 3 is out of range for 'nodes' (size 2, indices start at 0)"));
    // Only the list, or only each record
    CHECK(read("repeat 2 as xs { a: int }  let s = sum(x in xs: x.a)", "3 4").integer("s") == 7);
    CHECK(fails("repeat 3 { each x  a: int  b: int }  check x.a < x.b \"less\"", "1 2 3 4 6 5",
                "test.txt:1:9: check failed for record 3 of 'x': less"));
    // Inside the repeat, each record is the current one
    Data in = read("repeat 2 as xs { each x  a: int  b: int  let w = x.b - x.a  check x.a <= x.b }", "1 4 2 2");
    CHECK(equal(in.ints("w"), {3, 0}));
    // Nested lists: each record of a job has a list of steps
    const char* jobs = "n: int  m: int\n"
      "repeat n as jobs { each job  repeat m as steps { each step  machine: int  duration: int } }\n"
      "check step.machine < m \"exists\"\n"
      "check distinct(job.machine) \"twice\"\n"
      "check size(job.steps) == m && all(s in job.steps: s.duration >= 0)\n"
      "let x = jobs[1].steps[0].machine  let y = size(jobs[0].steps)  let z = sum(j in jobs: sum(j.duration))";
    Data j = read(jobs, "2 2\n0 5 1 6\n1 2 0 3\n");
    CHECK((j.integer("x") == 1) && (j.integer("y") == 2) && (j.integer("z") == 16));
    CHECK(fails(jobs, "2 2\n0 5 1 6\n1 2 7 3\n", "test.txt:3:5: check failed for record 2 of 'steps' in record 2 of "
                "'jobs': exists"));
    CHECK(fails(jobs, "2 2\n0 5 1 6\n1 2 1 3\n", "test.txt:3:1: check failed for record 2 of 'jobs': twice"));
    // The terminating token of a repeat is kept with store
    Data t = read("repeat until \"end\" \"stop\" store t as xs { a: int }  let k = size(xs)", "1 2 stop");
    CHECK((t.text("t") == "stop") && (t.integer("k") == 2));
    return true;
  });

  Case record_list_errors("Spec::RecordLists", [](void) {
    CHECK(rejects("repeat 2 { a: int  each x }", "s:1: 'each' must be the first statement of a repeat"));
    CHECK(rejects("repeat 2 { each x  each y }",
                  "'each' must be the first statement of a repeat, and comes at most once"));
    CHECK(rejects("each x", "'each' must be the first statement of a repeat"));
    CHECK(rejects("if 1 { each x }", "'each' must be the first statement of a repeat"));
    // The names are new
    CHECK(rejects("repeat 2 as xs { each xs }", "'xs' cannot name both the list and each record"));
    CHECK(rejects("repeat 2 as xs { a: int }\nrepeat 2 { each xs }", "s:2: 'xs' already names records"));
    CHECK(rejects("repeat 2 as a { a: int }", "'a' names records, so it cannot also be defined"));
    CHECK(rejects("repeat 2 { each x  a: int }  let x = 1", "'x' names records, so it cannot also be defined"));
    CHECK(rejects("repeat 2 as gecode.xs { a: int }", "names starting with 'gecode.' are Gecode constants"));
    CHECK(rejects("repeat 2 { each gecode.x }", "names starting with 'gecode.' are Gecode constants"));
    CHECK(rejects("repeat 2 as xs { a: int }  check all(xs in xs: 1)",
                  "'xs' is bound by a comprehension but names records"));
    // Where the names can be used
    CHECK(rejects("repeat 2 as xs { a: int  let k = size(xs) }", "'xs' cannot be used inside its own repeat"));
    CHECK(rejects("repeat 2 { each job  repeat 2 as steps { a: int } }  check size(steps) == 2",
                  "'steps' is a list in each record of the repeat around it; use 'job.steps'"));
    CHECK(rejects("repeat 2 { repeat 2 as steps { a: int } }  check size(steps) == 2",
                  "name each of these records with 'each'"));
    CHECK(rejects("repeat 2 { each x  a: int }  let k = x.a",
                  "'x' is each record of a repeat, so only checks can use it"));
    // Fields
    CHECK(rejects("repeat 2 as nodes { each node  ready: int }  check nodes.ready > 0",
                  "'nodes' is a list of records, so '.ready' needs one of them: use 'nodes[i].ready', or 'node.ready' "
                  "for each record"));
    CHECK(rejects("repeat 2 as nodes { ready: int  due: int }  check nodes[0].read > 0",
                  "the records of 'nodes' have no field 'read' (they have ready, due)"));
    CHECK(rejects("repeat 2 as xs { a: int }  let a = 1  check xs[0].a > 0", "'a' is defined inside and outside"));
    CHECK(rejects("repeat 2 { each job  repeat 2 { a: int  let b = job.a } }",
                  "the field 'a' cannot be used inside the repeat that reads it"));
    CHECK(rejects("a: int  check a.b > 0", "only records have fields such as '.b'"));
    // Records are not values
    CHECK(rejects("repeat 2 as xs { a: int }  let k = xs[0]", "a record of 'xs' is not a value; use one of its fields, "
                  "as in 'xs[0].a'"));
    CHECK(rejects("repeat 2 as xs { a: int }  let k = xs", "'xs' is a list of records, not a value"));
    CHECK(rejects("repeat 2 as xs { each x  a: int }  check x > 0", "as in 'x.a'"));
    CHECK(rejects("repeat 2 as xs { a: int }  check xs[0][1] > 0", "a record of 'xs' cannot be indexed"));
    return true;
  });

  Case failure_locations("Read::FailureLocations", [](void) {
    // A failed all names the element that fails it
    CHECK(fails("a[4]: int  check all(x in a: x > 0) \"positive\"", "1 2 0 3",
                "check failed for element 3 of 'a': positive"));
    CHECK(fails("repeat 3 as xs { a: int }  check all(x in xs: x.a > 0) \"positive\"", "1\n2\n0\n",
                "test.txt:3:1: check failed for record 3 of 'xs': positive"));
    CHECK(fails("n: int  repeat n { list row: int until eol }  check all(r in row: min(r) > 0) \"positive\"",
                "2\n1 2\n3 0\n", "test.txt:3:1: check failed for record 2 of 'row': positive"));
    // Nested comprehensions and records report every level
    const char* jobs = "repeat 2 as jobs { each job  repeat 2 as steps { d: int } }\n";
    std::string nested = std::string(jobs) + "check all(j in jobs: all(s in j.steps: s.d > 0)) \"positive\"";
    CHECK(fails(nested.c_str(), "1 2\n3 0\n", "test.txt:2:3: check failed for record 2 of 'steps' in record 2 of "
                "'jobs': positive"));
    std::string lifted = std::string(jobs) + "check all(x in job.d: x > 0) \"positive\"";
    CHECK(fails(lifted.c_str(), "1 2\n3 0\n", "test.txt:2:3: check failed for record 2 of 'steps' in record 2 of "
                "'jobs': positive"));
    CHECK(fails("a[2][2]: int  check all(r in a: all(x in r: x > 0)) \"positive\"", "1 2 3 0",
                "check failed for element 2 of 'r' in element 2 of 'a': positive"));
    // Only a failed all that fails the check is reported
    CHECK(fails("a[2]: int  check any(x in a: x > 5) \"big\"", "1 2", "check failed: big"));
    CHECK(fails("a[2]: int  check !all(x in a: x > 0) \"not all\"", "1 2", "check failed: not all"));
    CHECK(fails("a[2]: int  check 1 > 2 || all(x in a: x > 1) \"m\"", "1 2", "check failed for element 1 of 'a': m"));
    CHECK(fails("a[2]: int  check all(x in a: x > 1) == 1 \"m\"", "1 2", "check failed: m"));
    CHECK(fails("a[2]: int  check count(x in a: all(y in a: y > x)) == 2 \"m\"", "1 2", "check failed: m"));
    // Indexing a single value of a record suggests naming the records
    CHECK(fails("n: int  repeat n { ready: int }  check ready[0] == 0", "2 1 2",
                "which is one value of each record here; to index the records, name them, as in 'repeat n as nodes { "
                "each node ... }', and use 'nodes[0].ready'"));
    CHECK(fails("n: int  repeat n as nodes { ready: int }  check ready[0] == 0", "2 1 2",
                "to index the records, use 'nodes[0].ready'"));
    return true;
  });

#undef CHECK

}}

// STATISTICS: test-any
