/****   , [ network-pricing1.cpp ],
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
/////////////////////////////////////////////////////////////////////////////////////////
// This is an instance of the Network Pricing Problem, taken from :                    //
//                                                                                     //
// Bouhtou, M., Grigoriev, A., van Hoesel, S., van der Kraaij, A.F., Spieksma, F.C.,   //
// Uetz, M.: Pricing bridges to cross a river. Naval Research Logistics 54(4) (2007)   //
// 411Ð420                                                                             //
//                                                                                     //
// The problem is to set a tariff on some network links in a way that maximizes the    //
// profit of the owner of the links.                                                   //
// Customer i wishes to transmit di amount of data from a source xi to a target yi.    //
// Each path from a source to a target has to cross a tolled arc aj . On the way from  //
// si to aj , the cost of the links owned by other providers is cij . It is assumed    //
// that each customer i wishes to minimize the cost to route her data and that they can//
// always choose an other provider at a cost ui. The purpose of the problem is to      //
// determine the cost tj to cross a tolled arc aj in order to maximize the revenue of  //
// the telecom operator.                                                               //
// This instance has 5 links and 9 customers. The network operator may choose among 5  //
// tarifs for each link.                                                               //
/////////////////////////////////////////////////////////////////////////////////////////

int main() {
    int NCustomer = 9;
    int NArc = 5;
    int*c = new int[NCustomer*NArc]; // [NArc*i+j] : Initial cost for client i to reach way j
    int* d = new int[NCustomer]; // d[i] : amount of data client i wants to transmit
    int* u = new int[NCustomer]; // u[i] : maximum cost client i will accept
    int max = 19;

    c[0*NArc+0]=5;    c[0*NArc+1]=1;    c[0*NArc+2]=8;    c[0*NArc+3]=6;    c[0*NArc+4]=6;
    c[1*NArc+0]=2;    c[1*NArc+1]=8;    c[1*NArc+2]=6;    c[1*NArc+3]=3;    c[1*NArc+4]=1;
    c[2*NArc+0]=0;    c[2*NArc+1]=8;    c[2*NArc+2]=0;    c[2*NArc+3]=8;    c[2*NArc+4]=6;
    c[3*NArc+0]=2;    c[3*NArc+1]=4;    c[3*NArc+2]=7;    c[3*NArc+3]=5;    c[3*NArc+4]=8;
    c[4*NArc+0]=8;    c[4*NArc+1]=2;    c[4*NArc+2]=4;    c[4*NArc+3]=3;    c[4*NArc+4]=4;
    c[5*NArc+0]=5;    c[5*NArc+1]=4;    c[5*NArc+2]=6;    c[5*NArc+3]=4;    c[5*NArc+4]=1;
    c[6*NArc+0]=5;    c[6*NArc+1]=6;    c[6*NArc+2]=7;    c[6*NArc+3]=4;    c[6*NArc+4]=8;
    c[7*NArc+0]=5;    c[7*NArc+1]=8;    c[7*NArc+2]=1;    c[7*NArc+3]=3;    c[7*NArc+4]=6;
    c[8*NArc+0]=7;    c[8*NArc+1]=8;    c[8*NArc+2]=8;    c[8*NArc+3]=3;    c[8*NArc+4]=5;

    d[0]=7;    d[1]=16;    d[2]=16;    d[3]=19;    d[4]=18;    d[5]=13;    d[6]=8;    d[7]=11;    d[8]=18;
    u[0]=122;    u[1]=190;    u[2]=113;    u[3]=285;    u[4]=247;    u[5]=255;    u[6]=143;    u[7]=121;    u[8]=139;
    IntArgs carg(NCustomer*NArc,c); // Copy c in an IntArgs for further constraint posting
    IntArgs darg(NCustomer,d); // Copy d in an IntArgs for further constraint posting
    IntArgs uarg(NCustomer,u); // Copy u in an IntArgs for further constraint posting

    bool q[] = {false,true,false};
    int* nv = new int[3];
    nv[0]=NArc;
    nv[1]=1;
    nv[2]=9;

    Qcop problem(3,q,nv);
    int postarfs[] = {3,7,11,15,19};
    IntSet thePossibleTariffs(postarfs,5);
    for (int i=0;i<NArc;i++)
        problem.QIntVar(i,thePossibleTariffs); // tariff for way i
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
    problem.QIntVar(NArc+2,0,1000000); // cost
    problem.QIntVar(NArc+3,0,1000000); // Income
    IntVar a(problem.var(NArc+1));
    IntVar cost(problem.var(NArc+2));
    IntVar income(problem.var(NArc+3));
    problem.QIntVar(NArc+4,0,1000000);
    problem.QIntVar(NArc+5,0,1000000);
    problem.QIntVar(NArc+6,0,1000000);
    problem.QIntVar(NArc+7,0,1000000);
    problem.QIntVar(NArc+8,0,1000000);
    problem.QIntVar(NArc+9,0,1000000);
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

    Strategy hihihi = sol.solve(nodes);
    cout<<nodes<<" nodes encountered."<<endl;
    printStr(hihihi,0);
    return 0;
}
