/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
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

#ifndef GECODE_FLATZINC_BLACKBOX_HH
#define GECODE_FLATZINC_BLACKBOX_HH

#include <exception>
#include <string>
#include <vector>

#include <gecode/flatzinc.hh>
#include <gecode/kernel.hh>
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

namespace Gecode {
namespace FlatZinc {

class BlackBoxContext;

/// Model-local context shared by blackbox propagators and search support.
class BlackBoxContextHandle : public SharedHandle {
public:
  BlackBoxContextHandle(void) : SharedHandle() {}
  BlackBoxContextHandle(const BlackBoxContextHandle& handle)
    : SharedHandle(handle) {}
  BlackBoxContextHandle& operator=(const BlackBoxContextHandle& handle) {
    return static_cast<BlackBoxContextHandle&>(
      SharedHandle::operator=(handle));
  }

  /// Initialize this context if it is empty.
  void init(void);
  /// Return the backend selected for one constraint as an opaque handle.
  SharedHandle backendForConstraint(const std::string& mode,
                                    const std::string& target,
                                    const std::vector<std::string>& args) const;
  /// Record the first exception raised during blackbox propagation.
  void fail(std::exception_ptr e) const;
  /// Whether blackbox propagation raised an exception.
  bool failed(void) const;
  /// Rethrow the first recorded propagation exception.
  void rethrow(void) const;
};

/// Access to the model-local blackbox context while posting and searching.
class BlackBoxAccess {
public:
  static BlackBoxContextHandle& context(FlatZincSpace& s);
};

void blackbox(Home home, BlackBoxContextHandle& context,
              const IntVarArgs& int_in, const IntVarArgs& int_out,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs& float_in, const FloatVarArgs& float_out,
#endif
              const std::string& mode, const std::string& target,
              const std::vector<std::string>& args);

void blackbox_bounds(Home home, BlackBoxContextHandle& context,
                     const IntVarArgs& ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                     const FloatVarArgs& fvar,
#endif
                     const std::string& mode, const std::string& target,
                     const std::vector<std::string>& args,
                     const std::vector<int>& reason);

} // namespace FlatZinc
} // namespace Gecode

#endif // GECODE_FLATZINC_BLACKBOX_HH
