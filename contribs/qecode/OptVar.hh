/****   , [ OptVar.hh ], 
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
#ifndef __QECODE_OPTVAR__
#define __QECODE_OPTVAR__

#include "Strategy.hh"
#include <vector>

/** \brief Abstract class for an Aggregator
*   
*   An aggregator is defined as a function Multiset_of_int -> int. From an implementation
*   point of view, this multiset is a vector of int.
*/
class QECODE_VTABLE_EXPORT Aggregator {
public:
    virtual int eval(vector<int> values)=0;
    virtual ~Aggregator() {}
};

/** \brief Sum aggregator
*
* This aggregator computes the sum of all elements of the multiset
*/
class QECODE_VTABLE_EXPORT AggregatorSum : public Aggregator{
public:
    QECODE_EXPORT virtual int eval(vector<int> values) {
        int cpt=0;
        for (int i=0;i<values.size();i++) 
            cpt += values[i];
        return cpt;
    }
    virtual ~AggregatorSum() {}
};

/** \brief Mean aggregator
*
* This aggregator computes the mean of all elements of the multiset
*/
class QECODE_VTABLE_EXPORT AggregatorMean : public Aggregator{
public:
    QECODE_EXPORT virtual int eval(vector<int> values) {
        int size=values.size();
        if (size == 0) return 0;
        int cpt=0;
        for (int i=0;i<size; i++)
            cpt += values[i];
        cpt = cpt / size;
        return cpt;
    }
    virtual ~AggregatorMean() {}
};

/** \brief Abstract class for an optimization variable
*
* This class defines the interface of an optimization variable, which can be either an existential variable, of the result of an aggregator function
*/
class QECODE_VTABLE_EXPORT OptVar {
public:
    QECODE_EXPORT virtual int getVal(Strategy s)=0; ///< returns value of this optimization variable in the substrategy s
    QECODE_EXPORT virtual int getScope()=0; ///< returns the scope where this optimization variable belong
    virtual ~OptVar() {}
};

/** \brief Existential optimization variable
*
* This class defines an existential optimization variable. This variabe is just an other point of view for an existential variable of the problem.
*/
class QECODE_VTABLE_EXPORT ExistOptVar : public OptVar {
private:
    int varId;
    int scopeId;
public:
    QECODE_EXPORT ExistOptVar(int var,int scope);
    QECODE_EXPORT virtual int getVal(Strategy s);
    QECODE_EXPORT virtual int getScope();
    virtual ~ExistOptVar() {}
};

/** \brief Universal optimization variable (aggregator result)
*
* This class defines a universal optimization variable. Such a variable represents the result of an aggregator on the set of values that an inner
* optimization variable takes in every sub-strategy of the current scope. 
*/
class QECODE_VTABLE_EXPORT UnivOptVar : public OptVar {
private:
    int scopeId;
    OptVar* var;
    Aggregator* fct;
public:
        /** \brief constructor for an universal optimization variable
        *
        * Builds a universal optimzation variable at a given (universal) scope, that will represent the result of the agg aggregator on the set of all values 
        * taken by the optimization variable zevar in each substrategy below the current point.
        * @param scope the scope where this optimization variable belong
        * @param zevar the inner optimization variable that will be aggregated
        * @param agg the aggregator function to be used
        */
    QECODE_EXPORT UnivOptVar(int scope,OptVar* zevar,Aggregator* agg);
    QECODE_EXPORT virtual int getVal(Strategy s);
    QECODE_EXPORT virtual int getScope();
    virtual ~UnivOptVar() {delete fct;}
};
    
#endif

