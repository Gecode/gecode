/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

namespace Gecode { namespace Support {
  
  /** \brief Hash map
    *
    * This class is used as a symbol table. \a Key objects must
    * have a hash function (see String class as an example).
    */
  template <class Key, class Value>
  class Map {
  private:
    /// Key-value pairs stored in the hash table
    class Pair {
    public:
      Key k; Value v;
      Pair(const Key& k0, const Value& v0) : k(k0), v(v0) {}
    };
    
    /// The actual table
    Pair** a;
    /// The size of the table (approximately \f$2^n\f$)
    int n;
    /// How many table entries are actually used
    int usage;
    /// Compute the actual size from \a n
    int modulo(void) const;
    /// Resize and rehash the table
    void rehash(void);
  public:
    /// Constructor
    Map(void);
    /// Insert \a v at key \a k
    void put(const Key& k, Value v);
    /// Check if entry with key \a k exists and return it in \a v
    bool get(const Key& k, Value& v) const;
    /// Destructor
    GECODE_MSC_VIRTUAL ~Map(void);
  };
  
  /// Hash table for strings
  template <class Value>
  class SymbolMap : public Map<Symbol,Value> {
  };

  /// Pointer wrapper class to make pointers hashable
  template <class P>
  class Pointer {
  protected:
    /// The actual pointer
    P* p;
  public:
    /// Constructor
    Pointer(P* p0) : p(p0) {}
    /// Cast
    P* operator()(void) { return p; }
    /// Comparison
    bool operator==(const Pointer<P>& p0) const { 
      return p == p0.p; 
    }
    /// Hash function
    int hash(int m) const {
      return static_cast<unsigned int>(reinterpret_cast<ptrdiff_t>(p)) % m;
    }
  };
  
  /// Hash table for pointers
  template <class P, class Value>
  class PtrMap : public Map<Pointer<P>,Value> {
  };
  
  /*
   * Implementation
   *
   */
  
  template <class Key, class Value>
  forceinline int
  Map<Key,Value>::modulo(void) const {
    const int primes[] = { 17, 47, 1021, 2039, 4093, 8191,
                           16381, 32749, 65521, 131071,
                           262139, 524287, 1048573, 2097143,
                           4194301, 8388593 };
    return primes[n];
  }
  
  template <class Key, class Value>
  Map<Key,Value>::Map(void) : n(0), usage(0) {
    a = static_cast<Pair**>(heap.ralloc(sizeof(Pair*)*modulo()));
    for (int i=modulo(); i--; )
      a[i] = NULL;
  }
  
  template <class Key, class Value>
  void
  Map<Key,Value>::rehash(void) {
    int oldM = modulo();
    n++;
    Pair** olda = a;
    a = static_cast<Pair**>(heap.ralloc(sizeof(Pair*)*modulo()));
    for (int i=modulo(); i--; )
      a[i] = NULL;
    for (int i=oldM; i--;) {
      if (olda[i] != NULL) {
        int j = olda[i]->k.hash(modulo());
        for (; j < modulo() && a[j] != NULL; j++) {}
        if (j >= modulo()) {
          for (j=0; a[j] != NULL; j++) {}
          assert(j<olda[i]->k.hash(modulo()));
        }
        assert(j < modulo());
        assert(a[j] == NULL);
        a[j] = olda[i];
      }
    }
#ifndef NDEBUG
    int actualUsage = 0;
    for (int i=modulo(); i--;)
      if (a[i]) actualUsage++;
    assert(actualUsage == usage);
#endif
    heap.rfree(olda);
  }
  
  template <class Key, class Value>
  void
  Map<Key,Value>::put(const Key& k, Value v) {
    // If more than 75% are used, make some room
    if (usage * 3 >= modulo() * 2)
      rehash();
      
    int i = k.hash(modulo());
    for (; i < modulo() && a[i] != NULL; i++) {}
    if (i >= modulo()) {
      for (i=0; a[i] != NULL; i++) {}
      assert(i<k.hash(modulo()));
    }
    assert(i < modulo());
    assert(a[i] == NULL);
    a[i] = new Pair(k,v);
    usage++;
  }
  
  template <class Key, class Value>
  inline bool
  Map<Key,Value>::get(const Key& k, Value& v) const {
    int i=k.hash(modulo());
    for (; i<modulo() && a[i] != NULL; i++) {
      if (a[i]->k == k) {
        v = a[i]->v; return true;
      }
    }
    if (i >= modulo()) {
      for (i=0; a[i] != NULL; i++) {
        if (a[i]->k == k) {
          v = a[i]->v; return true;
        }        
      }
    }
    return false;
  }
  
  template <class Key, class Value>
  Map<Key,Value>::~Map(void) {
    for (int i=modulo(); i--; )
      delete a[i];
    heap.rfree(a);
  }

}}

// STATISTICS: support-any
