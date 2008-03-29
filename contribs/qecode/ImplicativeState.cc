/****   , [ ImplicativeState.cc ], 
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

#include "ImplicativeState.hh"

int ImplicativeState::nbVars() {
    return imp->nv();
}

int ImplicativeState::nbScopes() {
    return imp->nbSpaces+1;
}

int ImplicativeState::nbVarsByScope(int scope) {
    if (scope<0) abort();
    if (scope < imp->nbSpaces) return imp->nbVarBySpace[scope];
    return -1;
}

bool ImplicativeState::scopeDefined(int scope) {
    if (scope<0) abort();
    if (scope < imp->nbSpaces) return imp->ruleDefined[scope];
    if (scope = imp->nbSpaces) return imp->goalDefined;
    abort();
}

int ImplicativeState::scopeStatus(int scope) {
    if (scope<0) abort();
    if (scope < imp->nbSpaces) return imp->ruleStatus[scope];
    if (scope = imp->nbSpaces) return imp->goalStatus;
    abort();
}

VarType ImplicativeState::typeOfVar(int var) {
    if (var<0 || var > imp->nv() ) abort();
    return imp->type_of_v[var];
}

void* ImplicativeState::getVar(int var) {
    if (var <0 || var > imp->nv()) abort();
    return imp->v[var];
}
