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

#ifndef GECODE_SEARCH_SEQ_CHOICE_EXPANSION_HH
#define GECODE_SEARCH_SEQ_CHOICE_EXPANSION_HH

#include <gecode/search/worker.hh>
#include <memory>
#include <utility>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search { namespace Seq {

  /// Child, parent solved or failed by preparation, or exhausted choice
  struct ExpansionStep {
    enum class State { CHILD, PREPARED_TERMINAL, EXHAUSTED };
    State state;
    std::unique_ptr<Space> space;
    SpaceStatus status;
  };

  /// Own a parent and resume its saved choice after a solution or stop
  class ChoiceExpansion {
  private:
    // Member declaration order destroys the choice before its source.
    std::unique_ptr<Space> parent;
    std::unique_ptr<const Choice> choice;
    unsigned int alternative;
  public:
    ChoiceExpansion(void) : alternative(0) {}

    explicit operator bool(void) const { return static_cast<bool>(parent); }
    Space& space(void) const { return *parent; }

    /// Take ownership of a stable parent before generating its choice
    void select(std::unique_ptr<Space> s) {
      assert(!parent && !choice);
      parent = std::move(s);
      alternative = 0;
    }

    /// Take a solved root, which has no saved choice
    std::unique_ptr<Space> take(void) {
      assert(!choice);
      return std::move(parent);
    }

    /// Discard the saved choice before deleting its parent
    void reset(void) {
      choice.reset();
      parent.reset();
      alternative = 0;
    }

    /// Commit and propagate one original alternative without changing siblings
    ExpansionStep next(Worker& worker,
                       unsigned long int child_depth,
                       unsigned long long int source_revision,
                       const Space* incumbent,
                       unsigned long long int incumbent_revision) {
      assert(parent);
      if (!choice) {
        parent->prepare(worker);
        const SpaceStatus status = parent->status(worker);
        if (status != SS_BRANCH)
          return {ExpansionStep::State::PREPARED_TERMINAL,
                  std::move(parent),status};
        choice.reset(parent->choice());
      }
      if (alternative == choice->alternatives()) {
        reset();
        return {ExpansionStep::State::EXHAUSTED,nullptr,SS_FAILED};
      }
      std::unique_ptr<Space> child;
      // Reuse the parent for its last alternative. Keep the choice alive
      // through commit, incumbent restriction, and propagation, including
      // cleanup if any of those operations throws.
      std::unique_ptr<const Choice> final_choice;
      const Choice* decision = choice.get();
      if (alternative+1 == choice->alternatives()) {
        child = std::move(parent);
        final_choice = std::move(choice);
      } else {
        child.reset(parent->clone());
      }
      child->commit(*decision,alternative);
      if (incumbent && source_revision != incumbent_revision) {
        child->constrain(*incumbent);
      }
      worker.node++;
      worker.stack_depth(child_depth);
      const SpaceStatus status = child->status(worker);
      alternative++;
      // Reusing the parent leaves no extra parent to count against the quota.
      return {ExpansionStep::State::CHILD,std::move(child),status};
    }
  };

}}}

#endif

#endif

// STATISTICS: search-seq
