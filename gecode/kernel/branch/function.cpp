/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *     Mikael Lagerkvist, 2008
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

#include <gecode/kernel.hh>

namespace Gecode { namespace Kernel {

  /// %Brancher for calling a function
  class FunctionBranch : public Brancher {
  protected:
    /// Minimal brancher description storing no information
    class Description : public Choice {
    public:
      /// Initialize description for brancher \a b, number of alternatives \a a.
      Description(const Brancher& b, unsigned int a);
      /// Archive into \a e
      virtual void archive(Archive& e) const;
    };
    /// Function to call
    SharedData<std::function<void(Space& home)>> f;
    /// Call function just once
    bool done;
    /// Construct brancher
    FunctionBranch(Home home, std::function<void(Space& home)> f0);
    /// Copy constructor
    FunctionBranch(Space& home, FunctionBranch& b);
  public:
    /// Check status of brancher, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& a);
    /// Perform commit
    virtual ExecStatus commit(Space& home, const Choice& ch, unsigned int a);
    /// Print explanation
    virtual void print(const Space&, const Choice&, unsigned int,
                       std::ostream& o) const;
    /// Copy brancher
    virtual Actor* copy(Space& home);
    /// Post brancher
    static void post(Home home, std::function<void(Space& home)> f);
    /// Dispose brancher
    virtual size_t dispose(Space& home);
  };

  forceinline
  FunctionBranch::Description::Description(const Brancher& b, unsigned int a)
    : Choice(b,a) {}
  void
  FunctionBranch::Description::archive(Archive& e) const {
    Choice::archive(e);
  }

  forceinline
  FunctionBranch::FunctionBranch(Home home,
                                 std::function<void(Space& home)> f0)
    : Brancher(home), f(f0), done(false) {
    if (!f())
      throw InvalidFunction("FunctionBranch::FunctionBranch");
   home.notice(*this,AP_DISPOSE);
  }
  forceinline
  FunctionBranch::FunctionBranch(Space& home, FunctionBranch& b)
    : Brancher(home,b), f(b.f), done(b.done) {
  }
  bool
  FunctionBranch::status(const Space&) const {
    return !done;
  }
  const Choice*
  FunctionBranch::choice(Space&) {
    assert(!done);
    return new Description(*this,1);
  }
  const Choice*
  FunctionBranch::choice(const Space&, Archive&) {
    return new Description(*this,1);
  }
  ExecStatus
  FunctionBranch::commit(Space& home, const Choice&, unsigned int) {
    done = true;
    GECODE_VALID_FUNCTION(f());
    f()(home);
    return home.failed() ? ES_FAILED : ES_OK;
  }
  void
  FunctionBranch::print(const Space&, const Choice&, unsigned int,
                        std::ostream& o) const {
    o << "FunctionBranch()";
  }
  Actor*
  FunctionBranch::copy(Space& home) {
    return new (home) FunctionBranch(home,*this);
  }
  forceinline void
  FunctionBranch::post(Home home, std::function<void(Space& home)> f) {
    if (!f)
      throw InvalidFunction("FunctionBranch::post");
    (void) new (home) FunctionBranch(home,f);
  }
  size_t
  FunctionBranch::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    f.~SharedData<std::function<void(Space& home)>>();
    (void) Brancher::dispose(home);
    return sizeof(*this);
  }

}}

namespace Gecode {

  void
  branch(Home home, std::function<void(Space& home)> f) {
    Kernel::FunctionBranch::post(home,f);
  }

}

// STATISTICS: kernel-branch
