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

  class ValSelLsb : public ValSel<WordView,unsigned int> {
  public:
    ValSelLsb(Space& home, const ValBranch<Var>& vb);
    ValSelLsb(Space& home, ValSelLsb& vs);
    unsigned int val(const Space& home, WordView x, int i);
  };

  class ValSelMsb : public ValSel<WordView,unsigned int> {
  public:
    ValSelMsb(Space& home, const ValBranch<Var>& vb);
    ValSelMsb(Space& home, ValSelMsb& vs);
    unsigned int val(const Space& home, WordView x, int i);
  };

  class ValSelRnd : public ValSel<WordView,unsigned int> {
  protected:
    Rnd r;
  public:
    ValSelRnd(Space& home, const ValBranch<Var>& vb);
    ValSelRnd(Space& home, ValSelRnd& vs);
    unsigned int val(const Space& home, WordView x, int i);
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

  class ValCommitZero : public ValCommit<WordView,unsigned int> {
  public:
    ValCommitZero(Space& home, const ValBranch<Var>& vb);
    ValCommitZero(Space& home, ValCommitZero& vc);
    ModEvent commit(Space& home, unsigned int a, WordView x, int i,
                    unsigned int bit);
    NGL* ngl(Space& home, unsigned int a, WordView x,
             unsigned int bit) const;
    void print(const Space& home, unsigned int a, WordView x, int i,
               unsigned int bit, std::ostream& o) const;
  };

  ValSelCommitBase<WordView,unsigned int>*
  valselcommit(Space& home, const WordValBranch& wvb);
  ValSelCommitBase<WordView,unsigned int>*
  valselcommit(Space& home, const WordAssign& wa);

}}}

#endif
