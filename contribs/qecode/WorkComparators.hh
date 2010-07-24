/*
 *  WorkComparators.hh
 *  
 *
 *  Created by Jérémie Vautard on 31/03/10.
 *  Copyright 2010 Université d'Orléans. All rights reserved.
 *
 */

#include "WorkManager.hh"
#include "QCOPPlus.hh"

class BidonComparator : public WorkComparator {
	public : 
  virtual bool cmp(QWork a,QWork b) {
    return false;
  }
};

class DeepFirstComparator : public WorkComparator {
	public : 
  virtual bool cmp(QWork a,QWork b) {
    if (a.root().size() > b.root().size()) return true;
    if (a.root().size() < b.root().size()) return false;
    return ((a.getRemaining()) < (b.getRemaining()));
  }
};

class QuantifierThenDepthComparator : public WorkComparator {
	private :
	Qcop* problem;
	bool existsFirst;
	bool deepestFirst;
	
	public : 
	QuantifierThenDepthComparator(Qcop* p,bool existsFirst,bool deepestFirst) { 
		this->problem = p;
		this->existsFirst = existsFirst;
		this->deepestFirst = deepestFirst;
	}	
	virtual bool cmp(QWork a,QWork b) {
		bool q1 = (problem->qt_of_var(a.root().size()) != existsFirst);
		bool q2 = (problem->qt_of_var(b.root().size()) != existsFirst);
		if (q1 && !q2) return true;
		if (!q1 && q2) return false;
		int d1 = a.root().size();
		int d2 = b.root().size();
		if ( (d1 < d2) != deepestFirst) return true;
		return false;
	}
};

class DepthComparator : public WorkComparator {
	private : 
	bool deepestFirst;
	public : 
	DepthComparator(bool deepestFirst) {
		this->deepestfirst = deepestFirst;
	}
	
	virtual bool cmp(QWork a,QWork b) {
		int d1 = a.root().size();
		int d2 = b.root().size();
		if ( (d1 < d2) != deepestFirst) return true;
		return false;
	}
};
