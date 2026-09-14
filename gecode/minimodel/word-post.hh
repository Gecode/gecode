/* Internal posting context for mixed Boolean and word expressions. */
#ifndef GECODE_MINIMODEL_WORD_POST_HH
#define GECODE_MINIMODEL_WORD_POST_HH

#include <gecode/minimodel.hh>
#include <unordered_map>

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
    struct Hash {
      size_t operator ()(const Key& key) const {
        return std::hash<const WordExpr::Node*>()(key.node) ^
          static_cast<size_t>(key.domain_type);
      }
    };
    typedef std::unordered_map<Key,WordVar,Hash> Cache;
    explicit WordPostContext(Home home)
      : space(&static_cast<Space&>(home)), group(home.propagatorgroup().id()),
        branch_group(home.branchergroup().id()), propagator(home.propagator()) {}
    /// Inspect lowered values without transferring ownership of their nodes.
    const Cache& entries(void) const { return cache; }
    const WordVar* find(const Key& key) const {
      const auto entry = cache.find(key);
      return entry == cache.end() ? nullptr : &entry->second;
    }
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
    Cache cache;
    const Space* const space;
    const unsigned int group;
    const unsigned int branch_group;
    const Propagator* const propagator;
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
