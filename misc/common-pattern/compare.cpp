/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 * Benchmark companion to examples/common-pattern.cpp.
 * Copyright Mikael Zayenz Lagerkvist, 2026.
 * Distributed under the MIT license; see LICENSE.
 */

#include <examples/common-pattern.hh>
#include <gecode/search.hh>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace Gecode;
using namespace PatternMining;

namespace {

  /// The representations compared on identical enumeration problems
  enum class Model {
    OpenDFA, PaddedDFA, OpenProjection, PaddedProjection, PaperProjection
  };

  /// Earliest-match subsequence DFA, optionally accepting trailing zeroes
  DFA
  subsequences(const Database& database, int sequence, bool padded) {
    const int start = (sequence == 0) ? 0 : database.ends[sequence-1];
    const int n = database.ends[sequence]-start;
    std::vector<int> next(database.alphabet.size(),-1);
    std::vector<DFA::Transition> transitions;
    for (int i=n; i--; ) {
      next[database.items[start+i]] = i+1;
      for (int a=0; a<database.alphabet.size(); a++)
        if (next[a] >= 0)
          transitions.emplace_back(i,database.alphabet[a],next[a]);
    }
    if (padded) {
      for (int i=0; i<=n+1; i++)
        transitions.emplace_back(i,0,n+1);
    }
    transitions.emplace_back(-1,0,0);
    const int states = n+1+static_cast<int>(padded);
    std::vector<int> finals(states+1);
    for (int i=0; i<states; i++)
      finals[i] = i;
    finals[states] = -1;
    return DFA(0,transitions.data(),finals.data());
  }

  /**
   * \brief CP2015 Algorithm 2, with an optional extension-length bound
   *
   * The paper baseline filters after contiguous prefix assignments. The
   * padded-projection variant also filters initially and caps remaining
   * nonzero positions, matching the shared open projection's extra pruning.
   * Both retain the paper's fixed positions and zero termination.
   */
  class PaperProjection : public NaryPropagator<Int::IntView,Int::PC_INT_VAL> {
  protected:
    Projection projection;
    /// Number of consumed contiguous pattern positions
    int consumed;
    /// Match the open projection's initial filtering and suffix-length cap
    bool bounded;
  public:
    PaperProjection(Space& home, ViewArray<Int::IntView>& x,
                    const Database& database, int support, bool cap)
      : NaryPropagator<Int::IntView,Int::PC_INT_VAL>(home,x),
        projection(home,database,support), consumed(0), bounded(cap) {
      home.notice(*this,AP_DISPOSE);
      if (bounded)
        Int::IntView::schedule(home,*this,Int::ME_INT_VAL);
    }

    PaperProjection(Space& home, PaperProjection& p)
      : NaryPropagator<Int::IntView,Int::PC_INT_VAL>(home,p),
        projection(home,p.projection), consumed(p.consumed), bounded(p.bounded) {}

    virtual Actor* copy(Space& home) {
      return new (home) PaperProjection(home,*this);
    }

    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,projection.data.items.size());
    }

    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      projection.~Projection();
      (void) NaryPropagator<Int::IntView,Int::PC_INT_VAL>::dispose(home);
      return sizeof(*this);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      for (;;) {
        while ((consumed < x.size()) && x[consumed].assigned()) {
          const int item = x[consumed].val();
          if (item == 0) {
            for (int j=consumed+1; j<x.size(); j++)
              GECODE_ME_CHECK(x[j].eq(home,0));
            return home.ES_SUBSUMED(*this);
          }
          if (!projection.advance(item))
            return ES_FAILED;
          consumed++;
        }
        if (consumed == x.size())
          return home.ES_SUBSUMED(*this);

        Region region;
        if (bounded) {
          const int end = consumed+projection.remaining(region);
          for (int j=end; j<x.size(); j++)
            GECODE_ME_CHECK(x[j].eq(home,0));
        }
        const IntSet allowed = projection.frequent(region,true);
        for (int j=consumed; j<x.size(); j++) {
          IntSetValues values(allowed);
          GECODE_ME_CHECK(x[j].inter_v(home,values,false));
        }
        // Filtering can assign the next position: consume it before fixing.
        if (!x[consumed].assigned())
          return ES_FIX;
      }
    }
  };

  /// One enumeration problem using a selected pattern representation
  class Pattern : public Space {
  protected:
    OpenIntVarList open;
    IntVarArray padded;
    Model model;
  public:
    Pattern(const Database& database, Model m, Language restriction, int bound,
            int support, int minimum_length, const std::vector<int>& required)
      : model(m) {
      if (minimum_length > bound) {
        fail();
        return;
      }
      const bool growing = (model == Model::OpenDFA) ||
        (model == Model::OpenProjection);
      if (growing) {
        open = OpenIntVarList(*this,database.domain(),bound);
        rel(*this,open.length(),IRT_GQ,minimum_length);
        if (model == Model::OpenDFA) {
          for (int i=0; i<database.ends.size(); i++)
            extensional(*this,open,subsequences(database,i,false));
        } else {
          frequent(*this,open,database,support);
        }
      } else {
        std::vector<int> values(1,0);
        values.insert(values.end(),database.alphabet.begin(),database.alphabet.end());
        padded = IntVarArray(*this,bound,IntSet(values.data(),values.size()));
        for (int i=0; i<minimum_length; i++)
          rel(*this,padded[i],IRT_NQ,0);
        if (model == Model::PaddedDFA) {
          for (int i=0; i<database.ends.size(); i++)
            extensional(*this,padded,subsequences(database,i,true));
        } else {
          ViewArray<Int::IntView> x(*this,IntVarArgs(padded));
          (void) new (*this) PaperProjection(*this,x,database,support,
                                            model == Model::PaddedProjection);
        }
      }
      if (restriction != NONE) {
        REG expression = language(restriction,database);
        if (growing)
          extensional(*this,open,expression);
        else
          extensional(*this,padded,expression + *REG(0));
      }
      REG alphabet(IntArgs(database.alphabet.size(),database.alphabet.begin()));
      for (int item : required) {
        REG contains = *alphabet + REG(item) + *alphabet;
        if (growing)
          extensional(*this,open,contains);
        else
          extensional(*this,padded,contains + *REG(0));
      }
      if (growing)
        branch(*this,open,INT_VAR_NONE(),INT_VAL_MIN());
      else
        branch(*this,padded,INT_VAR_NONE(),INT_VAL_MIN());
    }

    Pattern(Pattern& s) : Space(s), model(s.model) {
      if ((model == Model::OpenDFA) || (model == Model::OpenProjection))
        open.update(*this,s.open);
      else
        padded.update(*this,s.padded);
    }

    virtual Space* copy(void) { return new Pattern(*this); }

    /// Emit actual items only, in the same format for all representations
    void print(Format format) const {
      const bool growing = (model == Model::OpenDFA) ||
        (model == Model::OpenProjection);
      const int n = growing ? open.prefix_size() : padded.size();
      for (int i=0; i<n; i++) {
        const int item = growing ? open[i].val() : padded[i].val();
        if (item == 0)
          break;
        if (format == LETTERS)
          std::cout << static_cast<char>('A'+item-1);
        else {
          if (i > 0)
            std::cout << ' ';
          std::cout << item;
        }
      }
      std::cout << '\n';
    }
  };

  /// Strictly parse positive query integers without accepting trailing text
  int
  positive_integer(const std::string& text) {
    const std::vector<int> items = read_sequence(text,INTEGERS);
    if (items.size() != 1)
      throw std::invalid_argument("Expected one positive integer");
    return items[0];
  }

  std::vector<int>
  required_items(const std::string& text) {
    std::vector<int> required;
    if (text == "-")
      return required;
    if (text.empty() || (text.back() == ','))
      throw std::invalid_argument("Expected comma-separated positive required items");
    std::istringstream stream(text);
    std::string item;
    while (std::getline(stream,item,','))
      required.push_back(positive_integer(item));
    return required;
  }

}

