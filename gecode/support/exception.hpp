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

#include <exception>

namespace Gecode {

  /**
   * \brief %Exception: Base-class for exceptions
   * \ingroup FuncThrow
   */
  class GECODE_SUPPORT_EXPORT Exception : public std::exception {
  private:
    /// Maximal length of information string
    static const int li_max = 127;
    /// Information string
    char li[li_max+1];
  public:
    /// Initialize with location \a l and information \a i
    Exception(const char* l, const char* i) throw ();
    /// Return information
    virtual const char* what(void) const throw();
  };


  /**
   * \defgroup FuncThrowSupport Support exceptions
   * \ingroup FuncThrow
   */

  //@{
  /// %Exception: %Memory exhausted
  class GECODE_SUPPORT_EXPORT MemoryExhausted : public Exception {
  public:
    /// Initialize
    MemoryExhausted(void);
  };
  /// %Exception: dynamic cast failed
  class GECODE_SUPPORT_EXPORT DynamicCastFailed : public Exception {
  public:
    /// Initialize with location \a l
    DynamicCastFailed(const char* l);
  };
  /// %Exception: operating system error
  class GECODE_SUPPORT_EXPORT OperatingSystemError : public Exception {
  public:
    /// Initialize with location \a l
    OperatingSystemError(const char* l);
  };
  //@}

}

// STATISTICS: support-any
