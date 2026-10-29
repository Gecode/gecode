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

#ifndef GECODE_INSTANCE_HH
#define GECODE_INSTANCE_HH

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include <iosfwd>
#include <memory>
#include <string>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_INSTANCE
#define GECODE_INSTANCE_EXPORT __declspec( dllexport )
#else
#define GECODE_INSTANCE_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_INSTANCE_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_INSTANCE_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_INSTANCE
#define GECODE_LIBRARY_NAME "Instance"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode::Instance
 * \brief Reading instance files described by reader specs
 *
 * A reader spec is a short text that describes how a textual instance
 * format is laid out: header counts, `KEY : VALUE` lines, sections,
 * matrices, lists ended by sentinels, records, and character grids.
 * Reading a file with a spec yields named values (Data): scalars and
 * arrays, where the fields of repeated records are stored column-wise.
 *
 * Programs supply the specs for the formats they read, typically as
 * string literals. The spec language is described in
 * \ref TaskInstanceSpec.
 *
 * \ingroup TaskInstance
 */

namespace Gecode { namespace Instance {

  /**
   * \defgroup TaskInstance Reading instance files
   *
   * Reading textual benchmark instances (DIMACS, TSPLIB, job shop, RCPSP,
   * sudoku, ...) with declarative reader specs. Scripts read instance
   * files through InstanceOptions; the SAT, TSP, TSPTW, job shop, sudoku,
   * and nonogram examples contain specs for their formats. The tool
   * \c gecode-instance reads a file with a spec and prints a summary of the
   * data or the data as MiniZinc data, which helps when writing a spec.
   *
   * \ingroup TaskModel
   */

