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
 */

#include "test/word.hh"
#include <algorithm>
#include <gecode/search.hh>
#include <random>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
namespace Test {
  namespace Word {
    namespace SparseDomain {
      using namespace Gecode;
      using U = WordValue;
      struct S : Space {
        WordVar x;
        S(unsigned w, U lo, U hi, WordDomainType k, U a, U b)
            : x(*this, w, lo, hi, k, a, b) {}
        S(S &s) : Space(s) { x.update(*this, s.x); }
        Space *copy() override { return new S(*this); }
      };
      static U rank(U v, unsigned w, WordDomainType k) {
        return v ^ (k == WDT_SIGNED ? (U(1) << (w - 1)) : 0);
      }
      static std::vector<U> enumerate(U lo, U unknown) {
        std::vector<U> v;
        U subset = unknown;
        do {
          v.push_back(lo | subset);
          if (!subset)
            break;
          subset = (subset - 1) & unknown;
        } while (true);
        return v;
      }
      static void check(S &s, const std::vector<U> &expected, unsigned w,
                        WordDomainType k) {
        if (expected.empty())
          throw std::runtime_error("unexpected empty oracle");
        U low = ~U(0), high = 0;
        for (U v : expected) {
          if (!s.x.in(v))
            throw std::runtime_error("lost member");
          U r = rank(v, w, k);
          low = std::min(low, r);
          high = std::max(high, r);
        }
        if (rank(s.x.minimum(), w, k) != low ||
            rank(s.x.maximum(), w, k) != high ||
            s.x.assigned() != (expected.size() == 1))
          throw std::runtime_error("noncanonical endpoints/assignment");
        for (U v : enumerate(s.x.lo(), s.x.unknown()))
          if (s.x.in(v) &&
              std::find(expected.begin(), expected.end(), v) == expected.end())
            throw std::runtime_error("unexpected member");
      }
      static bool exercise() {
        std::mt19937_64 rng(20260914);
        uint64_t trials = 0, searches = 0, solutions = 0, empty = 0;
        for (unsigned w : {1u, 2u, 7u, 8u, 16u, 31u, 32u, 33u, 63u, 64u})
          for (auto k : {WDT_UNSIGNED, WDT_SIGNED})
            for (unsigned t = 0; t < 2000; t++) {
              U mask = w == 64 ? ~U(0) : (U(1) << w) - 1;
              U unknown = 0;
              for (unsigned i = 0; i < std::min(w, 10u); i++)
                unknown |= U(1) << (rng() % w);
              U lo = (rng() & mask) & ~unknown, hi = lo | unknown;
              auto cube = enumerate(lo, unknown);
              U a = rng() & mask, b = rng() & mask;
              if (a > b)
                std::swap(a, b);
              if (t % 4 == 0) {
                a = 0;
                b = mask;
              } else if (t % 4 == 1) {
                a = rank(cube[rng() % cube.size()], w, k);
                b = rank(cube[rng() % cube.size()], w, k);
                if (a > b)
                  std::swap(a, b);
              }
              std::vector<U> expected;
              for (U v : cube)
                if (rank(v, w, k) >= a && rank(v, w, k) <= b)
                  expected.push_back(v);
              trials++;
              try {
                S s(w, lo, hi, k, rank(a, w, k), rank(b, w, k));
                if (expected.empty())
                  throw std::runtime_error("constructor accepted empty domain");
                check(s, expected, w, k);
                s.status();
                std::unique_ptr<S> c(static_cast<S *>(s.clone()));
                check(*c, expected, w, k);
                U na = rng() & mask, nb = rng() & mask;
                if (na > nb)
                  std::swap(na, nb);
                std::vector<U> narrowed;
                for (U v : expected)
                  if (rank(v, w, k) >= na && rank(v, w, k) <= nb)
                    narrowed.push_back(v);
                Gecode::Word::WordView cv(c->x);
                auto me = cv.narrow_rank_range(*c, na, nb);
                if (me_failed(me) != (narrowed.empty()))
                  throw std::runtime_error("range failure mismatch");
                if (!narrowed.empty())
                  check(*c, narrowed, w, k);
                check(s, expected, w, k);
                U nlo = lo, nhi = hi;
                U selected = U(1) << (rng() % w);
                if (rng() & 1)
                  nlo |= selected;
                else
                  nhi &= ~selected;
                narrowed.clear();
                for (U v : expected)
                  if ((v & nlo) == nlo && (v & ~nhi) == 0)
                    narrowed.push_back(v);
                Gecode::Word::WordView sv(s.x);
                me = sv.narrow(s, nlo, nhi);
                if (me_failed(me) != (narrowed.empty()))
                  throw std::runtime_error("cube failure mismatch");
                if (!narrowed.empty())
                  check(s, narrowed, w, k);
                if (t % 100 == 0 && !narrowed.empty()) {
                  for (auto strategy :
                       {WORD_VAL_LSB(), WORD_VAL_MSB(), WORD_VAL_SPLIT_MIN(),
                        WORD_VAL_SPLIT_MAX()}) {
                    s.status();
                    auto *root = static_cast<S *>(s.clone());
                    branch(*root, root->x, strategy);
                    Search::Options o;
                    o.c_d = 8;
                    o.a_d = 2;
                    DFS<S> dfs(root, o);
                    delete root;
                    std::set<U> got;
                    while (std::unique_ptr<S> sol{dfs.next()}) {
                      if (!sol->x.assigned() ||
                          !got.insert(sol->x.val()).second)
                        throw std::runtime_error("invalid/duplicate solution");
                      solutions++;
                    }
                    if (got != std::set<U>(narrowed.begin(), narrowed.end()))
                      throw std::runtime_error("search mismatch");
                    searches++;
                  }
                }
              } catch (const Gecode::Word::VariableEmptyDomain &) {
                if (!expected.empty())
                  throw std::runtime_error(
                      "constructor rejected nonempty domain");
                empty++;
              }
            }
        return trials == 40000 && searches > 0 && solutions > 0 && empty > 0;
      }
      class SparseDomainTest : public Base {
      public:
        SparseDomainTest() : Base("Word::Bounded::SparseWide") {}
        virtual bool run(void) {
          try {
            return exercise();
          } catch (const std::exception &) {
            return false;
          }
        }
      };
      namespace {
        SparseDomainTest sparse_domain_test;
      }
    } // namespace SparseDomain
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
