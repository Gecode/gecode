/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <cmath>

namespace Gecode { namespace Kernel {

  /// Global propagator information
  class GPI {
  public:
    /// Class for storing propagator information
    class Info {
    public:
      /// Propagator identifier
      unsigned int pid;
      /// Group identifier
      unsigned int gid;
      /// The afc value
      double afc;
      /// Initialize
      void init(unsigned int pid, unsigned int gid);
    };
  private:
    /// Block of propagator information
    class Block : public HeapAllocated {
    public:
      /// Number of propagator information entries in a block
      static const int n_info = 8192;
      /// Info entries
      Info info[n_info];
      /// Next block
      Block* next;
      /// Number of free blocks
      int free;
      /// Initialize
      Block(void);
      /// Rescale used afc values in entries
      void rescale(void);
    };
    /// The current block
    Block* b;
    /// The inverse decay factor
    double invd;
    /// Next free propagator id
    unsigned int npid;
    /// Whether to unshare
    bool us;
    /// The first block
    Block fst;
    /// Mutex to synchronize globally shared access
    GECODE_KERNEL_EXPORT static Support::Mutex m;
  public:
    /// Initialize
    GPI(void);
    /// Set decay factor to \a d
    void decay(double d);
    /// Return decay factor
    double decay(void) const;
    /// Increment failure count
    void fail(Info& c);
    /// Allocate info for existing propagator with pid \a p
    Info* allocate(unsigned int p, unsigned int gid);
    /// Allocate new actor info
    Info* allocate(unsigned int gid);
    /// Return next free propagator id
    unsigned int pid(void) const;
    /// Provide access to unshare info and set to true
    bool unshare(void);
    /// Delete
    ~GPI(void);
  };


  forceinline void
  GPI::Info::init(unsigned int pid0, unsigned int gid0) {
    pid=pid0; gid=gid0; afc=1.0;
  }


  forceinline
  GPI::Block::Block(void)
    : next(NULL), free(n_info) {}

  forceinline void
  GPI::Block::rescale(void) {
    for (int i=free; i < n_info; i++)
      info[i].afc *= Kernel::Config::rescale;
  }


  forceinline
  GPI::GPI(void)
    : b(&fst), invd(1.0), npid(0U), us(false) {}

  forceinline void
  GPI::fail(Info& c) {
    m.acquire();
    c.afc = invd * (c.afc + 1.0);
    if (c.afc > Kernel::Config::rescale_limit)
      for (Block* i = b; i != NULL; i = i->next)
        i->rescale();
    m.release();
  }

  forceinline double
  GPI::decay(void) const {
    double d;
    const_cast<GPI&>(*this).m.acquire();
    d = 1.0 / invd;
    const_cast<GPI&>(*this).m.release();
    return d;
  }

  forceinline unsigned int
  GPI::pid(void) const {
    unsigned int p;
    const_cast<GPI&>(*this).m.acquire();
    p = npid;
    const_cast<GPI&>(*this).m.release();
    return p;
  }

  forceinline bool
  GPI::unshare(void) {
    bool u;
    m.acquire();
    u = us; us = true;
    m.release();
    return u;
  }

  forceinline void
  GPI::decay(double d) {
    m.acquire();
    invd = 1.0 / d;
    m.release();
  }

  forceinline GPI::Info*
  GPI::allocate(unsigned int p, unsigned int gid) {
    Info* c;
    m.acquire();
    if (b->free == 0) {
      Block* n = new Block;
      n->next = b; b = n;
    }
    c = &b->info[--b->free];
    m.release();
    c->init(p,gid);
    return c;
  }

  forceinline GPI::Info*
  GPI::allocate(unsigned int gid) {
    Info* c;
    m.acquire();
    if (b->free == 0) {
      Block* n = new Block;
      n->next = b; b = n;
    }
    c = &b->info[--b->free];
    c->init(npid++,gid);
    m.release();
    return c;
  }

  forceinline
  GPI::~GPI(void) {
    Block* n = b;
    while (n != &fst) {
      Block* d = n;
      n = n->next;
      delete d;
    }
  }

}}

// STATISTICS: kernel-prop