  /**
   * \defgroup TaskInstanceSpec The reader spec language
   *
   * A spec is a sequence of statements executed from top to bottom
   * against the input. The input is read as whitespace-separated tokens,
   * so line breaks do not matter unless a statement works on lines. Names
   * defined by earlier statements can be used in later expressions. A
   * spec for DIMACS CNF is:
   * \code
   * format dimacs-cnf
   *
   * comment "c"
   * stop "%"
   * line "p cnf {vars:0..} {clauses:0..}"
   * repeat clauses {
   *   list clause: -vars..vars until 0
   * }
   * \endcode
   *
   * Statements that read values, where \c t is a type (see below):
   *  - <tt>x: t</tt>: read one value. The name \c _ discards it.
   *  - <tt>x[e1][e2]...: t</tt>: an array of fixed size, read row-major.
   *  - <tt>w[n][n]: t layout e</tt>: a symmetric matrix stored in the
   *    TSPLIB layout named by \c e (\c FULL_MATRIX, \c UPPER_ROW,
   *    \c LOWER_ROW, \c UPPER_DIAG_ROW, \c LOWER_DIAG_ROW, and the
   *    column-wise variants).
   *  - <tt>... offset e</tt>: add \c e to every number read, for example
   *    to make 1-based identifiers 0-based. For a triangular layout, the
   *    cells that are not read stay 0.
   *  - <tt>list x: t until 0</tt> or <tt>until eol</tt>: a list ended by
   *    a sentinel value (which is not stored) or by the end of the line.
   *    A line list starts on the next line when the current one is used
   *    up; as blank lines are skipped, an empty row needs a marker in the
   *    format (such as \c 0) rather than a blank line.
   *
   * The names of these statements can be keywords, as in <tt>line:
   * int</tt>, because a name followed by a colon always declares a value.
   *
   * Types:
   *  - \c int, \c float, \c num, \c word, and \c rest read one token.
   *    A \c num is an integer if possible and a float otherwise; \c rest
   *    reads the rest of the line and cannot be an array.
   *  - <tt>lo..hi</tt> and <tt>lo..<hi</tt> are the integers from \c lo to
   *    \c hi, including \c hi or not. One bound can be left out, so
   *    <tt>0..</tt> are the integers that are not negative, but
   *    <tt>..<</tt> needs its upper bound. The bounds are expressions,
   *    evaluated once before the statement reads, and a value outside the
   *    range is an error such as <tt>expected an int in 0..9, found
   *    '12'</tt>. The range applies to the numbers read, before an offset,
   *    and not to the sentinel of a list. A range without an upper bound
   *    ends before a statement, a declaration, or an option such as
   *    \c layout.
   *  - <tt>alphabet "123456789" blank "." strip "|-+" base 1</tt> reads
   *    the characters of a two-dimensional array, such as
   *    <tt>cells[n][n]: alphabet "123456789" blank "."</tt>: symbol \c k
   *    of the alphabet has value \c k plus \c base (1 by default), blank
   *    symbols are 0, and stripped symbols and white space are skipped.
   *  - <tt>type time = 0..gecode.int_max / 2</tt> names a type, which
   *    later statements in the same block, and the blocks inside it, can
   *    use as in <tt>ready: time</tt>. Its bounds are evaluated where it is
   *    defined, and errors name it, as in <tt>expected time (an int in
   *    0..1073741823), found '-1'</tt>. A type name must differ from the
   *    names of values.
   *
   * Statements that structure the input:
   *  - <tt>line "pattern"</tt>: the rest of the current line must match
   *    the pattern, or the next line if the current one is used up.
   *  - <tt>find "pattern"</tt>: skip lines until one matches.
   *  - <tt>skip line</tt>, <tt>skip e lines</tt>, <tt>expect "p" "cnf"</tt>.
   *  - <tt>repeat e { ... }</tt>: read a record \c e times. Every name
   *    defined in the body becomes an array with one entry per
   *    repetition, so records are stored column-wise. Rows may differ in
   *    length. A row that an iteration does not define is empty, which
   *    needs rows that are arrays.
   *  - <tt>repeat until "999" "EOF" store x { ... }</tt>, <tt>repeat until
   *    eof { ... }</tt>: repeat until one of the tokens comes next (it is
   *    consumed, and stored in \c x if given), or until the end of input.
   *  - <tt>repeat e as nodes { each node ... }</tt>: name the list of
   *    records and each record (see below); either is optional, and
   *    \c each comes first in the body.
   *  - <tt>index b</tt>: inside a repeat, read the row number counted from
   *    \c b and check it.
   *  - <tt>sections until "EOF" skip other { "pattern" { ... } ... }</tt>:
   *    lines in any order, each handled by the first arm whose pattern
   *    matches it, until one of the tokens or the end of the input (the
   *    terminating token is optional, as for TSPLIB's \c EOF, and
   *    <tt>store x</tt> keeps it as for repeat). Unknown lines
   *    are errors unless \c skip \c other is given. Each arm must match
   *    exactly one line, unless it is written <tt>"pattern" optional</tt>:
   *    then it matches at most one. An arm written
   *    <tt>"pattern" last { ... }</tt> ends the statement after it has run
   *    (an arm that is both is written <tt>optional last</tt>).
   *  - <tt>if e { ... } else { ... }</tt>, <tt>let x = e</tt>,
   *    <tt>check e "message"</tt> (see below for checks on records).
   *  - <tt>param x</tt> or <tt>param x = e</tt>: a value given by the
   *    caller (see Params). A given value has the type of the default; a
   *    parameter without a default is an integer if possible, otherwise a
   *    float or a string.
   *  - <tt>instances { ... }</tt>: a file with several instances, one per
   *    pass through the body. Only one instance is kept: the parameter
   *    \c instance selects it by number (from 1) if it is a number, and
   *    otherwise by the value of \c name, which the body may define. Without
   *    the parameter, the file must contain a single instance. Reading stops
   *    after the selected instance, so this must be the last statement of a
   *    spec. A failed check, or a missing or repeated sections line, in an
   *    instance before the selected one does not stop reading, so that an
   *    invalid instance does not hide later ones; other errors do, as the
   *    input cannot be read on. Spec::catalog lists the instances of a file
   *    and the first such failure of each.
   *
   * Settings: <tt>format name</tt> (the name of the format, see
   * Spec::name), <tt>comment "c" ...</tt> (prefixes of comment lines),
   * <tt>stop "%"</tt> (prefixes of lines that end the data; a prefix
   * ending in a letter or digit must not be followed by one, so \c "c"
   * matches <tt>c foo</tt> but not \c cfoo), <tt>tokens ";"</tt>
   * (characters that form tokens by themselves), <tt>separators ","</tt>
   * (characters that separate tokens like white space), and <tt>ignore
   * trailing</tt> (allow input after the last statement). Settings must
   * be at the top level of the spec, not inside a block.
   *
   * Strings in a spec are written in double quotes and end on the same
   * line. The escapes <tt>\\n</tt>, <tt>\\t</tt>, <tt>\\\\</tt>, and
   * <tt>\\\"</tt> stand for a newline, a tab, a backslash, and a double
   * quote; other escapes are errors.
   *
   * A line pattern is literal text with captures <tt>{x:t}</tt> of a type
   * that is not an alphabet, such as <tt>{x:int}</tt> or
   * <tt>{x:1..n}</tt>, and <tt>{$x}</tt> (the current value of \c x).
   * A capture without a type, <tt>{x}</tt>, is a word; one without a
   * name, such as <tt>{:int}</tt> or <tt>{_}</tt>, checks the text but
   * does not store it. The ranges of captures are checked after the line
   * matched, so they can use the captures before them. At the end of a
   * pattern, <tt>...</tt> matches the rest of the line.
   * White space in a pattern matches any amount of white space; at least
   * one blank is needed where two words would otherwise run together.
   * A pattern must match the rest of the line.
   *
   * Expressions have integer arithmetic (<tt>+ - * / % ^</tt>, where
   * division truncates towards zero, \c ^ is the power, and overflow and
   * negative exponents are errors), comparisons of numbers and strings (an
   * integer compared with a float is converted to a float),
   * <tt>&& || !</tt>, string literals, and the functions \c count(a) and
   * \c size(a) (the number of elements of \c a), \c min, \c max, and
   * \c sum (over all integers in their arguments), \c distinct (whether
   * these integers differ), and \c defined(x). The operators bind from
   * loosest to tightest: <tt>||</tt>, <tt>&&</tt>, <tt>== !=</tt>,
   * <tt>< > <= >=</tt>, <tt>+ -</tt>, <tt>* / %</tt>, the unary
   * <tt>- !</tt>, and \c ^ (which groups to the right), so <tt>-2^2</tt>
   * is -4 and <tt>2^-1</tt> is <tt>2^(-1)</tt>. The
   * constants \c gecode.int_max and \c gecode.int_min are the limits of
   * Gecode's integer values (Int::Limits); a spec cannot define names that
   * start with <tt>gecode.</tt>.
   *
   * <tt>a[i]</tt> is element \c i of array \c a, counted from 0 as in
   * Data: a number for a one-dimensional array and a row otherwise, so
   * <tt>a[i][j]</tt> works for arrays of rows. An index out of range is an
   * error. The comprehensions <tt>all(x in a: e)</tt>,
   * <tt>any(x in a: e)</tt>, <tt>count(x in a: e)</tt>, and
   * <tt>sum(x in a: e)</tt> evaluate \c e for each element \c x of \c a:
   * whether \c e holds for all or some elements, for how many it holds,
   * and the sum of its integer values (true, false, 0, and 0 for an empty
   * array). The name \c x is defined only in \c e and must differ from the
   * names of the spec and of enclosing comprehensions.
   *
   * In a check, a name defined inside a repeat that does not enclose the
   * check is a record name: it stands for one record, and the check must
   * hold for every record. Checks can thus follow the statements that
   * read the data. For the rows of a nonogram, read by
   * <tt>repeat height { list row: int until eol }</tt>,
   * \code
   * check sum(row) + count(row) - 1 <= width "the clues of a row do not fit"
   * \endcode
   * checks every row, and a failure names the record, as in <tt>check
   * failed for record 3 of 'row'</tt>, and points at its input. Record
   * names of one repeat are taken from the same record. With names of
   * nested repeats, the check holds for every inner record of every outer
   * record (<tt>record 2 of record 3</tt>). A check cannot use the record
   * names of two other repeats. A name directly after \c in and the
   * argument of \c size and \c defined are whole arrays:
   * <tt>size(row) == height</tt> counts the rows, and
   * <tt>sum(r in row: sum(r))</tt> adds all clues. Only checks range over
   * records: in \c let, \c if, and counts a record name is the whole array,
   * so <tt>let first = row[0]</tt> is the first row. A check cannot use a
   * name defined both inside and outside a repeat, or in several repeats,
   * unless all of them enclose the check. When an \c all makes a check
   * fail, the message names the element that fails it, as in
   * <tt>element 2 of 'a'</tt>, or the record.
   *
   * A repeat can name the list of its records and each record:
   * \code
   * repeat n as nodes {
   *   each node
   *   ready: int
   *   due: int
   * }
   * check node.ready <= node.due "the time window of a node is invalid"
   * check nodes[0].ready == 0 "the depot must open at time 0"
   * \endcode
   * The fields of a record are the names defined in the repeat, and the
   * lists of nested repeats. The list \c nodes is the whole list:
   * <tt>nodes[i]</tt> is record \c i, and <tt>size(nodes)</tt> and
   * <tt>x in nodes</tt> work as for arrays; a field needs one record, so
   * <tt>nodes.ready</tt> is an error. The name \c node is a record name,
   * like \c ready, and inside the repeat it is the current record. With
   * <tt>repeat n as jobs { each job  repeat m as steps { each step
   * machine: int } }</tt>, <tt>job.steps</tt> are the steps of a job and
   * <tt>job.machine</tt> their machines, so <tt>check
   * distinct(job.machine)</tt> checks each job and names it on failure
   * (<tt>record 3 of 'jobs'</tt>), while <tt>step.machine</tt> ranges
   * over all steps (<tt>record 2 of 'steps' in record 3 of 'jobs'</tt>).
   * Records are not values, and their names are not in the data and must
   * differ from all other names.
   *
   * The values that expressions visit, including the records of checks,
   * are limited to a multiple of the size of the input, so that checks
   * cannot take time far beyond reading.
   *
   * \ingroup TaskInstance
   */

