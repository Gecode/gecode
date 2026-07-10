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

#ifndef __FLATZINC_BLACKBOX_HH__
#define __FLATZINC_BLACKBOX_HH__

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <gecode/flatzinc.hh>
#include <gecode/kernel.hh>
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

#ifdef _WIN32
#define GECODE_BLACKBOX_CALL __stdcall
#else
#define GECODE_BLACKBOX_CALL
#endif

namespace Gecode {
namespace FlatZinc {

/// Abstract class implemented by different methods to run blackbox functions.
///
/// A blackbox function must be deterministic in the FlatZinc sense: the same
/// integer and float inputs must always produce the same integer and float
/// outputs. Implementations may keep internal caches or other private state,
/// provided that this state does not make the observable result depend on call
/// order.
class BlackBoxFn : public SharedHandle::Object {
public:
  virtual ~BlackBoxFn(void) {}
  virtual void run(const std::vector<int64_t> &int_in,
                   const std::vector<double> &float_in,
                   std::vector<int64_t> &int_out,
                   std::vector<double> &float_out) = 0;
};

/// Implementation of a black box function that dynamically loads a library and
/// run a contained function.
///
/// The native library entry points can be called concurrently by parallel
/// search workers and must therefore be thread-safe.
class BlackBoxLibrary : public BlackBoxFn {
public:
  BlackBoxLibrary(const std::string &name,
                  const std::vector<std::string> &args);
  ~BlackBoxLibrary();
  void run(const std::vector<int64_t> &int_in,
           const std::vector<double> &float_in,
           std::vector<int64_t> &int_out,
           std::vector<double> &float_out) override;

protected:
  void *library;
  void(GECODE_BLACKBOX_CALL *library_fzn_blackbox)(
      const int64_t *, size_t, const double *, size_t, int64_t *, size_t,
      double *, size_t);
};

/// Implementation of a blackbox function that starts a separate process to
/// repeatedly run a blackbox function, communicating over standard I/O.
///
/// Parallel search workers do not share a process stream: each calling thread
/// gets its own persistent process session, created lazily and reused by that
/// thread until the shared blackbox object is destroyed.
class BlackBoxExec : public BlackBoxFn {
public:
  BlackBoxExec(const std::string &program, const std::vector<std::string> &args);
  ~BlackBoxExec();
  void run(const std::vector<int64_t> &int_in,
           const std::vector<double> &float_in, std::vector<int64_t> &int_out,
           std::vector<double> &float_out) override;

protected:
  class Session;

  /// The executable to run for each worker-thread session.
  std::string program;
  /// Arguments passed to each executable session.
  std::vector<std::string> args;
  /// Mutex protecting the session table.
  Support::Mutex mutex;
  /// One persistent process session for each calling thread.
  std::vector<Session *> sessions;

  Session &session(void);
};

class BlackBoxHandle : public SharedHandle {
public:
  BlackBoxHandle(BlackBoxFn *fn) : SharedHandle() { object(fn); }
  BlackBoxHandle(const BlackBoxHandle &handle) : SharedHandle(handle) {}
  BlackBoxHandle &operator=(const BlackBoxHandle &handle) {
    return static_cast<BlackBoxHandle &>(SharedHandle::operator=(handle));
  }
  BlackBoxFn *operator()() { return static_cast<BlackBoxFn *>(object()); };
};

class BlackBox : public Propagator {
protected:
  /// Integer variables considered as the integer input to the blackbox function
  ViewArray<Int::IntView> int_input;
  /// Integer variables set to the integer output of the blackbox function
  ViewArray<Int::IntView> int_output;

#ifdef GECODE_HAS_FLOAT_VARS
  /// Floating-point variables considered as the floating-point input to the
  /// blackbox function
  ViewArray<Float::FloatView> float_input;
  /// Floating-point variables set to the floating-point output of the blackbox
  /// function
  ViewArray<Float::FloatView> float_output;
#endif

  /// Handle to the implementation of the blackbox function
  ///
  /// The handle ensures that the function implementation can be shared between
  /// copies of the propagator.
  BlackBoxHandle black_box;

