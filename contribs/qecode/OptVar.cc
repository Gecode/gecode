/****   , [ OptVar.cc ], 
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

#include "OptVar.hh"

ExistOptVar::ExistOptVar(int var,int scope) {
    varId = var;
    scopeId=scope;
}

int ExistOptVar::getVal(Strategy s) {
//    cout<<"getval on existoptvar "<<varId<<" at scope "<<scopeId<<endl;
    if (s.isTrue()) {cout<<"EOV "<<varId<<" scope "<<scopeId<<" try to get opt value on TRUE"<<endl;abort();}
    if (s.isFalse()) {cout<<"EOV "<<varId<<" scope "<<scopeId<<"Try to get opt value on FALSE"<<endl;abort();} // (sub)Strategy must neither be false
    if (s.isDummy()) {cout<<"EOV "<<varId<<" scope "<<scopeId<<"Try to get opt value on Dummy"<<endl;abort();} // nor begin with a universal scope
    if (s.scope() > scopeId) {cout<<"EOV "<<varId<<" scope "<<scopeId<<"Try to get opt value on  asubstrategy not containing opt variable"<<endl;abort();} // nor begin at a scope after the one we want ^^
    
    if (s.scope() == scopeId) return s.value(varId-s.VMin());
    // s.getTag() < scope
    Strategy nextScope = s.getChild(0);
    
    return getVal(nextScope);
}

int ExistOptVar::getScope() {return scopeId;}

UnivOptVar::UnivOptVar(int scope,OptVar* zevar, Aggregator* agg) {
    var=zevar;
    fct=agg;
    scopeId=scope;
}

int UnivOptVar::getVal(Strategy s) {
//    cout<<"getVal sur univoptvar "<<scopeId<<endl;
    vector<int> values; values.clear();
    if (s.isFalse()) {cout<<"UOV Try to get opt value on FALSE"<<endl;abort();}
    if (s.isTrue()) return fct->eval(values); 
    if (s.isDummy()) {
        for (int i=0;i<s.degree();i++) {
            values.push_back(var->getVal(s.getChild(i)));
        }
        return fct->eval(values);
    }

    if (s.scope() == (scopeId-1)) {
        for (int i=0;i<s.degree();i++) {
            values.push_back(var->getVal(s.getChild(i)));
        }
        return fct->eval(values);
    }
    
    if (s.scope() < (scopeId-1) ) {
        if (s.quantifier()) {cout<<"UOV universal scope too soon"<<endl;abort();} // universal quantifier too soon
        return getVal(s.getChild(0));
    }
    // s.scope() > scope-1 -> too far away
    cout<<"UOV try to get aggregate on a substrategy not containing required scope"<<endl;
    abort();
    return 0; // dummy return to avoid a compiler complain...
}    
        

int UnivOptVar::getScope() {return scopeId;}