  /**
   * \brief %Exception for errors in specs and instance files
   *
   * The message gives the location (file, line, and column), the offending
   * line, and the path through the spec at the point of the error. The
   * instance module reports every error in a spec or an instance by
   * throwing this exception and never writes diagnostics itself; programs
   * that check the data they read can throw it as well. Running out of
   * memory throws MemoryExhausted or \c std::bad_alloc.
   *
   * \ingroup TaskInstance
   */
  class GECODE_INSTANCE_EXPORT Error : public Exception {
  protected:
    /// The full message
    std::string msg;
  public:
    /// Initialize with message \a m
    explicit Error(const std::string& m);
    /// Return the full message
    virtual const char* what(void) const noexcept override;
  };

  /// Type of the values in a data entry
  enum class ValueType {
    INT,    ///< Integers
    FLOAT,  ///< Floating point numbers
    STRING  ///< Strings
  };

  class DataImpl;
  class SpecImpl;
  class ParamsImpl;

  /**
   * \brief Values for the `param` statements of a spec
   *
   * \ingroup TaskInstance
   */
  class GECODE_INSTANCE_EXPORT Params {
  private:
    /// The parameters (copied on write)
    std::shared_ptr<ParamsImpl> impl;
  public:
    /// No parameters
    Params(void);
    /// Set parameter \a name to \a value
    void set(const std::string& name, const std::string& value);
    /**
     * \brief Parse and set parameters given as `name=value`
     *
     * Several parameters are separated by commas, as in `a=1,b=2`, so a
     * value cannot contain a comma, and empty items are skipped. Throws Error if a parameter does not
     * have the form `name=value`.
     */
    void set(const std::string& assignments);
    /// Return whether \a name is set, and if so store its value in \a value
    bool get(const std::string& name, std::string& value) const;
    /// Return number of parameters
    int size(void) const;
    /// Return the name of parameter \a i
    std::string name(int i) const;
  };

