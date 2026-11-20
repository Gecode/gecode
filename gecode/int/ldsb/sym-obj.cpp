/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
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

#include <gecode/int/ldsb.hh>

namespace Gecode {
    SymmetryHandle::SymmetryHandle() : ref(nullptr) {}
    SymmetryHandle::SymmetryHandle(Int::LDSB::SymmetryObject* o) : ref(o) {}
    SymmetryHandle::SymmetryHandle(const SymmetryHandle& h) : ref(h.ref) {
        if (ref != nullptr)
            increment();
    }
    SymmetryHandle&
    SymmetryHandle::operator=(const SymmetryHandle& h) {
        if ((this == &h) || (h.ref == ref))
            return *this;
        if (ref != nullptr)
            decrement();
        ref = h.ref;
        if (ref != nullptr)
            increment();
        return *this;
    }
    SymmetryHandle::~SymmetryHandle() {
        if (ref != nullptr)
            decrement();
    }
    void
    SymmetryHandle::increment() {
        (ref->nrefs)++;
    }
    void
    SymmetryHandle::decrement() {
        (ref->nrefs)--;
        if (ref->nrefs == 0)
            delete ref;
        ref = nullptr;
    }
} // namespace Gecode

namespace Gecode::Int::LDSB {

    SymmetryObject::SymmetryObject() : nrefs(1) {}
    SymmetryObject::~SymmetryObject() = default;

    VariableSymmetryObject::VariableSymmetryObject(ArgArray<VarImpBase*> vars) {
        nxs = vars.size();
        xs = new VarImpBase*[nxs];
        for (int i = 0; i < nxs; i++)
            xs[i] = vars[i];
    }
    VariableSymmetryObject::~VariableSymmetryObject() {
        delete[] xs;
    }

    ValueSymmetryObject::ValueSymmetryObject(IntSet vs) : values(vs) {}

    VariableSequenceSymmetryObject::VariableSequenceSymmetryObject(ArgArray<VarImpBase*> vars, int ss) : seq_size(ss) {
        nxs = vars.size();
        xs = new VarImpBase*[nxs];
        for (int i = 0; i < nxs; i++)
            xs[i] = vars[i];
    }
    VariableSequenceSymmetryObject::~VariableSequenceSymmetryObject() {
        delete[] xs;
    }

    ValueSequenceSymmetryObject::ValueSequenceSymmetryObject(const IntArgs& vs, int ss) : values(vs), seq_size(ss) {}

} // namespace Gecode::Int::LDSB

// STATISTICS: int-branch
