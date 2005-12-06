/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SUPPORT_BLOCK_ALLOCATOR_HH__
#define __GECODE_SUPPORT_BLOCK_ALLOCATOR_HH__

#include "kernel.hh"

namespace Gecode { namespace Support {

  /**
   * \brief Manage memory organized into block lists (allocator)
   *
   * The allocation policy is to free all memory allocated when
   * the block allocator is deleted.
   *
   * Requires \code #include "support/block-allocator.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T>
  class BlockAllocator {
  private:
    /// One block of memory
    class Block {
    public:
      static const int blocksize = 512;
      T      b[blocksize];
      Block* next;
    };
    /// Pointer to current block
    Block* b;
    /// Pointer to next part of block (decreasing)
    T*     n;
    /// Size of allocated blocks
    size_t _size;
    /// Allocate additional block
    void allocate(void);
  public:
    /// Initialize
    BlockAllocator(void);
    /// Free all allocated blocks
    ~BlockAllocator(void);
    /// Return memory of size required by \a T
    T* operator()(void);
    /// Return size of memory required by allocator
    size_t size(void) const;
  };

  /**
   * \brief Client for block allocator of type \a T
   *
   * Provides memory management for objects of type \a T.
   *
   * Requires \code #include "support/block-allocator.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T>
  class BlockClient {
  public:
    /// Allocate memory from block allocator \a ba
    static void* operator new(size_t s, BlockAllocator<T>& ba);
    /// Noop (memory freed only when block allocator deleted)
    static void  operator delete(void*, BlockAllocator<T>& ba);
    /// Noop (memory freed only when block allocator deleted)
    static void  operator delete(void*);
  };



  template <class T>
  forceinline
  BlockAllocator<T>::BlockAllocator(void) {
    b = reinterpret_cast<Block*>(Memory::malloc(sizeof(Block)));
    b->next = NULL;
    n       = &b->b[Block::blocksize];
    _size = sizeof(Block);
  }

  template <class T>
  forceinline
  BlockAllocator<T>::~BlockAllocator(void) {
    while (b != NULL) {
      Block* f = b; b = b->next;
      Memory::free(f);
    }
  }

  template <class T>
  forceinline T*
  BlockAllocator<T>::operator()(void) {
    T* t = --n;
    if (t == &b->b[0])
      allocate();
    return t;
  }

  template <class T>
  void
  BlockAllocator<T>::allocate(void) {
    // Allocate another block
    Block* nb = reinterpret_cast<Block*>(Memory::malloc(sizeof(Block)));
    nb->next = b; b = nb;
    n = &nb->b[Block::blocksize];
    _size += sizeof(Block);
  }

  template <class T>
  forceinline size_t
  BlockAllocator<T>::size(void) const {
    return _size;
  }



  template <class T>
  forceinline void
  BlockClient<T>::operator delete(void*, BlockAllocator<T>&) {
  }
  template <class T>
  forceinline void
  BlockClient<T>::operator delete(void*) {
  }
  template <class T>
  forceinline void*
  BlockClient<T>::operator new(size_t s, BlockAllocator<T>& ba) {
    assert(s == sizeof(T));
    return ba();
  }

}}

#endif

// STATISTICS: support-any
