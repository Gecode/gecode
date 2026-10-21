/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 * Private modelling support for the common-pattern example.
 * Copyright Mikael Zayenz Lagerkvist, 2026.
 * Distributed under the MIT license; see LICENSE.
 */

#ifndef GECODE_EXAMPLES_COMMON_PATTERN_HH
#define GECODE_EXAMPLES_COMMON_PATTERN_HH

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <algorithm>
#include <charconv>
#include <functional>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace PatternMining {

  using namespace Gecode;
  typedef std::vector<std::vector<int>> Sequences;

  enum Format { LETTERS, INTEGERS, SPMF };
  enum Language { DEFAULT, NONE, MOTIF, RE10, RE14, RE2 };

  /// Read positive integer items or A..Z (represented by item IDs 1..26)
  inline std::vector<int>
  read_sequence(const std::string& line, Format format) {
    std::vector<int> sequence;
    if (format == LETTERS) {
      for (char a : line) {
        if ((a < 'A') || (a > 'Z'))
          throw std::invalid_argument("Letter sequences must contain A..Z only");
        sequence.push_back(a-'A'+1);
      }
    } else {
      std::istringstream stream(line);
      std::string token;
      bool open_itemset = false, ended = false;
      while (stream >> token) {
        int item;
        const auto parsed = std::from_chars(token.data(),token.data()+token.size(),item);
        if ((parsed.ec != std::errc()) || (parsed.ptr != token.data()+token.size()))
          throw std::invalid_argument("Invalid integer item");
        if (ended)
          throw std::invalid_argument("Items after SPMF sequence end");
        if ((format == SPMF) && (item == -1)) {
          if (!open_itemset)
            throw std::invalid_argument("Empty SPMF itemset");
          open_itemset = false;
        } else if ((format == SPMF) && (item == -2)) {
          if (open_itemset)
            throw std::invalid_argument("Missing SPMF itemset end");
          ended = true;
        } else {
          if ((item <= 0) || (item > Int::Limits::max) ||
              ((format == SPMF) && open_itemset))
            throw std::invalid_argument("Expected positive items; SPMF itemsets must be singletons");
          sequence.push_back(item);
          open_itemset = true;
        }
      }
      if (!stream.eof() || ((format == SPMF) && !ended))
        throw std::invalid_argument("Invalid sequence or missing SPMF sequence end");
    }
    if (sequence.empty())
      throw std::invalid_argument("Empty sequence");
    return sequence;
  }

  /// Read one sequence per line, retaining duplicate rows as distinct records
  inline Sequences
  read_database(std::istream& stream, Format format) {
    Sequences input;
    std::string line;
    while (std::getline(stream,line)) {
      if (!line.empty() && (line.back() == '\r'))
        line.pop_back();
      if (line.empty() || ((format == SPMF) &&
          ((line[0] == '#') || (line[0] == '%') || (line[0] == '@'))))
        continue;
      input.push_back(read_sequence(line,format));
    }
    if (!stream.eof() || input.empty())
      throw std::invalid_argument("Cannot read a nonempty sequence database");
    return input;
  }

  /// Round the support percentage (at most six decimals) up to a record count
  inline int
  minimum_support(const std::string& percent, int sequences) {
    unsigned long long numerator = 0, scale = 1;
    bool decimal = false, digit = false;
    for (char c : percent) {
      if ((c == '.') && !decimal) {
        decimal = true;
      } else if ((c >= '0') && (c <= '9')) {
        digit = true;
        numerator = numerator*10 + (c-'0');
        if (decimal)
          scale *= 10;
        if ((numerator > 100000000ULL) || (scale > 1000000ULL))
          throw std::invalid_argument("Support needs at most six decimals");
      } else {
        throw std::invalid_argument("Support must be a percentage in (0,100]");
      }
    }
    if (!digit || (numerator == 0) || (numerator > 100*scale))
      throw std::invalid_argument("Support must be a percentage in (0,100]");
    const unsigned long long denominator = 100*scale;
    return static_cast<int>((sequences*numerator+denominator-1)/denominator);
  }

  /// Immutable input, with dense ranks internally and original IDs externally
  class Database {
  public:
    /// Concatenated item ranks, exclusive record ends, and rank-to-ID mapping
    IntSharedArray items, ends, alphabet;
    /// Each item's last position per record, in decreasing position order
    IntSharedArray last_positions, last_ends;

    explicit Database(const Sequences& input) {
      if (input.empty())
        throw std::invalid_argument("Empty database");
      std::vector<int> symbols;
      for (const std::vector<int>& sequence : input) {
        if (sequence.empty() ||
            (sequence.size() >= static_cast<size_t>(Int::Limits::max)-symbols.size()))
          throw std::invalid_argument("Empty sequence or database too large");
        for (int item : sequence) {
          if ((item <= 0) || (item > Int::Limits::max))
            throw std::invalid_argument("Item IDs must be positive Gecode integers");
          symbols.push_back(item);
        }
      }
      items.init(static_cast<int>(symbols.size()));
      std::sort(symbols.begin(),symbols.end());
      symbols.erase(std::unique(symbols.begin(),symbols.end()),symbols.end());
      alphabet.init(static_cast<int>(symbols.size()));
      std::copy(symbols.begin(),symbols.end(),alphabet.begin());
      ends.init(static_cast<int>(input.size()));
      last_ends.init(static_cast<int>(input.size()));
      Region region;
      int* seen = region.alloc<int>(alphabet.size());
      std::fill(seen,seen+alphabet.size(),-1);
      std::vector<int> last;
      int k = 0, i = 0;
      for (const std::vector<int>& sequence : input) {
        const int start = k;
        for (int item : sequence)
          items[k++] = static_cast<int>(std::lower_bound(
            symbols.begin(),symbols.end(),item)-symbols.begin());
        ends[i] = k;
        for (int j=k; j-- > start; )
          if (seen[items[j]] != i) {
            seen[items[j]] = i;
            last.push_back(j);
          }
        last_ends[i++] = static_cast<int>(last.size());
      }
      last_positions.init(static_cast<int>(last.size()));
      std::copy(last.begin(),last.end(),last_positions.begin());
    }

    IntSet domain(void) const { return IntSet(alphabet.begin(),alphabet.size()); }

    int max_length(void) const {
      int bound = 0, start = 0;
      for (int end : ends) {
        bound = std::max(bound,end-start);
        start = end;
      }
      return bound;
    }

    /// Count supporting records, rather than embeddings within a record
    int support(const std::vector<int>& pattern) const {
      int count = 0, start = 0;
      for (int end : ends) {
        size_t matched = 0;
        for (int i=start; (i<end) && (matched<pattern.size()); i++)
          if (alphabet[items[i]] == pattern[matched])
            matched++;
        count += matched == pattern.size();
        start = end;
      }
      return count;
    }
  };

  /**
   * \brief Suffixes of records supporting the assigned pattern prefix
   *
   * Each supporting record contributes one suffix beginning immediately after
   * its earliest match of the prefix. This leaves every possible extension
   * available for ordinary subsequences, where matching may skip any items.
   * Suffix positions refer to the shared immutable database; the suffix array
   * itself is copied with the space.
   */
  class Projection {
  protected:
    struct Suffix {
      int sequence, begin;
    };
    Suffix* suffixes;
    int size;
  public:
    Database data;
    int threshold;

    Projection(Space& home, const Database& database, int support)
      : suffixes(home.alloc<Suffix>(database.ends.size())),
        size(database.ends.size()), data(database), threshold(support) {
      for (int i=0; i<size; i++)
        suffixes[i] = {i,(i == 0) ? 0 : data.ends[i-1]};
    }

    Projection(Space& home, const Projection& p)
      : suffixes(home.alloc<Suffix>(p.size)), size(p.size),
        data(p.data), threshold(p.threshold) {
      std::copy(p.suffixes,p.suffixes+size,suffixes);
    }

    /// Extend the prefix by one item and report whether enough records support it
    bool advance(int item) {
      const int rank = static_cast<int>(std::lower_bound(
        data.alphabet.begin(),data.alphabet.end(),item)-data.alphabet.begin());
      if ((rank == data.alphabet.size()) || (data.alphabet[rank] != item))
        return false;
      int n = 0;
      for (int i=0; i<size; i++) {
        const int sequence = suffixes[i].sequence;
        int pos = suffixes[i].begin;
        const int end = data.ends[sequence];
        while ((pos < end) && (data.items[pos] != rank))
          pos++;
        if (pos < end)
          suffixes[n++] = {sequence,pos+1};
      }
      size = n;
      return size >= threshold;
    }

    int support(void) const { return size; }

    /**
     * \brief Items occurring in at least threshold supporting suffixes
     *
     * Last occurrences count each item at most once per record. They are
     * scanned in decreasing position order, so the scan stops at the suffix
     * boundary.
     */
    IntSet frequent(Region& region) const {
      const int d = data.alphabet.size();
      int* count = region.alloc<int>(d);
      std::fill(count,count+d,0);
      for (int i=0; i<size; i++) {
        const int sequence = suffixes[i].sequence;
        const int start = (sequence == 0) ? 0 : data.last_ends[sequence-1];
        for (int j=start; j<data.last_ends[sequence]; j++) {
          const int pos = data.last_positions[j];
          if (pos < suffixes[i].begin)
            break;
          count[data.items[pos]]++;
        }
      }
      int* values = region.alloc<int>(d);
      int n = 0;
      for (int a=0; a<d; a++)
        if (count[a] >= threshold)
          values[n++] = data.alphabet[a];
      return IntSet(values,n);
    }

    /// The threshold-th largest suffix length bounds every frequent extension
    int remaining(Region& region) const {
      int* sizes = region.alloc<int>(size);
      for (int i=0; i<size; i++)
        sizes[i] = data.ends[suffixes[i].sequence]-suffixes[i].begin;
      std::nth_element(sizes,sizes+threshold-1,sizes+size,std::greater<int>());
      return sizes[threshold-1];
    }
  };

  /**
   * \brief Private prefix projection for a growing list
   *
   * The projection advances only through the leading assigned positions.
   * Its frequent items filter every remaining materialized position. Adding
   * positions reuses the cached items and suffix-length bound until the
   * assigned prefix grows.
   *
   * The support variable is bounded by the prefix's supporting-record count
   * and becomes exact once the list is closed and fully assigned. See
   * CommonPattern for the source of the prefix-projection method.
   */
  class OpenProjection : public Propagator {
  protected:
    OpenIntVarList list;
    Int::IntView length, frequency;
    Projection projection;
    IntSet available;
    int consumed, subscribed;
    /// Filtering depends on the assigned prefix, not on constructed positions
    int cached_prefix, extension_bound;

    OpenProjection(Space& home, OpenIntVarList pattern,
                   const Database& database, int support, IntVar count)
      : Propagator(home), list(pattern), length(pattern.length()), frequency(count),
        projection(home,database,support), consumed(0),
        subscribed(pattern.known_length()), cached_prefix(-1), extension_bound(0) {
      home.notice(*this,AP_DISPOSE);
      list.subscribe(home,*this);
      length.subscribe(home,*this,Int::PC_INT_BND);
      for (int i=0; i<subscribed; i++)
        if (!list[i].assigned())
          Int::IntView(list[i]).subscribe(home,*this,Int::PC_INT_VAL);
    }

    OpenProjection(Space& home, OpenProjection& p)
      : Propagator(home,p), projection(home,p.projection), available(p.available),
        consumed(p.consumed), subscribed(p.subscribed),
        cached_prefix(p.cached_prefix), extension_bound(p.extension_bound) {
      list.update(home,p.list);
      length.update(home,p.length);
      frequency.update(home,p.frequency);
    }

  public:
    virtual Actor* copy(Space& home) { return new (home) OpenProjection(home,*this); }

    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,projection.data.items.size());
    }

    virtual void reschedule(Space& home) {
      list.reschedule(home,*this);
      length.reschedule(home,*this,Int::PC_INT_BND);
      for (int i=consumed; i<subscribed; i++)
        if (!list[i].assigned())
          Int::IntView(list[i]).reschedule(home,*this,Int::PC_INT_VAL);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      for (int i=subscribed; i<size; i++)
        if (!list[i].assigned())
          Int::IntView(list[i]).subscribe(home,*this,Int::PC_INT_VAL,false);
      subscribed = size;
      for (;;) {
        while ((consumed < size) && list[consumed].assigned()) {
          if (!projection.advance(list[consumed].val()))
            return ES_FAILED;
          consumed++;
        }
        GECODE_ME_CHECK(frequency.lq(home,projection.support()));
        if ((consumed == size) && (length.max() == size)) {
          GECODE_ME_CHECK(frequency.eq(home,projection.support()));
          return home.ES_SUBSUMED(*this);
        }
        if (cached_prefix != consumed) {
          Region region;
          extension_bound = projection.remaining(region);
          GECODE_ME_CHECK(length.lq(home,consumed+extension_bound));
          if ((consumed == size) && (length.max() == size)) {
            GECODE_ME_CHECK(frequency.eq(home,projection.support()));
            return home.ES_SUBSUMED(*this);
          }
          available = projection.frequent(region);
          cached_prefix = consumed;
        } else {
          GECODE_ME_CHECK(length.lq(home,consumed+extension_bound));
        }
        if (available.size() == 0) {
          GECODE_ME_CHECK(length.lq(home,consumed));
          if (consumed != size)
            return ES_FAILED;
          GECODE_ME_CHECK(frequency.eq(home,projection.support()));
          return home.ES_SUBSUMED(*this);
        }
        for (int i=consumed; i<size; i++) {
          IntSetValues values(available);
          GECODE_ME_CHECK(Int::IntView(list[i]).inter_v(home,values,false));
        }
        if ((consumed == size) || !list[consumed].assigned())
          return ES_FIX;
      }
    }

    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      list.cancel(home,*this);
      length.cancel(home,*this,Int::PC_INT_BND);
      if (!home.failed())
        for (int i=consumed; i<subscribed; i++)
          if (!list[i].assigned())
            Int::IntView(list[i]).cancel(home,*this,Int::PC_INT_VAL);
      list.~OpenIntVarList();
      projection.~Projection();
      available.~IntSet();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static void post(Home home, OpenIntVarList pattern,
                     const Database& database, int support, IntVar count) {
      (void) new (home) OpenProjection(home,pattern,database,support,count);
    }
  };

  /// Enforce minimum support and return its exact count when the pattern is complete
  inline IntVar
  frequent(Home home, OpenIntVarList pattern,
           const Database& database, int support) {
    if (home.failed())
      return IntVar();
    if ((support < 1) || (support > database.ends.size())) {
      home.fail();
      return IntVar();
    }
    IntVar count(home,support,database.ends.size());
    OpenProjection::post(home,pattern,database,support,count);
    return count;
  }

  /// Built-in motif and benchmark languages described in CommonPattern
  inline REG
  language(Language name, const Database& database) {
    if (name == RE10)
      return *REG(1) + REG(2) + (REG(2) | REG(3)) + *REG(4) +
        REG(5) + *REG(6) + (REG(7) | REG(8)) + *REG(9);
    if (name == RE14)
      return *REG(1) + (REG(17) | (REG(2) + *REG(19) +
        (REG(2) | REG(3)))) + *REG(4) + REG(5) + *(REG(9) | REG(19)) +
        (REG(6) | REG(8)) + *REG(7) + REG(18);
    REG alphabet(IntArgs(database.alphabet.size(),database.alphabet.begin()));
    if (name == MOTIF)
      return REG(1) + *alphabet + REG(3);
    return (REG(19) | REG(20)) + alphabet + (REG(18) | REG(11));
  }

}

#endif
