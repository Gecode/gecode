/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
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

#include <gecode/flatzinc/blackbox.hh>
#include <gecode/flatzinc/blackbox-backend.hh>

#include <atomic>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

namespace Gecode {
namespace FlatZinc {

namespace {

class BlackBoxHandle : public SharedHandle {
public:
  explicit BlackBoxHandle(BlackBoxBackend* backend) : SharedHandle() {
    object(backend);
  }
  explicit BlackBoxHandle(const SharedHandle& handle) : SharedHandle(handle) {}
  BlackBoxHandle(const BlackBoxHandle& handle) : SharedHandle(handle) {}
  BlackBoxHandle& operator=(const BlackBoxHandle& handle) {
    return static_cast<BlackBoxHandle&>(SharedHandle::operator=(handle));
  }
  BlackBoxBackend* operator()(void) const {
    return static_cast<BlackBoxBackend*>(object());
  }
};

}

class BlackBoxContext : public SharedHandle::Object {
protected:
  class ExecEntry {
  public:
    std::string program;
    std::vector<std::string> args;
    BlackBoxHandle handle;
    ExecEntry(const std::string &program0, const std::vector<std::string> &args0,
              const BlackBoxHandle &handle0)
        : program(program0), args(args0), handle(handle0) {}
  };
  mutable Support::Mutex mutex;
  std::vector<ExecEntry> exec;
  std::exception_ptr exception;
  std::atomic<bool> error_recorded;

public:
  BlackBoxContext(void) : error_recorded(false) {}
  BlackBoxHandle backendForConstraint(const std::string& mode,
                                      const std::string& target,
                                      const std::vector<std::string>& args);
  void fail(std::exception_ptr e);
  bool failed(void) const;
  void rethrow(void) const;
};

void
BlackBoxContextHandle::init(void) {
  if (!*this) {
    object(new BlackBoxContext);
  }
}

SharedHandle
BlackBoxContextHandle::backendForConstraint(
  const std::string& mode, const std::string& target,
  const std::vector<std::string>& args) const {
  return static_cast<BlackBoxContext*>(object())
    ->backendForConstraint(mode, target, args);
}

void
BlackBoxContextHandle::fail(std::exception_ptr e) const {
  static_cast<BlackBoxContext*>(object())->fail(e);
}

bool
BlackBoxContextHandle::failed(void) const {
  return static_cast<bool>(*this) &&
    static_cast<BlackBoxContext*>(object())->failed();
}

void
BlackBoxContextHandle::rethrow(void) const {
  if (*this) {
    static_cast<BlackBoxContext*>(object())->rethrow();
  }
}

BlackBoxHandle
BlackBoxContext::backendForConstraint(const std::string& mode,
                                      const std::string& target,
                                      const std::vector<std::string>& args) {
  if (mode == "exec") {
    Support::Lock lock(mutex);
    for (const ExecEntry &e : exec) {
      if ((e.program == target) && (e.args == args)) {
        return e.handle;
      }
    }
    BlackBoxHandle handle(new BlackBoxExec(target, args));
    exec.push_back(ExecEntry(target, args, handle));
    return handle;
  }
  if (mode == "dll") {
    return BlackBoxHandle(new BlackBoxLibrary(target, args));
  }
  throw Error("Blackbox", "Unknown blackbox protocol `" + mode + "'");
}

void
BlackBoxContext::fail(std::exception_ptr e) {
  Support::Lock lock(mutex);
  if (!error_recorded.load(std::memory_order_relaxed)) {
    exception = e;
    error_recorded.store(true, std::memory_order_release);
  }
}

bool
BlackBoxContext::failed(void) const {
  return error_recorded.load(std::memory_order_acquire);
}

void
BlackBoxContext::rethrow(void) const {
  if (!error_recorded.load(std::memory_order_acquire)) {
    return;
  }
  std::exception_ptr e;
  {
    Support::Lock lock(mutex);
    e = exception;
  }
  if (e != nullptr) {
    std::rethrow_exception(e);
  }
}

namespace {

class BlackBox : public Propagator {
protected:
  ViewArray<Int::IntView> int_input;
  ViewArray<Int::IntView> int_output;
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> float_input;
  ViewArray<Float::FloatView> float_output;
#endif
  BlackBoxHandle backend;
  BlackBoxContextHandle context;

