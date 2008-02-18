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

#include "qsolver.hh"
#include "implicative.hh"
#include "SDFVariableHeuristic.hh"
#include "NaiveValueHeuristics.hh"


// This is a set of tiny problems which are used to dtect errors in QeCode. 
// This set is likely to progressively enlarge...

int main() {
    unsigned long int nodes;
    unsigned long int steps;
    SmallestDomainFirst heur;
    SmallestValueFirst vHeur;
    
    // Ax in 1..3 []  -> x=1
    int sc1[] = {1};
    Implicative test1(1,QECODE_UNIVERSAL,sc1);
    test1.QIntVar(0,1,3);
    test1.nextScope();
    post(test1.space(),test1.var(0) == 1);
    test1.makeStructure();
    QSolver s1(&test1,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret1=s1.solve(nodes,steps);
    cout<<"Problem 1 : result = "<<(ret1?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    //Ex in 1..3 [] st x=1
    int sc2[] = {1};
    Implicative test2(1,QECODE_EXISTENTIAL,sc2);
    test2.QIntVar(0,1,3);
    test2.nextScope();
    post(test2.space(),test2.var(0) == 1);
    test2.makeStructure();
    QSolver s2(&test2,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret2=s2.solve(nodes,steps);
    cout<<"Problem 2 : result = "<<(ret2?"TRUE":"FALSE")<<", sould be TRUE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    
    //Ax in 1..3 [x=1] -> x=2
    int sc3[] = {1};
    Implicative test3(1,QECODE_UNIVERSAL,sc3);
    test3.QIntVar(0,1,3);
    post(test3.space(),test3.var(0) == 1);
    test3.nextScope();
    post(test3.space(),test3.var(0) == 2);
    test3.makeStructure();;
    QSolver s3(&test3,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret3=s3.solve(nodes,steps);
    cout<<"Problem 3 : result = "<<(ret3?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    // Ex in 1..3 [x=1] st x=2
    int sc4[] = {1};
    Implicative test4(1,QECODE_EXISTENTIAL,sc4);
    test4.QIntVar(0,1,3);
    post(test4.space(),test4.var(0) == 1);
    test4.nextScope();
    post(test4.space(),test4.var(0) == 2);
    test4.makeStructure();
    QSolver s4(&test4,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret4=s4.solve(nodes,steps);
    cout<<"Problem 4 : result = "<<(ret4?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    // Ax in 1..3 [x=1] -> Ey in 1..3 [x=2] -> y=1
    int sc5[] = {1,1};
    Implicative test5(2,QECODE_UNIVERSAL,sc5);
    test5.QIntVar(0,1,3);
    test5.QIntVar(1,1,3);
    post(test5.space(),test5.var(0) == 1);
    test5.nextScope();
    post(test5.space(),test5.var(0) == 2);
    test5.nextScope();
    post(test5.space(),test5.var(1) == 1);
    test5.makeStructure();
    QSolver s5(&test5,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret5=s5.solve(nodes,steps);
    cout<<"Problem 5 : result = "<<(ret5?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    

    // Ax in 1..3 [x=1] -> Ey in 1..3 [x=1] -> x=2    
    int sc6[] = {1,1};
    Implicative test6(2,QECODE_UNIVERSAL,sc6);
    test6.QIntVar(0,1,3);
    test6.QIntVar(1,1,3);
    post(test6.space(),test6.var(0) == 1);
    test6.nextScope();
    post(test6.space(),test6.var(0) == 1);
    test6.nextScope();
    post(test6.space(),test6.var(0) == 2);
    test6.makeStructure();
    QSolver s6(&test6,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret6=s6.solve(nodes,steps);
    cout<<"Problem 6 : result = "<<(ret6?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    //Ex in 1..3 [] Ay in 0..3 [y<2] -> y=0
    int sc7[] = {1,1};
    Implicative test7(2,QECODE_EXISTENTIAL,sc7);
    test7.QIntVar(0,1,3);
    test7.QIntVar(1,0,3);
    test7.nextScope();
    post(test7.space(),test7.var(1) <= 2);
    test7.nextScope();
    post(test7.space(),test7.var(1) == 0);
    test7.makeStructure();
    QSolver s7(&test7,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret7=s7.solve(nodes,steps);
    cout<<"Problem 7 : result = "<<(ret7?"TRUE":"FALSE")<<", sould be FALSE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    //Ex in 1..3 [] Ay in 0..3 [y=0] -> y=0    
    int sc8[] = {1,1};
    Implicative test8(2,QECODE_EXISTENTIAL,sc8);
    test8.QIntVar(0,1,3);
    test8.QIntVar(1,0,3);
    test8.nextScope();
    post(test8.space(),test8.var(1) == 0);
    test8.nextScope();
    post(test8.space(),test8.var(1) == 0);
    test8.makeStructure();
    QSolver s8(&test8,&heur,&vHeur);
    nodes=0;
    steps=0;
    bool ret8=s8.solve(nodes,steps);
    cout<<"Problem 8 : result = "<<(ret8?"TRUE":"FALSE")<<", sould be TRUE."<<endl;
    cout<<nodes<<" nodes and "<<steps<<" steps."<<endl;
    
    
    
    
}
    
    