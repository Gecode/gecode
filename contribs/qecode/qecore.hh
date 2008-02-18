/*****************************************************************[qecore.hh]
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
#ifndef __QECODE_QECORE__
#define __QECODE_QECORE__


#include "qecode.hh"
#include <cstdlib>
#include <vector>
#include <list>
#include <stack>
#include "extensivecomparator.hh"
#include "heap.cc"
#include "gecode/minimodel.hh"
#include "vartype.hh"

using namespace Gecode;
using namespace std;
using namespace Gecode::Int;


class BranchingHeuristic;


/** \brief Base class for a quantified space.
 *  Base class for a quantified space (with restricted quantifiers or not). The array v has to be kept up-to-date by the copy constructor of any descendant of this class.
 */
class QECODE_VTABLE_EXPORT QSpace {
protected:
  unsigned int n; 
  unsigned long int ps;
public:

  /** Constructor for a new quantified space with n variables. */
  QECODE_EXPORT QSpace();


  /** Copy constructor. */
  QECODE_EXPORT QSpace(QSpace& qs);

  QECODE_EXPORT virtual ~QSpace() {delete [] v;}


  /** \brief Returns the number of variablas.
  * returns the number of variables this quantified space contains.
   */
  QECODE_EXPORT int nv() {return n;}


  /** \brief Array of the variables of the quantified space.
  * This array contains all the variables of the quantified space (with current domain). 
   */
  void** v;
  
  /** \brief Types of the variables of the quantified space.
      * This aray contains the type of each variable (integer, boolean, etc.)
      */
  
  VarType* type_of_v;


  /** \brief Returns a variable quantificaion.
  * Returns wether a variable is quantified existentially (false) or universally (true).
   *  @param v The variable number.
   */
  virtual bool quantification(int v)=0;



  /** \brief Returns the current partial status of the quantified space.
  * Returns the current status of the quantified space, but limits the computation of this status at the variable given in parameter. So, this method will return "don't know" more often than a complete status computation. Returns 1 for "true", 0 for "false", and -1 for "Don't know, try a next variable". A "Don't know" answer while all the variables have been assigned can be considered as a "true".
   *  @param var The number of the variable to compute the status around.
   *  @param propsteps Will be increased by the number of propagation steps performed (for statistical purposes).
   */
  virtual int status(int var, unsigned long int& propsteps)=0;

  /** \brief Returns the thuth value of a completely assigned problem.
  * Performs the last checks to return in a complete way the truth value of the problem. All non-subsumes variables must be assigned for this method to be correct.
  * @param propsteps Will be increased by the number of propagation steps performed (for statistical purposes).
  */
  virtual int finalStatus(unsigned long int& propsteps)=0;

  /** \brief Returns wether a variable is subsumed or not.
  * A variable is subsumed if it is assigned or it is bound by no more constraint. The answer must be correct, but not complete (so, a "not sure" must return False for the solver to remain correct.
   *  @param var The variable number.
   */
  virtual bool subsumed(int var)=0;


  /** Returns a clone of the quantified space (possibly after a propagation call)
   */
  virtual QSpace* clone()=0; 


  /** \brief Assigns a set of value to an integer variable. 
      * assigns a set of value to an integer variable. Used only by the search engine.
      *  @param var The number of the variable to assign.
      *  @param vals The values to assign to this variable (represented y ranges).
      *  @param nbVals The number of ranges in vals.
      */
  virtual void assign_int(int var,int** vals,int nbVals)=0;
  
  /** \brief Removes a set of value to an integer variable. 
      * Removes a set of value to an integer variable. Used only by the search engine.
      *  @param var The number of the variable to assign.
      *  @param vals The values to remove from this variable (represented y ranges).
      *  @param nbVals The number of ranges in vals.
      */
  virtual void remove_int(int var,int** vals,int nbVals)=0;
  
  /** \brief Assigns a set of value to an integer variable. 
      * assigns a set of value to an integer variable. Used only by the search engine. 
      *  @param var The number of the variable to assign.
      *  @param vals The values to assign to this variable (represented y ranges).
      *  @param nbVals The number of ranges in vals.
      */
  virtual void assign_bool(int var,int** vals,int nbVals)=0;
  