  BlackBox(Space& home, BlackBox& p)
    : Propagator(home, p), backend(p.backend), context(p.context) {
    int_input.update(home, p.int_input);
    int_output.update(home, p.int_output);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.update(home, p.float_input);
    float_output.update(home, p.float_output);
#endif
  }

public:
  BlackBox(Home home, ViewArray<Int::IntView>& int_in,
           ViewArray<Int::IntView>& int_out,
#ifdef GECODE_HAS_FLOAT_VARS
           ViewArray<Float::FloatView>& float_in,
           ViewArray<Float::FloatView>& float_out,
#endif
           const BlackBoxHandle& backend0,
           const BlackBoxContextHandle& context0)
    : Propagator(home), int_input(int_in), int_output(int_out),
#ifdef GECODE_HAS_FLOAT_VARS
      float_input(float_in), float_output(float_out),
#endif
      backend(backend0), context(context0) {
    int_input.subscribe(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.subscribe(home, *this, Float::PC_FLOAT_VAL);
#endif
    home.notice(*this, AP_DISPOSE);
  }

  PropCost cost(const Space&, const ModEventDelta&) const override {
    return PropCost::crazy(PropCost::HI, int_input.size()
#ifdef GECODE_HAS_FLOAT_VARS
      + float_input.size()
#endif
    );
  }

  void reschedule(Space& home) override {
    int_input.reschedule(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.reschedule(home, *this, Float::PC_FLOAT_VAL);
#endif
  }

  size_t dispose(Space& home) override {
    int_input.cancel(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.cancel(home, *this, Float::PC_FLOAT_VAL);
#endif
    home.ignore(*this, AP_DISPOSE);
    backend.~BlackBoxHandle();
    context.~BlackBoxContextHandle();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  ExecStatus propagate(Space& home, const ModEventDelta&) override;

  Propagator* copy(Space& home) override {
    return new (home) BlackBox(home, *this);
  }

  static ExecStatus post(Home home, ViewArray<Int::IntView>& int_input,
                         ViewArray<Int::IntView>& int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView>& float_input,
                         ViewArray<Float::FloatView>& float_output,
#endif
                         const BlackBoxContextHandle& context,
                         const std::string& mode, const std::string& target,
                         const std::vector<std::string>& args);
};

class BlackBoxBounds : public Propagator {
protected:
  ViewArray<Int::IntView> ivar;
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> fvar;
#endif
  SharedArray<bool> sub_int;
#ifdef GECODE_HAS_FLOAT_VARS
  SharedArray<bool> sub_float;
#endif
  BlackBoxHandle backend;
  BlackBoxContextHandle context;

  BlackBoxBounds(Space& home, BlackBoxBounds& p)
    : Propagator(home, p), sub_int(p.sub_int),
#ifdef GECODE_HAS_FLOAT_VARS
      sub_float(p.sub_float),
#endif
      backend(p.backend), context(p.context) {
    ivar.update(home, p.ivar);
#ifdef GECODE_HAS_FLOAT_VARS
    fvar.update(home, p.fvar);
#endif
  }

public:
  BlackBoxBounds(Home home, ViewArray<Int::IntView>& ivar0,
#ifdef GECODE_HAS_FLOAT_VARS
                 ViewArray<Float::FloatView>& fvar0,
#endif
                 SharedArray<bool> sub_int0,
#ifdef GECODE_HAS_FLOAT_VARS
                 SharedArray<bool> sub_float0,
#endif
                 const BlackBoxHandle& backend0,
                 const BlackBoxContextHandle& context0)
    : Propagator(home), ivar(ivar0),
#ifdef GECODE_HAS_FLOAT_VARS
      fvar(fvar0),
#endif
      sub_int(sub_int0),
#ifdef GECODE_HAS_FLOAT_VARS
      sub_float(sub_float0),
#endif
      backend(backend0), context(context0) {
    for (int i = 0; i < ivar.size(); i++)
      if (sub_int[i])
        ivar[i].subscribe(home, *this, Int::PC_INT_BND);
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++)
      if (sub_float[i])
        fvar[i].subscribe(home, *this, Float::PC_FLOAT_BND);
#endif
    home.notice(*this, AP_DISPOSE);
    home.notice(*this, AP_WEAKLY);
  }

  PropCost cost(const Space&, const ModEventDelta&) const override {
    return PropCost::crazy(PropCost::HI, ivar.size()
#ifdef GECODE_HAS_FLOAT_VARS
      + fvar.size()
#endif
    );
  }

  void reschedule(Space& home) override {
    for (int i = 0; i < ivar.size(); i++)
      if (sub_int[i])
        ivar[i].reschedule(home, *this, Int::PC_INT_BND);
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++)
      if (sub_float[i])
        fvar[i].reschedule(home, *this, Float::PC_FLOAT_BND);
#endif
  }

  size_t dispose(Space& home) override {
    for (int i = 0; i < ivar.size(); i++)
      if (sub_int[i])
        ivar[i].cancel(home, *this, Int::PC_INT_BND);
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++)
      if (sub_float[i])
        fvar[i].cancel(home, *this, Float::PC_FLOAT_BND);
#endif
    home.ignore(*this, AP_DISPOSE);
    home.ignore(*this, AP_WEAKLY);
    backend.~BlackBoxHandle();
    context.~BlackBoxContextHandle();
    sub_int.~SharedArray<bool>();
#ifdef GECODE_HAS_FLOAT_VARS
    sub_float.~SharedArray<bool>();
#endif
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  ExecStatus propagate(Space& home, const ModEventDelta&) override;
  Propagator* copy(Space& home) override {
    return new (home) BlackBoxBounds(home, *this);
  }

  static ExecStatus evaluate(Home home, ViewArray<Int::IntView>& ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                             ViewArray<Float::FloatView>& fvar,
#endif
                             BlackBoxHandle& backend,
                             const BlackBoxContextHandle& context);
  static ExecStatus post(Home home, ViewArray<Int::IntView>& ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView>& fvar,
#endif
                         SharedArray<bool> sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                         SharedArray<bool> sub_float,
#endif
                         const BlackBoxContextHandle& context,
                         const std::string& mode, const std::string& target,
                         const std::vector<std::string>& args);
};

} // namespace

ExecStatus
BlackBox::post(Home home, ViewArray<Int::IntView>& int_input,
               ViewArray<Int::IntView>& int_output,
#ifdef GECODE_HAS_FLOAT_VARS
               ViewArray<Float::FloatView>& float_input,
               ViewArray<Float::FloatView>& float_output,
#endif
               const BlackBoxContextHandle& context,
               const std::string& mode, const std::string& target,
               const std::vector<std::string>& args) {
  BlackBoxHandle backend(context.backendForConstraint(mode, target, args));
  if ((int_input.size() == 0)
#ifdef GECODE_HAS_FLOAT_VARS
      && (float_input.size() == 0)
#endif
  ) {
    std::vector<int64_t> int_in;
    std::vector<int64_t> int_out(int_output.size());
    std::vector<double> float_in;
    std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
    float_out.resize(float_output.size());
#endif
    BlackBoxCall call = {int_in, float_in, int_out, float_out};
    backend()->run(call);
    for (int i = 0; i < int_output.size(); i++)
      if (me_failed(int_output[i].eq(home, static_cast<int>(int_out[i]))))
        return ES_FAILED;
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < float_output.size(); i++)
      if (me_failed(float_output[i].eq(home, float_out[i])))
        return ES_FAILED;
#endif
    return ES_OK;
  }

