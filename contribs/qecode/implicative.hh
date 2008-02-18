/*****************************************************************[implicative.hh]
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
#ifndef __QECODE_IMPLICATIVE__
#define __QECODE_IMPLICATIVE__
#include "qecode.hh"
#include <iostream>
#include <cstdlib>
#include "gecode/minimodel.hh"
#include "warner.hh"
#include "qecore.hh"
#include "myspace.hh"

using namespace Gecode;
using namespace std;
using namespace Gecode::Int;


class QECODE_VTABLE_EXPORT Implicative : public QSpace {
    friend class valueHeuristic;

protected : 
  bool* q;                ///< Quantification of the variables (true for universal, false for existential)
  int nbSpaces;           ///< Number of rule spaces
  int currentDeclareSpace;///< The rule space in which we are currently declaring constraints (for declaration part only)
  int* nbVarBySpace;      ///< Number of variables by rule space.
  int* whichSpaceOwns;    ///< Indicates in which space a given variable first appears.
  MySpace** rules;        ///< The rule spaces
  MySpace* goal;          ///< The goal space
  bool* varInitialised;   ///< Indicates wether a variable has been initialised or not (for declaration part only)
  bool* ruleDefined;      ///< Indicates wether a rule space is defind or not. If not, it has a failed or solved value.
  int* ruleStatus;        ///< Indicates wether a rule space is failed (0), solved (1), or not yet decided (-1).
  bool goalDefined;       ///< Indicates wether the goal space is defined or not.
  int goalStatus;         ///< Indicates wether the goal is failed, solved, or not yet decided.
  Warner* w;
  int prop_power;         ///< Propagation power applied.
  /// Cascade propagation 
  int cascade(int firstSpace, unsigned long int& propsteps);

public : 

  /** \brief  Default constructor for a restricted-quantified space.
   *  This is the first step for building a restricted-quantified space. The prefix is defined here.
   *  @param ns The number of scope i.e. the number of quantifier alternations in the prefix.
   *  @param firstQ Must be set to true if the first quantifier is universal, false otherwise.
   *  @param nv Array of integer which contains the number of variables by scope.
   */
  QECODE_EXPORT Implicative(int ns,bool firstQ,int* nv); 

  /** Copy constructor for a restricted-quantified space.
   *  @param im The space we want to copy.
   */
  QECODE_EXPORT Implicative(Implicative& im);

  /** Destructor for a restricted-quantified space.
   *  This doesn't destruct the shared datastructures between a quantified space and its copy. 
   */
  QECODE_EXPORT virtual ~Implicative();

  /** \brief Number of "rules" the quantified space contains.
   *  Returns the number of rules the quantified space contains. This is equal to the number of quantifier alternations in the prefix. 
   */
  QECODE_EXPORT int spaces();

  /** \brief Defines an integer variable in a quantified space.
   *  Defines an integer variable in the quantified space with an interval domain. 
   *  @param var number of the variable to be defined.
   *  @param min The min element of the domain
   *  @param max the max element of the domain
   */
  QECODE_EXPORT void QIntVar(int var,int min,int max);

  /** \brief Defines an integer variable in the quantified space.
   *  Defines an integer variable in the quantifies space using a fully declared domain.
   *  @param var Number of the variable to be  defined.
   *  @param dom The initial domain of the variable.
   */
  QECODE_EXPORT void QIntVar(int var,IntSet dom);

  /** \brief Defines a boolean variable in the quantified space.
   * Defines a boolean variable in the quantified space, uning a min and a max domain value (with false < true).
   *  @param var Number of the variable to be defined.
   *  @param min The min element of the domain (0 or 1, respectively for false or true)
   *  @param min The max element of the domain (0 or 1, respectively for false or true)
   */
  QECODE_EXPORT void QBoolVar(int var,int min, int max);

  /** \brief Defines a boolean variable in the quantified space.
   *  Defines a boolean variable with a full initial domain in the quantified space.
   *  @param var Number of the variable to be defined.
   */
  QECODE_EXPORT void QBoolVar(int var) {QBoolVar(var,0,1);}


//  /** \brief Returns one of the restrictor space.
//   * Returns one of the restrictor space. Deprecated : use space() and nextScope instead for constraint posting.
//   *  @param sp the number of the restrictor to get (first restrictor is 0).
//   */
//  MySpace* getRuleSpace(int sp);
//
//
//  /** \brief Returns the goal space.
//   *  Returns the goal space. Deprecated : use space() and nextScope instead for constraint posting.
//   */
//  MySpace* getGoalSpace();

  
  /** \brief Returns the current declarating space. 
   *  Return the space we are currently declarating constraints in. nextScope() is used to set the nextspace as the current one.
   */
  QECODE_EXPORT MySpace* space();


  /** \brief Returns an integer variable from the space we are currently declaring.
   *  Returns an integer variable from the cpace we are currently declaring. Will abort if the variable is not integer.
   *  @param n The number of the variable to return.
   */
  QECODE_EXPORT IntVar var(int n); 


  /** \brief Returns a boolean variable from the space we are currently declaring.
   * Returns a boolean variable from the space we are currently declaring. Will abort if the variable is not boolean.
   *  @param n The number of the variable to return.
   */
  QECODE_EXPORT BoolVar bvar(int n); 



  /** \brief Switch to the next space for constraint declaration.
   *   Switches to the next space for constraint declaration. var, bvar and space methods will now refer to the next rule space (or to the goal space if the current space was the last rule one).
   *  Returns the new space number, or -1 if it was called while there was no next space to declare constraints in.
   */
  QECODE_EXPORT int nextScope();


  /** \brief Finalizes the restricted-quantified space construction. 
   * Finalizes the restricted-quantified space construction. Must be invoked once all the variable and all the constraints have been declared, and before the search is performed on this space.
   */
  QECODE_EXPORT void makeStructure();

  /** \brief Prints the current state of an implicative object.
   * Prints the variable domains of all the spaces (if defined). Does not perform an ounce of propagation
  */
  QECODE_EXPORT virtual void print();
      
      
  QECODE_EXPORT virtual bool quantification(int v);
  QECODE_EXPORT virtual int status(int var,unsigned long int& propsteps);
  QECODE_EXPORT virtual int finalStatus(unsigned long int& propsteps);
  QECODE_EXPORT virtual bool subsumed(int var);
  QECODE_EXPORT virtual Implicative* clone(); 
  QECODE_EXPORT virtual void assign_int(int var,int** vals,int nbVals);
  QECODE_EXPORT virtual void remove_int(int var,int** vals,int nbVals);
  QECODE_EXPORT virtual void assign_bool(int var,int** vals,int nbVals);
  QECODE_EXPORT virtual void remove_bool(int var,int** vals,int nbVals);
  QECODE_EXPORT virtual void assign_bool(int var,int b);
  QECODE_EXPORT virtual void remove_bool(int var,int b);
//  virtual void infEqual(int var,int val);
//  virtual void sup(int var,int val);
  QECODE_EXPORT virtual void backtrack();
  QECODE_EXPORT virtual void indicateBranchingHeuristic(BranchingHeuristic* bh);
};


#endif