  /// Constructor for cloning \a p
  BlackBox(Space &home, BlackBox &p)
      : Propagator(home, p), black_box(p.black_box) {
    int_input.update(home, p.int_input);
    int_output.update(home, p.int_output);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.update(home, p.float_input);
    float_output.update(home, p.float_output);
#endif
  }

public:
  /// Constructor for creation
  BlackBox(Home home, ViewArray<Int::IntView> &int_in,
           ViewArray<Int::IntView> &int_out,
#ifdef GECODE_HAS_FLOAT_VARS
           ViewArray<Float::FloatView> &float_in,
           ViewArray<Float::FloatView> &float_out,
#endif
           const BlackBoxHandle &black_box0)
      : Propagator(home), int_input(int_in), int_output(int_out),
#ifdef GECODE_HAS_FLOAT_VARS
        float_input(float_in), float_output(float_out),
#endif
        black_box(black_box0) {
    int_input.subscribe(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.subscribe(home, *this, Float::PC_FLOAT_VAL);
#endif
    home.notice(*this, AP_DISPOSE);
  }
  /// Cost function (defined as exponential)
  PropCost cost(const Space &home, const ModEventDelta &med) const override {
    return PropCost::crazy(PropCost::HI, int_input.size()
#ifdef GECODE_HAS_FLOAT_VARS
    + float_input.size()
#endif
    );
  };
  /// Schedule function
  void reschedule(Space &home) override {
    int_input.reschedule(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.reschedule(home, *this, Float::PC_FLOAT_VAL);
#endif
  }
  /// Delete propagator and return its size
  size_t dispose(Space &home) override {
    int_input.cancel(home, *this, Int::PC_INT_VAL);
#ifdef GECODE_HAS_FLOAT_VARS
    float_input.cancel(home, *this, Float::PC_FLOAT_VAL);
#endif
    home.ignore(*this, AP_DISPOSE);
    black_box.~BlackBoxHandle();
    (void)Propagator::dispose(home);
    return sizeof(*this);
  };

  ExecStatus propagate(Space &home, const ModEventDelta &) override;

  Propagator *copy(Space &home) override {
    return new (home) BlackBox(home, *this);
  }

  static ExecStatus post(Home home, ViewArray<Int::IntView> &int_input,
                         ViewArray<Int::IntView> &int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView> &float_input,
                         ViewArray<Float::FloatView> &float_output,
#endif
                         const std::string &mode,
                         const std::string &instantiation,
                         const std::vector<std::string> &args) {
    BlackBoxFn *black_box(nullptr);
    if (mode == "dll") {
      black_box = new BlackBoxLibrary(instantiation, args);
    } else if (mode == "exec") {
      black_box = new BlackBoxExec(instantiation, args);
    } else {
      throw Error("Blackbox", "Unknown blackbox protocol `" + mode + "'");
    }

    BlackBoxHandle black_box_handle(black_box);
    new (home) BlackBox(home, int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
                        float_input, float_output,
#endif
                        black_box_handle);
    return ES_OK;
  }
};

class BlackBoxBounds : public Propagator {
protected:
  /// Integer variables whose bounds are input and computed by the blackbox
  /// function, in order.
  ViewArray<Int::IntView> ivar;

#ifdef GECODE_HAS_FLOAT_VARS
  /// Floating-point variables whose bounds are input and computed by the
  /// blackbox function, in order.
  ViewArray<Float::FloatView> fvar;
#endif

  /// For each variable in \a ivar, whether the propagator depends on its bounds
  /// (derived from the reason). Only marked variables are subscribed, so the
  /// propagator is scheduled precisely when one of the relevant bounds changes.
  ///
  /// The marking is constant during search and is shared between all copies of
  /// the propagator.
  SharedArray<bool> sub_int;
#ifdef GECODE_HAS_FLOAT_VARS
  /// For each variable in \a fvar, whether the propagator depends on its bounds.
  SharedArray<bool> sub_float;
#endif

  /// Handle to the implementation of the blackbox function
  ///
  /// The handle ensures that the function implementation can be shared between
  /// copies of the propagator.
  BlackBoxHandle black_box;

  /// Constructor for cloning \a p
  BlackBoxBounds(Space &home, BlackBoxBounds &p)
      : Propagator(home, p), sub_int(p.sub_int),
#ifdef GECODE_HAS_FLOAT_VARS
        sub_float(p.sub_float),
#endif
        black_box(p.black_box) {
    ivar.update(home, p.ivar);
#ifdef GECODE_HAS_FLOAT_VARS
    fvar.update(home, p.fvar);
#endif
  }

public:
  /// Constructor for creation
  BlackBoxBounds(Home home, ViewArray<Int::IntView> &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
           ViewArray<Float::FloatView> &fvar,
#endif
           SharedArray<bool> sub_int0,
#ifdef GECODE_HAS_FLOAT_VARS
           SharedArray<bool> sub_float0,
#endif
           const BlackBoxHandle &black_box0)
      : Propagator(home), ivar(ivar),
#ifdef GECODE_HAS_FLOAT_VARS
        fvar(fvar),
#endif
        sub_int(sub_int0),
#ifdef GECODE_HAS_FLOAT_VARS
        sub_float(sub_float0),
#endif
        black_box(black_box0) {
    for (int i = 0; i < ivar.size(); i++) {
      if (sub_int[i]) {
        ivar[i].subscribe(home, *this, Int::PC_INT_BND);
      }
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++) {
      if (sub_float[i]) {
        fvar[i].subscribe(home, *this, Float::PC_FLOAT_BND);
      }
    }
#endif
    home.notice(*this, AP_DISPOSE);
  }
  /// Cost function (defined as exponential)
  PropCost cost(const Space &home, const ModEventDelta &med) const override {
    return PropCost::crazy(PropCost::HI, ivar.size()
#ifdef GECODE_HAS_FLOAT_VARS
    + fvar.size()
#endif
    );
  };
  /// Schedule function
  void reschedule(Space &home) override {
    for (int i = 0; i < ivar.size(); i++) {
      if (sub_int[i]) {
        ivar[i].reschedule(home, *this, Int::PC_INT_BND);
      }
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++) {
      if (sub_float[i]) {
        fvar[i].reschedule(home, *this, Float::PC_FLOAT_BND);
      }
    }
#endif
  }
  /// Delete propagator and return its size
  size_t dispose(Space &home) override {
    for (int i = 0; i < ivar.size(); i++) {
      if (sub_int[i]) {
        ivar[i].cancel(home, *this, Int::PC_INT_BND);
      }
    }
#ifdef GECODE_HAS_FLOAT_VARS
    for (int i = 0; i < fvar.size(); i++) {
      if (sub_float[i]) {
        fvar[i].cancel(home, *this, Float::PC_FLOAT_BND);
      }
    }
#endif
    home.ignore(*this, AP_DISPOSE);
    black_box.~BlackBoxHandle();
    sub_int.~SharedArray<bool>();
#ifdef GECODE_HAS_FLOAT_VARS
    sub_float.~SharedArray<bool>();
#endif
    (void)Propagator::dispose(home);
    return sizeof(*this);
  };

  ExecStatus propagate(Space &home, const ModEventDelta &) override;

  Propagator *copy(Space &home) override {
    return new (home) BlackBoxBounds(home, *this);
  }

  static ExecStatus post(Home home, ViewArray<Int::IntView> &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                         ViewArray<Float::FloatView> &fvar,
#endif
                         SharedArray<bool> sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                         SharedArray<bool> sub_float,
#endif
                         const std::string &mode,
                         const std::string &instantiation,
                         const std::vector<std::string> &args) {
    BlackBoxFn *black_box(nullptr);
    if (mode == "dll") {
      black_box = new BlackBoxLibrary(instantiation, args);
    } else if (mode == "exec") {
      black_box = new BlackBoxExec(instantiation, args);
    } else {
      throw Error("Blackbox", "Unknown blackbox protocol `" + mode + "'");
    }

    BlackBoxHandle black_box_handle(black_box);
    new (home) BlackBoxBounds(home, ivar,
#ifdef GECODE_HAS_FLOAT_VARS
                        fvar,
#endif
                        sub_int,
#ifdef GECODE_HAS_FLOAT_VARS
                        sub_float,
#endif
                        black_box_handle);
    return ES_OK;
  }
};

void blackbox(Home home, const IntVarArgs &int_in, const IntVarArgs &int_out,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &float_in, const FloatVarArgs &float_out,
#endif
              const std::string &mode, const std::string &instantiation,
              const std::vector<std::string> &args);

void blackbox_bounds(Home home, const IntVarArgs &ivar,
#ifdef GECODE_HAS_FLOAT_VARS
              const FloatVarArgs &fvar,
#endif
              const std::string &mode, const std::string &instantiation,
              const std::vector<std::string> &args,
              const std::vector<int> &reason);

} // namespace FlatZinc
} // namespace Gecode

#endif //__FLATZINC_BLACKBOX_HH__
