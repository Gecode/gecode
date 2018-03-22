/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

namespace Gecode { namespace Search {

  /**
   * \defgroup FuncThrowSearch %Search exceptions
   * \ingroup FuncThrow
   */
  //@{
  /// %Exception: Uninitialized cutoff for restart-based search
  class GECODE_SEARCH_EXPORT UninitializedCutoff : public Exception {
  public:
    /// Initialize with location \a l
    UninitializedCutoff(const char* l);
  };
  /// %Exception: No assets requested for portfolio-based search
  class GECODE_SEARCH_EXPORT NoAssets : public Exception {
  public:
    /// Initialize with location \a l
    NoAssets(const char* l);
  };
  /// %Exception: Mixed non-best and best solution search requested
  class GECODE_SEARCH_EXPORT MixedBest : public Exception {
  public:
    /// Initialize with location \a l
    MixedBest(const char* l);
  };
  /// %Exception: Best solution search is not supported
  class GECODE_SEARCH_EXPORT NoBest : public Exception {
  public:
    /// Initialize with location \a l
    NoBest(const char* l);
  };
  //@}
}}

// STATISTICS: search-other
