/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 * Private modelling support for the common-pattern example and its comparison.
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

  /// Convert a percentage with at most six decimals to an exact sequence count
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
    IntSharedArray items, ends, alphabet;

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
      int k = 0, i = 0;
      for (const std::vector<int>& sequence : input) {
        for (int item : sequence)
          items[k++] = static_cast<int>(std::lower_bound(
            symbols.begin(),symbols.end(),item)-symbols.begin());
        ends[i++] = k;
      }
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

  /// Current pseudo-projection, shared by the open and padded implementations
  class Projection {
  public:
    Database data;
    int threshold;
    int* cursor;

    Projection(Space& home, const Database& database, int support)
      : data(database), threshold(support),
        cursor(home.alloc<int>(data.ends.size())) {
      for (int i=0; i<data.ends.size(); i++)
        cursor[i] = (i == 0) ? 0 : data.ends[i-1];
    }

    Projection(Space& home, const Projection& p)
      : data(p.data), threshold(p.threshold),
        cursor(home.alloc<int>(data.ends.size())) {
      std::copy(p.cursor,p.cursor+data.ends.size(),cursor);
    }

    /// Advance supporting records past the earliest occurrence of an item
    bool advance(int item) {
      const int rank = static_cast<int>(std::lower_bound(
        data.alphabet.begin(),data.alphabet.end(),item)-data.alphabet.begin());
      if ((rank == data.alphabet.size()) || (data.alphabet[rank] != item))
        return false;
      int count = 0;
      for (int i=0; i<data.ends.size(); i++) {
        if (cursor[i] < 0)
          continue;
        while ((cursor[i] < data.ends[i]) && (data.items[cursor[i]] != rank))
          cursor[i]++;
        if (cursor[i] == data.ends[i])
          cursor[i] = -1;
        else {
          cursor[i]++;
          count++;
        }
      }
      return count >= threshold;
    }

    /// Locally frequent items, counting each item once per supporting record
    IntSet frequent(Region& region, bool termination) const {
      const int d = data.alphabet.size();
      int* count = region.alloc<int>(d);
      int* seen = region.alloc<int>(d);
      std::fill(count,count+d,0);
      std::fill(seen,seen+d,-1);
      for (int i=0; i<data.ends.size(); i++) {
        if (cursor[i] < 0)
          continue;
        for (int j=cursor[i]; j<data.ends[i]; j++) {
          const int rank = data.items[j];
          if (seen[rank] != i) {
            seen[rank] = i;
            count[rank]++;
          }
        }
      }
      int* values = region.alloc<int>(d+1);
      int n = 0;
      if (termination)
        values[n++] = 0;
      for (int a=0; a<d; a++)
        if (count[a] >= threshold)
          values[n++] = data.alphabet[a];
      return IntSet(values,n);
    }

    /// The threshold-th largest suffix length bounds every frequent extension
    int remaining(Region& region) const {
      int* sizes = region.alloc<int>(data.ends.size());
      int n = 0;
      for (int i=0; i<data.ends.size(); i++)
        if (cursor[i] >= 0)
          sizes[n++] = data.ends[i]-cursor[i];
      std::nth_element(sizes,sizes+threshold-1,sizes+n,std::greater<int>());
      return sizes[threshold-1];
    }
  };

  /**
   * \brief Private prefix projection for a growing list
   *
   * Adapts CP2015 Algorithms 1--2: no termination item, future variables or
   * per-input DFA. Unconstructed positions are filtered when they appear.
   * Earliest matching is sound for ordinary subsequences, not gap constraints.
   */
  class OpenProjection : public Propagator {
  protected:
    OpenIntVarList list;
    Int::IntView length;
    Projection projection;
    int consumed, subscribed;

    OpenProjection(Space& home, OpenIntVarList pattern,
                   const Database& database, int support)
      : Propagator(home), list(pattern), length(pattern.length()),
        projection(home,database,support), consumed(0),
        subscribed(pattern.prefix_size()) {
      home.notice(*this,AP_DISPOSE);
      list.subscribe(home,*this);
      length.subscribe(home,*this,Int::PC_INT_BND);
      for (int i=0; i<subscribed; i++)
        if (!list[i].assigned())
          Int::IntView(list[i]).subscribe(home,*this,Int::PC_INT_VAL);
    }

    OpenProjection(Space& home, OpenProjection& p)
      : Propagator(home,p), projection(home,p.projection),
        consumed(p.consumed), subscribed(p.subscribed) {
      list.update(home,p.list);
      length.update(home,p.length);
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
      const int size = list.prefix_size();
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
        Region region;
        GECODE_ME_CHECK(length.lq(home,consumed+projection.remaining(region)));
        if ((consumed == size) && (length.max() == size))
          return home.ES_SUBSUMED(*this);
        const IntSet available = projection.frequent(region,false);
        if (available.size() == 0) {
          GECODE_ME_CHECK(length.lq(home,consumed));
          return (consumed == size) ? home.ES_SUBSUMED(*this) : ES_FAILED;
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
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static void post(Home home, OpenIntVarList pattern,
                     const Database& database, int support) {
      if (home.failed())
        return;
      if ((support < 1) || (support > database.ends.size())) {
        home.fail();
        return;
      }
      (void) new (home) OpenProjection(home,pattern,database,support);
    }
  };

  /// Require at least support records to contain the pattern as a subsequence
  inline void
  frequent(Home home, OpenIntVarList pattern,
           const Database& database, int support) {
    OpenProjection::post(home,pattern,database,support);
  }

  /// Pattern languages from the example and CP2015 Section 5(d)
  inline REG
  language(Language name, const Database& database) {
    REG alphabet(IntArgs(database.alphabet.size(),database.alphabet.begin()));
    if (name == MOTIF)
      return REG(1) + *alphabet + REG(3);
    if (name == RE10)
      return *REG(1) + REG(2) + (REG(2) | REG(3)) + *REG(4) +
        REG(5) + *REG(6) + (REG(7) | REG(8)) + *REG(9);
    if (name == RE14)
      return *REG(1) + (REG(17) | (REG(2) + *REG(19) +
        (REG(2) | REG(3)))) + *REG(4) + REG(5) + *(REG(9) | REG(19)) +
        (REG(6) | REG(8)) + *REG(7) + REG(18);
    return (REG(19) | REG(20)) + alphabet + (REG(18) | REG(11));
  }

}

#endif
