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

  /// Result of expanding one alternative
  class ExpansionStep {
  public:
    /// What the step produced
    enum State {
      CHILD,     ///< A propagated child
      TERMINAL,  ///< The node itself, as it does not branch
      EXHAUSTED  ///< Nothing, all alternatives have been expanded
    };
    /// The state
    State state;
    /// The child or node
    std::unique_ptr<Space> space;
    /// Status of the space
    SpaceStatus status;
  };

  /// Node whose alternatives are expanded one at a time
  class ChoiceExpansion {
  private:
    /// The node (declared before the choice so the choice is deleted first)
    std::unique_ptr<Space> parent;
    /// The choice of the node
    std::unique_ptr<const Choice> choice;
    /// Next alternative
    unsigned int alternative;
  public:
    /// Initialize without node
    ChoiceExpansion(void) : alternative(0) {}
    /// Whether there is a node
    explicit operator bool(void) const { return static_cast<bool>(parent); }
    /// Return the node
    Space& space(void) const { return *parent; }
    /// Take stable node \a s
    void select(std::unique_ptr<Space> s) {
      assert(!parent && !choice);
      parent = std::move(s);
      alternative = 0;
    }
    /// Return the node before its choice has been computed
    std::unique_ptr<Space> take(void) {
      assert(!choice);
      return std::move(parent);
    }
    /// Delete the node
    void reset(void) {
      choice.reset();
      parent.reset();
      alternative = 0;
    }
    /**
     * \brief Commit and propagate the next alternative
     *
     * The child is constrained by \a incumbent if the node has been
     * constrained with an older revision than \a revision. The last
     * alternative reuses the node itself.
     */
    ExpansionStep next(Worker& worker, unsigned long int depth,
                       unsigned long long int node_revision,
                       const Space* incumbent,
                       unsigned long long int revision) {
      assert(parent);
      if (!choice) {
        SpaceStatus status = parent->status(worker);
        if (status != SS_BRANCH)
          return {ExpansionStep::TERMINAL,std::move(parent),status};
        choice.reset(parent->choice());
        alternative = 0;
      }
      if (alternative == choice->alternatives()) {
        reset();
        return {ExpansionStep::EXHAUSTED,nullptr,SS_FAILED};
      }
      std::unique_ptr<Space> child;
      // Keeps the choice alive while committing the last alternative
      std::unique_ptr<const Choice> last;
      const Choice* c = choice.get();
      if (alternative+1 == choice->alternatives()) {
        child = std::move(parent);
        last = std::move(choice);
      } else {
        child.reset(parent->clone());
      }
      child->commit(*c,alternative);
      if ((incumbent != nullptr) && (node_revision != revision))
        child->constrain(*incumbent);
      worker.node++;
      worker.stack_depth(depth);
      SpaceStatus status = child->status(worker);
      alternative++;
      return {ExpansionStep::CHILD,std::move(child),status};
    }
  };

}}}

#endif

#endif

// STATISTICS: search-seq
