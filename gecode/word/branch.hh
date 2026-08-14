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

#ifndef GECODE_WORD_BRANCH_HH
#define GECODE_WORD_BRANCH_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Branch {

  class MeritSize : public MeritBase<WordView,unsigned int> {
  public:
    MeritSize(Space& home, const VarBranch<Var>& vb);
    MeritSize(Space& home, MeritSize& m);
    unsigned int operator ()(const Space& home, WordView x, int i);
  };

  class MeritDegreeSize : public MeritBase<WordView,double> {
  public:
    MeritDegreeSize(Space& home, const VarBranch<Var>& vb);
    MeritDegreeSize(Space& home, MeritDegreeSize& m);
    double operator ()(const Space& home, WordView x, int i);
  };

  class MeritAFCSize : public MeritBase<WordView,double> {
  protected:
    AFC afc;
  public:
    MeritAFCSize(Space& home, const VarBranch<Var>& vb);
    MeritAFCSize(Space& home, MeritAFCSize& m);
    double operator ()(const Space& home, WordView x, int i);
    bool notice(void) const;
    void dispose(Space& home);
  };

  class MeritActionSize : public MeritBase<WordView,double> {
  protected:
    Action action;
  public:
    MeritActionSize(Space& home, const VarBranch<Var>& vb);
    MeritActionSize(Space& home, MeritActionSize& m);
    double operator ()(const Space& home, WordView x, int i);
    bool notice(void) const;
    void dispose(Space& home);
  };

  class MeritCHBSize : public MeritBase<WordView,double> {
  protected:
    CHB chb;
  public:
    MeritCHBSize(Space& home, const VarBranch<Var>& vb);
    MeritCHBSize(Space& home, MeritCHBSize& m);
    double operator ()(const Space& home, WordView x, int i);
    bool notice(void) const;
    void dispose(Space& home);
  };

  ViewSel<WordView>* viewsel(Space& home, const WordVarBranch& wvb);

  class ValSelLsb : public ValSel<WordView,WordValue> {
  public:
    ValSelLsb(Space& home, const ValBranch<Var>& vb);
    ValSelLsb(Space& home, ValSelLsb& vs);
    WordValue val(const Space& home, WordView x, int i);
  };

  class ValSelMsb : public ValSel<WordView,WordValue> {
  public:
    ValSelMsb(Space& home, const ValBranch<Var>& vb);
    ValSelMsb(Space& home, ValSelMsb& vs);
    WordValue val(const Space& home, WordView x, int i);
  };

  class ValSelRnd : public ValSel<WordView,WordValue> {
  protected:
    Rnd r;
  public:
    ValSelRnd(Space& home, const ValBranch<Var>& vb);
    ValSelRnd(Space& home, ValSelRnd& vs);
    WordValue val(const Space& home, WordView x, int i);
    bool notice(void) const;
    void dispose(Space& home);
  };

  class ZeroNGL : public ViewValNGL<WordView,unsigned int,PC_WORD_BITS> {
  public:
    ZeroNGL(Space& home, WordView x, unsigned int bit);
    ZeroNGL(Space& home, ZeroNGL& ngl);
    virtual NGL* copy(Space& home);
    virtual Status status(const Space& home) const;
    virtual ExecStatus prune(Space& home);
  };

  class ValCommitZero : public ValCommit<WordView,WordValue> {
  public:
    ValCommitZero(Space& home, const ValBranch<Var>& vb);
    ValCommitZero(Space& home, ValCommitZero& vc);
    ModEvent commit(Space& home, unsigned int a, WordView x, int i,
                    WordValue bit);
    NGL* ngl(Space& home, unsigned int a, WordView x,
             WordValue bit) const;
    void print(const Space& home, unsigned int a, WordView x, int i,
               WordValue bit, std::ostream& o) const;
  };

  /// Select an admitted endpoint or median rank
  class ValSelRank : public ValSel<WordView,WordValue> {
  protected:
    WordAssign::Select select;
  public:
    ValSelRank(Space& home, const ValBranch<WordVar>& wa);
    ValSelRank(Space& home, ValSelRank& vs);
    WordValue val(const Space& home, WordView x, int i);
  };

  /// Select a ranked interval split point
  class ValSelSplit : public ValSel<WordView,WordValue> {
  public:
    ValSelSplit(Space& home, const ValBranch<WordVar>& wvb);
    ValSelSplit(Space& home, ValSelSplit& vs);
    WordValue val(const Space& home, WordView x, int i);
  };

  /// No-good literal for a ranked upper bound
  class RankLqNGL : public ViewValNGL<WordView,WordValue,PC_WORD_BND> {
  public:
    RankLqNGL(Space& home, WordView x, WordValue rank);
    RankLqNGL(Space& home, RankLqNGL& ngl);
    virtual NGL* copy(Space& home);
    virtual Status status(const Space& home) const;
    virtual ExecStatus prune(Space& home);
  };

  /// No-good literal for a ranked strict lower bound
  class RankGrNGL : public ViewValNGL<WordView,WordValue,PC_WORD_BND> {
  public:
    RankGrNGL(Space& home, WordView x, WordValue rank);
    RankGrNGL(Space& home, RankGrNGL& ngl);
    virtual NGL* copy(Space& home);
    virtual Status status(const Space& home) const;
    virtual ExecStatus prune(Space& home);
  };

  /// Commit a ranked interval split
  class ValCommitSplit : public ValCommit<WordView,WordValue> {
  protected:
    bool lower_first;
  public:
    ValCommitSplit(Space& home, const ValBranch<WordVar>& wvb);
    ValCommitSplit(Space& home, ValCommitSplit& vc);
    ModEvent commit(Space& home, unsigned int a, WordView x, int i,
                    WordValue rank);
    NGL* ngl(Space& home, unsigned int a, WordView x,
             WordValue rank) const;
    void print(const Space& home, unsigned int a, WordView x, int i,
               WordValue rank, std::ostream& o) const;
  };

  /// Commit assignment to an admitted ranked value
  class ValCommitRank : public ValCommit<WordView,WordValue> {
  public:
    ValCommitRank(Space& home, const ValBranch<WordVar>& wa);
    ValCommitRank(Space& home, ValCommitRank& vc);
    ModEvent commit(Space& home, unsigned int a, WordView x, int i,
                    WordValue rank);
    NGL* ngl(Space& home, unsigned int a, WordView x,
             WordValue rank) const;
    void print(const Space& home, unsigned int a, WordView x, int i,
               WordValue rank, std::ostream& o) const;
  };

  ValSelCommitBase<WordView,WordValue>*
  valselcommit(Space& home, const WordValBranch& wvb);
  ValSelCommitBase<WordView,WordValue>*
  valselcommit(Space& home, const WordAssign& wa);

}}}

#endif

// STATISTICS: word-branch
