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

#include <gecode/search.hh>
#include <sstream>

namespace Test { namespace Word {

  class BranchSpace : public Gecode::Space {
  public:
    Gecode::WordVarArray x;

    BranchSpace(Gecode::WordValBranch vals)
      : x(*this,1,3,2,7) {
      Gecode::branch(*this,x,vals);
    }
    BranchSpace(Gecode::WordAssign vals)
      : x(*this,1,3,2,7) {
      Gecode::assign(*this,x,vals);
    }
    BranchSpace(BranchSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
    }
    virtual Gecode::Space* copy(void) {
      return new BranchSpace(*this);
    }
  };

  class Branch : public Base {
  private:
    bool selectors(void) const {
      using namespace Gecode;
      class SelectorSpace : public Space {
      public:
        WordVarArray x;
        SelectorSpace(WordVarBranch vars)
          : x(*this,2) {
          x[0] = WordVar(*this,4,0,1);
          x[1] = WordVar(*this,4,0,15);
          branch(*this,x,vars,WORD_VAL_LSB());
        }
        SelectorSpace(SelectorSpace& s) : Space(s) {
          x.update(*this,s.x);
        }
        virtual Space* copy(void) { return new SelectorSpace(*this); }
      };

      SelectorSpace min_space(WORD_VAR_SIZE_MIN());
      SelectorSpace max_space(WORD_VAR_SIZE_MAX());
      const Choice* min_choice = min_space.choice();
      const Choice* max_choice = max_space.choice();
      std::ostringstream min_out, max_out;
      min_space.print(*min_choice,0,min_out);
      max_space.print(*max_choice,0,max_out);
      delete min_choice; delete max_choice;
      if ((min_out.str().find("var[0]") == std::string::npos) ||
          (max_out.str().find("var[1]") == std::string::npos))
        return false;

      class StateSpace : public Space {
      public:
        WordVarArray x;
        StateSpace(int state) : x(*this,2,4,0,15) {
          if (state == 0) {
            WordAFC afc(*this,x);
            branch(*this,x,WORD_VAR_AFC_MAX(afc),WORD_VAL_LSB());
          } else if (state == 1) {
            WordAction action(*this,x);
            branch(*this,x,WORD_VAR_ACTION_MAX(action),WORD_VAL_LSB());
          } else {
            WordCHB chb(*this,x);
            branch(*this,x,WORD_VAR_CHB_MAX(chb),WORD_VAL_LSB());
          }
        }
        StateSpace(StateSpace& s) : Space(s) { x.update(*this,s.x); }
        virtual Space* copy(void) { return new StateSpace(*this); }
      };
      for (int state=0; state<3; state++) {
        StateSpace* root = new StateSpace(state);
        Search::Options o; o.c_d = 1; o.a_d = 1;
        DFS<StateSpace> dfs(root,o);
        delete root;
        StateSpace* solution = dfs.next();
        const bool ok = (solution != nullptr) &&
          solution->x[0].assigned() && solution->x[1].assigned();
        delete solution;
        if (!ok) return false;
      }
      return true;
    }

