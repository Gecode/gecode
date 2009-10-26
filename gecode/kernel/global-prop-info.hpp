/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode {

  class GlobalPropInfo;

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
    void fail(GlobalPropInfo& gpi);
  };

  /// Globally shared object for propagator information
  class GlobalPropInfo {
    friend class PropInfo;
  private:
    /// Block of propagator information (of variable size)
    class Block {
    public:
      /// Next block
      Block* next;
      /// Start of information entries
      PropInfo pi[1];
      /// Allocate block with \a n entries and previous block \a p
      static Block* allocate(unsigned int n, Block* p=NULL);
    };
    /// Initial smallest number of entries per block
    static const unsigned int size_min = 32;
    /// Largest possible number of entries per block
    static const unsigned int size_max = 32 * 1024;
    /// The actual object to store the required information
    class Object {
    public:
      /// Mutex to synchronize globally shared access
      Support::Mutex* mutex;
      /// Link to previous object (NULL if none)
      Object* parent;
      /// How many spaces or objects use this object
      unsigned int use_cnt;
      /// Size of current block
      unsigned int size;
      /// Number of free entries in current block
      unsigned int free;
      /// Currently used block
      Block* cur;
      /// Constructor
      Object(Support::Mutex* m, Object* p=NULL);
      /// Allocate memory from heap
      static void* operator new(size_t s);
      /// Free memory allocated from heap
      static void  operator delete(void* p);
    };
    /// Pointer to object, possibly marked
    void* mo;
    /// Pointer to object without mark
    Object* object(void) const;
    /// Whether pointer points to local object
    bool local(void) const;
    /// Set pointer to local object
    void local(Object* o);
    /// Set global pointer to possibly marked object
    void global(void* mo);
  public:
    /// Initialize
    GlobalPropInfo(void);
    /// Copy during cloning
    GlobalPropInfo(const GlobalPropInfo& gpi);
    /// Destructor
    ~GlobalPropInfo(void);
    /// Allocate new propagator info
    PropInfo& allocate(void);
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

  forceinline GlobalPropInfo::Block*
  GlobalPropInfo::Block::allocate(unsigned int n, GlobalPropInfo::Block* p) {
    Block* b = static_cast<Block*>(heap.ralloc(sizeof(Block)+
                                               (n-1)*sizeof(PropInfo)));
    b->next = p;
    return b;
  }

  forceinline
  GlobalPropInfo::Object::Object(Support::Mutex* m, Object* p)
    : mutex(m), parent(p), use_cnt(1), size(size_min), free(size_min),
      cur(Block::allocate(size)) {}

  forceinline GlobalPropInfo::Object*
  GlobalPropInfo::object(void) const {
    return static_cast<Object*>(Support::funmark(mo));
  }
  forceinline bool
  GlobalPropInfo::local(void) const {
    return !Support::marked(mo);
  }
  forceinline void
  GlobalPropInfo::local(Object* o) {
    assert(!Support::marked(o));
    mo = o;
  }
  forceinline void
  GlobalPropInfo::global(void* o) {
    mo = Support::fmark(o);
  }

  forceinline
  GlobalPropInfo::GlobalPropInfo(void) {
    // No synchronization needed as single thread is creating this object
    local(new Object(new Support::Mutex));
  }

  forceinline
  GlobalPropInfo::GlobalPropInfo(const GlobalPropInfo& gpi) {
    global(gpi.mo);
    Object* o = object();
    o->mutex->acquire();
    o->use_cnt++;
    o->mutex->release();
  }

  forceinline
  GlobalPropInfo::~GlobalPropInfo(void) {
    Support::Mutex* m = object()->mutex;
    m->acquire();
    Object* c = object();
    while ((c != NULL) && (--c->use_cnt == 0)) {
      // Delete all blocks for c
      Block* b = c->cur;
      while (b != NULL) {
        Block* d = b; b=b->next;
        heap.rfree(d);
      }
      // Delete object
      Object* d = c; c = c->parent;
      delete d; 
    }
    m->release();
    // All objects are deleted, so also delete mutex
    if (c == NULL)
      delete m;
  }

  forceinline void
  PropInfo::fail(GlobalPropInfo& gpi) {
    Support::Mutex& m = *gpi.object()->mutex;
    m.acquire();
    _afc++;
    m.release();
  }

  forceinline PropInfo&
  GlobalPropInfo::allocate(void) {
    /*
     * If there is no local object, create one.
     *
     * There is no synchronization needed as only ONE space has access
     * to the marked pointer AND the local object.
     */
    if (!local())
      local(new Object(object()->mutex,object()));

    assert(local());

    Object* o = object();

    if (o->free == 0) {
      if (2*o->size <= size_max)
        o->size *= 2;
      o->free = o->size;
      o->cur  = Block::allocate(o->size,o->cur);
    }

    PropInfo* pi = &o->cur->pi[--o->free];
    pi->init();

    return *pi;
  }

}

// STATISTICS: kernel-prop
