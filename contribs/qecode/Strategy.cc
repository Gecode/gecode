/****   , [ bobocheTree.cc ], 
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

#include "Strategy.hh"


StrategyImp::StrategyImp(StrategyNode tag) {
//    cout<<"Strategy imp constructor"<<endl;
    pointers=1;
    zetag=tag;
//    cout<<"strategyimp constructor fini"<<endl;
}


StrategyImp::~StrategyImp() {
}


Strategy::Strategy() {
//    cout<<"strategy default"<<endl;
    StrategyNode tag = StrategyNode::Dummy();
    imp = new StrategyImp(tag);
}

Strategy::Strategy(StrategyNode tag) {
//    cout<<"Strategy with tag"<<endl;
    imp = new StrategyImp(tag);
//cout<<"passed imp creation. End of strategy creator"<<endl;
}


Strategy::Strategy(bool qt,int VMin, int VMax, int scope, vector<int> values) {
//    cout<<"strategy with values"<<endl;
    StrategyNode tag(2,qt,VMin,VMax,scope);
    tag.valeurs=values;
    imp = new StrategyImp(tag);
    
}


Strategy::Strategy(const Strategy& tree) {
//    cout<<"Strategy copy"<<endl;
    imp = tree.imp;
    (imp->pointers)++;
}


Strategy& Strategy::operator = (const Strategy& rvalue) {
//    cout<<"Strategy = "<<endl;
    if (imp != NULL) {
        (imp->pointers)--;
        if ( (imp->pointers) == 0) {
            //        cout<<"no more references for the imp. Delete"<<endl;
            delete imp;
        }
    }
    imp = rvalue.imp;
    (imp->pointers)++;
    return *this;
}
    

Strategy::~Strategy() {
//    cout<<"strategy destructor"<<endl;
    (imp->pointers)--;
    if ( (imp->pointers) == 0) {
//        cout<<"no more references for the imp. Delete"<<endl;
        delete imp;
    }
}


StrategyNode Strategy::getTag() {
    return imp->zetag;
}


int Strategy::degree() {
    return imp->nodes.size();
}


Strategy Strategy::getChild(int i) {
    if (i<0 || i>=degree() ) {cout<<"Child "<<i<<" does not exist"<<endl;abort();}
    return imp->nodes[i];
}


void Strategy::attach(Strategy child) {
    if (child.isDummy()) {
        for (int i=0;i<child.degree();i++) {
            imp->nodes.push_back(child.getChild(i));
        }
    }
    else
        imp->nodes.push_back(child);
}

Strategy Strategy::STrue() {
    Strategy ret(StrategyNode::STrue());
    return ret;
}

Strategy Strategy::SFalse() {
    Strategy ret(StrategyNode::SFalse());
    return ret;
}

Strategy Strategy::Dummy() {
    Strategy ret(StrategyNode::Dummy());
    return ret;
}