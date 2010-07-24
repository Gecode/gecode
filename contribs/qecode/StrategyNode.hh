/****   , [ StrategyNode.hh ], 
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
#ifndef __QECODE_STRATEGY_NODE__
#define __QECODE_STRATEGY_NODE__
 #include "qecode.hh"
#include <vector>
using namespace std;
class QECODE_VTABLE_EXPORT StrategyNode {
public:
    int type; // 0 = dummy, 1 = empty, 2 = normal, 3 = Todo.
    bool quantifier;
    int Vmin;
    int Vmax;
    int scope;
    vector<int> valeurs;
    
    QECODE_EXPORT StrategyNode();
    QECODE_EXPORT StrategyNode(int type,bool qt,int Vmin, int Vmax, int scope);
    QECODE_EXPORT ~StrategyNode();
    QECODE_EXPORT static StrategyNode STrue() { return StrategyNode(1,true,-1,-1,-1);}
    QECODE_EXPORT static StrategyNode SFalse() { return StrategyNode(1,false,-1,-1,-1);}
    QECODE_EXPORT static StrategyNode Dummy() { return StrategyNode(0,true,-1,-1,-1);}
  	QECODE_EXPORT static StrategyNode Todo() { return StrategyNode(3,false,-1,-1,-1);}
    QECODE_EXPORT forceinline bool isFalse() { return ( (type==1) && (quantifier == false) );}
    QECODE_EXPORT forceinline bool isTrue() { return ( (type==1) && (quantifier == true) );}
    QECODE_EXPORT forceinline bool isDummy() { return (type==0);}
	QECODE_EXPORT forceinline bool isTodo() { return (type==3);}
};
#endif