  /** \brief Removes a set of value to an boolean variable. 
      * Removes a set of value to an boolean variable. Used only by the search engine.
      *  @param var The number of the variable to assign.
      *  @param vals The values to remove from this variable (represented y ranges).
      *  @param nbVals The number of ranges in vals.
      */
  virtual void remove_bool(int var,int** vals,int nbVals)=0;
  
  /** \brief Assigns a value to a boolean variable. 
      * assigns a set of value to an integer variable. Used only by the search engine.
      *  @param var The number of the variable to assign.
      *  @param b The value to assign to this variable.
      */
  virtual void assign_bool(int var,int b)=0;

  /** \brief Removes a value to a boolean variable. 
      * removes a set of value to an integer variable. Used only by the search engine.
      *  @param var The number of the variable to assign.
      *  @param b The value to remove from this variable.
      */
  virtual void remove_bool(int var,int b)=0;
  

  /** \brief Method called when the solver backtracks to this quantified space.
  * This method is useful to indicate some new information to the branchong heuristic, like simply updating the scores.
   */
  virtual void backtrack()=0;       


  /** \brief Indicates the branching heuristic to be used when performing search. 
  * Indicates the branching heuristic to be used when performing search. This method is typically invoked by the solver at its construction.
   */
  virtual void indicateBranchingHeuristic(BranchingHeuristic* bh)=0;
  
  /** \brief Print the space details to the default output.
      */
  virtual void print()=0;
};



/** \brief Base class for variable heuristics.
 Variable heuristics are used by the branching heuristic to define the current score of each variable, and branch on the variable which have the biggest score.
 */
class VariableHeuristic {    
public:

  /** \brief Returns the score of a variable.
   * Returns the score of the variable var in the quantified space qs.
   *  @param var The variable to obtain the score
   *  @param qs A pointer to the quantified space in which the variable is.
   */
  virtual int score(QSpace* qs,int var)=0;
};


typedef Heap<ExtensiveComparator> cheap;
typedef stack<int> stackint;



/** \brief Branching heurstic for a quantified space.
 * Branching heuristic using a score attribuer to give the next variable. This branching heuristic keeps trace of the variables on which we have branched, and so requires a backtrack signal (send by the solver using its backtrack method). The next variable musi be (and is) chosen respect to the variables partial order in the prefix. Heaps are used to provide the next variable in logarithmic time. Variables declared as subsumed are ignored by the branching heuristic.
 */
class QECODE_VTABLE_EXPORT BranchingHeuristic {
private : 
  int size;
  int blocks;
  int* bloc; 
  stackint treated;
  cheap* heaps;
  stackint touched;
  int* score;
  int curHeap;
  VariableHeuristic* eval;

public : 
      QECODE_EXPORT void updateVar(int pos, QSpace* qs);

  /** \brief Constructor for a branching heuristic.
   *  constructor for a branching heuristic on a quantified space, using a score attribuer.
   *  @param qs The quantified space the branching heuristic is for.
   *  @param ev the variable heuristic this branching heuristic will use to determine the next variable
   */
  QECODE_EXPORT BranchingHeuristic(QSpace* qs, VariableHeuristic* ev);

  /** Returns the next variable to branch on.
   * @param qs The current quantified space on which we are searching (must be a clone, possibly cloned, propagated and assigned multiple times of the quantified space given at the construction.)
   */
  QECODE_EXPORT int nextVar(QSpace* qs);


  /** Used by the solver to indicate it backtracked.
   *  @param qs The current quantified space on which we are searching, i.e. the one on which the solver just backtracked (must be a clone, possibly cloned, propagated and assigned multiple times of the quantified space given at the construction).
   */
  QECODE_EXPORT void backtrack(QSpace* qs);



  /** Used by the solver to indicate that a variable has been touched, and so thai its scors have possibly changed.
   *  @param qs The current quantified space on which we are searching (must be a clone, possibly cloned, propagated and assigned multiple times of the quantified space given at the construction).
   */
  QECODE_EXPORT void vartouched(int pos,QSpace* qs);
};




#endif
