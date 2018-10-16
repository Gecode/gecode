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

#include <cstddef>

namespace Gecode {

  /**
   * \defgroup FuncMemRegion Region memory management
   *
   * A region provides a handle to temporary memory owned by
   * a space. The memory will be managed in a stack fashion, that is,
   * the memory allocated through a region will be released
   * only after the region is deleted and all other regions
   * created later also have been deleted.
   *
   * In case a memory request cannot be fulfilled from a space's region,
   * heap memory is allocated and returned to the operating system
   * as soon as the region is deleted.
   *
   * \ingroup FuncMem
   */
  //@{
  /// Handle to region
  class Region {
  private:
    /// Heap chunks used for regions
    class Chunk : public HeapAllocated {
    public:
      /// Amount of free memory
      size_t free;
      /// The actual memory area (allocated from top to bottom)
      alignas((alignof(std::max_align_t) > GECODE_MEMORY_ALIGNMENT) ?
              alignof(std::max_align_t) : GECODE_MEMORY_ALIGNMENT)
        double area[Kernel::MemoryConfig::region_area_size / sizeof(double)];
      /// A pointer to another chunk
      Chunk* next;
      /// Return memory if available
      bool alloc(size_t s, void*& p);
      /// Free allocated memory (reset chunk)
      void reset(void);
    };
    /// The heap chunk in use
    Chunk* chunk;
    /// A pool of heap chunks to be used for regions
    class GECODE_KERNEL_EXPORT Pool {
    protected:
      /// The current chunk
      Chunk* c;
      /// Number of cached chunks
      unsigned int n_c;
      /// Mutex to synchronize globally shared access
      Support::Mutex m;
    public:
      /// Initialize pool
      Pool(void);
      /// Get a new chunk
      Chunk* chunk(void);
      /// Return chunk and possible free unused chunk \a u
      void chunk(Chunk* u);
      /// Delete pool
      ~Pool(void);
    };
    /// Just use a single static pool for heap chunks
    GECODE_KERNEL_EXPORT static Pool& pool();
    /// Heap information data structure
    class HeapInfo {
    public:
      /// Number of allocated heap blocks
      unsigned int n;
      /// Limit of allocated heap blocks
      unsigned int size;
      /// Pointers to allocated heap blocks (more entries)
      void* blocks[1];
    };
    /**
     * \brief Heap allocation information
     *
     * If NULL, no heap memory has been allocated. If the pointer
     * is marked, it points to a single heap allocated block. Otherwise,
     * it points to a HeapInfo data structure.
     */
    void* hi;
    /// Allocate memory from heap
    GECODE_KERNEL_EXPORT void* heap_alloc(size_t s);
    /// Free memory previously allocated from heap
    GECODE_KERNEL_EXPORT void heap_free(void);
  public:
    /// Initialize region
    Region(void);
    /**
     * \brief Free allocate memory
     *
     * Note that in fact not all memory is freed, memory that has been
     * allocated for large allocation requests will not be freed.
     */
    void free(void);
    /// \name Typed allocation routines
    //@{
    /**
     * \brief Allocate block of \a n objects of type \a T from region
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(long unsigned int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from region
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(long int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from region
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(unsigned int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from region
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(int n);
    /**
     * \brief Delete \a n objects allocated from the region starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, the only effect is running the
     * destructors.
     */
    template<class T>
    void free(T* b, long unsigned int n);
    /**
     * \brief Delete \a n objects allocated from the region starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, the only effect is running the
     * destructors.
     */
    template<class T>
    void free(T* b, long int n);
    /**
     * \brief Delete \a n objects allocated from the region starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, the only effect is running the
     * destructors.
     */
    template<class T>
    void free(T* b, unsigned int n);
    /**
     * \brief Delete \a n objects allocated from the region starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, the only effect is running the
     * destructors.
     */
    template<class T>
    void free(T* b, int n);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the region
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, long unsigned int n, long unsigned int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the region
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, long int n, long int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the region
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, unsigned int n, unsigned int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the region
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, int n, int m);
    //@}
    /// \name Raw allocation routines
    //@{
    /// Allocate memory from region
    void* ralloc(size_t s);
    /**
     * \brief Free memory previously allocated
     *
     * Note that the memory is only guaranteed to be freed after the
     * region object itself gets deleted.
     */
    void rfree(void* p, size_t s);
    //@}
    /// \name Construction routines
    //@{
    /**
     * \brief Constructs a single object of type \a T from region using the default constructor.
     */
    template<class T>
    T& construct(void);
    /**
     * \brief Constructs a single object of type \a T from region using a unary constructor.
     *
     * The parameter is passed as a const reference.
     */
    template<class T, typename A1>
    T& construct(A1 const& a1);
    /**
     * \brief Constructs a single object of type \a T from region using a binary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2>
    T& construct(A1 const& a1, A2 const& a2);
    /**
     * \brief Constructs a single object of type \a T from region using a ternary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3);
    /**
     * \brief Constructs a single object of type \a T from region using a quaternary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3, typename A4>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4);
    /**
     * \brief Constructs a single object of type \a T from region using a quinary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5);
    //@}
    /// Return memory
    ~Region(void);
  private:
    /// Allocate memory from heap (disabled)
    static void* operator new(size_t s) throw() { (void) s; return NULL; }
    /// Free memory allocated from heap (disabled)
    static void  operator delete(void* p) { (void) p; };
    /// Copy constructor (disabled)
    Region(const Region&) {}
    /// Assignment operator (disabled)
    const Region& operator =(const Region&) { return *this; }
  };
  //@}


  /*
   * Implementation
   *
   */
  forceinline bool
  Region::Chunk::alloc(size_t s, void*& p) {
    Kernel::MemoryConfig::align
      (s,((alignof(std::max_align_t) > GECODE_MEMORY_ALIGNMENT) ?
          alignof(std::max_align_t) : GECODE_MEMORY_ALIGNMENT));
    if (s > free)
      return false;
    free -= s;
    p = ptr_cast<char*>(&area[0]) + free;
    return true;
  }

