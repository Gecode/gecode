/*********************************************************[valueheuristic.hh]
Copyright (c) 2007, Universite d'Orleans - Jeremie Vautard, Marco Benedetti,
Arnaud Lallouet.

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
*****************************************************************************/


#ifndef VALUEHEUR
#define VALUEHEUR
#include "qecode.hh"
#include "implicative.hh"
using namespace Gecode;
using namespace Gecode::Int;
using namespace std;


class QECODE_VTABLE_EXPORT valueHeuristic {
    friend class Implicative;
protected :
    int whichSpaceOwns(Implicative* imp,int var) {
        return imp->whichSpaceOwns[var];
    }
    
    int nbSpaces(Implicative* imp) {
        return imp->nbSpaces;
    }
    
    IntVar* getIntVar(Implicative* imp,int scope,int var) {
        if (imp->type_of_v[var] != VTYPE_INT) {cout<< "valueHeuristic getIntVar : Variable "<<var<<" is not int"<<endl; abort();}
        if (scope<imp->nbSpaces) {
            if (imp->ruleDefined[scope]) 
                return (static_cast<IntVar*>(imp->rules[scope]->v[var]));
            else 
                return NULL;
        }
        else {
            if (imp->goalDefined) 
                return (static_cast<IntVar*>(imp->goal->v[var]));
            else 
                return NULL;
        }
    }
    
    bool scopeDefined(Implicative* imp,int scope) {
        if (scope == imp->nbSpaces) return imp->goalDefined;
        else return imp->ruleDefined[scope];
    }

    BoolVar* getBoolVar(Implicative* imp,int scope,int var) {
        if (imp->type_of_v[var] != VTYPE_INT) {cout<< "valueHeuristic getBoolVar : Variable "<<var<<" is not bool"<<endl; abort();}
        if (scope<imp->nbSpaces) {
            if (imp->ruleDefined[scope]) 
                return (static_cast<BoolVar*>(imp->rules[scope]->v[var]));
            else 
                return NULL;
        }
        else {
            if (imp->goalDefined) 
                return (static_cast<BoolVar*>(imp->goal->v[var]));
            else 
                return NULL;
        }
    }

public :
    virtual int** subSet(Implicative* home,int var,int* nbRanges)=0;
    virtual ~valueHeuristic() {}
};





#endif
