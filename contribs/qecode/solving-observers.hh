/****   , [ solving-observers.hh ], 
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
#ifndef __QECODE_SOLVING_OBSERVERS__
#define __QECODE_SOLVING_OBSERVERS__
#include "ImplicativeState.hh"
class LeafObserver {
    /** Called when a leaf is encountered in the search tree.
    * @param truthvalue    truth value of the leaf */
public:
    virtual void observeLeaf(ImplicativeState st,bool truthValue) = 0;
    virtual ~LeafObserver() {};
};

class BacktrackObserver {
    /** Called when a backtrack occurs.
    * @param depth   Depth in the search tree BEFORE backtracking */
public:
    virtual void observeBacktrack(ImplicativeState st,int depth) = 0;
    virtual ~BacktrackObserver() {};
};

class ChoicePointObserver {
    /** Called when a choice point is encountered.
    * @param depth      Depth in the search tree where choice point occurs
    * @param lastVar    number of the last variable we have branched on (for scope change detection)
    * @param variable   number of the variable which will be assigned
    * @param assign     true : we will INTERSECT the given values with the variable domain. False : we will REMOVE the given values from the variable domain.
    * @param ranges     array of intervals to be intersected to / removed from the chosen variable.
    * @param nbranges   length of ranges array ( ranges is a int[2][nbranges] array).
    */
public:
    virtual void observeChoicePoint(ImplicativeState st,int depth, int lastVar, int variable, bool assign, int** ranges, int nbranges) = 0;
    virtual ~ChoicePointObserver() {};
};

class SubsumedObserver {
    /** Called when the solver finds a subsumed value.
    * @param variable   number of the variable which has been found subsumed
    */
public:
    virtual void observeSubsumed(int variable) = 0;
    virtual ~SubsumedObserver() {};
};

#endif
