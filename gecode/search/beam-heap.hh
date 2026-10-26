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

#ifndef GECODE_SEARCH_BEAM_HEAP_HH
#define GECODE_SEARCH_BEAM_HEAP_HH

#include <gecode/search.hh>
#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search {

  /**
   * \brief Beam admission, dominance, and bounds for work discarded by width
   *
   * Better candidates precede worse ones in the ordering. The heap's front
   * entry is therefore the worst candidate, ready for replacement.
   */
  class BeamHeap {
  public:
    struct Entry {
      std::unique_ptr<Space> space;
      std::int64_t bound;
      std::int64_t rank;
      std::size_t key;
      unsigned long int depth;
      unsigned long long int order;
      unsigned long long int revision;
      /// Address of this entry's slot in the key index, or null if unindexed.
      std::size_t* slot=nullptr;
    };
    /// Better priority first, with generation order breaking ties
    struct Earlier {
      HeuristicGuidance::Sense sense;
      bool ranked;
      bool operator()(const Entry& a, const Entry& b) const noexcept {
        if (ranked) {
          if (a.rank != b.rank) {
            return a.rank < b.rank;
          }
        } else if (a.bound != b.bound) {
          return sense == HeuristicGuidance::Sense::MINIMIZE ?
            a.bound < b.bound : a.bound > b.bound;
        }
        return a.order < b.order;
      }
    };
    struct Admission {
      /// Rejected candidate or first replaced candidate, for caller disposal
      std::unique_ptr<Space> discarded;
      /// Insertion would grow the beam without room in the engine's quota
      bool limited;
    };
  private:
    using Index=std::unordered_multimap<std::size_t,std::size_t>;
    std::vector<Entry> entries;
    Index index;
    /// Best optimistic bound discarded for width during the current pass
    std::optional<std::int64_t> discarded_bound_;

  public:
    bool empty(void) const {
      return entries.empty();
    }
    std::size_t size(void) const {
      return entries.size();
    }
    /// Admit a candidate; quota room is required only when growing the beam.
    Admission admit(Entry entry, unsigned long int width,
                    const CABSOptions& limits, Earlier earlier, bool has_room) {
      const bool keyed=limits.dominates && limits.dominance_key;
      // A dominator inherits a victim's slot regardless of rank or quota.
      if (limits.dominates) {
        if (keyed) {
          const auto range=index.equal_range(entry.key);
          for (auto i=range.first; i!=range.second; ++i) {
            if (limits.dominates(*entries[i->second].space,*entry.space)) {
              return {std::move(entry.space),false};
            }
          }
          std::vector<const std::size_t*> victims;
          for (auto i=range.first; i!=range.second; ++i) {
            if (limits.dominates(*entry.space,*entries[i->second].space)) {
              victims.push_back(&i->second);
            }
          }
          if (!victims.empty()) {
            // Slot addresses survive heap moves and index rehashes.
            Entry removed=replace(*victims.front(),std::move(entry),true,earlier);
            for (std::size_t i=1; i<victims.size(); i++) {
              erase(*victims[i],earlier);
            }
            return {std::move(removed.space),false};
          }
        } else {
          for (const auto& candidate : entries) {
            if (limits.dominates(*candidate.space,*entry.space)) {
              return {std::move(entry.space),false};
            }
          }
          std::vector<std::size_t> victims;
          for (std::size_t i=0; i<entries.size(); i++) {
            if (limits.dominates(*entry.space,*entries[i].space)) {
              victims.push_back(i);
            }
          }
          if (!victims.empty()) {
            Entry removed=replace(victims,std::move(entry),earlier);
            return {std::move(removed.space),false};
          }
        }
      }
      if (entries.size() == width) {
        const bool preferred=earlier(entry,entries.front());
        const std::int64_t bound=preferred ? entries.front().bound : entry.bound;
        if (!discarded_bound_ ||
            (earlier.sense == HeuristicGuidance::Sense::MINIMIZE ?
             bound < *discarded_bound_ : bound > *discarded_bound_)) {
          discarded_bound_=bound;
        }
        if (!preferred) {
          return {std::move(entry.space),false};
        }
        Entry removed=replace(0,std::move(entry),keyed,earlier);
        return {std::move(removed.space),false};
      }
      if (!has_room) {
        return {std::move(entry.space),true};
      }
      push(std::move(entry),keyed,earlier);
      return {nullptr,false};
    }
    const std::optional<std::int64_t>& discarded_bound(void) const {
      return discarded_bound_;
    }
    void reset_discarded_bound(void) {
      discarded_bound_.reset();
    }

    /// Transfer a finished beam and clear its index references.
    void release(std::vector<Entry>& layer) {
      assert(layer.empty());
      for (auto& entry : entries) {
        entry.slot=nullptr;
      }
      index.clear();
      layer.swap(entries);
    }
    void clear(void) {
      index.clear();
      entries.clear();
      discarded_bound_.reset();
    }

  private:
    void place(std::size_t i, Entry&& entry) {
      entries[i]=std::move(entry);
      if (entries[i].slot) {
        *entries[i].slot=i;
      }
    }
    void unindex(const Entry& entry) {
      if (!entry.slot) {
        return;
      }
      const auto range=index.equal_range(entry.key);
      for (auto i=range.first; i!=range.second; ++i) {
        if (&i->second == entry.slot) {
          index.erase(i);
          return;
        }
      }
      GECODE_NEVER;
    }
    void repair(std::size_t i, Earlier compare) {
      if (i && compare(entries[(i-1)/2],entries[i])) {
        Entry entry=std::move(entries[i]);
        do {
          const std::size_t parent=(i-1)/2;
          place(i,std::move(entries[parent]));
          i=parent;
        } while (i && compare(entries[(i-1)/2],entry));
        place(i,std::move(entry));
      } else if (i<entries.size()/2) {
        Entry entry=std::move(entries[i]);
        while (i<entries.size()/2) {
          std::size_t child=2*i+1;
          if (child+1<entries.size() && compare(entries[child],entries[child+1])) {
            child++;
          }
          if (!compare(entry,entries[child])) {
            break;
          }
          place(i,std::move(entries[child]));
          i=child;
        }
        place(i,std::move(entry));
      }
    }
    /// Insert a candidate and optionally index its dominance key
    void push(Entry entry, bool keyed, Earlier compare) {
      // Map nodes, and therefore slot addresses, survive map rehashes.
      auto inserted=index.end();
      if (keyed) {
        inserted=index.emplace(entry.key,entries.size());
        entry.slot=&inserted->second;
      }
      try {
        entries.push_back(std::move(entry));
      } catch (...) {
        if (keyed) {
          index.erase(inserted);
        }
        throw;
      }
      repair(entries.size()-1,compare);
    }
    Entry replace(std::size_t i, Entry entry, bool keyed, Earlier compare) {
      // Complete the only allocation before removing the old representative.
      if (keyed) {
        entry.slot=&index.emplace(entry.key,i)->second;
      }
      unindex(entries[i]);
      Entry removed=std::move(entries[i]);
      removed.slot=nullptr;
      entries[i]=std::move(entry);
      repair(i,compare);
      return removed;
    }
    Entry erase(std::size_t i, Earlier compare) {
      unindex(entries[i]);
      Entry removed=std::move(entries[i]);
      removed.slot=nullptr;
      if (i+1 != entries.size()) {
        entries[i]=std::move(entries.back());
        if (entries[i].slot) {
          *entries[i].slot=i;
        }
      }
      entries.pop_back();
      if (i<entries.size()) {
        repair(i,compare);
      }
      return removed;
    }
    /// Replace the first victim and remove the rest; the heap must be unindexed
    // Victim indices must be nonempty and sorted in ascending order.
    Entry replace(const std::vector<std::size_t>& victims, Entry entry,
                  Earlier compare) {
      assert(index.empty());
      Entry removed=std::move(entries[victims.front()]);
      entries[victims.front()]=std::move(entry);
      if (victims.size()==1) {
        repair(victims.front(),compare);
        return removed;
      }
      std::size_t victim=1;
      std::size_t next=victims[1];
      for (std::size_t i=next; i<entries.size(); i++) {
        if (victim<victims.size() && i==victims[victim]) {
          victim++;
        } else {
          entries[next++]=std::move(entries[i]);
        }
      }
      entries.resize(next);
      std::make_heap(entries.begin(),entries.end(),compare);
      return removed;
    }
  };

}}
#endif
#endif

// STATISTICS: search-other
