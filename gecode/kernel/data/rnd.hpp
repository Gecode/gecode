/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <ctime>

namespace Gecode {

  /**
   * \brief Random number generator
   * \ingroup TaskModel
   */
  class Rnd : public SharedHandle {
  private:
    /// Implementation of generator
    class IMP : public SharedHandle::Object {
    protected:
      /// Mutex for locking
      GECODE_KERNEL_EXPORT static Support::Mutex m;
      /// The actual generator
      Support::RandomGenerator rg;
    public:
      /// Initialize generator with seed \a s
      IMP(unsigned int s);
      /// Return seed
      unsigned int seed(void) const;
      /// Set seed to \a s
      void seed(unsigned int s);
      /// Get number between zero and \a n
      unsigned int operator ()(unsigned int n);
      /// Delete implemenentation
      virtual ~IMP(void);
    };
    /// Set the current seed to \a s (initializes if needed)
    void _seed(unsigned int s);
  public:
    /// Default constructor that does not initialize the generator
    GECODE_KERNEL_EXPORT
    Rnd(void);
    /// Initialize from generator \a r
    GECODE_KERNEL_EXPORT
    Rnd(const Rnd& r);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    Rnd& operator =(const Rnd& r);
    /// Destructor
    GECODE_KERNEL_EXPORT
    ~Rnd(void);
    /// Initialize with seed \a s
    GECODE_KERNEL_EXPORT
    Rnd(unsigned int s);
    /// Set the current seed to \a s (initializes if needed)
    GECODE_KERNEL_EXPORT
    void seed(unsigned int s);
    /// Set current seed based on time (initializes if needed)
    GECODE_KERNEL_EXPORT
    void time(void);
    /// Set current seed to hardware-based random number (initializes if needed)
    GECODE_KERNEL_EXPORT
    void hw(void);
    /// Return current seed
    unsigned int seed(void) const;
    /// Return a random integer from the interval [0..n)
    unsigned int operator ()(unsigned int n);
  };

  forceinline unsigned int
  Rnd::IMP::seed(void) const {
    unsigned int s;
    const_cast<Rnd::IMP&>(*this).m.acquire();
    s = rg.seed();
    const_cast<Rnd::IMP&>(*this).m.release();
    return s;
  }
  forceinline void
  Rnd::IMP::seed(unsigned int s) {
    m.acquire();
    rg.seed(s);
    m.release();
  }
  forceinline unsigned int
  Rnd::IMP::operator ()(unsigned int n) {
    unsigned int r;
    m.acquire();
    r=rg(n);
    m.release();
    return r;
  }

  forceinline unsigned int
  Rnd::seed(void) const {
    const IMP* i = static_cast<const IMP*>(object());
    return i->seed();
  }
  forceinline unsigned int
  Rnd::operator ()(unsigned int n) {
    IMP* i = static_cast<IMP*>(object());
    return (*i)(n);
  }

}

// STATISTICS: kernel-other
