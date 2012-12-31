/****   , [ stress_test.cpp ],
Copyright (c) 2007 Universite d'Orleans - Jeremie Vautard

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

#include "qsolver_qcop.hh"
#include "QCOPPlus.hh"
#include <iostream>

using namespace std;
// This is a set of tiny problems which are used to dtect errors in QeCode.
// This set is likely to progressively enlarge...

int main() {
    unsigned long int nodes;
    unsigned long int steps;

    // Ax in 1..3 []  -> x=1
    int sc1[] = {1};
    bool q1[] = {QECODE_UNIVERSAL};
    Qcop test1(1,q1,sc1);
    test1.QIntVar(0,1,3);
    IntVarArgs b1(1);
    b1[0] = test1.var(0);
    branch(*(test1.space()),b1,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
    test1.nextScope();
    rel(*(test1.space()),test1.var(0) == 1);
    test1.makeStructure();
    QCOP_solver s1(&test1);
    nodes=0;
    Strategy ret1=s1.solve(nodes);
    cout<<"Problem 1 : result = "<<(ret1.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;

    //Ex in 1..3 [] st x=1
    int sc2[] = {1};
    bool q2[] = {QECODE_EXISTENTIAL};
    Qcop test2(1,q2,sc2);
    test2.QIntVar(0,1,3);

    IntVarArgs b2(1);
    b2[0] = test2.var(0);
    branch(*(test2.space()),b2,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test2.nextScope();
    rel(*(test2.space()),test2.var(0) == 1);
    test2.makeStructure();
    QCOP_solver s2(&test2);
    nodes=0;
    Strategy ret2=s2.solve(nodes);
    cout<<"Problem 2 : result = "<<(ret2.isFalse()?"FALSE":"TRUE")<<", sould be TRUE."<<endl;
    cout<<nodes<<" nodes."<<endl;



    //Ax in 1..3 [x=1] -> x=2
    int sc3[] = {1};
    bool q3[] = {QECODE_UNIVERSAL};
    Qcop test3(1,q3,sc3);
    test3.QIntVar(0,1,3);
    rel(*(test3.space()),test3.var(0) == 1);

    IntVarArgs b3(1);
    b3[0] = test3.var(0);
    branch(*(test3.space()),b3,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test3.nextScope();
    rel(*(test3.space()),test3.var(0) == 2);
    test3.makeStructure();;
    QCOP_solver s3(&test3);
    nodes=0;
    steps=0;
    Strategy ret3=s3.solve(nodes);
    cout<<"Problem 3 : result = "<<(ret3.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;


    // Ex in 1..3 [x=1] st x=2
    int sc4[] = {1};
    bool q4[] = {QECODE_EXISTENTIAL};
    Qcop test4(1,q4,sc4);
    test4.QIntVar(0,1,3);
    rel(*(test4.space()),test4.var(0) == 1);

    IntVarArgs b4(1);
    b4[0] = test4.var(0);
    branch(*(test4.space()),b4,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test4.nextScope();
    rel(*(test4.space()),test4.var(0) == 2);
    test4.makeStructure();
    QCOP_solver s4(&test4);
    nodes=0;
    Strategy ret4=s4.solve(nodes);
    cout<<"Problem 4 : result = "<<(ret4.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;


    // Ax in 1..3 [x=1] -> Ey in 1..3 [x=2] -> y=1
    int sc5[] = {1,1};
    bool q5[] = {QECODE_UNIVERSAL,QECODE_EXISTENTIAL};
    Qcop test5(2,q5,sc5);
    test5.QIntVar(0,1,3);
    test5.QIntVar(1,1,3);
    rel(*(test5.space()),test5.var(0) == 1);

    IntVarArgs b5(1);
    b5[0] = test5.var(0);
    branch(*(test5.space()),b5,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test5.nextScope();
    rel(*(test5.space()),test5.var(0) == 2);

    IntVarArgs b52(2);
    b52[0] = test5.var(0);
    b52[1] = test5.var(1);
    branch(*(test5.space()),b52,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test5.nextScope();
    rel(*(test5.space()),test5.var(1) == 1);
    test5.makeStructure();
    QCOP_solver s5(&test5);
    nodes=0;
    steps=0;
    Strategy ret5=s5.solve(nodes);
    cout<<"Problem 5 : result = "<<(ret5.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;


    // Ax in 1..3 [x=1] -> Ey in 1..3 [x=1] -> x=2
    int sc6[] = {1,1};
    bool q6[] = {QECODE_UNIVERSAL,QECODE_EXISTENTIAL};
    Qcop test6(2,q6,sc6);
    test6.QIntVar(0,1,3);
    test6.QIntVar(1,1,3);
    rel(*(test6.space()),test6.var(0) == 1);

    IntVarArgs b6(1);
    b6[0] = test6.var(0);
    branch(*(test6.space()),b6,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test6.nextScope();
    rel(*(test6.space()),test6.var(0) == 1);

    IntVarArgs b62(2);
    b62[0] = test6.var(0);
    b62[1] = test6.var(1);
    branch(*(test6.space()),b62,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test6.nextScope();
    rel(*(test6.space()),test6.var(0) == 2);
    test6.makeStructure();
    QCOP_solver s6(&test6);
    nodes=0;
    steps=0;
    Strategy ret6=s6.solve(nodes);
    cout<<"Problem 6 : result = "<<(ret6.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;


    //Ex in 1..3 [] Ay in 0..3 [y<2] -> y=0
    int sc7[] = {1,1};
    bool q7[] = {QECODE_EXISTENTIAL,QECODE_UNIVERSAL};
    Qcop test7(2,q7,sc7);
    test7.QIntVar(0,1,3);
    test7.QIntVar(1,0,3);

    IntVarArgs b7(1);
    b7[0] = test7.var(0);
    branch(*(test7.space()),b7,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test7.nextScope();
    rel(*(test7.space()),test7.var(1) <= 2);

    IntVarArgs b72(2);
    b72[0] = test7.var(0);
    b72[1] = test7.var(1);
    branch(*(test7.space()),b72,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test7.nextScope();
    rel(*(test7.space()),test7.var(1) == 0);
    test7.makeStructure();
    QCOP_solver s7(&test7);
    nodes=0;
    steps=0;
    Strategy ret7=s7.solve(nodes);
    cout<<"Problem 7 : result = "<<(ret7.isFalse()?"FALSE":"TRUE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes."<<endl;


    //Ex in 1..3 [] Ay in 0..3 [y=0] -> y=0
    int sc8[] = {1,1};
    bool q8[] = {QECODE_EXISTENTIAL,QECODE_UNIVERSAL};
    Qcop test8(2,q8,sc8);
    test8.QIntVar(0,1,3);
    test8.QIntVar(1,0,3);

    IntVarArgs b8(1);
    b8[0] = test8.var(0);
    branch(*(test8.space()),b8,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test8.nextScope();
    rel(*(test8.space()),test8.var(1) == 0);

    IntVarArgs b82(2);
    b82[0] = test8.var(0);
    b82[1] = test8.var(1);
    branch(*(test8.space()),b82,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    test8.nextScope();
    rel(*(test8.space()),test8.var(1) == 0);
    test8.makeStructure();
    QCOP_solver s8(&test8);
    nodes=0;
    steps=0;
    Strategy ret8=s8.solve(nodes);
    cout<<"Problem 8 : result = "<<(ret8.isFalse()?"FALSE":"TRUE")<<", sould be TRUE."<<endl;
    cout<<nodes<<" nodes."<<endl;

}