int
main(int argc, char* argv[]) {
  if ((argc < 6) || (argc > 10)) {
    std::cerr << "Usage: compare MODEL none|motif|re10|re14|re2 FILE "
              << "print|count max|shortest [SUPPORT_PERCENT [MIN_LENGTH "
              << "[REQUIRED_ITEMS [letters|integers|spmf]]]]\n"
              << "Models: open-dfa, padded-dfa, open-projection, "
              << "padded-projection, paper-projection\n";
    return 1;
  }
  try {
    const std::string variant(argv[1]), restriction(argv[2]),
      output(argv[4]), length_bound(argv[5]);
    Model model;
    if (variant == "open-dfa") model = Model::OpenDFA;
    else if (variant == "padded-dfa") model = Model::PaddedDFA;
    else if (variant == "open-projection") model = Model::OpenProjection;
    else if (variant == "padded-projection") model = Model::PaddedProjection;
    else if (variant == "paper-projection") model = Model::PaperProjection;
    else throw std::invalid_argument("Unknown comparison model");
    Language query;
    if (restriction == "none") query = NONE;
    else if (restriction == "motif") query = MOTIF;
    else if (restriction == "re10") query = RE10;
    else if (restriction == "re14") query = RE14;
    else if (restriction == "re2") query = RE2;
    else throw std::invalid_argument("Unknown pattern language");
    if (((output != "print") && (output != "count")) ||
        ((length_bound != "max") && (length_bound != "shortest")))
      throw std::invalid_argument("Expected print|count and max|shortest");
    Format format = LETTERS;
    if (argc > 9) {
      const std::string name(argv[9]);
      if (name == "integers") format = INTEGERS;
      else if (name == "spmf") format = SPMF;
      else if (name != "letters")
        throw std::invalid_argument("Unknown database format");
    }
    const std::string percent = (argc > 6) ? argv[6] : "100";
    (void) minimum_support(percent,1);
    const int minimum_length = (argc > 7) ? positive_integer(argv[7]) : 1;
    const std::vector<int> required = required_items((argc > 8) ? argv[8] : "-");
    std::ifstream file(argv[3]);
    const Sequences input = read_database(file,format);

    const auto start = std::chrono::steady_clock::now();
    const Database database(input);
    const int support = minimum_support(percent,database.ends.size());
    if ((support != database.ends.size()) &&
        ((model == Model::OpenDFA) || (model == Model::PaddedDFA) ||
         (length_bound == "shortest")))
      throw std::invalid_argument("DFA models and shortest bound require all-input support");
    int bound = database.max_length();
    if (length_bound == "shortest")
      for (const std::vector<int>& sequence : input)
        bound = std::min(bound,static_cast<int>(sequence.size()));
    Pattern* root = new Pattern(database,model,query,bound,support,
                                minimum_length,required);
    Search::Options options;
    options.threads = 1;
    options.clone = false;
    DFS<Pattern> engine(root,options);
    unsigned long long solutions = 0;
    while (Pattern* solution = engine.next()) {
      solutions++;
      if (output == "print")
        solution->print(format);
      delete solution;
    }
    const auto end = std::chrono::steady_clock::now();
    const Search::Statistics stats = engine.statistics();
    std::cerr << "{\"solutions\":" << solutions
              << ",\"nodes\":" << stats.node
              << ",\"failures\":" << stats.fail
              << ",\"propagations\":" << stats.propagate
              << ",\"model_search_seconds\":"
              << std::chrono::duration<double>(end-start).count() << "}\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
