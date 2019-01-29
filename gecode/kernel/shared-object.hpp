/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

namespace Gecode {

  /**
   * \brief The shared handle
   *
   * A shared handle provides access to an object that lives outside a space,
   * and is shared between entities that possibly reside inside different
   * spaces.
   *
   * This is the base class that all shared handles must inherit from.
   *
   */
  class SharedHandle {
  public:
    /**
     * \brief The shared object
     *
     * Shared objects must inherit from this base class.
     */
    class GECODE_KERNEL_EXPORT Object : public HeapAllocated {
      friend class SharedHandle;
    private:
      /// The counter used for reference counting
      Support::RefCount rc;
    public:
      /// Initialize
      Object(void);
      /// Delete shared object
      virtual ~Object(void);
    };
  private:
    /// The shared object
    Object* o;
    /// Subscribe handle to object
    void subscribe(void);
    /// Cancel subscription of handle to object
    void cancel(void);
  public:
    /// Create shared handle with no object pointing to
    SharedHandle(void);
    /// Create shared handle that points to shared object \a so
    SharedHandle(SharedHandle::Object* so);
    /// Copy constructor maintaining reference count
    SharedHandle(const SharedHandle& sh);
    /// Assignment operator maintaining reference count
    SharedHandle& operator =(const SharedHandle& sh);
    /// Destructor that maintains reference count
    ~SharedHandle(void);
    /// Whether handle points to an object
    explicit operator bool(void) const;
  protected:
    /// Access to the shared object
    SharedHandle::Object* object(void) const;
    /// Modify shared object
    void object(SharedHandle::Object* n);
  };


  forceinline
  SharedHandle::Object::Object(void)
    : rc(0) {}
  forceinline
  SharedHandle::Object::~Object(void) {
    assert(!rc);
  }


  forceinline SharedHandle::Object*
  SharedHandle::object(void) const {
    return o;
  }
  forceinline void
  SharedHandle::subscribe(void) {
    if (o != nullptr) o->rc.inc();
  }
  forceinline void
  SharedHandle::cancel(void) {
    if ((o != nullptr) && o->rc.dec())
      delete o;
    o=nullptr;
  }
  forceinline void
  SharedHandle::object(SharedHandle::Object* n) {
    if (n != o) {
      cancel(); o=n; subscribe();
    }
  }
  forceinline
  SharedHandle::SharedHandle(void) : o(nullptr) {}
  forceinline
  SharedHandle::SharedHandle(SharedHandle::Object* so) : o(so) {
    subscribe();
  }
  forceinline
  SharedHandle::SharedHandle(const SharedHandle& sh) : o(sh.o) {
    subscribe();
  }
  forceinline SharedHandle&
  SharedHandle::operator =(const SharedHandle& sh) {
    if (&sh != this) {
      cancel(); o=sh.o; subscribe();
    }
    return *this;
  }
  forceinline
  SharedHandle::operator bool(void) const {
    return o != nullptr;
  }
  forceinline
  SharedHandle::~SharedHandle(void) {
    cancel();
  }

}

// STATISTICS: kernel-memory
