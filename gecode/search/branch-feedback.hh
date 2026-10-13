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

#ifndef GECODE_SEARCH_BRANCH_FEEDBACK_HH
#define GECODE_SEARCH_BRANCH_FEEDBACK_HH

#include <gecode/search.hh>

namespace Gecode { namespace Search {

  /// Post-propagation outcome of a newly explored branch
  enum class BranchOutcome {
    FAILED,
    SOLVED,
    BRANCH
  };

  /// Shared receiver for internal branch feedback
  class BranchFeedback : public SharedHandle {
  public:
    /// Receiver implementation
    class Receiver : public SharedHandle::Object {
    public:
      /// Observe alternative \a alternative of stable choice \a id
      virtual void observe(unsigned int id, unsigned int alternative,
                           const Space& space,
                           BranchOutcome outcome) noexcept = 0;
      /// Destructor
      virtual ~Receiver(void) {}
    };

    /// Construct an empty handle
    BranchFeedback(void) {}
    /// Construct a handle for \a receiver
    explicit BranchFeedback(Receiver* receiver)
      : SharedHandle(receiver) {}
    /// Return the internal receiver
    Receiver* receiver(void) const {
      return static_cast<Receiver*>(object());
    }
    /// Report one branch outcome
    void observe(unsigned int id, unsigned int alternative,
                 const Space& space, BranchOutcome outcome) const noexcept {
      receiver()->observe(id,alternative,space,outcome);
    }
  };

  /// Internal opt-in choice carrying stable feedback identity
  class GECODE_VTABLE_EXPORT BranchFeedbackChoice : public Choice {
  private:
    BranchFeedback _feedback;
    unsigned int _feedback_id;
  protected:
    /// Initialize for brancher \a brancher with \a alternatives
    BranchFeedbackChoice(const Brancher& brancher, unsigned int alternatives,
                         const BranchFeedback& feedback, unsigned int id)
      : Choice(brancher,alternatives), _feedback(feedback),
        _feedback_id(id) {}
  public:
    /// Map search alternative to the stable semantic alternative
    virtual unsigned int
    feedback_alternative(unsigned int alternative) const noexcept {
      return alternative;
    }
    /// Report the post-propagation outcome for \a alternative
    void feedback(unsigned int alternative, const Space& space,
                  BranchOutcome outcome) const noexcept {
      _feedback.observe(_feedback_id,feedback_alternative(alternative),
                        space,outcome);
    }
    /// Return receiver handle
    const BranchFeedback& feedback(void) const {
      return _feedback;
    }
    /// Return stable choice identity
    unsigned int feedback_id(void) const {
      return _feedback_id;
    }
  };

  /// Saved feedback for a stolen alternative whose choice can be deleted
  class PendingBranchFeedback {
  private:
    BranchFeedback _feedback;
    unsigned int _id;
    unsigned int _alternative;
  public:
    /// Construct an empty pending observation
    PendingBranchFeedback(void) : _id(0U), _alternative(0U) {}
    /// Whether an observation is pending
    explicit operator bool(void) const {
      return static_cast<bool>(_feedback);
    }
    /// Copy feedback from \a choice for \a alternative, if opted in
    void set(const Choice& choice, unsigned int alternative) {
      if (const BranchFeedbackChoice* feedback_choice =
            dynamic_cast<const BranchFeedbackChoice*>(&choice)) {
        _feedback = feedback_choice->feedback();
        _id = feedback_choice->feedback_id();
        _alternative = feedback_choice->feedback_alternative(alternative);
      } else {
        clear();
      }
    }
    /// Report and clear the pending observation
    void observe(const Space& space, BranchOutcome outcome) noexcept {
      assert(_feedback);
      BranchFeedback feedback(_feedback);
      const unsigned int id = _id;
      const unsigned int alternative = _alternative;
      clear();
      feedback.observe(id,alternative,space,outcome);
    }
    /// Clear the pending observation
    void clear(void) {
      _feedback = BranchFeedback();
    }
  };

  /// Report \a outcome for an opt-in \a choice
  forceinline void
  branch_feedback(const Choice& choice, unsigned int alternative,
                  const Space& space, BranchOutcome outcome) noexcept {
    if (const BranchFeedbackChoice* feedback_choice =
          dynamic_cast<const BranchFeedbackChoice*>(&choice))
      feedback_choice->feedback(alternative,space,outcome);
  }

  /// Convert a space status to its branch-feedback outcome
  forceinline BranchOutcome
  branch_outcome(SpaceStatus status) {
    switch (status) {
    case SS_FAILED: return BranchOutcome::FAILED;
    case SS_SOLVED: return BranchOutcome::SOLVED;
    case SS_BRANCH: return BranchOutcome::BRANCH;
    default: GECODE_NEVER;
    }
    GECODE_NEVER;
    return BranchOutcome::FAILED;
  }

}}

#endif

// STATISTICS: search-other
