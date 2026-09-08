/* Internal posting context for mixed Boolean and word expressions. */
#ifndef GECODE_MINIMODEL_WORD_POST_HH
#define GECODE_MINIMODEL_WORD_POST_HH

#include <gecode/minimodel.hh>
#include <unordered_map>

namespace Gecode { namespace MiniModel {
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
    Cache cache;
    const Space* space;
    unsigned int group;
    unsigned int branch_group;
    const Propagator* propagator;
    explicit WordPostContext(Home home)
      : space(&static_cast<Space&>(home)), group(home.propagatorgroup().id()),
        branch_group(home.branchergroup().id()), propagator(home.propagator()) {}
    bool matches(Home home) const {
      return space == &static_cast<Space&>(home) &&
        group == home.propagatorgroup().id() &&
        branch_group == home.branchergroup().id() &&
        propagator == home.propagator();
    }
    ~WordPostContext(void);
    WordPostContext(const WordPostContext&) = delete;
    WordPostContext& operator =(const WordPostContext&) = delete;
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
