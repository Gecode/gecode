/*************************************************************[implicative.cc]
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
#include "implicative.hh"
#include "myDom.cc"

Implicative::Implicative(int ns,bool firstQ,int* nv) : QSpace() {
    n=0;
    for (int i=0;i<ns;i++) {
        n += nv[i];
    }
    nbSpaces=ns;
    v=new void*[n];
    type_of_v = new VarType[n];
    q=new bool[n];
    whichSpaceOwns=new int[n];
    nbVarBySpace=new int[nbSpaces];
    rules=new MySpace*[ns];
    ruleDefined=new bool[n];
    nbVarBySpace[0]=nv[0];
    rules[0]=new MySpace(nbVarBySpace[0]);
    ruleDefined[0] = true;
    for (int i=1;i<nbSpaces;i++) {
        nbVarBySpace[i]=nbVarBySpace[i-1]+nv[i];
        rules[i]=new MySpace(nbVarBySpace[i]);
        ruleDefined[i]=true;
    }
    goal=new MySpace(n);
    goalDefined=true;
    for(unsigned int i=0;i<n;i++) {
        int lespace=0;
        while (nbVarBySpace[lespace]<=i) lespace++;
        whichSpaceOwns[i]=lespace;
        q[i] = ((lespace%2 == 0) ? firstQ : !firstQ);
    }
    
    varInitialised=new bool[n];
    for (unsigned int i=0;i<n;i++) varInitialised[i]=false;
    currentDeclareSpace=0;
    prop_power=2;
}


Implicative::Implicative(Implicative& qc) : QSpace(qc) {
    
    unsigned long int rien;
    q=qc.q;
    nbSpaces=qc.nbSpaces;             // | Shared among every descendants of Implicatives
    nbVarBySpace=qc.nbVarBySpace;     // |
    whichSpaceOwns=qc.whichSpaceOwns; // |
    varInitialised=qc.varInitialised; // |
    rules = new MySpace*[n];
    ruleDefined=new bool[n];
    w=qc.w;
    w->update(this);
    prop_power=qc.prop_power;
    
    for (int i=0;i<nbSpaces;i++) {
        if (!qc.ruleDefined[i]) {
            ruleDefined[i]=false;
        }
        else if (qc.rules[i]->status(rien) == SS_FAILED) {
            ruleDefined[i]=false;
        }
        else {
            rules[i]=static_cast<MySpace*>(qc.rules[i]->clone());
            ruleDefined[i]=true;
        }
    }
    if (!qc.goalDefined) {
        goalDefined=false;
    }
    else if (qc.goal->status(rien) == SS_FAILED) {
        goalDefined=false;
    }
    else {
        goal=static_cast<MySpace*>(qc.goal->clone());
        goalDefined=true;
    }
    
    for (unsigned int i=0;i<n;i++) {
        if (ruleDefined[whichSpaceOwns[i]]) {
            v[i] = (rules[whichSpaceOwns[i]]->v[i]);
        }
    }
    
    
}


Implicative::~Implicative() {
    for (int i=0;i<nbSpaces;i++) {
        if (ruleDefined[i])
            delete rules[i];
    }    
    
    if (goalDefined) 
        delete goal;
    delete [] rules;
    delete [] ruleDefined;
}


int Implicative::spaces() {
    return nbSpaces;
}


void Implicative::QIntVar(int var,int min,int max) {
    if (varInitialised[var]) {
        cout<<"Variable "<<var<<"  Already created !!"<<endl;
        abort();
    }
    
    for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
        rules[i]->v[var] = new IntVar(rules[i],min,max);
        rules[i]->type_of_v[var] = VTYPE_INT;
    }
    goal->v[var] = new IntVar(goal,min,max);
    goal->type_of_v[var] = VTYPE_INT;
    varInitialised[var]=true;
    type_of_v[var]=VTYPE_INT;
}


void Implicative::QBoolVar(int var,int min,int max) {
    if (varInitialised[var]) {
        cout<<"Variable "<<var<<" Already created !!"<<endl;
        abort();
    }
    
    for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
        rules[i]->v[var] = new BoolVar(rules[i],min,max);
        rules[i]->type_of_v[var]=VTYPE_BOOL;
    }
    goal->v[var] = new BoolVar(goal,min,max);
    goal->type_of_v[var]=VTYPE_BOOL;
    varInitialised[var]=true;
    type_of_v[var]=VTYPE_BOOL;
}


void Implicative::QIntVar(int var,IntSet dom) {
    if (varInitialised[var]) {
        cout<<"Variable "<<var<<" Already created !!"<<endl;
        abort();
    }
    
    for (int i=whichSpaceOwns[var];i<nbSpaces;i++) {
        rules[i]->v[var] = new IntVar(rules[i],dom);
        rules[i]->type_of_v[var] = VTYPE_INT;
    }
    goal->v[var] = new IntVar(goal,dom);
    goal->type_of_v[var] = VTYPE_INT;
    varInitialised[var]=true;
    type_of_v[var]=VTYPE_INT;
}


MySpace* Implicative::space() {
    if (currentDeclareSpace<nbSpaces)
        return rules[currentDeclareSpace];
    if (currentDeclareSpace==nbSpaces)
        return goal;
    return NULL;
}


IntVar Implicative::var(int n) {
    return *(static_cast<IntVar*>(space()->v[n]));
}


BoolVar Implicative::bvar(int n) {
    return *(static_cast<BoolVar*>(space()->v[n]));
}


int Implicative::nextScope() {
    if (currentDeclareSpace == -1) return -1;
    currentDeclareSpace++;
    if (currentDeclareSpace>nbSpaces) return -1;
    return currentDeclareSpace;
}

/*
MySpace* Implicative::getRuleSpace(int sp) {
    bool var=true;
    for(int i=0;i<n;i++) {
        var = var && varInitialised[i];
    }
    if (!var) return NULL;
    if ( sp >= nbSpaces ) return NULL;
    return rules[sp];
}


MySpace* Implicative::getGoalSpace() {
    bool var=true;
    for(int i=0;i<n;i++) {
        var = var && varInitialised[i];
    }
    if (!var) return NULL;
    return goal;
}
*/

