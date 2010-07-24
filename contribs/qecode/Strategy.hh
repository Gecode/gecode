/****   , [ bobocheTree.hh ], 
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

#ifndef __BOBOCHE_TREE__
#define __BOBOCHE_TREE__
#include <vector>
#include <iostream>
#include <string>
#include "StrategyNode.hh"
#include "qecode.hh"

using namespace std;
class Strategy;

class StrategyImp {
	friend class Strategy;
private:
	unsigned int pointers; // number of Strategy objects pointing to this
	StrategyNode zetag; // Node information
	vector<Strategy> nodes; // children of this node
	unsigned int todos; // number of todo nodes recursively present in the children
	StrategyImp* father;
	void todosUpdate(int i);
	StrategyImp();
	
public:
	StrategyImp(StrategyNode tag);
	~StrategyImp();    
};

/** \brief Strategy of a QCSP+ problem
 *
 * This class represents a solution of a QCSP+. Basically it consists in the tree-representation of the winning strategy.
 * 3 spacial cases exists : the trivially true strategy, the trivially false strategy (used for the UNSAT answer), 
 * and the "Dummy" node, used to link together each tree of a strategy which first player is universal (such a strategy is not a tree but a forest) 
 */
class QECODE_VTABLE_EXPORT Strategy {
	friend class StrategyImp;
private:
	StrategyImp* imp;
	void todosUpdate(int i) {imp->todosUpdate(i);}
	Strategy(StrategyImp* imp);
public:
	QECODE_EXPORT Strategy(); ///< default constructor
	QECODE_EXPORT Strategy(StrategyNode tag); ///< builds a strategy on a given strategy node (deprecated)
	
	/* \brief builds a one node (sub)strategy
	 *
	 * this method builds a one-node strategy that will typically be attached as child of another strategy. A strategy node embeds informations about quantification, 
	 * scope and values of variables that must be provided
	 * @param qt quantifier of the scope this node represents
	 * @param VMin index of the first variable of the scope this node represents
	 * @param VMax index of the last variable of the scope this node represents
	 * @param values values taken by the variables between VMin and VMax in this part of the strategy
	 */
	QECODE_EXPORT Strategy(bool qt,int VMin, int VMax, int scope,vector<int> values); 
	QECODE_EXPORT Strategy(const Strategy& tree); ///< copy constructor
	QECODE_EXPORT Strategy& operator = (const Strategy& rvalue); 
	QECODE_EXPORT ~Strategy();
	QECODE_EXPORT StrategyNode getTag();///< DEPRECATED returns the StrategyNode object corresponding to the root of this strategy
	QECODE_EXPORT forceinline unsigned int degree() {return imp->nodes.size();}///< returns this strategy's number of children
	QECODE_EXPORT Strategy getChild(int i); ///< returns the i-th child of this strategy
	QECODE_EXPORT Strategy getFather();
	QECODE_EXPORT bool hasFather();
	QECODE_EXPORT Strategy getSubStrategy(vector<int> position); ///< returns the substrategy at given position, dummy if does not exists
	QECODE_EXPORT void attach(Strategy child);///< attach the strategy given in parameter as a child of the current strategy
	QECODE_EXPORT void detach(unsigned int i); ///< detach the i-th child of this strategy (provided it exists)
	QECODE_EXPORT void detach(Strategy son);
	QECODE_EXPORT forceinline bool isFalse() {return imp->zetag.isFalse();} ///< returns wether this strategy represents the UNSAT answer
	QECODE_EXPORT forceinline bool isTrue() {return imp->zetag.isTrue();} ///< returns wether this strategy is trivially true
	QECODE_EXPORT forceinline bool isComplete() {return ((imp->todos) == 0);} ///< returns wether this is a complete sub-strategy (without todo nodes)
	QECODE_EXPORT forceinline bool isDummy() {return imp->zetag.isDummy();} ///< returns wether this strategy is a set of 
	QECODE_EXPORT forceinline bool isTodo() {return imp->zetag.isTodo();} ///< return wether this strategy is a "ToDo" marker or not
	
	QECODE_EXPORT static Strategy STrue(); ///< returns the trivially true strategy
	QECODE_EXPORT static Strategy SFalse(); ///< returns the trivially false strategy
	QECODE_EXPORT static Strategy Dummy(); ///< returns a "dummy" node
	QECODE_EXPORT static Strategy Stodo(); ///< returns a "todo" node
	QECODE_EXPORT forceinline bool quantifier() {return imp->zetag.quantifier;} ///< returns the quantifier of the root (true for universal, false for existential)
	QECODE_EXPORT forceinline int VMin() {return imp->zetag.Vmin;} ///< returns the index of the first variable of the scope of the root
	QECODE_EXPORT forceinline int VMax() {return imp->zetag.Vmax;} ///< returns the index of the last variable of the scope of the root
	QECODE_EXPORT forceinline int scope() {return imp->zetag.scope;} ///< returns the scope of the root
	QECODE_EXPORT forceinline vector<int> values() {return imp->zetag.valeurs;} ///< returns the values taken by the variables of the scope of the root in this (sub)strategy
	QECODE_EXPORT forceinline int value(int var) {return imp->zetag.valeurs[var];}
	QECODE_EXPORT forceinline void* id() {return imp;} ///< Return an identifier for this strategy (this identifier is shared among multiples instances of this strategy)
	QECODE_EXPORT vector<int> getPosition();
	QECODE_EXPORT forceinline int nbTodos() {return imp->todos;}
	
	QECODE_EXPORT int checkIntegrity();
};


#endif
