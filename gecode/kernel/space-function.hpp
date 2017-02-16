/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <functional>
#include <iostream>
#include <sstream>

namespace Gecode {

  /// Shared function taking a single argument of type \a Space&
  class SpaceFunction : public SharedData<std::function<void(Space&)> > {
  public:
    /// Initialize with function \a f
    SpaceFunction(std::function<void(Space&)> f);
    /// Create as uninitalized
    SpaceFunction(void);
    /// Copy constructor
    SpaceFunction(const SpaceFunction& sf);
    /// Assignment operator
    SpaceFunction& operator =(const SpaceFunction& sf);
    /// Call function
    void operator ()(Space& s) const;
    /// Return function
    const std::function<void(Space&)>& function(void) const;
  };

  /**
   * \brief Print function
   * \relates SpaceFunction
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const SpaceFunction& sf);


  forceinline
  SpaceFunction::SpaceFunction(std::function<void(Space&)> f) 
    : SharedData<std::function<void(Space&)> >(f) {
    if (!f)
      throw InvalidFunction("SpaceFunction::SpaceFunction");
  }

  forceinline
  SpaceFunction::SpaceFunction(void) {}

  forceinline
  SpaceFunction::SpaceFunction(const SpaceFunction& sd)
    : SharedData<std::function<void(Space&)> >(sd) {}

  forceinline SpaceFunction&
  SpaceFunction::operator =(const SpaceFunction& sd) {
    return static_cast<SpaceFunction&>
      (SharedData<std::function<void(Space&)> >::operator =(sd));
  }

  forceinline void
  SpaceFunction::operator ()(Space& home) const {
    GECODE_ASSUME(SharedData<std::function<void(Space&)>>::operator ()());
    SharedData<std::function<void(Space&)> >::operator ()()(home);
  }

  forceinline const std::function<void(Space&)>&
  SpaceFunction::function(void) const {
    GECODE_ASSUME(SharedData<std::function<void(Space&)>>::operator ()());
    return SharedData<std::function<void(Space&)>>::operator ()();
  }


  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const SpaceFunction& sf) {
    return os << sf.function();
  }


}

// STATISTICS: kernel-other