  forceinline void
  Region::Chunk::reset(void) {
    free = Kernel::MemoryConfig::region_area_size;
  }


  forceinline
  Region::Region(void)
    : chunk(pool().chunk()), hi(0) {}

  forceinline void
  Region::free(void) {
    chunk->reset();
  }

  forceinline void*
  Region::ralloc(size_t s) {
    void* p;
    if (chunk->alloc(s,p))
      return p;
    else
      return heap_alloc(s);
  }

  forceinline void
  Region::rfree(void*, size_t) {}

  forceinline
  Region::~Region(void) {
    pool().chunk(chunk);
    if (hi != NULL)
      heap_free();
  }


  /*
   * Typed allocation routines
   *
   */
  template<class T>
  forceinline T*
  Region::alloc(long unsigned int n) {
    T* p = static_cast<T*>(ralloc(sizeof(T)*n));
    for (long unsigned int i=0U; i<n; i++)
      (void) new (p+i) T();
    return p;
  }
  template<class T>
  forceinline T*
  Region::alloc(long int n) {
    assert(n >= 0);
    return alloc<T>(static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline T*
  Region::alloc(unsigned int n) {
    return alloc<T>(static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline T*
  Region::alloc(int n) {
    assert(n >= 0);
    return alloc<T>(static_cast<long unsigned int>(n));
  }

  template<class T>
  forceinline void
  Region::free(T* b, long unsigned int n) {
    for (long unsigned int i=0U; i<n; i++)
      b[i].~T();
    rfree(b,n*sizeof(T));
  }
  template<class T>
  forceinline void
  Region::free(T* b, long int n) {
    assert(n >= 0);
    free<T>(b,static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline void
  Region::free(T* b, unsigned int n) {
    free<T>(b,static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline void
  Region::free(T* b, int n) {
    assert(n >= 0);
    free<T>(b,static_cast<long unsigned int>(n));
  }

  template<class T>
  forceinline T*
  Region::realloc(T* b, long unsigned int n, long unsigned int m) {
    if (n < m) {
      T* p = static_cast<T*>(ralloc(sizeof(T)*m));
      for (long unsigned int i=0U; i<n; i++)
        (void) new (p+i) T(b[i]);
      for (long unsigned int i=n; i<m; i++)
        (void) new (p+i) T();
      free<T>(b,n);
      return p;
    } else {
      free<T>(b+m,m-n);
      return b;
    }
  }
  template<class T>
  forceinline T*
  Region::realloc(T* b, long int n, long int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T*
  Region::realloc(T* b, unsigned int n, unsigned int m) {
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T*
  Region::realloc(T* b, int n, int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }

  /*
   * Region construction support
   *
   */
  template<class T>
  forceinline T&
  Region::construct(void) {
    return alloc<T>(1);
  }
  template<class T, typename A1>
  forceinline T&
  Region::construct(A1 const& a1) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1);
    return t;
  }
  template<class T, typename A1, typename A2>
  forceinline T&
  Region::construct(A1 const& a1, A2 const& a2) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3>
  forceinline T&
  Region::construct(A1 const& a1, A2 const& a2, A3 const& a3) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3, typename A4>
  forceinline T&
  Region::construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3,a4);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
  forceinline T&
  Region::construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3,a4,a5);
    return t;
  }

}

// STATISTICS: kernel-memory
