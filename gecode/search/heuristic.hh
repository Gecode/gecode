/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_SEARCH_HEURISTIC_HH
#define GECODE_SEARCH_HEURISTIC_HH

#include <gecode/search.hh>
#include <algorithm>
#include <climits>
#include <memory>
#include <optional>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search {

  /// Whether bound \a a is better than bound \a b for objective direction \a s
  forceinline bool
  better(HeuristicGuidance::Sense s, std::int64_t a, std::int64_t b) {
    return (s == HeuristicGuidance::MINIMIZE) ? (a < b) : (a > b);
  }

  /// Open node of a heuristic search engine
  class Entry {
  public:
    /// The stable space (owned by the container holding the entry)
    Space* space;
    /// Bound and rank of the space
    std::int64_t bound, rank;
    /// Dominance key of the space
    std::size_t key;
    /// Depth of the node
    unsigned long int depth;
    /// Creation order, for breaking ties
    unsigned long long int order;
    /// Incumbent revision the space has been constrained with
    unsigned long long int revision;
  };

  /// Order of open nodes: whether \a a is preferred to \a b
  class Priority {
  public:
    /// Objective direction
    HeuristicGuidance::Sense sense;
    /// Whether to order by rank only (CABS with rank)
    bool rank_only;
    /// Test whether \a a is preferred to \a b
    bool operator ()(const Entry& a, const Entry& b) const {
      if (!rank_only && (a.bound != b.bound))
        return better(sense,a.bound,b.bound);
      if (a.rank != b.rank)
        return a.rank < b.rank;
      return a.order < b.order;
    }
  };

  /// Best solution found so far
  class Incumbent {
  public:
    /// Objective direction
    HeuristicGuidance::Sense sense;
    /// The solution (if any)
    std::unique_ptr<Space> space;
    /// Its objective value
    std::int64_t bound;
    /// Incremented for every new incumbent
    unsigned long long int revision;
    /// Initialize without incumbent
    Incumbent(HeuristicGuidance::Sense s)
      : sense(s), bound(0), revision(0) {}
    /// Whether a node with bound \a b can improve the incumbent
    bool improves(std::int64_t b) const {
      return !space || better(sense,b,bound);
    }
    /// Make \a s with objective value \a b the incumbent
    void update(Space* s, std::int64_t b) {
      space.reset(s); bound = b; revision++;
    }
    /// Remove incumbent
    void reset(void) {
      space.reset(); revision = 0;
    }
  };

  /// Stack of entries owning their spaces
  class Entries : public Support::DynamicStack<Entry,Heap> {
  public:
    /// Initialize
    Entries(void) : Support::DynamicStack<Entry,Heap>(heap) {}
    /// Delete all entries
    void clear(void) {
      while (!empty())
        delete pop().space;
    }
    /// Release memory
    ~Entries(void) { clear(); }
    /// Sort such that the preferred entry is on top
    void sort(const Priority& p) {
      if (entries() > 1)
        std::sort(&(*this)[0],&(*this)[0]+entries(),
                  [&](const Entry& a, const Entry& b) { return p(b,a); });
    }
  };

  /// Priority queue of open nodes for A*
  class Queue {
  protected:
    /// The entries as a binary heap with the preferred entry first
    Entries entries;
    /// The priority
    Priority p;
  public:
    /// Initialize with priority \a p
    Queue(const Priority& p0) : p(p0) {}
    /// Whether the queue is empty
    bool empty(void) const { return entries.empty(); }
    /// Add entry \a e
    void push(const Entry& e) {
      entries.push(e);
      Entry* f = &entries[0];
      std::push_heap(f,f+entries.entries(),
                     [this](const Entry& a, const Entry& b) { return p(b,a); });
    }
    /// Remove and return the preferred entry
    Entry pop(void) {
      Entry* f = &entries[0];
      std::pop_heap(f,f+entries.entries(),
                    [this](const Entry& a, const Entry& b) { return p(b,a); });
      return entries.pop();
    }
    /// Delete all entries
    void clear(void) { entries.clear(); }
  };

  /**
   * \brief Candidates for the next layer of CABS
   *
   * Keeps at most as many entries as the current width, with the least
 * preferred one on top.
   * Entries are stored in fixed slots so that slots with the same
   * dominance key can be chained for dominance checks.
   */
  class Beam {
  public:
    /// Result of offering an entry
    enum Offer {
      TAKEN, ///< The entry has been added or deleted
      FULL   ///< The entry needs a new slot but there is no room
    };
  private:
    /// Slot of an entry
    class Slot {
    public:
      /// The entry
      Entry e;
      /// Position in heap (-1 for a free slot)
      int pos;
      /// Next and previous slot in bucket (or next free slot)
      int next, prev;
    };
    /// The slots
    Support::DynamicArray<Slot,Heap> slot;
    /// Number of slots used so far and first free slot
    int n_slot, free;
    /// Heap of slot indices with the least preferred entry first
    Support::DynamicArray<int,Heap> pq;
    /// Number of entries
    int n;
    /// First slot of each bucket for keyed dominance
    Support::DynamicArray<int,Heap> bucket;
    /// Number of buckets (a power of two, zero if not keyed)
    int n_bucket;
    /// The priority
    Priority p;
    /// The current width
    unsigned long int _width;
    /// Best bound of discarded entries (if any)
    std::optional<std::int64_t> discarded;

    /// Return bucket for key \a k
    int index(std::size_t k) const {
      return static_cast<int>((k ^ (k >> 16)) & (n_bucket-1));
    }
    /// Add slot \a s to its bucket
    void link(int s) {
      int b = index(slot[s].e.key);
      slot[s].prev = -1; slot[s].next = bucket[b];
      if (bucket[b] >= 0)
        slot[bucket[b]].prev = s;
      bucket[b] = s;
    }
    /// Remove slot \a s from its bucket
    void unlink(int s) {
      Slot& x = slot[s];
      if (x.prev >= 0)
        slot[x.prev].next = x.next;
      else
        bucket[index(x.e.key)] = x.next;
      if (x.next >= 0)
        slot[x.next].prev = x.prev;
    }
    /// Whether slot \a a must be above slot \a b in the heap
    bool above(int a, int b) const {
      return p(slot[b].e,slot[a].e);
    }
    /// Put slot \a s at heap position \a i
    void place(int i, int s) {
      pq[i] = s; slot[s].pos = i;
    }
    /// Restore heap order for slot at position \a i
    void repair(int i) {
      int s = pq[i];
      while ((i > 0) && above(s,pq[(i-1)/2])) {
        place(i,pq[(i-1)/2]); i = (i-1)/2;
      }
      while (2*i+1 < n) {
        int c = 2*i+1;
        if ((c+1 < n) && above(pq[c+1],pq[c]))
          c++;
        if (!above(pq[c],s))
          break;
        place(i,pq[c]); i = c;
      }
      place(i,s);
    }
    /// Add entry \a e
    void add(const Entry& e) {
      int s;
      if (free >= 0) {
        s = free; free = slot[s].next;
      } else {
        s = n_slot++;
      }
      slot[s].e = e;
      if (n_bucket > 0) {
        if (n >= n_bucket) {
          n_bucket *= 2;
          for (int b=0; b<n_bucket; b++)
            bucket[b] = -1;
          for (int i=0; i<n; i++)
            link(pq[i]);
        }
        link(s);
      }
      place(n++,s);
      repair(n-1);
    }
    /// Remove and delete entry in slot \a s
    void remove(int s) {
      int i = slot[s].pos;
      delete slot[s].e.space;
      if (n_bucket > 0)
        unlink(s);
      slot[s].pos = -1; slot[s].next = free; free = s;
      if (i < --n) {
        place(i,pq[n]); repair(i);
      }
    }
    /// Whether a dominance check between \a e and slot \a s is needed
    bool comparable(const Entry& e, int s) const {
      return (slot[s].pos >= 0) && ((n_bucket == 0) || (slot[s].e.key == e.key));
    }
    /// Return first slot to check for dominance with \a e
    int chain_first(const Entry& e) const {
      return (n_bucket > 0) ? bucket[index(e.key)] : 0;
    }
    /// Return slot after \a s to check for dominance
    int chain_next(int s) const {
      if (n_bucket > 0)
        return slot[s].next;
      return (s+1 < n_slot) ? s+1 : -1;
    }
  public:
    /// Initialize with priority \a p, using keys for dominance if \a keyed
    Beam(const Priority& p0, bool keyed)
      : slot(heap), n_slot(0), free(-1), pq(heap), n(0), bucket(heap),
        n_bucket(keyed ? 16 : 0), p(p0), _width(1) {
      for (int b=0; b<n_bucket; b++)
        bucket[b] = -1;
    }
    /// Delete all entries
    ~Beam(void) { clear(); }
    /// Return the number of entries
    int size(void) const { return n; }
    /// Whether there are no entries
    bool empty(void) const { return n == 0; }
    /// Whether discarded entries could improve an incumbent \a i
    bool improving(const Incumbent& i) const {
      return discarded && i.improves(*discarded);
    }
    /// Double the width up to \a m (zero for no limit), false if not possible
    bool widen(unsigned long int m) {
      if ((_width > ULONG_MAX/2) || ((m > 0UL) && (2*_width > m)))
        return false;
      _width *= 2; discarded.reset();
      return true;
    }
    /**
     * \brief Offer entry \a e
     *
     * Deletes \a e if a candidate dominates it or if the beam is full and
     * all candidates are preferred. Deletes candidates that \a e dominates
     * and the least preferred candidate if \a e replaces it. Returns FULL
     * without deleting \a e if \a e needs a new slot and \a room is false.
     */
    Offer offer(const Entry& e, const HeuristicGuidance& g, bool room) {
      if (g.dominates && (n > 0)) {
        for (int s=chain_first(e); s>=0; s=chain_next(s))
          if (comparable(e,s) && g.dominates(*slot[s].e.space,*e.space)) {
            delete e.space;
            return TAKEN;
          }
        bool victim = false;
        for (int s=chain_first(e); s>=0; ) {
          int t = chain_next(s);
          if (comparable(e,s) && g.dominates(*e.space,*slot[s].e.space)) {
            remove(s); victim = true;
          }
          s = t;
        }
        if (victim) {
          add(e);
          return TAKEN;
        }
      }
      if (static_cast<unsigned long int>(n) == _width) {
        const Entry& worst = slot[pq[0]].e;
        bool replace = p(e,worst);
        std::int64_t b = replace ? worst.bound : e.bound;
        if (!discarded || better(p.sense,b,*discarded))
          discarded = b;
        if (replace) {
          remove(pq[0]); add(e);
        } else {
          delete e.space;
        }
        return TAKEN;
      }
      if (!room)
        return FULL;
      add(e);
      return TAKEN;
    }
    /// Move all entries to \a l
    void release(Entries& l) {
      for (int i=0; i<n; i++)
        l.push(slot[pq[i]].e);
      n = 0; n_slot = 0; free = -1;
      for (int b=0; b<n_bucket; b++)
        bucket[b] = -1;
    }
    /// Delete all entries and reset width
    void clear(void) {
      for (int i=0; i<n; i++)
        delete slot[pq[i]].e.space;
      n = 0; n_slot = 0; free = -1;
      for (int b=0; b<n_bucket; b++)
        bucket[b] = -1;
      _width = 1; discarded.reset();
    }
  };

}}
#endif

#endif

// STATISTICS: search-other