void Implicative::makeStructure() {
//    bool varInit=true;
    for (unsigned int i=0;i<n;i++) {
        if (varInitialised[i] == false) {
            cout<<"Can't make structure : variable "<<i<<" not initialised"<<endl;
            abort();
        }
    }
    
    for (unsigned int i=0;i<n;i++) {
        v[i] = rules[whichSpaceOwns[i]]->v[i];
    }
    w=new Warner(this);
    for(unsigned int i=0;i<n;i++) {
        switch (type_of_v[i]) {
            case VTYPE_INT:
                IntWarningProp::IntWarning(rules[whichSpaceOwns[i]],i,*(static_cast<IntVar*>(rules[whichSpaceOwns[i]]->v[i])),w);
                break;
            case VTYPE_BOOL:
                BoolWarningProp::BoolWarning(rules[whichSpaceOwns[i]],i,*(static_cast<BoolVar*>(rules[whichSpaceOwns[i]]->v[i])),w);
                break;
            default:
                abort();
        }
    } 
}


bool Implicative::quantification(int v) {
    return q[v];
}

// Returns the number of the first failed space (number of rule space, nbSpaces if goal, nbSpaces+1 if there is no failed space)
int Implicative::cascade(int firstSpace, unsigned long int& propsteps) {
    int curSpace = firstSpace;
    
    while(curSpace < nbSpaces) {
        MySpace* nextSpace = (curSpace==nbSpaces-1)?goal:rules[curSpace+1];
        bool* nextDefined  = (curSpace==nbSpaces-1)?&goalDefined:&(ruleDefined[curSpace+1]);
        
        if (!ruleDefined[curSpace]) {
            return curSpace;
        }
        if (rules[curSpace]->status(propsteps) == SS_FAILED) {
            return curSpace;
        }
        if (*nextDefined) {
            if (nextSpace->status(propsteps) != SS_FAILED) {
                for (int i=0;i<nbVarBySpace[curSpace];i++) {
                    switch (type_of_v[i]) {
                        case VTYPE_INT :
                        {
                            IntView dest(*(static_cast<IntVar*>(nextSpace->v[i])));
                            IntView src(*(static_cast<IntVar*>(rules[curSpace]->v[i])));
                            ViewRanges<IntView> rg(src);
                            if (dest.inter_r(nextSpace,rg) == ME_GEN_FAILED) {
                                nextSpace->fail();
                            };
                        }
                            break;
                        case VTYPE_BOOL :
                        {
                            BoolView bdest(*(static_cast<BoolVar*>(nextSpace->v[i])));
                            BoolView bsrc(*(static_cast<BoolVar*>(rules[curSpace]->v[i])));
                            ViewRanges<BoolView> brg(bsrc);
                            if (bdest.inter_r(nextSpace,brg) == ME_GEN_FAILED) {
                                nextSpace->fail();
                            };
                        }
                            break;
                    }
                }
            }
        }
        curSpace++;
    }
    
    if (!goalDefined) {
        return nbSpaces;
    }
    if (goal->status(propsteps) == SS_FAILED) {
        return nbSpaces;
    }
    
    return nbSpaces+1;
}


