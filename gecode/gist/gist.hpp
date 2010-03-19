/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

namespace Gecode { namespace Gist {

  template<class S>
  Print<S>::Print(const std::string& name)
    : TextInspector(name) {}

  template<class S>
  void
  Print<S>::inspect(const Space& node) {
    init();
    addHtml("<pre>\n");
    dynamic_cast<const S&>(node).print(getStream());
    addHtml("</pre><hr />");
  }

  forceinline
  Options::Options(void) {} 

  forceinline
  Options::_I::_I(void) : _click(heap,1), n_click(0),
    _solution(heap,1), n_solution(0), _move(heap,1), n_move(0) {}

  forceinline void
  Options::_I::click(Inspector* i) {
    _click[n_click++] = i;
  }
  forceinline void
  Options::_I::solution(Inspector* i) {
    _solution[n_solution++] = i;
  }
  forceinline void
  Options::_I::move(Inspector* i) {
    _move[n_move++] = i;
  }
  forceinline Inspector*
  Options::_I::click(unsigned int i) const {
    return (i < n_click) ? _click[i] : NULL;
  }
  forceinline Inspector*
  Options::_I::solution(unsigned int i) const {
    return (i < n_solution) ? _solution[i] : NULL;
  }
  forceinline Inspector*
  Options::_I::move(unsigned int i) const {
    return (i < n_move) ? _move[i] : NULL;
  }

  inline int
  dfs(Space* root, const Gist::Options& opt) {
    return explore(root, false, opt);
  }

  inline int
  bab(Space* root, const Gist::Options& opt) {
    return Gist::explore(root, true, opt);
  }

}}

// STATISTICS: gist-any
