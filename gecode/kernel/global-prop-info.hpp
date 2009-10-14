/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date: 2009-09-08 21:10:29 +0200 (Tue, 08 Sep 2009) $ by $Author: schulte $
 *     $Revision: 9692 $
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

#include <algorithm>

namespace Gecode {

  /// Class for propagator information
  class PropInfo {
  private:
    /// Accumulated failure count
    double _afc;
  public:
    /// Initialize
    PropInfo(void);
    /// Initialize
    void init(void);
    /// Return accumulated failure count
    double afc(void) const;
    /// Increment failure count
    void fail(void);
  };

  /// Globally shared object for propagator information
  class GlobalPropInfo {
  private:
    /// Block of propagator information (of variable size)
    class Block {
    public:
      /// Next block
      Block* next;
      /// Start of information entries
      PropInfo pi[1];
    };
    /// Initial smallest number of entries per block
    static const unsigned int size_min = 32;
    /// Largest possible number of entries per block
    static const unsigned int size_max = 32 * 1024;
    /// The actual object to store the required information
    class Object {
    public:
      /// Mutex to synchronize globally shared access
      Support::Mutex m;
      /// How many spaces use this globally shared object
      unsigned int use_cnt;
      /// Currently used block
      Block* cur;
      /// Size of current block
      unsigned int size;
      /// Number of free entries in current block
      unsigned int free;
      /// Default constructor
      Object(void);
      /// Allocate memory from heap
      static void* operator new(size_t s);
      /// Free memory allocated from heap
      static void  operator delete(void* p);
    };
    /// The object
    Object* o;
  public:
    /// Initialize
    GlobalPropInfo(void);
    /// Copy during cloning
    GlobalPropInfo(const GlobalPropInfo& gpi);
    /// Destructor
    ~GlobalPropInfo(void);
    /// Allocate new propagator info
    PropInfo& allocate(void);
    /// Return copy during cloning
    GlobalPropInfo* copy(bool share);
    /// Release by one space
    bool release(void);
  };


  /*
   * Propagator information
   *
   */
  forceinline
  PropInfo::PropInfo(void)
    : _afc(0.0) {}
  forceinline void
  PropInfo::init(void) {
    _afc=0.0;
  }
  forceinline double
  PropInfo::afc(void) const {
    return _afc;
  }
  forceinline void
  PropInfo::fail(void) {
    _afc++;
  }


  /*
   * Global propagator information
   *
   */

  forceinline void*
  GlobalPropInfo::Object::operator new(size_t s) {
    return Gecode::heap.ralloc(s);
  }

  forceinline void
  GlobalPropInfo::Object::operator delete(void* p) {
    Gecode::heap.rfree(p);
  }

  forceinline
  GlobalPropInfo::Object::Object(void)
    : use_cnt(1), size(size_min), free(size_min) {
    cur = static_cast<Block*>(heap.ralloc(sizeof(Block)+
                                          (size-1)*sizeof(PropInfo)));
    cur->next = NULL;
  }

  forceinline
  GlobalPropInfo::GlobalPropInfo(void)
    : o(new Object) {
    // No synchronization needed as single thread is creating this object
  }

  forceinline
  GlobalPropInfo::GlobalPropInfo(const GlobalPropInfo& gpi)
    : o(gpi.o) {
    o->m.acquire();
    o->use_cnt++;
    o->m.release();
  }

  forceinline
  GlobalPropInfo::~GlobalPropInfo(void) {
    o->m.acquire();
    if (--o->use_cnt == 0) {
      o->m.release();
      // No synchronization needed as only one space and hence one thread left
      Block* b = o->cur;
      while (b != NULL) {
        Block* d = b; b=b->next;
        heap.rfree(d);
      }
      delete o;
    } else {
      o->m.release();
    }
  }

  forceinline PropInfo&
  GlobalPropInfo::allocate(void) {
    PropInfo* pi;
    o->m.acquire();
    if (o->free == 0) {
      o->size = std::min(2*o->size,size_max);
      o->free = o->size;
      Block* b = static_cast<Block*>(heap.ralloc(sizeof(Block)+
                                                 (o->size-1)*sizeof(PropInfo)));
      b->next = o->cur;
      o->cur = b;
    }
    pi = &o->cur->pi[--o->free];
    pi->init();
    o->m.release();
    return *pi;
  }

}

// STATISTICS: kernel-prop
