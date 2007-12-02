/*****************************************************************[myspace.cc]
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
#include "./myspace.hh"
#include <iostream>


using namespace std;

MySpace::MySpace(unsigned int nv) {
    n=nv;
    v=new void*[n];
    type_of_v=new VarType[n];
}


MySpace::~MySpace() {
    for (int i=0;i<n;i++)
        delete v[i];
    
    delete[] v;
    delete[] type_of_v;
}

MySpace::MySpace(bool share,MySpace& ms) : Space(share,ms) {
  n=ms.n;
    v=new void*[n];
    type_of_v=new VarType[n];
    for (int i=0;i<n;i++) {
        type_of_v[i] = ms.type_of_v[i];
        switch (type_of_v[i]) {
            case VTYPE_INT :
                v[i] = new IntVar(*(static_cast<IntVar*>(ms.v[i])));
                (static_cast<IntVar*>(v[i]))->update(this,true,*(static_cast<IntVar*>(ms.v[i])));
                break;
            case VTYPE_BOOL :
                v[i] = new BoolVar(*(static_cast<BoolVar*>(ms.v[i])));
                (static_cast<BoolVar*>(v[i]))->update(this,true,*(static_cast<BoolVar*>(ms.v[i])));
                break;
            default:
                cout<<"Unsupported variable type"<<endl;
                abort();
        }
    }
}


MySpace* MySpace::copy(bool share) {return new MySpace(share,*this);}


