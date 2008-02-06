/*****************************************************************[qecore.cc]
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
#include "qecore.hh"

#include <iostream>


QSpace::QSpace() {}

QSpace::QSpace(QSpace& qs) {
  n=qs.n;
  ps=qs.ps;
  v = new void*[n];
  type_of_v = qs.type_of_v;
}


#define MARKER size

BranchingHeuristic::BranchingHeuristic(QSpace* qs,VariableHeuristic* ev) {
  
  eval=ev;
  size=qs->nv();
  score=new int[size];
  touched.push(MARKER);
  bloc=new int[size];
  
  blocks=1; bool q=qs->quantification(0);
  for (int i=0;i<size;i++) {
    if ((qs->quantification(i)) != q) {
      q=qs->quantification(i);
      blocks++;
    }
    bloc[i]=(blocks-1);
  }

  treated.push(MARKER);

  for (int i=0;i<size;i++) {
    score[i]=eval->score(qs,i);
  }

  ExtensiveComparator* c = new ExtensiveComparator(size,score);

  heaps=new cheap[blocks];
  for (int i=0;i<blocks;i++) {
    heaps[i]=cheap(c);
    heaps[i].setBounds(size);
  }

  
  int whichblock=0; bool qf=(qs->quantification(0));
  for (int i=0;i<size;i++) {
    if (qf != (qs->quantification(i)) ) {
      whichblock++; 
      qf=(qs->quantification(i));
    }
    heaps[whichblock].insert(i);
  }
  
  curHeap=0;
}

int BranchingHeuristic::nextVar(QSpace* qs) {
  int ret;
  touched.push(MARKER);
  while (! (heaps[blocks-1]).empty() ) {
    if (heaps[curHeap].empty()) {
      curHeap++;
    }
    if (curHeap >= blocks ) break;
    ret=heaps[curHeap].getminwopop();
    treated.push(ret);

    if( !(qs->subsumed(ret)) ) {      // If the variable is a good one to branch on.../!\ A INSTANCIATED VARIABLE MUST BE SUBSUMED
      treated.push(MARKER);
      return ret;
    }
    else {    // If the variable is subsumed, we delete it fron the heap...
        int rien=heaps[curHeap].getmin();
    }
  }
  treated.push(MARKER);
  return MARKER;
}

void BranchingHeuristic::updateVar(int pos, QSpace* qs) {

  if (!(heaps[bloc[pos]].inHeap(pos)) ) return;         // If the variable is not in the heap (so, it is already assigned or ignored), do nothing
  if ( eval->score(qs,pos) == score[pos] ) return;      // if no update is necessary, do nothing

  if ( eval->score(qs,pos) < score[pos] ) {
    score[pos]=eval->score(qs,pos);
    heaps[bloc[pos]].increase(pos);
  }
  else {
    score[pos]=eval->score(qs,pos);
    heaps[bloc[pos]].decrease(pos);
  }
}

void BranchingHeuristic::vartouched(int pos,QSpace* qs) {
  touched.push(pos);
  updateVar(pos,qs);
}

void BranchingHeuristic::backtrack(QSpace* qs) {
  if (treated.top() != MARKER ) {
    cout<<"Branching Heuristic : Probleme de pile... "<<endl;
    abort();
  }
  else {
    treated.pop();
  }
  while (treated.top() != MARKER ) {
    heaps[bloc[treated.top()]].insert(treated.top() );
    curHeap=bloc[treated.top()];
    treated.pop();
  }
  
  while (touched.top() != MARKER ) {
    updateVar(touched.top(),qs);
    touched.pop();
  }
}
