/****   , [ StrategyNode.cc ], 
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

#include "StrategyNode.hh"

StrategyNode::StrategyNode() {
    type=0;
    quantifier=true;
    Vmin=-1;
    Vmax=-1;
    scope=-1;
}

StrategyNode::StrategyNode(int typ,bool qt,int min, int max, int sco) {
    type=typ;
    quantifier=qt;
    Vmin=min;
    Vmax=max;
    scope=sco;
}

/*
StrategyNode::StrategyNode(const StrategyNode& st) {
    cout<<"Strategynode copy constructor"<<endl;
    type=st.type;
    quantifier=st.quantifier;
    Vmin=st.Vmin;
    Vmax=st.Vmax;
    if (type == 2) {
        valeurs=new int[Vmax-Vmin];
        for (int i=0;i<Vmax-Vmin;i++)
            valeurs[i]=st.valeurs[i];
    }
}
*/

StrategyNode::~StrategyNode() {

}

