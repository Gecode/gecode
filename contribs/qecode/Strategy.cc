/****   , [ bobocheTree.cc ], 
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

#include "Strategy.hh"

StrategyImp::StrategyImp() {
	cout<<"Default constructor of StrategyImp should not be called !"<<endl;
	pointers=1;
	zetag=StrategyNode::Dummy();
	todos=0;
	father=NULL;
}	
	
void StrategyImp::todosUpdate(int i) {
	todos += i;
	if (father != NULL) father->todosUpdate(i);
}

StrategyImp::StrategyImp(StrategyNode tag) {
	//    cout<<"Strategy imp constructor"<<endl;
	pointers=1;
	zetag=tag;
	todos=0;
	father=NULL;
	//    cout<<"strategyimp constructor fini"<<endl;
}


StrategyImp::~StrategyImp() {
	for (vector<Strategy>::iterator i = nodes.begin();i != nodes.end();i++) {
		if (((*i).imp->father) == this) {
			(*i).imp->father = NULL;
		}
	}
}


Strategy::Strategy() {
	//    cout<<"strategy default"<<endl;
	StrategyNode tag = StrategyNode::Dummy();
	imp = new StrategyImp(tag);
}

Strategy::Strategy(StrategyNode tag) {
	//    cout<<"Strategy with tag"<<endl;
	imp = new StrategyImp(tag);
	//cout<<"passed imp creation. End of strategy creator"<<endl;
}

Strategy::Strategy(StrategyImp* imp) {
	this->imp = imp;
	this->imp->pointers++;
}

Strategy::Strategy(bool qt,int VMin, int VMax, int scope, vector<int> values) {
	//    cout<<"strategy with values"<<endl;
	StrategyNode tag(2,qt,VMin,VMax,scope);
	tag.valeurs=values;
	imp = new StrategyImp(tag);
	
}


Strategy::Strategy(const Strategy& tree) {
	//    cout<<"Strategy copy"<<endl;
	imp = tree.imp;
	(imp->pointers)++;
}


Strategy& Strategy::operator = (const Strategy& rvalue) {
	//    cout<<"Strategy = "<<endl;
	if (imp != NULL) {
		(imp->pointers)--;
		if ( (imp->pointers) == 0) {
			//        cout<<"no more references for the imp. Delete"<<endl;
			delete imp;
		}
	}
	imp = rvalue.imp;
	(imp->pointers)++;
	return *this;
}


Strategy::~Strategy() {
	//    cout<<"strategy destructor"<<endl;
	(imp->pointers)--;
	if ( (imp->pointers) == 0) {
		//        cout<<"no more references for the imp. Delete"<<endl;
		delete imp;
	}
}


StrategyNode Strategy::getTag() {
	return imp->zetag;
}

Strategy Strategy::getFather() {
	if (hasFather()) return Strategy(imp->father);
	return Dummy();
}

bool Strategy::hasFather() {
	if (imp->father != NULL) {
		for (int i=0;i< imp->father->nodes.size();i++) {
			if ((imp->father->nodes[i].imp)  == imp)
				return true;
		}
	}
	return false;
}

Strategy Strategy::getChild(int i) {
	if (i<0 || i>=degree() ) {cout<<"Child "<<i<<" does not exist"<<endl;abort();}
	return imp->nodes[i];
}

Strategy Strategy::getSubStrategy(vector<int> position) {
	if (position.empty()) return *this;
	int deg = degree();
	if (deg == 0) {
		cout<<"Did not find substrategy"<<endl;
		return Strategy::Dummy();
	}
	for (int i=0;i<deg;i++) {
		Strategy child=getChild(i);
		bool ok=true;
		if (child.values().size() == 0) {
			ok = false;
		}
		for (int j=0;(j<child.values().size()) && ok;j++) {
			if (child.value(j) != position[j]) ok=false;
		}
		if (ok) {
			position.erase(position.begin(),position.begin() + (child.values().size()));
			return child.getSubStrategy(position);
		}
	}
	cout<<"Did not find substrategy"<<endl;
	return Strategy::Dummy();
}

void Strategy::attach(Strategy child) {
	if (child.isDummy()) {
		int todosToAdd = 0;

		for (int i=0;i<child.degree();i++) {
			this->attach(child.getChild(i));
		}
	}
	else {
		imp->nodes.push_back(child);
		todosUpdate(child.imp->todos);
		(child.imp)->father = this->imp;
	}
}

void Strategy::detach(Strategy son) {
	
	vector<Strategy>::iterator it = imp->nodes.begin();
	while (it != (imp->nodes.end()) && ( (*it).id() != son.id())) {
	it++;}
	if ( it != imp->nodes.end()) {
		todosUpdate(0-((*it).imp->todos));
		(*it).imp->father=NULL;
		imp->nodes.erase(it);
	}
}


void Strategy::detach(unsigned int i) {
	if (imp->nodes.size() < i) return;
	
	vector<Strategy>::iterator it = imp->nodes.begin()+i;
	todosUpdate(0-((*it).imp->todos));
	(*it).imp->father=NULL;
	imp->nodes.erase(it);
}

Strategy Strategy::STrue() {
	Strategy ret(StrategyNode::STrue());
	return ret; 
}

Strategy Strategy::SFalse() {
	Strategy ret(StrategyNode::SFalse());
	return ret;
}

Strategy Strategy::Dummy() {
	Strategy ret(StrategyNode::Dummy());
	return ret;
}

Strategy Strategy::Stodo() {
	Strategy ret(StrategyNode::Todo());
	ret.imp->todos=1;
	return ret;
}

vector<int> Strategy::getPosition() {
	vector<int> ret;
	Strategy asc = *this;
	while (!asc.isDummy()) {
//		cout<<"GetPosition adding "<<asc.values().size()<<" elements to a vector of size "<<ret.size()<<endl;
		vector<int> ret2;
		ret2.reserve(ret.size() + asc.values().size() +1);
		for (int i=0;i<asc.values().size();i++) {
			ret2.push_back(asc.values()[i]);
		}
		for (int i=0;i<ret.size();i++) {
			ret2.push_back(ret[i]);
		}
		ret=ret2;
		asc = asc.getFather();
	}
	return ret;
}

int Strategy::checkIntegrity() {
	int ret=0;
	for (unsigned int i=0;i < (this->degree());i++) {
		if ( (((imp->nodes[i]).imp)->father) != imp) {
			ret++;
			cout<< (((imp->nodes[i]).imp)->father) << " should be " << imp <<endl;
		}
		ret += (getChild(i).checkIntegrity());
	}
	return ret;
}
		