int Implicative::status(int var, unsigned long int& propsteps) {
    int currentSpace=whichSpaceOwns[var];
    currentSpace--;
    if (currentSpace<0) currentSpace=0;
    
    // First case : Strong propagation : ///////////////////////////////////////////
    // First, call cascade propagation. ////////////////////////////////////////////
    // We fail the spaces after the first failed one (so they are not //////////////
    // copied anymore)//////////////////////////////////////////////////////////////
    if (prop_power == 2) {                                                        //
        int firstFailed=cascade(currentSpace,propsteps);                          //
        for (int i=firstFailed;i<nbSpaces;i++) {                                  //
            if (ruleDefined[i]) rules[i]->fail();                                 //
        }                                                                         //
        if (firstFailed <= nbSpaces) {                                            //
            if (goalDefined)                                                      //
                goal->fail();                                                     //
        }                                                                         //
                                                                                  //
        if (firstFailed==currentSpace) {                                          //
            return (quantification(var)?1:0);                                     //
        }                                                                         //
                                                                                  //
        return -1;                                                                //
    }                                                                             //
    ////////////////////////////////////////////////////////////////////////////////
    
    else {
        cout<<"Propagation "<<prop_power<<" is not supported in this version of Qecode."<<endl;
        abort();
    }
}


int Implicative::finalStatus(unsigned long int& propsteps) {
    
    // Step 1 : we check every rule space in the order. If one of them is failed, //
    // we can answer. //////////////////////////////////////////////////////////////
    for (int i=0;i<nbSpaces;i++) {                                                //
        if (!ruleDefined[i])                                                      //
            return q[ nbVarBySpace[i]-1 ] ? 1 : 0;                                //
        if (rules[i]->status(propsteps) == SS_FAILED)                             //
            return q[ nbVarBySpace[i]-1 ] ? 1 : 0;                                //
    }                                                                             //
    ////////////////////////////////////////////////////////////////////////////////
    
    // Step 2 : if we are here, then the rules were not violated. So, we check /////
    // the goal. ///////////////////////////////////////////////////////////////////
    if (!goalDefined)                                                             //
        return 0;                                                                 //
    if (goal->status(propsteps) == SS_FAILED)                                     //
        return 0;                                                                 //
    ////////////////////////////////////////////////////////////////////////////////
    
    // Step 3 : if we are here, then the rules were respected, and the goal ////////
    // is also true. So, the whole problem is true. ////////////////////////////////
    return 1;                                                                     //
    ////////////////////////////////////////////////////////////////////////////////
}

bool Implicative::subsumed(int var) {
    
    int varSpace=whichSpaceOwns[var];
    if (!ruleDefined[varSpace])
        return true;
    
    switch (type_of_v[var]) {
        case VTYPE_INT : 
        {
            IntVar* zeVar = static_cast<IntVar*>(rules[varSpace]->v[var]);
            unsigned int varSize=zeVar->size();
            
            if (zeVar->size() < 2) {
                return true;
            }

            if (zeVar->degree() > 1) {
                return false;
            }        
                
            for (int i=varSpace+1;i<nbSpaces;i++) {
                if (!ruleDefined[i])
                    return true;
                if (static_cast<IntVar*>(rules[i]->v[var])->degree() > 0)
                    return false;
                if (static_cast<IntVar*>(rules[i]->v[var])->size() < varSize)
                    return false;
            }
                
            if (!goalDefined) {
                return true;
            }
            if (static_cast<IntVar*>(goal->v[var])->degree() >0) {
                return false;
            }
            if (static_cast<IntVar*>(goal->v[var])->size() < varSize) {
                return false;
            }
            //else
            return true;
        }
            break;
        case VTYPE_BOOL :
        {
            BoolVar* zeBVar = static_cast<BoolVar*>(rules[varSpace]->v[var]);
            unsigned int varBSize=zeBVar->size();
            
            if (zeBVar->size() < 2) {
                return true;
            }
                
            if (zeBVar->degree() > 1) {
                return false;
            }        
            
            for (int i=varSpace+1;i<nbSpaces;i++) {
                if (!ruleDefined[i])
                    return true;
                if (static_cast<BoolVar*>(rules[i]->v[var])->degree() > 0)
                    return false;
                if (static_cast<BoolVar*>(rules[i]->v[var])->size() < varBSize)
                    return false;
            }
                
            if (!goalDefined) {
                return true;
            }
            if (static_cast<BoolVar*>(goal->v[var])->degree() >0) {
                return false;
            }
            if (static_cast<BoolVar*>(goal->v[var])->size() < varBSize) {
                return false;
            }
            //else
            return true;
        }
            break;
        default :
            return false;
    }
}


