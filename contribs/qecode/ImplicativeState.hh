/****   , [ ImplicativeState.hh ], 
Copyright (c) 2008 Universite d'Orleans - Jeremie Vautard 

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

#ifndef __QECODE_IMPLICATIVE_STATE__
#define __QECODE_IMPLICATIVE_STATE__
#include "qecode.hh"
#include "implicative.hh"
#include "vartype.hh"
#include "gecode/minimodel.hh"
using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

class QECODE_VTABLE_EXPORT ImplicativeState {
    friend class Implicative;
private : 
    Implicative* imp;
public :
    QECODE_EXPORT ImplicativeState(Implicative* i) {imp = i;}
    QECODE_EXPORT int nbVars();
    QECODE_EXPORT int nbScopes();
    QECODE_EXPORT int nbVarsByScope(int scope);
    QECODE_EXPORT bool scopeDefined(int scope);
    QECODE_EXPORT int scopeStatus(int scope);
    QECODE_EXPORT VarType typeOfVar(int var);
    QECODE_EXPORT void* getVar(int var);
};


#endif