  /**
   * \brief Named values read from an instance
   *
   * Every entry has a name, a value type (see ValueType), and a depth:
   * depth 0 is a scalar, depth 1 an array, depth 2 an array of arrays,
   * and so on. Arrays may be ragged, that is, rows can have different
   * lengths.
   *
   * The accessors take a path of indices (from 0) that selects a row or a
   * single value, such as <tt>d.ints("clause", {3})</tt> for row 3 of
   * \c clause or <tt>d.text("s", {1, 0, 2})</tt>; without a path they
   * refer to the whole entry. They throw Error if the name is unknown, if
   * the value has the wrong type or depth, if an index is out of range, or
   * if an integer does not fit into an \c int.
   *
   * \ingroup TaskInstance
   */
  class GECODE_INSTANCE_EXPORT Data {
    friend class SpecImpl;
  private:
    /// The values
    std::shared_ptr<const DataImpl> impl;
  public:
    /// Empty data
    Data(void);

    /// \name Entries
    //@{
    /// Return number of entries
    int entries(void) const;
    /// Return name of entry \a i, in the order they were defined
    std::string name(int i) const;
    /// Test whether there is an entry \a n
    bool has(const std::string& n) const;
    /// Return the value type of entry \a n
    ValueType type(const std::string& n) const;
    /// Return the depth of entry \a n (0 for scalars)
    int depth(const std::string& n) const;
    //@}

