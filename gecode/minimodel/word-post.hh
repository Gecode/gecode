/* Internal posting context for mixed Boolean and word expressions. */
#ifndef GECODE_MINIMODEL_WORD_POST_HH
#define GECODE_MINIMODEL_WORD_POST_HH

#include <gecode/minimodel.hh>
#include <utility>

namespace Gecode { namespace MiniModel {
  /// One posting operation, retaining one node reference per cached key.
  /// The Home identity is immutable; the context must not outlive its Space.
  class WordPostContext {
  public:
    struct Key {
      const WordExpr::Node* node;
      WordDomainType domain_type;
      bool operator ==(const Key& other) const {
        return node == other.node && domain_type == other.domain_type;
      }
    };
    typedef std::pair<Key,WordVar> Entry;
    /// Cached entries in insertion order
    class Entries {
    public:
      Entries(const Entry* first, const Entry* last)
        : first(first), last(last) {}
      const Entry* begin(void) const { return first; }
      const Entry* end(void) const { return last; }
    private:
      const Entry* first;
      const Entry* last;
    };
    explicit WordPostContext(Home home)
      : space(&static_cast<Space&>(home)), group(home.propagatorgroup().id()),
        branch_group(home.branchergroup().id()), propagator(home.propagator()),
        entry_array(nullptr), entry_count(0), entry_capacity(0),
        slots(nullptr) {}
    /// Inspect lowered values without transferring ownership of their nodes.
    Entries entries(void) const {
      return Entries(entry_array,entry_array+entry_count);
    }
    GECODE_MINIMODEL_EXPORT const WordVar* find(const Key& key) const;
    /// Retain the node only after successful insertion of a new key.
    GECODE_MINIMODEL_EXPORT void insert(const Key& key, WordVar value);
    bool matches(Home home) const {
      return space == &static_cast<Space&>(home) &&
        group == home.propagatorgroup().id() &&
        branch_group == home.branchergroup().id() &&
        propagator == home.propagator();
    }
    GECODE_MINIMODEL_EXPORT ~WordPostContext(void);
    WordPostContext(const WordPostContext&) = delete;
    WordPostContext& operator =(const WordPostContext&) = delete;
  private:
    const Space* const space;
    const unsigned int group;
    const unsigned int branch_group;
    const Propagator* const propagator;
    /// Heap-allocated entries, the first \a entry_count of them in use
    Entry* entry_array;
    int entry_count;
    int entry_capacity;
    /// Open-addressing table of indices into \a entry_array, -1 when empty;
    /// it has twice \a entry_capacity slots
    int* slots;
    /// Return the slot holding \a key, or the empty slot where it belongs
    int slot(const Key& key) const;
    /// Grow both arrays so that one more entry fits
    void grow(void);
  };

  /// Access to the private expression posting that shares a context
  class WordPostAccess {
  public:
    static BoolVar expr(const BoolExpr& e, Home home, const IntPropLevels& ipls,
                        WordPostContext* context) {
      return e.expr(home,ipls,context);
    }
    static WordVar post(const WordExpr& e, Home home,
                        WordDomainType domain_type, WordPostContext* context) {
      return e.post(home,domain_type,context);
    }
  };

  // Separate internal interface: BoolExpr::Misc's public vtable is unchanged.
  class WordMisc : public BoolExpr::Misc {
  public:
    void post(Home home, BoolVar b, bool neg, const IntPropLevels& ipls) {
      WordPostContext context(home);
      post(home,b,neg,ipls,&context);
    }
    virtual void post(Home home, BoolVar b, bool neg,
                      const IntPropLevels& ipls, WordPostContext* context) = 0;
  };
}}
#endif

// STATISTICS: minimodel-any
