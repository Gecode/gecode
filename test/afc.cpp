/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include "test/test.hh"

#include <memory>

namespace Test {

  /// %Test for %AFC infrastructure
  class AFC : public Test::Base {
  protected:
    /// Test space
    class TestSpace : public Gecode::Space {
    protected:
      /// Two integer variables
      Gecode::IntVar x, y;
    public:
      /// Constructor for creation
      TestSpace(void) : x(*this,0,10), y(*this,0,10) {}
      /// Constructor for cloning \a s
      TestSpace(TestSpace& s) : Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
      }
      /// Post arbitrary propagator
      void post(void) {
        Gecode::rel(*this, x, Gecode::IRT_LE, y);
      }
      /// Copy during cloning
      virtual Space* copy(void) {
        return new TestSpace(*this);
      }
    };
    /// How many test operations to be performed
    static const int n_ops = 8 * 1024;
    /// How many spaces to maintain
    static const int n = 16;
    /// Return random index of non-null space
    int space(TestSpace* s[]) {
      int i = _rand(n);
      while (s[i] == nullptr)
        i = (i+1) % n;
      return i;
    }
    /// Return random index
    int index(void) {
      return _rand(n);
    }
  public:
    /// Initialize test
    AFC(void) : Test::Base("AFC") {}
    /// Perform actual tests
    bool run(void) {
      // Array of spaces for tests
      TestSpace* s[n];
      // How many spaces exist in s
      int n_s = 1;

      for (int i=n; i--; )
        s[i] = nullptr;
      s[0] = new TestSpace;

      for (int o=n_ops; o--; )
        switch (_rand(3)) {
        case 0:
          // clone space
          {
            int i = index();
            if ((s[i] != nullptr)) {
              if (n_s > 1) {
                delete s[i]; s[i]=nullptr; n_s--;
              } else {
                break;
              }
            }
            int j = space(s);
            (void) s[j]->status();
            s[i] = static_cast<TestSpace*>(s[j]->clone());
            n_s++;
          }
          break;
        case 1:
          // delete space
          if (n_s > 1) {
            int i = space(s);
            delete s[i]; s[i]=nullptr; n_s--;
          }
          break;
        case 2:
          // post propagator
          s[space(s)]->post();
          break;
        default:
          GECODE_NEVER;
        }
      // Delete all remaining spaces
      for (int i=n; i--; )
        delete s[i];
      return true;
    }
  };

  /// Scoped suppression of global propagator information
  class NoGPI : public Test::Base {
    /// Space with variables for posting and rewriting relations
    class TestSpace : public Gecode::Space {
    public:
      Gecode::IntVar x, y;
      Gecode::BoolVar b;
      TestSpace(void) : x(*this,0,10), y(*this,0,10), b(*this,0,1) {}
      TestSpace(TestSpace& s) : Space(s) {
        x.update(*this,s.x); y.update(*this,s.y); b.update(*this,s.b);
      }
      virtual Space* copy(void) { return new TestSpace(*this); }
      /// Find a surviving propagator by its identifier
      Gecode::Propagator* find(unsigned int id) {
        for (Propagators ps(*this); ps(); ++ps)
          if (ps.propagator().id() == id)
            return &ps.propagator();
        return nullptr;
      }
      /// Post a non-subsumed relation and return its propagator
      Gecode::Propagator& post(Gecode::PropagatorGroup g =
                              Gecode::PropagatorGroup::def) {
        Gecode::rel((*this)(g), x, Gecode::IRT_LE, y);
        Gecode::Propagator* p = nullptr;
        for (Propagators ps(*this); ps(); ++ps)
          if ((p == nullptr) || (ps.propagator().id() > p->id()))
            p = &ps.propagator();
        return *p;
      }
    };

    /// Check nesting, unwinding, isolation, and posting in a clone
    bool scopes(void) {
      using namespace Gecode;
      TestSpace s, other;
      if (s.post().afc() != 1.0)
        return false;
      {
        Space::NoGPI outer(s);
        if ((s.post().afc() != 0.0) || (other.post().afc() != 1.0))
          return false;
        try {
          Space::NoGPI inner(s);
          if (s.post().afc() != 0.0)
            return false;
          throw 1;
        } catch (int) {}
        if (s.post().afc() != 0.0)
          return false;
        {
          Space::UseGPI use(s);
          if (s.post().afc() != 1.0)
            return false;
          {
            Space::NoGPI suppress(s);
            if (s.post().afc() != 0.0)
              return false;
          }
          if (s.post().afc() != 1.0)
            return false;
        }
        if (s.post().afc() != 0.0)
          return false;
        try {
          Space::UseGPI use(s);
          if (s.post().afc() != 1.0)
            return false;
          throw 1;
        } catch (int) {}
        if (s.post().afc() != 0.0)
          return false;
        (void) s.status();
        std::unique_ptr<TestSpace> c(static_cast<TestSpace*>(s.clone()));
        if (c->post().afc() != 1.0)
          return false;
      }
      try {
        Space::NoGPI context(s);
        throw 1;
      } catch (int) {}
      return s.post().afc() == 1.0;
    }

    /// Check local groups, disabled pointer tags, and lifetime after cloning
    bool lifetime(void) {
      using namespace Gecode;
      std::unique_ptr<TestSpace> s(new TestSpace);
      PropagatorGroup a, b;
      unsigned int id;
      {
        Space::NoGPI context(*s);
        Propagator& p = s->post(a);
        id = p.id();
        if ((p.group() != a) || (s->post().id() == id))
          return false;
      }
      a.disable(*s);
      (void) s->status();
      std::unique_ptr<TestSpace> c(static_cast<TestSpace*>(s->clone()));
      // The first call marks the shared pool; the second really unshares.
      s->afc_unshare();
      c->afc_unshare();
      Propagator* p = c->find(id);
      if (!p || !p->disabled() || (p->afc() != 0.0))
        return false;
      b.move(*c,id);
      if ((p->group() != b) || (s->find(id)->group() != a))
        return false;
      s.reset();
      b.enable(*c);
      if (p->disabled() || (c->status() == SS_FAILED))
        return false;
      b.kill(*c);
      return (c->find(id) == nullptr) && (b.size(*c) == 0);
    }

    /// Check AFC failure accounting, initialization, and rescaling
    bool failure(bool local) {
      using namespace Gecode;
      TestSpace s;
      Propagator* p;
      if (local) {
        Space::NoGPI context(s);
        p = &s.post();
      } else {
        p = &s.post();
      }
      (void) s.status();
      std::unique_ptr<TestSpace> c(static_cast<TestSpace*>(s.clone()));
      // Force rescaling on failure for the ordinary propagator.
      IntAFC afc(*c,IntVarArgs() << c->x << c->y,1.0e-300,false);
      c->afc_unshare();
      Propagator* q = c->find(p->id());
      if (!q || (q->afc() != (local ? 0.0 : 1.0)))
        return false;
      rel(*c,c->x,IRT_EQ,5);
      rel(*c,c->y,IRT_EQ,4);
      if (c->status() != SS_FAILED)
        return false;
      return local ? ((q->afc() == 0.0) && (p->afc() == 0.0)) :
        ((q->afc() > 1.0) && (p->afc() < 1.0));
    }

    /// A real reified propagator must retain its information when rewritten
    bool rewrite(bool local) {
      using namespace Gecode;
      TestSpace s;
      PropagatorGroup g;
      if (local) {
        Space::NoGPI context(s);
        rel(s(g),s.x,IRT_EQ,s.y,Reify(s.b));
      } else {
        rel(s(g),s.x,IRT_EQ,s.y,Reify(s.b));
      }
      (void) s.status();
      Gecode::Propagators ps(s,PropagatorGroup::all);
      unsigned int id = ps.propagator().id();
      rel(s,s.b,IRT_EQ,1);
      if (local) {
        Space::UseGPI context(s);
        (void) s.status();
      } else {
        Space::NoGPI context(s);
        (void) s.status();
      }
      Propagator* p = s.find(id);
      if (!p || (p->group() != g) ||
          (p->afc() != (local ? 0.0 : 1.0)))
        return false;
      rel(s,s.x,IRT_EQ,3);
      return (s.status() == SS_SOLVED) && s.y.assigned() && (s.y.val() == 3);
    }

  public:
    NoGPI(void) : Test::Base("AFC::NoGPI") {}
    virtual bool run(void) {
      return scopes() && lifetime() && failure(false) && failure(true) &&
        rewrite(false) && rewrite(true);
    }
  };

  NoGPI no_gpi;

  AFC afc;

}

// STATISTICS: test-core