    /// \name Values
    //@{
    /// Return the length of the array or row \a n at \a path
    int size(const std::string& n, const IntArgs& path = IntArgs()) const;
    /// Return the integer \a n at \a path
    long long int integer(const std::string& n, const IntArgs& path = IntArgs()) const;
    /// Return the number \a n at \a path (integers are converted)
    double number(const std::string& n, const IntArgs& path = IntArgs()) const;
    /// Return the string \a n at \a path
    std::string text(const std::string& n, const IntArgs& path = IntArgs()) const;
    /// Return all integers in the array or row \a n at \a path
    IntArgs ints(const std::string& n, const IntArgs& path = IntArgs()) const;
    /// Return all numbers in the array or row \a n at \a path
    ArgArray<double> numbers(const std::string& n, const IntArgs& path = IntArgs()) const;
    //@}

    /// \name Output
    //@{
    /**
     * \brief Print all entries as MiniZinc data
     *
     * Rectangular arrays of up to six dimensions are printed as MiniZinc
     * arrays, so the form depends on the data: rows that happen to have
     * equal lengths give a two-dimensional array. Other arrays are
     * printed as arrays of 1-tuples of arrays (MiniZinc 2.7 or later),
     * preceded by a comment with its type, for example
     * \code
     * % x: array[int] of tuple(array[int] of int)
     * x = [([5],), ([6, 7],)];
     * \endcode
     * where row \c i is \c x[i].1.
     */
    void dzn(std::ostream& os) const;
    //@}
  };

  /**
   * \brief Reader spec for a textual instance format
   *
   * \ingroup TaskInstance
   */
  class GECODE_INSTANCE_EXPORT Spec {
  private:
    /// The parsed spec
    std::shared_ptr<const SpecImpl> impl;
  public:
    /// The empty spec (reads only empty input)
    Spec(void);

    /// \name Creating specs
    //@{
    /// Parse spec \a text, using \a origin in error messages
    static Spec parse(const std::string& text, const std::string& origin = "<spec>");
    /// Load the spec in file \a path
    static Spec load(const std::string& path);
    //@}

    /// \name Information
    //@{
    /// Return the format name declared by the spec (may be empty)
    std::string name(void) const;
    //@}

    /// \name Reading
    //@{
    /**
     * \brief Read instance file \a path
     *
     * The parameters \a p give values for the spec's \c param
     * statements.
     */
    Data read(const std::string& path, const Params& p = Params()) const;
    /// Read instance \a text, using \a origin in error messages
    Data read_text(const std::string& text, const std::string& origin = "<input>", const Params& p = Params()) const;
    /**
     * \brief List the instances in file \a path
     *
     * The result has the integer \c instances, the number of instances in
     * the file (1 if the spec has no \c instances statement), the
     * string array \c name when every instance defines a \c name, and
     * the string array \c failed when an instance fails a check or
     * lacks or repeats a sections line: the first such failure of each
     * instance, or the empty string.
     */
    Data catalog(const std::string& path, const Params& p = Params()) const;
    /// List the instances in \a text, using \a origin in error messages
    Data catalog_text(const std::string& text, const std::string& origin = "<input>", const Params& p = Params()) const;
    //@}
  };

}}

#endif

// STATISTICS: instance-any