  new (home) BlackBox(home, int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                      float_input, float_output,
#endif
                      backend, context);
  return ES_OK;
}

ExecStatus
BlackBoxBounds::post(Home home, ViewArray<Int::IntView>& ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                     ViewArray<Float::FloatView>& fvar,
#endif
                     SharedArray<bool> sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                     SharedArray<bool> sub_float,
#endif
                     const BlackBoxContextHandle& context,
                     const std::string& mode, const std::string& target,
                     const std::vector<std::string>& args) {
  BlackBoxHandle backend(context.backendForConstraint(mode, target, args));
  bool has_subscription = false;
  for (int i = 0; i < ivar.size(); i++)
    has_subscription = has_subscription || sub_int[i];
#ifdef GECODE_HAS_FLOAT_VARS
  for (int i = 0; i < fvar.size(); i++)
    has_subscription = has_subscription || sub_float[i];
#endif
  if (!has_subscription)
    return evaluate(home, ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                    fvar,
#endif
                    backend, context);

  new (home) BlackBoxBounds(home, ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                            fvar,
#endif
                            sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                            sub_float,
#endif
                            backend, context);
  return ES_OK;
}

ExecStatus BlackBox::propagate(Space &home, const ModEventDelta &) {
  if (int_input.assigned()
#ifdef GECODE_HAS_FLOAT_VARS
      && float_input.assigned()
#endif
  ) {
    std::vector<int64_t> int_in(int_input.size());
    std::vector<int64_t> int_out(int_output.size());
    for (size_t i = 0; i < int_in.size(); i++) {
      int_in[i] = static_cast<int64_t>(int_input[i].val());
    }
    std::vector<double> float_in;
    std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
    float_in.resize(float_input.size());
    float_out.resize(float_output.size());
    for (size_t i = 0; i < float_in.size(); i++) {
      float_in[i] = float_input[i].val().med();
    }
#endif

    try {
      BlackBoxCall call = {int_in, float_in, int_out, float_out};
      backend()->run(call);
    } catch (...) {
      context.fail(std::current_exception());
      return ES_FAILED;
    }

    for (size_t i = 0; i < int_out.size(); i++) {
      GECODE_ME_CHECK(int_output[i].eq(home, static_cast<int>(int_out[i])));
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (size_t i = 0; i < float_out.size(); i++) {
      GECODE_ME_CHECK(float_output[i].eq(home, float_out[i]));
    }
#endif

    return home.ES_SUBSUMED(*this);
  }
  return ES_FIX;
}

ExecStatus
BlackBoxBounds::evaluate(Home home, ViewArray<Int::IntView> &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView> &fvar,
#endif
                         BlackBoxHandle& backend,
                         const BlackBoxContextHandle& context) {
  std::vector<int64_t> int_in(ivar.size() * 2);
  std::vector<int64_t> int_out(ivar.size() * 2);
  for (int i = 0; i < ivar.size(); i++) {
    int_in[i*2] = static_cast<int64_t>(ivar[i].min());
    int_in[i*2+1] = static_cast<int64_t>(ivar[i].max());
  }
  std::vector<double> float_in;
  std::vector<double> float_out;
#ifdef GECODE_HAS_FLOAT_VARS
  float_in.resize(fvar.size() * 2);
  float_out.resize(fvar.size() * 2);
  for (int i = 0; i < fvar.size(); i++) {
    float_in[i*2] = fvar[i].min();
    float_in[i*2+1] = fvar[i].max();
  }
#endif

  try {
    BlackBoxCall call = {int_in, float_in, int_out, float_out};
    backend()->run(call);
  } catch (...) {
    context.fail(std::current_exception());
    return ES_FAILED;
  }

  for (int i = 0; i < ivar.size(); i++) {
    if (me_failed(ivar[i].gq(home, static_cast<int>(int_out[i*2]))) ||
        me_failed(ivar[i].lq(home, static_cast<int>(int_out[i*2+1])))) {
      return ES_FAILED;
    }
  }
#ifdef GECODE_HAS_FLOAT_VARS
  for (int i = 0; i < fvar.size(); i++) {
    if (me_failed(fvar[i].gq(home, float_out[i*2])) ||
        me_failed(fvar[i].lq(home, float_out[i*2+1]))) {
      return ES_FAILED;
    }
  }
#endif

  return ES_OK;
}

ExecStatus BlackBoxBounds::propagate(Space &home, const ModEventDelta &) {
  ExecStatus es = evaluate(home, ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                           fvar,
#endif
                           backend, context);
  return (es == ES_OK) ? ES_NOFIX : es;
}

void blackbox(Home home, BlackBoxContextHandle& context,
              const IntVarArgs &int_in, const IntVarArgs &int_out,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &float_in, const FloatVarArgs &float_out,
#endif
              const std::string &mode, const std::string &target,
              const std::vector<std::string> &args) {
  ViewArray<Int::IntView> int_input(home, int_in);
  ViewArray<Int::IntView> int_output(home, int_out);
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> float_input(home, float_in);
  ViewArray<Float::FloatView> float_output(home, float_out);
#endif

  if (home.failed())
    return;
  context.init();
  PostInfo pi(home);
  ExecStatus es = BlackBox::post(home, int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_input, float_output,
#endif
                                 context, mode, target, args);
  GECODE_ES_FAIL(es);
}

/// Parse the flat reason and mark, per channel, the variables whose bounds the
/// propagator depends on (the variables that appear as literals in any reason).
/// \a sub_int / \a sub_float are filled with one boolean per variable. Variable
/// indices in the reason are 1-based over the combined variable list, integer
/// variables first, then float variables.
///
/// The flat reason is a concatenation of one entry per variable, each entry
/// being `[idx, |R_lb|, (var, bnd)..., |R_ub|, (var, bnd)...]`.
static void reason_subscriptions(const std::vector<int> &reason, int n_int,
                                 int n_float, SharedArray<bool> &sub_int,
                                 SharedArray<bool> &sub_float) {
  for (int i = 0; i < n_int; i++) {
    sub_int[i] = false;
  }
  for (int i = 0; i < n_float; i++) {
    sub_float[i] = false;
  }

  const int n_total = n_int + n_float;
  std::vector<bool> explained(n_total, false);
  size_t pos = 0;
  auto require = [&](size_t n, const char *what) {
    if (reason.size() - pos < n) {
      throw Error("Blackbox", std::string("Malformed blackbox bounds reason: ") +
                                  what + ".");
    }
  };
  while (pos < reason.size()) {
    require(1, "missing explained variable index");
    int idx = reason[pos++];
    if ((idx < 1) || (idx > n_total)) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: explained variable index "
                  "is out of range.");
    }
    if (explained[idx - 1]) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: duplicate explained "
                  "variable index.");
    }
    explained[idx - 1] = true;
    for (int side = 0; side < 2; side++) { // lower- then upper-bound literals
      require(1, "missing reason literal count");
      int count = reason[pos++];
      if (count < 0) {
        throw Error("Blackbox",
                    "Malformed blackbox bounds reason: negative literal count.");
      }
      if (static_cast<size_t>(count) > (reason.size() - pos) / 2) {
        throw Error("Blackbox",
                    "Malformed blackbox bounds reason: truncated reason "
                    "literals.");
      }
      for (int k = 0; k < count; k++) {
        int var = reason[pos++]; // 1-based combined variable index
        int bnd = reason[pos++];
        if (var >= 1 && var <= n_int) {
          sub_int[var - 1] = true;
        } else if (var > n_int && var <= n_int + n_float) {
          sub_float[var - 1 - n_int] = true;
        } else {
          throw Error("Blackbox",
                      "Malformed blackbox bounds reason: dependency variable "
                      "index is out of range.");
        }
        if (bnd != 1 && bnd != 2) { // MiniZinc PropBnd: PR_LB, PR_UB
          throw Error("Blackbox",
                      "Malformed blackbox bounds reason: dependency bound "
                      "code is out of range.");
        }
      }
    }
  }
  for (int i = 0; i < n_total; i++) {
    if (!explained[i]) {
      throw Error("Blackbox",
                  "Malformed blackbox bounds reason: missing explained "
                  "variable entry.");
    }
  }
}