    bool mixed_recomputation(void) const {
      using namespace Gecode;
      class MixedSpace : public Space {
      public:
        WordVar x;
        BoolVar b;
        MixedSpace(void) : x(*this,2), b(*this,0,1) {
          channel(*this,x,0,b);
          WordVarArgs xv(1); xv[0] = x;
          branch(*this,xv,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
        }
        MixedSpace(MixedSpace& s) : Space(s) {
          x.update(*this,s.x); b.update(*this,s.b);
        }
        virtual Space* copy(void) { return new MixedSpace(*this); }
      };
      MixedSpace* root = new MixedSpace;
      Search::Options o; o.c_d = 1; o.a_d = 1;
      DFS<MixedSpace> dfs(root,o);
      delete root;
      int count = 0;
      while (MixedSpace* s = dfs.next()) {
        const bool ok = s->x.assigned() && s->b.assigned() &&
          (static_cast<unsigned int>(s->b.val()) == (s->x.val() & 1U));
        delete s;
        if (!ok) return false;
        count++;
      }
      return count == 4;
    }

    bool search(Gecode::WordValBranch vals) const {
      BranchSpace* root = new BranchSpace(vals);
      Gecode::Search::Options o;
      o.c_d = 1;
      o.a_d = 1;
      Gecode::DFS<BranchSpace> dfs(root,o);
      delete root;
      int solutions = 0;
      while (BranchSpace* solution = dfs.next()) {
        if (!solution->x[0].assigned()) {
          delete solution;
          return false;
        }
        solutions++;
        delete solution;
      }
      return solutions == 4;
    }

    bool assignment(Gecode::WordAssign vals) const {
      BranchSpace* root = new BranchSpace(vals);
      Gecode::DFS<BranchSpace> dfs(root);
      delete root;
      BranchSpace* solution = dfs.next();
      const bool ok = (solution != nullptr) && solution->x[0].assigned() &&
        (solution->x[0].val() == 2);
      delete solution;
      solution = dfs.next();
      const bool one = solution == nullptr;
      delete solution;
      return ok && one;
    }

    bool lifecycle(void) const {
      using namespace Gecode;
      BranchSpace root(WORD_VAL_LSB());
      if (root.status() != SS_BRANCH)
        return false;
      const Choice* choice = root.choice();
      if ((choice == nullptr) || (choice->alternatives() != 2)) {
        delete choice;
        return false;
      }

      std::ostringstream decision;
      root.print(*choice,0,decision);
      if (decision.str() != "var[0].bit(0)=0") {
        delete choice;
        return false;
      }

      Archive archive;
      choice->archive(archive);
      BranchSpace* replay = static_cast<BranchSpace*>(root.clone());
      const Choice* restored = replay->choice(archive);
      replay->commit(*restored,1);
      const bool reconstructed = (replay->status() == SS_BRANCH) &&
        ((replay->x[0].lo() & 1U) != 0);
      delete restored;
      delete replay;

      BranchSpace* zero = static_cast<BranchSpace*>(root.clone());
      zero->commit(*choice,0);
      const bool complementary = (zero->status() == SS_BRANCH) &&
        ((zero->x[0].hi() & 1U) == 0);
      delete zero;

      BranchSpace* nogood = static_cast<BranchSpace*>(root.clone());
      NGL* ngl = nogood->ngl(*choice,0);
      const bool ngl_ok = (ngl != nullptr) &&
        (ngl->status(*nogood) == NGL::NONE) &&
        (ngl->prune(*nogood) == ES_OK) &&
        ((nogood->x[0].lo() & 1U) != 0);
      delete nogood;
      delete choice;
      return reconstructed && complementary && ngl_ok;
    }

    bool random(void) const {
      using namespace Gecode;
      Rnd r0(17), r1(17);
      BranchSpace a(WORD_VAL_RND(r0));
      BranchSpace b(WORD_VAL_RND(r1));
      if ((a.status() != SS_BRANCH) || (b.status() != SS_BRANCH))
        return false;
      const Choice* ca = a.choice();
      const Choice* cb = b.choice();
      std::ostringstream oa, ob;
      a.print(*ca,0,oa);
      b.print(*cb,0,ob);
      delete ca;
      delete cb;
      return oa.str() == ob.str();
    }

  public:
    Branch(void) : Base("Word::Branch") {}
    virtual bool run(void) {
      using namespace Gecode;
      Rnd rb(11), ra(11);
      return search(WORD_VAL_LSB()) && search(WORD_VAL_MSB()) &&
        search(WORD_VAL_RND(rb)) &&
        assignment(WORD_ASSIGN_LSB()) && assignment(WORD_ASSIGN_MSB()) &&
        assignment(WORD_ASSIGN_RND(ra)) && lifecycle() && random() &&
        selectors() && mixed_recomputation();
    }
  };

  namespace {
    Branch branch;
  }

}}

// STATISTICS: test-word
