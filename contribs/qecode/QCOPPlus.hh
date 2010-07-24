/****   , [ QCOPPlus.hh ], 
 Copyright (c) 2009 Universite d'Orleans - Jeremie Vautard 
 
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

#ifndef __QECODE_QCOP__
#define __QECODE_QCOP__

#include "myspace.hh"
#include "vartype.hh"
#include "OptVar.hh"
#include <vector>


using namespace std;
using namespace Gecode;
class QECODE_VTABLE_EXPORT Qcop {
private:
	struct Opts {
		vector<OptVar*> vars; // Several Optvars in universal scopes, one in existential ones
		int opt_type;        // Used in existential scopes : 0 for ANY, 1 for MIN, 2 for MAX.
	};
	int* nvar;
	int n;
	int nbSpaces;
//	void** v;
	VarType* type_of_v;
	
	bool* Quantifiers;
	MySpace** rules;
	MySpace* goal;
	Opts* optim;
	int* nbVarBySpace;
	int* whichSpaceOwns;
	bool* varInitialised;
	
	int currentDeclareSpace;
	Qcop() {}
public:
	forceinline QECODE_EXPORT int nv() {return n;}
	
	/** \brief  Default constructor for a restricted-quantified space
	 *
	 *  This is the first step for building a QCSP+/QCOP+ problem. The prefix is defined here.
	 *  @param ns The number of scopes in the prefix.
	 *  @param quant Array of booleans which contains the quantifier of every scope (true for universal, false for existential).
	 *  @param nv Array of integer which contains the number of variables by scope.
	 */    
	QECODE_EXPORT Qcop(int ns, bool* quant,int* nv);
	QECODE_EXPORT ~Qcop();
	
	/** \brief Defines an integer variable in the quantified space
	 *
	 *  Defines an integer variable in the quantifies space using a fully declared domain.
	 *  @param var Number of the variable to be  defined.
	 *  @param dom The initial domain of the variable.
	 */    
	QECODE_EXPORT void QIntVar(int var,int min,int max);
	
	/** \brief Defines an integer variable in the quantified space
	 *
	 *  Defines an integer variable in the quantifies space using a fully declared domain.
	 *  @param var Number of the variable to be  defined.
	 *  @param dom The initial domain of the variable.
	 */
	QECODE_EXPORT void QIntVar(int var,IntSet dom);
	
	/** \brief Defines a boolean variable in the quantified space
	 *
	 *  Defines a boolean variable with a full initial domain in the quantified space.
	 *  @param var Number of the variable to be defined.
	 */
	QECODE_EXPORT void QBoolVar(int var);
	
	/** \brief Returns the current declarating space
	 *
	 *  Return the space we are currently declarating constraints in. nextScope() is used to set the nextspace as the current one.
	 */    
	QECODE_EXPORT MySpace* space();
	
	/** \brief Returns an integer variable from the space we are currently declaring
	 *
	 *  Returns an integer variable from the cpace we are currently declaring. Will abort if the variable is not integer.
	 *  @param n The number of the variable to return.
	 */
	QECODE_EXPORT IntVar var(int n);
	
	/** \brief Returns a boolean variable from the space we are currently declaring
	 *
	 * Returns a boolean variable from the space we are currently declaring. Will abort if the variable is not boolean.
	 *  @param n The number of the variable to return.
	 */
	QECODE_EXPORT BoolVar bvar(int n);
	
	/** \brief Switch to the next space for constraint declaration
	 *
	 *   Switches to the next space for constraint declaration. var, bvar and space methods will now refer to the next rule space (or to the goal space if the current space was the last rule one).
	 *  Returns the new space number, or -1 if it was called while there was no next space to declare constraints in.
	 */
	QECODE_EXPORT int nextScope();
	
	/** \brief Finalizes the object construction
	 *
	 * Finalizes the restricted-quantified space construction. Must be invoked once all the variable and all the constraints have been declared, and before the search is performed on this space.
	 * calling this method is not mandatory, although it is recommanded to besure that the problem have been well built.
	 * For the existential scopes on which an optimization condition have not been defined yet, this method will post a "Any" optimization condition (i.e. do not optimize).
	 */
	QECODE_EXPORT void makeStructure();
	
	/** \brief returns an aggregate of the problem
	 *
	 * Creates an aggregate at a given universal scope. This aggregate is an optimization variable that an existential scope can use for optimizing.
	 * @param scope the scope where this aggregate will be defined. Must be an unversal scope
	 * @param opt The optimization variable we want to aggregate. There must not be any universal scope between an agregate and thisoptimization variable.
	 * @param agg The aggregator function that will be used for this aggregate.
	 */
	QECODE_EXPORT OptVar* getAggregate(int scope, OptVar* opt, Aggregator* agg);
	
	/** returns an existential optimization variable
	 *
	 * Creates an optimization variable that will have the value of an existential variable defined in the problem. 
	 * @param var the number of the existential variable that must be considered
	 */
	QECODE_EXPORT OptVar* getExistential(int var);
	
	/** set an optimization condition on an existential scope
	 *
	 * set an optimization condition on a given existential scope of the problem. An optimizaiton condition is composed of an optimization variable (aggregate or existential variable), and of an
	 * optimization type. 
	 * @param scope the scope on which the optimization condition is posted. Must be existential.
	 * @param optType the optimization type of the condision we post. 0 is for ANY, 1 is for MIN, 2 is for MAX.
	 * @param var the optimization variable to be minimized/maximized
	 */
	QECODE_EXPORT void optimize(int scope, int optType, OptVar* var);
	//    QECODE_EXPORT void print();
	
	QECODE_EXPORT forceinline bool qt_of_var(int v) {	return Quantifiers[whichSpaceOwns[v]];} ///< returns uantifier of variable 'v'
	QECODE_EXPORT forceinline bool quantification(int scope) {return Quantifiers[scope];} ///< returns quantifier of scope 'scope'
	QECODE_EXPORT int spaces(); ///< returns the number of scopes of the problem
	QECODE_EXPORT forceinline int nbVarInScope(int scope) {return nbVarBySpace[scope];}///< returns the number of variables present in scope 'scope'
	QECODE_EXPORT MySpace* getSpace(int scope);///< returns a copy of the Gecode::Space corresponding to the 'scope'-th restricted quantifier of the problem
	QECODE_EXPORT MySpace* getGoal();
	QECODE_EXPORT int getOptType(int scope); ///< returns the optimization type of the 'scope'-th scope of the problem
	QECODE_EXPORT OptVar* getOptVar(int scope);///< returns the optimization variable of the 'scope'-th scope of the problem
	QECODE_EXPORT Qcop* clone(); ///< makes a copy of the quantified problem
	QECODE_EXPORT forceinline int whoOwns(int var) {return whichSpaceOwns[var];}
};

#endif
