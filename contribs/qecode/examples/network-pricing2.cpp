/****   , [ optim2.cc ],
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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include "QCOPPlus.hh"
#include "qsolver_qcop.hh"
#include <iostream>
#include <vector>

using namespace std;
using namespace Gecode;
using namespace Gecode::Int;

void printStr(Strategy s,int depth) {
    StrategyNode plop = s.getTag();
    for (int glou=0;glou<depth;glou++) cout<<" ";
    if (s.isTrue()) cout<<"TRUE"<<endl;
    else if (s.isFalse()) cout<<"FALSE"<<endl;
    else cout<<"type "<<plop.type<<" qt "<<plop.quantifier<<" vmin "<<plop.Vmin<<" vmax "<<plop.Vmax<<" scope "<<plop.scope<<" - ";
    for (int i=0;i<s.getTag().valeurs.size();i++) cout<<s.getTag().valeurs[i]<<" ";
    cout<<endl;
    for (int glou=0;glou<depth;glou++) cout<<" ";
    cout<<s.degree()<<" child(ren)"<<endl;
    for (int i=0;i<s.degree();i++) {
        for (int glou=0;glou<depth;glou++) cout<<" ";
        cout<<"Child "<<i<<" : "<<endl;
        printStr(s.getChild(i),depth+1);
    }
}

vector<int> assignment;
void listAssignments(Strategy s) {
  StrategyNode tag = s.getTag();
  if (s.isTrue()) {
    // We are at the end of a branch : we print the assignment)
    for (int i=0;i<assignment.size();++i) cout<<assignment[i]<<" ";
    cout<<endl;
    return;
  }
  if (assignment.size()<= tag.Vmax) assignment.resize(tag.Vmax + 1);
  for (int i=tag.Vmin;i<=tag.Vmax;++i) {
    assignment[i] = tag.valeurs[i-(tag.Vmin)];
  }
  // recursively extracting each child
  for (int i=0;i<s.degree();++i) {
    listAssignments(s.getChild(i));
  }
}

int main() {
    int NCustomer = 2;
    int NArc = 3;
    int* c = new int[NCustomer*NArc]; // c[NArc*i + j] cost for ci to reach Aj
    int* d = new int[NCustomer]; // demand for customer i
    int* u = new int[NCustomer]; // max price for customer i
    int max;

    c[0*NArc+0] = 5;  c[1*NArc+0] = 3;
    c[0*NArc+1] = 4;  c[1*NArc+1] = 6;
    c[0*NArc+2] = 2;  c[1*NArc+2] = 5;

    d[0] = 10;  d[1] = 7;
    u[0] = 70;  u[1] = 90;

    max = 100;


    IntArgs carg(NCustomer*NArc,c);
    IntArgs darg(NCustomer,d);
    IntArgs uarg(NCustomer,u);


    bool q[] = {false,true,false};
    int* nv = new int[3];
    nv[0] = NArc;
    nv[1] = 1;
    nv[2] = 9;

    Qcop problem(3,q,nv);
    for (int i=0;i<NArc;i++)
        problem.QIntVar(i,0,10); // t[i]
    IntVarArgs branch1(NArc);
    for (int i=0;i<NArc;i++)
        branch1[i] = problem.var(i);
    branch(*(problem.space()),branch1,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
    problem.nextScope();

    problem.QIntVar(NArc,0,NCustomer-1); // k
    IntVarArgs branch2(NArc+1);
    for (int i=0;i<NArc+1;i++)
        branch2[i] = problem.var(i);
    branch(*(problem.space()),branch2,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
    problem.nextScope();

    problem.QIntVar(NArc+1,0,NArc-1); // a
    problem.QIntVar(NArc+2,0,max); // cost
    problem.QIntVar(NArc+3,0,max); // Income
    IntVar a(problem.var(NArc+1));
    IntVar cost(problem.var(NArc+2));
    IntVar income(problem.var(NArc+3));
    problem.QIntVar(NArc+4,0,max);
    problem.QIntVar(NArc+5,0,max);
    problem.QIntVar(NArc+6,0,max);
    problem.QIntVar(NArc+7,0,max);
    problem.QIntVar(NArc+8,0,max);
    problem.QIntVar(NArc+9,0,max);
    IntVar aux1(problem.var(NArc+4)); // k* NArc + a
    IntVar aux2(problem.var(NArc+5)); // c[k*Narc+a]
    IntVar aux3(problem.var(NArc+6)); // t[a]
    IntVar aux4(problem.var(NArc+7)); // d[k]
    IntVar aux5(problem.var(NArc+8)); // c[]+t[]
    IntVar aux6(problem.var(NArc+9)); // u[k]
    IntVar k(problem.var(NArc));
    rel(*(problem.space()), aux1 == ( NArc * k + a) );
    element(*(problem.space()),carg,aux1,aux2);
    IntVarArgs t(NArc);
    for (int i=0;i<NArc;i++) t[i]=problem.var(i);
    element(*(problem.space()),t,a,aux3);
    element(*(problem.space()),darg,k,aux4);
    rel(*(problem.space()), aux5 == aux2 + aux3);
    mult(*(problem.space()),aux5,aux4,cost); // cost = aux5 * aux4
    mult(*(problem.space()),aux3,aux4,income);
    element(*(problem.space()),uarg,k,aux6);
    rel(*(problem.space()),cost <= aux6);

    IntVarArgs branch3(NArc+10);
    for (int i=0;i<NArc+10;i++)
        branch3[i] = problem.var(i);
    branch(*(problem.space()),branch3,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    OptVar* costopt = problem.getExistential(NArc+2);
    OptVar* incomeopt = problem.getExistential(NArc+3);
    problem.optimize(2,1,costopt); // at scope 2, we minimize (1) the variable cost
    AggregatorSum somme;
    OptVar* sumvar = problem.getAggregate(1,incomeopt,&somme);
    problem.optimize(0,2,sumvar);


    QCOP_solver sol(&problem);
    unsigned long int nodes=0;

    Strategy outcome = sol.solve(nodes);

    cout<<"STRATEGY DESCRIPTION : "<<endl;
    printStr(outcome,0);
    cout<<endl<<"____________________"<<endl;
    listAssignments(outcome);
    return 0;
}


