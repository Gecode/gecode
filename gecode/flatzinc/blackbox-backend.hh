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

#ifndef GECODE_FLATZINC_BLACKBOX_BACKEND_HH
#define GECODE_FLATZINC_BLACKBOX_BACKEND_HH

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <gecode/flatzinc.hh>
#include <gecode/kernel.hh>
#ifdef GECODE_HAS_THREADS
#include <thread>
#endif

#ifdef _WIN32
#define GECODE_BLACKBOX_CALL __stdcall
#else
#define GECODE_BLACKBOX_CALL
#endif

namespace Gecode {
namespace FlatZinc {

/// Inputs and pre-sized output buffers for one backend call.
struct BlackBoxCall {
  const std::vector<int64_t>& int_input;
  const std::vector<double>& float_input;
  std::vector<int64_t>& int_output;
  std::vector<double>& float_output;
};

/// Backend for a deterministic FlatZinc blackbox function.
class BlackBoxBackend : public SharedHandle::Object {
public:
  virtual ~BlackBoxBackend(void) {}
  virtual void run(BlackBoxCall& call) = 0;
  void run(const std::vector<int64_t>& int_input,
           const std::vector<double>& float_input,
           std::vector<int64_t>& int_output,
           std::vector<double>& float_output) {
    BlackBoxCall call = {int_input, float_input, int_output, float_output};
    run(call);
  }
};

/// Dynamic-library backend owned by one blackbox constraint.
class GECODE_FLATZINC_EXPORT BlackBoxLibrary : public BlackBoxBackend {
public:
  using BlackBoxBackend::run;
  BlackBoxLibrary(const std::string& name,
                  const std::vector<std::string>& args);
  ~BlackBoxLibrary();
  void run(BlackBoxCall& call) override;

protected:
  void* library;
  void* (GECODE_BLACKBOX_CALL *library_fzn_init)(const char**, size_t);
  void* (GECODE_BLACKBOX_CALL *library_fzn_clone)(void*);
  void (GECODE_BLACKBOX_CALL *library_fzn_blackbox)(
    void*, const int64_t*, size_t, const double*, size_t, int64_t*, size_t,
    double*, size_t);
  void (GECODE_BLACKBOX_CALL *library_fzn_free)(void*);
  void* root_instance;

#ifdef GECODE_HAS_THREADS
  class Instance;
  Support::Mutex mutex;
  std::vector<Instance*> instances;
  Instance* instance(void);
#endif
};

/// Persistent-process backend shared by equal executable configurations.
class GECODE_FLATZINC_EXPORT BlackBoxExec : public BlackBoxBackend {
public:
  using BlackBoxBackend::run;
  BlackBoxExec(const std::string& program,
               const std::vector<std::string>& args);
  ~BlackBoxExec();
  void run(BlackBoxCall& call) override;

protected:
  class Session;
  std::string program;
  std::vector<std::string> args;
  Support::Mutex mutex;
  std::vector<Session*> sessions;
  Session& session(void);
};

/// Encode one request for the executable backend's line protocol.
GECODE_FLATZINC_EXPORT
std::string encode_blackbox_request(const BlackBoxCall& call);
/// Decode and validate one response from the executable backend.
GECODE_FLATZINC_EXPORT
void decode_blackbox_response(const std::string& response,
                              BlackBoxCall& call);

} // namespace FlatZinc
} // namespace Gecode

#endif // GECODE_FLATZINC_BLACKBOX_BACKEND_HH
