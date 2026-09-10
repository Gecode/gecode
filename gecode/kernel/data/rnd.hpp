/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <ctime>
#include <memory>
#include <vector>

namespace Gecode {

  class RandomContext;

  /**
   * \brief Handle to a random stream
   *
   * Standalone handles share a stream. Binding to a space creates a local
   * stream, shared by handles bound from the same source. Space clones have
   * independent stream state. Use copy() for an independent standalone copy.
   * A stream must not be drawn from concurrently.
   * \ingroup TaskModel
   */
  class Rnd : public SharedHandle {
    friend class RandomContext;
  private:
    class Origin : public SharedHandle {
    public:
      Origin(void) = default;
      explicit Origin(Object* value) : SharedHandle(value) {}
      const Object* get(void) const { return object(); }
    };
    class IMP : public SharedHandle::Object {
    public:
      Origin origin;
      const Object* identity(void) const {
        return origin ? origin.get() : this;
      }
      virtual IMP* copy(void) const = 0;
      virtual IMP* split(uint32_t a) const = 0;
      virtual void seed(uint64_t value) = 0;
      virtual uint64_t draw(uint64_t bound) = 0;
      virtual size_t words(void) const = 0;
      virtual void save(uint64_t* out) const = 0;
      virtual void commit(const uint64_t* in, uint32_t a) = 0;
      virtual std::string state(void) const = 0;
      virtual void state(const std::string& text) = 0;
      virtual const char* name(void) const = 0;
    };
    template<class Engine>
    class Implementation : public IMP {
      Support::Random<Engine> r;
    public:
      explicit Implementation(const Support::Random<Engine>& value) : r(value) {}
      IMP* copy(void) const override { return new Implementation(r); }
      IMP* split(uint32_t a) const override {
        return new Implementation(r.split(a));
      }
      void seed(uint64_t value) override { r.seed(value); }
      uint64_t draw(uint64_t bound) override { return r(bound); }
      size_t words(void) const override {
        return std::tuple_size<typename Engine::State>::value;
      }
      void save(uint64_t* out) const override {
        auto s = r.state();
        std::copy(s.begin(),s.end(),out);
      }
      void commit(const uint64_t* in, uint32_t a) override {
        typename Engine::State s;
        std::copy(in,in+s.size(),s.begin());
        auto parent = r;
        parent.state(s);
        r = parent.split(a);
      }
      std::string state(void) const override { return r.state_string(); }
      void state(const std::string& text) override { r.state(text); }
      const char* name(void) const override { return Engine::name(); }
    };
    IMP& imp(void) const {
      if (!object())
        throw UninitializedRnd("Rnd");
      return *static_cast<IMP*>(object());
    }
    Rnd(IMP* value, bool) : SharedHandle(value) {}
    Rnd local_copy(void) const {
      Rnd result(imp().copy(),true);
      result.imp().origin = imp().origin ? imp().origin : Origin(&imp());
      return result;
    }
  public:
    /// Uninitialized handle
    Rnd(void) = default;
    /// Share an existing handle
    Rnd(const Rnd&) = default;
    Rnd& operator =(const Rnd&) = default;
    ~Rnd(void) = default;
    /// Create a standalone default stream
    explicit Rnd(uint64_t seed)
      : Rnd(Support::RandomGenerator(seed)) {}
    /// Create a standalone user-defined splittable stream
    template<class Engine>
    explicit Rnd(const Support::Random<Engine>& r)
      : SharedHandle(new Implementation<Engine>(r)) {}
    /// Bind a source stream to a space, or update its handle during cloning
    GECODE_KERNEL_EXPORT Rnd(Space& home, const Rnd& source);
    /// Access an already bound stream through a const space
    GECODE_KERNEL_EXPORT Rnd(const Space& home, const Rnd& source);
    /// Create and bind a default stream to a space
    Rnd(Space& home, uint64_t seed) : Rnd(home,Rnd(seed)) {}
    /// Make an independent exact copy; does not split or advance the source
    Rnd copy(void) const { return Rnd(imp().copy(),true); }
    /// Derive an alternative stream without modifying this stream
    Rnd split(uint32_t a) const { return Rnd(imp().split(a),true); }
    /// Seed this stream (initializes a default engine if uninitialized)
    GECODE_KERNEL_EXPORT void seed(uint64_t value);
    /// Initialize using time or hardware entropy
    GECODE_KERNEL_EXPORT void time(void);
    GECODE_KERNEL_EXPORT void hw(void);
    /// Complete state, with algorithm identifier
    std::string state(void) const { return imp().state(); }
    /// Restore a state for this engine (default engine if uninitialized)
    GECODE_KERNEL_EXPORT void state(const std::string& text);
    const char* name(void) const { return imp().name(); }
    /// Number of 64-bit words in the engine state
    size_t words(void) const { return imp().words(); }
    /// Draw an integer in [0,bound); bounds <= 1 consume no values
    template<class Type>
    Type operator ()(Type bound) {
      static_assert(std::is_integral<Type>::value && sizeof(Type)<=8,
                    "Random bound must be an integer of at most 64 bits");
      return bound<=1 ? 0 :
        static_cast<Type>(imp().draw(static_cast<uint64_t>(bound)));
    }
  };

  /// Internal space-local stream storage. Local handles retain their stream origin.
  class RandomContext {
    std::vector<Rnd> streams;
  public:
    RandomContext(void) = default;
    RandomContext(const RandomContext& other) {
      streams.reserve(other.streams.size());
      for (const auto& entry : other.streams)
        streams.push_back(entry.local_copy());
    }
    size_t find(const Rnd& source) const {
      for (size_t i=0; i<streams.size(); ++i)
        if (streams[i].imp().identity()==source.imp().identity())
          return i;
      return streams.size();
    }
    Rnd at(size_t i) const { return streams[i]; }
    size_t size(void) const { return streams.size(); }
    Rnd bind(const Rnd& source) {
      size_t i = find(source);
      if (i<streams.size())
        return streams[i];
      streams.push_back(source.local_copy());
      return streams.back();
    }
    /// Allocate one packed snapshot: word count followed by all engine states.
    uint64_t* snapshot(void) const {
      size_t n=0;
      for (const auto& entry : streams)
        n += entry.words();
      auto data = std::make_unique<uint64_t[]>(n+1);
      data[0]=n;
      size_t pos=1;
      for (const auto& entry : streams) {
        entry.imp().save(data.get()+pos);
        pos += entry.words();
      }
      return data.release();
    }
    /// Restore and split the recorded streams; layout follows registration order.
    void commit(const uint64_t* data, uint32_t a) {
      size_t n=0;
      for (const auto& entry : streams)
        n += entry.words();
      if (data[0] != n)
        throw std::invalid_argument("Random choice does not match space streams");
      size_t pos=1;
      for (auto& entry : streams) {
        entry.imp().commit(data+pos,a);
        pos += entry.words();
      }
    }
  };
}

// STATISTICS: kernel-other
