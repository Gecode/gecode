/****   , [ QCOPPlus.cc ], 
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

#include "QCOPPlus.hh"

Qcop::Qcop(int ns, bool* quant, int* nv) {
	n=0;
	this->nvar = nv;
	for (int i=0;i<ns;i++) {
		n += nv[i];
	}
	nbSpaces=ns;
//	v=new void*[n];
	type_of_v = new VarType[n];
	Quantifiers = quant;
	whichSpaceOwns=new int[n];
	nbVarBySpace=new int[nbSpaces];
	rules=new MySpace*[ns];
	nbVarBySpace[0]=nv[0];
	rules[0]=new MySpace(nbVarBySpace[0]);
	for (int i=1;i<nbSpaces;i++) {
		nbVarBySpace[i]=nbVarBySpace[i-1]+nv[i];
		rules[i]=new MySpace(nbVarBySpace[i]);
	}
	goal=new MySpace(n);
	for(unsigned int i=0;i<n;i++) {
		int lespace=0;
		while (nbVarBySpace[lespace]<=i) lespace++;
		whichSpaceOwns[i]=lespace;
	}
	
	
	varInitialised=new bool[n];
	for (unsigned int i=0;i<n;i++) varInitialised[i]=false;
	currentDeclareSpace=0;
	
	optim = new Opts[nbSpaces];
	for (int i=0;i<nbSpaces;i++) 
		optim[i].opt_type = 0;
}


Qcop::~Qcop() {
	for (int i=0;i<nbSpaces;i++) {
		delete rules[i];
	}    
	
	delete goal;
	delete [] rules;
}


int Qcop::spaces() {
	return nbSpaces;
}


void Qcop::QIntVar(int var,int min,int max) {
	if (varInitialised[var]) {
		cout<<"Variable "<<var<<"  Already created !!"<<endl;
		abort();
	}
	
	for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
		rules[i]->v[var] = new IntVar(*rules[i],min,max);
		rules[i]->type_of_v[var] = VTYPE_INT;
	}
	goal->v[var] = new IntVar(*goal,min,max);
	goal->type_of_v[var] = VTYPE_INT;
	varInitialised[var]=true;
	type_of_v[var]=VTYPE_INT;
}


void Qcop::QIntVar(int var,IntSet dom) {
	if (varInitialised[var]) {
		cout<<"Variable "<<var<<"  Already created !!"<<endl;
		abort();
	}
	
	for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
		rules[i]->v[var] = new IntVar(*rules[i],dom);
		rules[i]->type_of_v[var] = VTYPE_INT;
	}
	goal->v[var] = new IntVar(*goal,dom);
	goal->type_of_v[var] = VTYPE_INT;
	varInitialised[var]=true;
	type_of_v[var]=VTYPE_INT;
}


void Qcop::QBoolVar(int var) {
	if (varInitialised[var]) {
		cout<<"Variable "<<var<<" Already created !!"<<endl;
		abort();
	}
	
	for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
		rules[i]->v[var] = new BoolVar(*rules[i],0,1);
		rules[i]->type_of_v[var]=VTYPE_BOOL;
	}
	goal->v[var] = new BoolVar(*goal,0,1);
	goal->type_of_v[var]=VTYPE_BOOL;
	varInitialised[var]=true;
	type_of_v[var]=VTYPE_BOOL;
}

MySpace* Qcop::space() {
	if (currentDeclareSpace<nbSpaces)
		return rules[currentDeclareSpace];
	if (currentDeclareSpace==nbSpaces)
		return goal;
	return NULL;
}


IntVar Qcop::var(int n) {
	if (!varInitialised[n]) {
		cout<<"Variable "<<n<<" not initialized !"<<endl;
		abort();
	}
	if (type_of_v[n] != VTYPE_INT) {
		cout<<"Variable "<<n<<" is not INT"<<endl;
		abort();
	}
	return *(static_cast<IntVar*>(space()->v[n]));
}


BoolVar Qcop::bvar(int n) {
	if (!varInitialised[n]) {
		cout<<"Variable "<<n<<" not initialized !"<<endl;
		abort();
	}
	if (type_of_v[n] != VTYPE_BOOL){
		cout<<"Variable "<<n<<" is not BOOL"<<endl;
		abort();
	}
	return *(static_cast<BoolVar*>(space()->v[n]));
}


int Qcop::nextScope() {
	if (currentDeclareSpace == -1) return -1;
	currentDeclareSpace++;
	if (currentDeclareSpace>nbSpaces) return -1;
	return currentDeclareSpace;
}

void Qcop::makeStructure() {
	for (unsigned int i=0;i<n;i++) {
		if (varInitialised[i] == false) {
			cout<<"Can't make structure : variable "<<i<<" not initialised"<<endl;
			abort();
		}
	}
	
	for (int i=0;i<nbSpaces;i++) {
		if (!Quantifiers[i])
			if (optim[i].vars.empty())
				optimize(i,0,getExistential( (i==0)?0:nbVarBySpace[i-1]));
	}
	
	for (unsigned int i=0;i<this->nbSpaces;i++) {
		if (rules[i]->status() == SS_FAILED) {
			cout<<"MakeStructure : rule space "<<i<<" is already failed."<<endl;
		}
	}
	if (goal->status() == SS_FAILED) {
		cout<<"MakeStructure : goal space is already failed."<<endl;
	}
	
}


OptVar* Qcop::getAggregate(int scope, OptVar* opt, Aggregator* agg) {
	if (!Quantifiers[scope]) {cout<<"Try to get aggregate on existential scope"<<endl;abort();} // aggregateur sur existentiel
	if (opt->getScope() < scope) {cout<<"aggregated variable out of aggregator scope"<<endl;abort();} // Variable aggrégée avant aggregateur
	for (int i=scope+1; i<opt->getScope();i++) // Universelle entre aggregateur et variable aggrégée
		if (Quantifiers[i]) 
		{cout<<"Universal scope between variable and aggregator"<<endl;abort();}
	
	UnivOptVar* zeopt = new UnivOptVar(scope,opt,agg);
	optim[scope].vars.push_back(zeopt);
	return zeopt;
}


OptVar* Qcop::getExistential(int var) {
	if (Quantifiers[whichSpaceOwns[var]]) {cout<<"can't create existOptVar : variable "<<var<<" is universal"<<endl;abort();} // Var est universelle
	ExistOptVar* opt = new ExistOptVar(var,whichSpaceOwns[var]);
	return opt;
}


void Qcop::optimize(int scope, int optType,OptVar* var) {
	if (var->getScope() < scope) {cout<<"optvar out of optimizing scope"<<endl;abort();} // Variable à optimiser avant décideur
	for (int i=scope; i<var->getScope();i++) {
		if (Quantifiers[i])  // universelle entre variable et décideur
		{ cout<<"universal scope between variable and optimizing scope"<<endl;abort();}
	}
	//    cout<<"I add an optVar* for scope "<<scope<<endl;
	optim[scope].vars.clear();
	optim[scope].vars.push_back(var);
	optim[scope].opt_type=optType;
}


MySpace* Qcop::getSpace(int scope) {
	if (scope<0 || scope>nbSpaces) {
		cout<<"I return NULL coz of bad scope value (<0)"<<endl;
		return NULL;
	}
	if (scope==nbSpaces) {
		if (goal->status() == SS_FAILED) {
			cout<<"I return NULL coz goal is failed"<<endl;
			return NULL;
		}
		//        cout<<"I return the goal"<<endl;
		return static_cast<MySpace*>(goal->clone());
	}
	if (rules[scope]->status() == SS_FAILED) {
		cout<<"I return NULL coz scope "<<scope<<" is failed"<<endl;
		return NULL;
	}
	//    cout<<"I return the rule "<<scope<<endl;
	return (static_cast<MySpace*>(rules[scope]->clone()));
}


MySpace* Qcop::getGoal() {
	if (goal->status() == SS_FAILED) return NULL;
	return static_cast<MySpace*>(goal->clone());
}


int Qcop::getOptType(int scope) {
	if (Quantifiers[scope]) {cout<<"Try to get OptType on universal scope"<<endl;abort();}
	return optim[scope].opt_type;
}


OptVar* Qcop::getOptVar(int scope) {
	if (Quantifiers[scope]) abort();
	//    if (optim[scope].vars.size() == 0) cout<<"no optvar to return, ca va planter...";
	//    cout<<"getoptvar returns optvar* of scope "<<scope;
	return static_cast<OptVar*>(optim[scope].vars[0]);
}

Qcop* Qcop::clone() {
	bool* qt = new bool[nbSpaces];
	int* nvv = new int[nbSpaces];
	MySpace** zerulz = new MySpace*[nbSpaces];
	Opts* opop = new Opts[nbSpaces];
	int* nbvbs = new int[nbSpaces];
	for (unsigned int i=0;i < nbSpaces;i++) {
		qt[i] = this->Quantifiers[i];
		nvv[i] = this->nvar[i];
		zerulz[i] = static_cast<MySpace*>(this->rules[i]->clone(false));
		opop[i] = this->optim[i];
		nbvbs[i] = this->nbVarBySpace[i];
	}
	
	//	void** v = new void*[this->n];
	VarType* typeofv = new VarType[n];
	int* wso = new int[n];
	for (unsigned int i=0;i<n;i++) {
		typeofv[i] = this->type_of_v[i];
		wso[i] = this->whichSpaceOwns[i];
	}
	
	Qcop* ret = new Qcop();
	ret->nvar = nvv;
	ret->n = this->n;
	ret->nbSpaces = this->nbSpaces;
	ret->type_of_v = typeofv;
	ret->Quantifiers = qt;
	ret->rules = zerulz;
	ret->goal = static_cast<MySpace*>(this->goal->clone(false));
	ret->optim = opop;
	ret->nbVarBySpace = nbvbs;
	ret->whichSpaceOwns = wso;
	ret->varInitialised = this->varInitialised;
	currentDeclareSpace = this->currentDeclareSpace;

return ret;
}