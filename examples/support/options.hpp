/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
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
 * Option baseclass
 *
 */
inline
BaseOption::BaseOption(const char* o, const char* e)
  : opt(o), exp(e) {}
inline
BaseOption::~BaseOption(void) {
}


/*
 * String option
 *
 */
inline
StringOption::StringOption(const char* o, const char* e, int v)
  : BaseOption(o,e), cur(v), fst(NULL), lst(NULL) {}
inline void
StringOption::value(int v) {
  cur = v;
}
inline int
StringOption::value(void) const {
  return cur;
}

/*
 * Unsigned integer option
 *
 */
inline
UnsignedIntOption::UnsignedIntOption(const char* o, const char* e,
                                     unsigned int v)
  : BaseOption(o,e), cur(v) {}
inline void
UnsignedIntOption::value(unsigned int v) {
  cur = v;
}
inline unsigned int
UnsignedIntOption::value(void) const {
  return cur;
}


/*
 * Options
 *
 */
inline void
Options::add(BaseOption& o) {
  o.next = NULL;
  if (fst == NULL) {
    fst=&o;
  } else {
    lst->next=&o;
  }
  lst=&o;
}
inline const char*
Options::name(void) const {
  return _name;
}

/*
 * Model options
 *
 */
inline void
Options::model(int v) {
  _model.value(v);
}
inline void
Options::model(int v, const char* o, const char* h) {
  _model.add(v,o,h);
}
inline int
Options::model(void) const {
  return _model.value();
}

inline void
Options::propagation(int v) {
  _propagation.value(v);
}
inline void
Options::propagation(int v, const char* o, const char* h) {
  _propagation.add(v,o,h);
}
inline int
Options::propagation(void) const {
  return _propagation.value();
}

inline void
Options::icl(IntConLevel i) {
  _icl.value(i);
}
inline IntConLevel
Options::icl(void) const {
  return static_cast<IntConLevel>(_icl.value());
}

inline void
Options::branching(int v) {
  _branching.value(v);
}
inline void
Options::branching(int v, const char* o, const char* h) {
  _branching.add(v,o,h);
}
inline int
Options::branching(void) const {
  return _branching.value();
}

/*
 * Search options
 *
 */
inline void
Options::search(int v) {
  _search.value(v);
}
inline void
Options::search(int v, const char* o, const char* h) {
  _search.add(v,o,h);
}
inline int
Options::search(void) const {
  return _search.value();
}

inline void
Options::solutions(unsigned int n) {
  _solutions.value(n);
}
inline unsigned int
Options::solutions(void) const {
  return _solutions.value();
}

inline void
Options::c_d(unsigned int d) {
  _c_d.value(d);
}
inline unsigned int
Options::c_d(void) const {
  return _c_d.value();
}

inline void
Options::a_d(unsigned int d) {
  _a_d.value(d);
}
inline unsigned int
Options::a_d(void) const {
  return _a_d.value();
}

inline void
Options::node(unsigned int n) {
  _node.value(n);
}
inline unsigned int
Options::node(void) const {
  return _node.value();
}

inline void
Options::fail(unsigned int n) {
  _fail.value(n);
}
inline unsigned int
Options::fail(void) const {
  return _fail.value();
}

inline void
Options::time(unsigned int t) {
  _time.value(t);
}
inline unsigned int
Options::time(void) const {
  return _time.value();
}



/*
 * Execution options
 *
 */
inline void
Options::mode(ExampleMode em) {
  _mode.value(em);
}
inline ExampleMode
Options::mode(void) const {
  return static_cast<ExampleMode>(_mode.value());
}

inline void
Options::iterations(unsigned int i) {
  _iterations.value(i);
}
inline unsigned int
Options::iterations(void) const {
  return _iterations.value();
}

inline void
Options::samples(unsigned int s) {
  _samples.value(s);
}
inline unsigned int
Options::samples(void) const {
  return _samples.value();
}

inline
Options::~Options(void) {}

/*
 * Options with additional size argument
 *
 */
inline void
SizeOptions::size(unsigned int s) {
  _size = s;
}
inline unsigned int
SizeOptions::size(void) const {
  return _size;
}

// STATISTICS: example-any