void blackbox_bounds(Home home, BlackBoxContextHandle& context,
                     const IntVarArgs &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &fvar,
#endif
              const std::string &mode, const std::string &target,
              const std::vector<std::string> &args,
              const std::vector<int> &reason) {
  ViewArray<Int::IntView> int_var(home, ivar);
#ifdef GECODE_HAS_FLOAT_VARS
  ViewArray<Float::FloatView> float_var(home, fvar);
  int n_float = fvar.size();
#else
  int n_float = 0;
#endif

  // Determine which variables the propagator depends on, so it is only
  // subscribed (and thus scheduled) on the bounds mentioned in the reason. The
  // marking is constant and shared between all copies of the propagator.
  SharedArray<bool> sub_int(ivar.size());
  SharedArray<bool> sub_float(n_float);
  reason_subscriptions(reason, ivar.size(), n_float, sub_int, sub_float);

  if (home.failed())
    return;
  context.init();
  PostInfo pi(home);
  ExecStatus es = BlackBoxBounds::post(home, int_var,
#ifdef GECODE_HAS_FLOAT_VARS
                                 float_var,
#endif
                                 sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                                 sub_float,
#endif
                                 context, mode, target, args);
  GECODE_ES_FAIL(es);
}

} // namespace FlatZinc
} // namespace Gecode