Implicative* Implicative::clone() {
    Implicative* resultat=new Implicative(*this);
    return resultat;
}


void Implicative::assign_int(int var,int** vals,int nbVals) {
    if (type_of_v[var] != VTYPE_INT) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            post(s,*(static_cast<IntVar*>(s->v[var])) >= vals[0][0]);
            for (int j=1;j<nbVals;j++) {
                IntSet rg(vals[1][j-1]+1,vals[0][j]-1);
                myAntidom_int(s,*(static_cast<IntVar*>(s->v[var])),rg,ICL_DOM);
            }
            post(s,*(static_cast<IntVar*>(s->v[var])) <= vals[1][nbVals-1]);
        }
    }
}


void Implicative::remove_int(int var,int** vals,int nbVals) {
    if (type_of_v[var] != VTYPE_INT) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            for (int j=0;j<nbVals;j++) {
                IntSet rg(vals[0][j],vals[1][j]);
                myAntidom_int(s,*(static_cast<IntVar*>(s->v[var])),rg,ICL_DOM);
            }
        }
    }
}

void Implicative::assign_bool(int var,int** vals,int nbVals) {
    if (type_of_v[var] != VTYPE_BOOL) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            post(s,*(static_cast<BoolVar*>(s->v[var])) >= vals[0][0]);
            for (int j=1;j<nbVals;j++) {
                IntSet rg(vals[1][j-1]+1,vals[0][j]-1);
                myAntidom_bool(s,*(static_cast<BoolVar*>(s->v[var])),rg,ICL_DOM);
            }
            post(s,*(static_cast<BoolVar*>(s->v[var])) <= vals[1][nbVals-1]);
        }
    }
}


void Implicative::remove_bool(int var,int** vals,int nbVals) {
    if (type_of_v[var] != VTYPE_BOOL) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            for (int j=0;j<nbVals;j++) {
                IntSet rg(vals[0][j],vals[1][j]);
                myAntidom_bool(s,*(static_cast<BoolVar*>(s->v[var])),rg,ICL_DOM);
            }
        }
    }
}

void Implicative::assign_bool(int var,int b) {
    if (type_of_v[var] != VTYPE_INT) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            rel(s,*(static_cast<BoolVar*>(s->v[var])),IRT_EQ,b);
        }
    }
}

void Implicative::remove_bool(int var,int b) {
    if (type_of_v[var] != VTYPE_INT) {
        cout<<"Wrong variable type for assigning"<<endl;
        abort();
    }
    int varspace = whichSpaceOwns[var];
    for (int i=varspace;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            MySpace* s = rules[i];
            rel(s,*(static_cast<BoolVar*>(s->v[var])),IRT_NQ,b);
        }
    }
}


void Implicative::backtrack() {
    w->update(this);
}


void Implicative::indicateBranchingHeuristic(BranchingHeuristic* bh) {
    w->setBH(bh);
}

void Implicative::print() {
    for (int i=0;i<nv();i++)
        cout<<(q[i]?"A":"E");
    cout<<endl;
    for (int i=0;i<nbSpaces;i++) {
        if (ruleDefined[i]) {
            for (int j=0;j<nbVarBySpace[i];j++)
                cout<<rules[i]->v[j]<<" ";
            cout<<endl;
        }
        else cout<<"Undefined"<<endl;
    }
    if (goalDefined) {
        for (int j=0;j<nv();j++)
            cout<<goal->v[j]<<" ";
        cout<<endl;
    }
    else cout<<"Undefined"<<endl;
    cout<<endl<<endl;
}
