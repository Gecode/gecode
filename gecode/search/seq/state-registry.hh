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

#ifndef GECODE_SEARCH_SEQ_STATE_REGISTRY_HH
#define GECODE_SEARCH_SEQ_STATE_REGISTRY_HH

#include <gecode/search.hh>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search { namespace Seq {

  /// Persistent descriptor metadata, independent of pending search work
  class StateRegistry {
  private:
    StatePruning::Compare compare;
    unsigned long int limit;
    std::unordered_map<std::size_t,
                       std::vector<StatePruning::Descriptor>> buckets;
    unsigned long int descriptors;
    std::size_t descriptor_bytes;
  public:
    StateRegistry(const StatePruning::Compare& c, unsigned long int l)
      : compare(c), limit(l), descriptors(0), descriptor_bytes(0) {}

    /// Whether an equivalent or better stored label subsumes this descriptor
    bool subsumes(const StatePruning::Descriptor& descriptor) const {
      const auto bucket = buckets.find(descriptor.hash);
      if (bucket != buckets.end())
        for (const auto& existing : bucket->second) {
          if (descriptor.key != existing.key) continue;
          const auto relation = compare(descriptor,existing);
          if (relation == StatePruning::EQUIVALENT ||
              relation == StatePruning::DOMINATED)
            return true;
        }
      return false;
    }

    /// Store admitted metadata, replacing dominated labels and forgetting at limit
    void store(StatePruning::Descriptor descriptor) {
      auto& bucket = buckets[descriptor.hash];
      bucket.erase(std::remove_if(bucket.begin(),bucket.end(),
        [&](const StatePruning::Descriptor& existing) {
          if (descriptor.key != existing.key ||
              compare(descriptor,existing) != StatePruning::DOMINATES)
            return false;
          descriptors--;
          descriptor_bytes -= existing.storage();
          return true;
        }),bucket.end());
      if (limit && descriptors >= limit) clear();
      descriptor_bytes += descriptor.storage();
      buckets[descriptor.hash].push_back(std::move(descriptor));
      descriptors++;
    }

    /// Forget metadata without affecting search coverage
    void clear(void) {
      buckets.clear();
      descriptors = 0;
      descriptor_bytes = 0;
    }

    unsigned long int size(void) const { return descriptors; }
    std::size_t storage(void) const { return descriptor_bytes; }
  };

}}}

#endif

#endif

// STATISTICS: search-seq
