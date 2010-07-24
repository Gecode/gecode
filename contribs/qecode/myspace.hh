/*****************************************************************[myspace.hh]
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
#ifndef __QECODE_MYSPACE__
#define __QECODE_MYSPACE__

#include "qecode.hh"
#include <vector>
#include "gecode/minimodel.hh"
#include "vartype.hh"

using namespace Gecode;
using namespace Gecode::Int;

/** \brief A simple extension of Gecode::Space class
 *
 *  A simple extension of the Space class from Gecode, in order to have access to the variables it contains.
 */

class QECODE_VTABLE_EXPORT MySpace : public Space {
	
	protected :
	unsigned int n;
	
public:
	/** \brief This array contains all the variables this space contains.
	 */
	void** v;
	
	/** \brief This array indicates the type of each variable
	 */
	VarType* type_of_v;
	
	/** \brief Constructor of a space with a fixed number of variables
	 *
	 * Builds a space which will contain  nv variables (the variables themselves are however not declared).
	 *  @param nv the number of variable the space must contain.
	 */
	QECODE_EXPORT MySpace(unsigned int nv);
	QECODE_EXPORT int nbVars() {return n;}
	QECODE_EXPORT MySpace(bool share,MySpace& ms);
	QECODE_EXPORT virtual MySpace* copy(bool share);
	QECODE_EXPORT virtual ~MySpace();
	QECODE_EXPORT int getValue(unsigned int i); ///< returns the value of variable i. If boolean : 0 or 1 (false / true).
	
	/** \brief Returns the integer variables before idMax
	 *
	 * Returns an IntVarArgs containing all the integer variables of index inferior than parameter idMax
	 */
	QECODE_EXPORT IntVarArgs getIntVars(unsigned int idMax);
	
	/** \brief Returns the boolean variables before idMax
	 *
	 * Returns a BoolVarArgs containing all the boolean variables of index inferior than parameter idMax
	 */
	QECODE_EXPORT BoolVarArgs getBoolVars(unsigned int idMax);
	
};

#endif
