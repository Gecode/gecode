/****   , [ UnblockableViewValBranching.hh ], 
Copyright (c) 2009 Universite d'Orleans - Jeremie Vautard 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 *************************************************************************/
#ifndef __QECODE_UNBLOK_VIEWVAL_BRANCH__
#define __QECODE_UNBLOK_VIEWVAL_BRANCH__

#include "qecode.hh"
#include "gecode/minimodel.hh"
#include "qecode.hh"
#include "UnblockableBranching.hh"

class QECODE_VTABLE_EXPORT UnblockableViewValBranching : public UnblockableBranching {
private :
    IntVarBranch ivrb;
    IntVarBranch bvrb;
    IntValBranch ivlb;
    IntValBranch bvlb;
    bool before;
    
public :
    QECODE_EXPORT UnblockableViewValBranching(IntVarBranch vrb,IntValBranch vlb,bool booleans_before);
    QECODE_EXPORT UnblockableViewValBranching(IntVarBranch Ivrb,IntValBranch Ivlb,IntVarBranch Bvrb,IntValBranch Bvlb,bool booleans_before);
    QECODE_EXPORT virtual void branch(MySpace* s,IntVarArgs ivars, BoolVarArgs bvars);
};

#endif

