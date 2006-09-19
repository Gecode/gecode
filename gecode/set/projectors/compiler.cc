/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-04-11 15:58:37 +0200 (Tue, 11 Apr 2006) $ by $Author: tack $
 *     $Revision: 3188 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set/projectors-compiler.hh"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace Gecode {

  ProjectorCompiler::Indent::Indent(void) : i(0) {}

  void
  ProjectorCompiler::Indent::operator++(void) { i+=2; }
  void
  ProjectorCompiler::Indent::operator--(void) { i-=2; }

  std::ostream&
  operator<<(std::ostream& os, const ProjectorCompiler::Indent& indent) {
    for (int i=0; i<indent.i; i++)
      os << " ";
    return os;
  }
  
  bool
  ProjectorCompiler::nary() {
    return spec._arity <= 0 || spec._arity >= 4;
  }

  string
  ProjectorCompiler::propcost() {
    switch (spec._arity) {
    case 1:  return "PC_UNARY_HI";
    case 2:  return "PC_BINARY_HI";
    case 3:  return "PC_TERNARY_HI";
    default: return "PC_LINEAR_HI";
    }
  }

  void
  ProjectorCompiler::header(ostream& os) {
    os
      << "// This file was generated from projector specifications." << endl
      << endl;

    if (spec._namespace != "") {
      os << indent << spec._namespace << endl;
      ++indent;
    }
  }

  void
  ProjectorCompiler::footer(ostream& os) {
    if (spec._namespace != "") {
      --indent;
      os << indent << "}";
    }
    os << endl;
  }

  void
  ProjectorCompiler::viewsarglist(ostream& os) {
    if (nary()) {
      os << "ViewArray<View>& x";
    } else {
      for (int i=0; i<spec._arity; i++) {
	if (views==SINGLE_VIEW)
	  os << "View";
	else
	  os << "View" << i;
	os << " x" << i;
	if (i<spec._arity-1)
	  os << ", ";
      }
    }
    if (spec._reified)
      os << ", Gecode::Int::BoolView b";
  }

  void
  ProjectorCompiler::initarglist(ostream& os) {
    if (nary()) {
      os << "_x(x)";
    } else {
      for (int i=0; i<spec._arity; i++) {
	os << "_x"<< i << "("
	   << "x" << i << ")";
	if (i<spec._arity-1)
	  os <<", ";
      }
    }
    if (spec._reified)
      os << ", _b(b)";
  }

  void ProjectorCompiler::templateparams(void) {
    iccos << "<";
    if (nary() || views==SINGLE_VIEW)
      iccos << "View";
    else
      for (int i=0; i<spec._arity; i++) {
	iccos << "View" << i;
	if (i!=spec._arity-1)
	  hhos << ", ";
      }
    iccos << ">";
  }

  void ProjectorCompiler::templatehead(ostream& os) {
    if (nary() || views==SINGLE_VIEW)
      os << indent << "template <class View>" << endl;
    else {
      os << indent << "template <";
      for (int i=0; i<spec._arity; i++) {
	os << "class View" << i;
	if (i!=spec._arity-1)
	  os << ", ";
      }
      os << ">" << endl;
    }
  }

  void
  ProjectorCompiler::classdef() {
    templatehead(hhos);
    hhos << indent
	 << "class " << spec._name << " : public Propagator { " << endl
	 << indent << "protected:" << endl;
      
    if (nary())
      hhos << indent
	   << "  ViewArray<View> _x; ///< The view array" << endl;
    else
      for (int i=0; i<spec._arity; i++) {
	hhos << indent << "  ";
	if (views==SINGLE_VIEW)
	  hhos << "View";
	else
	  hhos << "View" << i;
	hhos << " _x" << i << "; ///< View #" << i << endl;
      }

    if (spec._reified)
      hhos << indent
	   << "  Gecode::Int::BoolView _b; ///< Boolean view for reification"
	   << endl;

    hhos << indent
	 << "  /// Constructor for cloning" << endl << indent
	 << "  " << spec._name << "(Space* home,bool,"
	 << spec._name << "&);" << endl;

    hhos << indent
	 << "  /// Constructor for creation" << endl << indent
	 << "  " << spec._name << "(Space* home, ";
    viewsarglist(hhos);
    hhos
      << ");" << endl;

    hhos << endl;
    hhos << indent << "  /// Propagate non-negated version" << endl;
    hhos << indent << "  ExecStatus propagatePositive(Space* home);"
         << endl;

   if (spec._reified || spec._negated) {
      hhos << indent << "  /// Check if the constraint holds" << endl;
      hhos << indent << "  ExecStatus check(Space* home);" << endl;
      hhos << indent << "  /// Propagate negated version" << endl;
      hhos << indent << "  ExecStatus propagateNegative(Space* home);"
           << endl << endl;	
    }
     
    hhos << indent
	 << "public:" << endl;

    hhos << indent
	 << "  /// Cost function (defined as " << propcost() << ")" 
	 << endl << indent
	 << "  virtual PropCost cost(void) const;" << endl;

    hhos << indent
	 << "  /// Delete propagator and return its size"
	 << endl << indent
	 << "  virtual size_t dispose(Space* home);" << endl;

    hhos << indent
	 << "  /// Copy propagator during cloning" << endl << indent
	 << "  virtual Actor*      copy(Space* home,bool);" << endl
	 << endl << indent
	 << "  /// Perform propagation" << endl << indent
	 << "  virtual ExecStatus propagate(Space* home);" << endl
	 << endl << indent
	 << "  /// Post projection propagator" << endl << indent
	 << "  static  ExecStatus post(Space* home, ";
    viewsarglist(hhos);
    hhos
      << ");" << endl;

    hhos << indent	<< "};" << endl;

  }

  string
  ProjectorCompiler::propcond(PropCond pc) {
    if (spec._reified || spec._negated) {
      switch (pc) {
      case Gecode::Set::PC_SET_VAL : return "Gecode::Set::PC_SET_VAL";
      case Gecode::Set::PC_SET_CARD : return "Gecode::Set::PC_SET_CARD";
      case Gecode::Set::PC_SET_CGLB :
      case Gecode::Set::PC_SET_CLUB :
      case Gecode::Set::PC_SET_ANY : return "Gecode::Set::PC_SET_ANY";
      default: GECODE_NEVER;
      }	
    }

    switch (pc) {
    case Gecode::Set::PC_SET_VAL : return "Gecode::Set::PC_SET_VAL";
    case Gecode::Set::PC_SET_CARD : return "Gecode::Set::PC_SET_CARD";
    case Gecode::Set::PC_SET_CGLB : return "Gecode::Set::PC_SET_CGLB";
    case Gecode::Set::PC_SET_CLUB : return "Gecode::Set::PC_SET_CLUB";
    case Gecode::Set::PC_SET_ANY : return "Gecode::Set::PC_SET_ANY";
    default:
      /// TODO: Proper error handling
      cerr << "Unknown PropCond" << endl;
      exit(2);
    }
  }

  void
  ProjectorCompiler::standardMemberFunctions() {

    ///////////////////////////////////////////////////
    // Constructor
    templatehead(iccos);
    iccos << indent << spec._name;
    templateparams();
    iccos << "::" << spec._name << endl << indent
	  << "(Space* home, ";
    viewsarglist(iccos);
    iccos << ")" << endl;

    ++indent;

    iccos << indent
	 << ": Propagator(home), ";
    initarglist(iccos);
    iccos << " {" << endl;
      
    int projArity = spec._ps.arity();
    Support::DynamicArray<int> scope;
    spec._ps.scope(scope);
    bool isNary = nary();

    for (int i=0; i<=projArity; i++) {
      iccos << indent << "_x";
      if (isNary) iccos << "[";
      iccos << i;
      if (isNary) iccos << "]";
      iccos << ".subscribe(home, this, " << propcond(scope[i])
	   << ");" << endl;
    }
      
    if (spec._reified)
      iccos << indent << "_b.subscribe(home, this, Gecode::Int::PC_INT_VAL);"
	    << endl;

    --indent;
    iccos << indent << "}" << endl;
    ///////////////////////////////////////////////////

    iccos << endl;

    ///////////////////////////////////////////////////
    // Copy constructor
    templatehead(iccos);
    iccos << indent
	 << spec._name;
    templateparams();
    iccos << "::" << spec._name << endl << indent
	 << "(Space* home, bool share, " << spec._name << "& p)" << endl;

    ++indent;

    iccos << indent
	 << ": Propagator(home, share, p) {" << endl;

    if (isNary) {
      iccos << indent << "_x.update(home, share, p._x);" << endl;
    } else {
      for (int i=0; i<spec._arity; i++)
	iccos << indent
	     << "_x" << i << ".update(home, share, p._x" << i << ");" << endl;
    }
      
    if (spec._reified)
      iccos << indent << "_b.update(home, share, p._b);" << endl;

    --indent;
    iccos << indent << "}" << endl;
    ///////////////////////////////////////////////////


    ///////////////////////////////////////////////////
    // Dispose function
    iccos << endl;
    templatehead(iccos);
    iccos << indent << "size_t" << endl;
    iccos << indent << spec._name;
    templateparams();
    iccos << "::dispose(Space* home) {" << endl;
    ++indent;
    iccos << indent << "if (!home->failed()) {" << endl;
    ++indent;

    for (int i=0; i<=projArity; i++) {
      iccos << indent << "_x";
      if (isNary) iccos << "[";
      iccos << i;
      if (isNary) iccos << "]";
      iccos << ".cancel(home, this, " << propcond(scope[i])
	   << ");" << endl;
    }
      
    if (spec._reified)
      iccos << indent << "_b.cancel(home, this, Gecode::Int::PC_INT_VAL);"
	    << endl;

    --indent;
    iccos << indent << "}" << endl;

    iccos << indent << "(void) Propagator::dispose(home);" << endl
	 << indent << "return sizeof(*this);" << endl;      

    --indent;
    iccos << indent << "}" << endl << endl;
    ///////////////////////////////////////////////////


    ///////////////////////////////////////////////////
    // Copy function
    templatehead(iccos);
    iccos << indent
	  << "Actor*" << endl << indent
	  << spec._name;
    templateparams();
    iccos << "::copy(Space* home, bool share) {"
	 << endl << indent
	 << "  return new (home) " << spec._name << "(home,share,*this);"
	 << endl << indent << "}" << endl << endl;
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    // Post member function
    templatehead(iccos);
    iccos << indent
	  << "ExecStatus" << endl << indent
	  << spec._name;
    templateparams();
    iccos << "::post(Space* home, ";
    viewsarglist(iccos);
    iccos << ") {" << endl << indent
	 << "  (void) new (home) " << spec._name << "(home, ";
    if (isNary) {
      iccos << "x";
    } else {
      for (int i=0; i<spec._arity; i++) {
	iccos << "x" << i;
	if (i<spec._arity-1)
	  iccos << ", ";
      }
    }
    if (spec._reified)
      iccos << ", b";
    iccos << ");" << endl
	 << indent << "  return ES_OK;" << endl
	 << indent << "}" << endl;
    ///////////////////////////////////////////////////


    ///////////////////////////////////////////////////
    // Cost member function
    iccos << endl;
    templatehead(iccos);
    iccos << indent
	  << "PropCost" << endl << indent
	  << spec._name;
    templateparams();
    iccos << "::cost(void) const {" << endl << indent
	  << "  return " << propcost() << ";" << endl << indent
	  << "}" << endl;
    ///////////////////////////////////////////////////

  }

  int
  ProjectorCompiler::iterator(const SetExprCode& instrs,
                              bool countSize, bool invert) {
    vector<string> typestack;
    vector<int> argstack;

    int arg = 0;

    for (int i=0; i<instrs.size(); i++) {
      switch (instrs[i]) {
      case SetExprCode::COMPLEMENT:
	{
	  string t = typestack.back(); typestack.pop_back();
	  int a = argstack.back(); argstack.pop_back();
	  iccos << indent
		<< "RangesCompl<" << t << " > i" << (++arg)
		<< "(i" << a << ");" << endl;
	  typestack.push_back("RangesCompl<"+t+" >");
	  argstack.push_back(arg);
	}
	break;
      case SetExprCode::INTER:
	{
	  string t2 = typestack.back(); typestack.pop_back();
	  string t1 = typestack.back(); typestack.pop_back();
	  int a2 = argstack.back(); argstack.pop_back();
	  int a1 = argstack.back(); argstack.pop_back();
	  iccos << indent
		<< "Iter::Ranges::Inter<" << t1 << ", " << t2 << " > i" 
		<< (++arg)
		<< "(i" << a1 << ", i" << a2 << ");" << endl;
	  typestack.push_back("Iter::Ranges::Inter<"+t1+", "+t2+" >");
	  argstack.push_back(arg);
	}
	break;
      case SetExprCode::UNION:
	{
	  string t2 = typestack.back(); typestack.pop_back();
	  string t1 = typestack.back(); typestack.pop_back();
	  int a2 = argstack.back(); argstack.pop_back();
	  int a1 = argstack.back(); argstack.pop_back();
	  iccos << indent
		<< "Iter::Ranges::Union<" << t1 << ", " << t2 << " > i" 
		<< (++arg)
		<< "(i" << a1 << ", i" << a2 << ");" << endl;
	  typestack.push_back("Iter::Ranges::Union<"+t1+", "+t2+" >");
	  argstack.push_back(arg);
	}
	break;
      case SetExprCode::GLB:
      case SetExprCode::LUB:
        {
          string bound;
          if (invert) {
	    if (instrs[i] == SetExprCode::GLB)
	      bound = "Lub";
	    else
	      bound = "Glb";
          } else {
	    if (instrs[i] == SetExprCode::GLB)
	      bound = "Glb";
	    else
	      bound = "Lub";	
          }

	  int a = argstack.back(); argstack.pop_back();
	  iccos << indent;
	  if (nary() || views==SINGLE_VIEW) {
	    iccos << bound << "Ranges<View> i";
	    string ty = bound + "Ranges<View>";
	    typestack.push_back(ty);
	  } else {
	    iccos << bound << "Ranges<View" << a << "> i";
	    stringstream s;
	    s << a;
	    string ty = bound + "Ranges<View";
	    ty += s.str();
	    ty += ">";
	    typestack.push_back(ty);
	  }
	  iccos << (++arg);
	  if (nary())
	    iccos << "(_x[" << a << "]);" << endl;
	  else
	    iccos << "(_x" << a << ");" << endl;
	  argstack.push_back(arg);
	}
	break;
      case SetExprCode::EMPTY : iccos << " e "; break;
      case SetExprCode::UNIVERSE : iccos << " u "; break;
      default:
	{
	  argstack.push_back((instrs[i])-SetExprCode::LAST);
	}
	break;
      }
    }

    if (countSize) {
      ++arg;
      iccos << indent << "Iter::Ranges::Size<" << typestack.back()
            << " > i" << arg << "(i" << (arg-1) << ");" << endl;
    }

    return arg;
  }

  void
  ProjectorCompiler::allAssigned(ostream& os) {
    os << indent << "bool assigned=true;" << endl;
    if (nary()) {
      os << indent << "for (int i=_x.size(); i--;)" << endl;
      os << indent << "  assigned = assigned && _x[i].assigned();" << endl;
    } else {
      for (int i=0; i<spec._arity; i++)
        os << indent << "assigned = assigned && _x" << i << ".assigned();"
           << endl;
    }
  }

  void
  ProjectorCompiler::propagation(void) {
    if (!spec._negated || spec._reified) {
      templatehead(iccos);
      iccos << indent << "ExecStatus" << endl << indent << spec._name;
      templateparams();
      iccos << "::propagatePositive(Space* home) {" << endl;
      ++indent;

      string me_check;
      switch (fixpoint) {
        case NO_FIX:
          {
            me_check="GECODE_ME_CHECK(";
            allAssigned(iccos);
          }
          break;
        case ITER_FIX:
          {
  	    iccos << indent << "bool modified=true;" << endl;
  	    iccos << indent << "while (modified) {" << endl;
  	    ++indent;
  	    iccos << indent << "modified = false;" << endl;
  	    me_check = "GECODE_ME_CHECK_MODIFIED(modified, ";
          }
          break;
        default: GECODE_NEVER;
      }

      for (int i=spec._ps.size(); i--; ) {
        SetExprCode glb = spec._ps[i].getGlb();
        if (glb.size() > 0 && glb[0] != SetExprCode::EMPTY) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(glb, false, true);
	  iccos << indent << me_check;
	  if (nary())
	    iccos << "_x[" << spec._ps[i].getIdx() << "]";
	  else
	    iccos << "_x" << spec._ps[i].getIdx();
	  iccos << ".includeI(home, i" << arg << "));" << endl;
	  --indent;
	  iccos << indent << "}" << endl;
        }
        SetExprCode lub = spec._ps[i].getLub();
        if (lub.size() > 0 && lub[0] != SetExprCode::UNIVERSE) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(lub);
	  iccos << indent << me_check;
	  if (nary())
	    iccos << "_x[" << spec._ps[i].getIdx() << "]";
	  else
	    iccos << "_x" << spec._ps[i].getIdx();
	  iccos << ".intersectI(home, i" << arg << "));" << endl;
	  --indent;
	  iccos << indent << "}" << endl;
        }
      }

      switch (fixpoint) {
        case NO_FIX:
          iccos << indent << "return assigned ? ES_SUBSUMED : ES_NOFIX;"
                << endl;
          break;
        case ITER_FIX:
          {
	    --indent;
	    iccos << indent << "}" << endl;
            allAssigned(iccos);
            iccos << indent << "return assigned ? ES_SUBSUMED : ES_FIX;"
                  << endl;
          }
          break;
        default: GECODE_NEVER;
      }

      --indent;
      iccos << indent << "}" << endl << endl;
    }

    if (spec._negated || spec._reified) {
      templatehead(iccos);
      iccos << indent << "ExecStatus" << endl << indent << spec._name;
      templateparams();
      iccos << "::check(Space* home) {" << endl;
      ++indent;
      iccos << indent << "ExecStatus es = ES_SUBSUMED;" << endl;
      for (int i=spec._ps.size(); i--; ) {
        SetExprCode glb = spec._ps[i].getGlb();
        if (glb.size() > 0 && glb[0] != SetExprCode::EMPTY) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(glb,true,true);

          iccos << indent;
	  if (nary())
	    iccos << "LubRanges<View> j(_x[" << spec._ps[i].getIdx() << "]);";
	  else if (views==SINGLE_VIEW)
	    iccos << "LubRanges<View> j(_x" << spec._ps[i].getIdx() << ");";
	  else
	    iccos << "LubRanges<View" << spec._ps[i].getIdx() << "> j(x"
	          << spec._ps[i].getIdx() << ");";
          iccos << endl;
          iccos << indent
                << "if (!Iter::Ranges::subset(i" << arg
                << ",j)) return ES_FAILED;"
                << endl;
          iccos << indent << "while (i" << arg << "()) ++i" << arg << ";"
                << endl;
          if (nary())
            iccos << indent << "if (i" << arg << ".size() > _x[" << i << "].cardMax())";
          else
            iccos << indent << "if (i" << arg << ".size() > _x" << i << ".cardMax())";
          iccos << endl;
	  iccos << indent << "  return ES_FAILED;" << endl;
	  --indent;
	  iccos << indent << "}" << endl;
        }
        SetExprCode lub = spec._ps[i].getLub();
        if (lub.size() > 0 && lub[0] != SetExprCode::UNIVERSE) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(lub,true);

          iccos << indent;
	  if (nary())
	    iccos << "GlbRanges<View> j(_x[" << spec._ps[i].getIdx() << "]);";
	  else if (views==SINGLE_VIEW)
	    iccos << "GlbRanges<View> j(_x" << spec._ps[i].getIdx() << ");";
	  else
	    iccos << "GlbRanges<View" << spec._ps[i].getIdx() << "> j(x"
	          << spec._ps[i].getIdx() << ");";
          iccos << endl;
          iccos << indent
                << "if (!Iter::Ranges::subset(j,i" << arg
                << ")) return ES_FAILED;"
                << endl;
          iccos << indent << "while (i" << arg << "()) ++i" << arg << ";"
                << endl;
          if (nary())
            iccos << indent << "if (i" << arg << ".size() < _x[" << i << "].cardMin())";
          else
            iccos << indent << "if (i" << arg << ".size() < _x" << i << ".cardMin())";
          iccos << endl;

	  iccos << indent << "  return ES_FAILED;" << endl;
	  --indent;
	  iccos << indent << "}" << endl;
        }
        if (glb.size() > 0 && glb[0] != SetExprCode::EMPTY) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(glb,false,false);

          iccos << indent;
	  if (nary())
	    iccos << "GlbRanges<View> j(_x[" << spec._ps[i].getIdx() << "]);";
	  else if (views==SINGLE_VIEW)
	    iccos << "GlbRanges<View> j(_x" << spec._ps[i].getIdx() << ");";
	  else
	    iccos << "GlbRanges<View" << spec._ps[i].getIdx() << "> j(_x"
	          << spec._ps[i].getIdx() << ");";
          iccos << endl;
          iccos << indent
                << "if (!Iter::Ranges::subset(i" << arg
                << ",j)) es = ES_FIX;"
                << endl;	  
	  --indent;
	  iccos << indent << "}" << endl;
        }
        if (lub.size() > 0 && lub[0] != SetExprCode::UNIVERSE) {
  	  iccos << indent << "{" << endl;
	  ++indent;
	  int arg = iterator(lub,false,true);

          iccos << indent;
	  if (nary())
	    iccos << "LubRanges<View> j(_x[" << spec._ps[i].getIdx() << "]);";
	  else if (views==SINGLE_VIEW)
	    iccos << "LubRanges<View> j(_x" << spec._ps[i].getIdx() << ");";
	  else
	    iccos << "LubRanges<View" << spec._ps[i].getIdx() << "> j(_x"
	          << spec._ps[i].getIdx() << ");";
          iccos << endl;
          iccos << indent
                << "if (!Iter::Ranges::subset(j,i" << arg
                << ")) es = ES_FIX;"
                << endl;	  
	  --indent;
	  iccos << indent << "}" << endl;
        }
      }
      iccos << indent << "return es;" << endl;
      --indent;
      iccos << indent << "}" << endl << endl;
	
      templatehead(iccos);
      iccos << indent << "ExecStatus" << endl << indent << spec._name;
      templateparams();
      iccos << "::propagateNegative(Space* home) {" << endl;
      ++indent;
      iccos << indent << "switch (ExecStatus es=check(home)) {" << endl;
      iccos << indent << "case ES_FAILED: return ES_SUBSUMED;" << endl;
      iccos << indent << "case ES_SUBSUMED: return ES_FAILED;" << endl;
      iccos << indent << "default: return es;" << endl;
      iccos << indent << "}" << endl;
      --indent;
      iccos << indent << "}" << endl << endl;
    }

    templatehead(iccos);
    iccos << indent << "ExecStatus" << endl << indent << spec._name;
    templateparams();
    iccos << "::propagate(Space* home) {" << endl;
    ++indent;

    if (spec._negated && ! spec._reified) {
      iccos << indent << "return propagateNegative(home);" << endl;
      --indent;
      iccos << indent << "}" << endl;
      return;	
    }

    if (spec._reified) {
      if (spec._negated) {
      	iccos << indent << "if (_b.zero()) return propagatePositive(home);"
	      << endl;
	iccos << indent << "if (_b.one()) return propagateNegative(home);"
	      << endl;
	iccos << indent << "switch (check(home)) {" << endl;
	iccos << indent << "case ES_SUBSUMED:" << endl;
	iccos << indent << "  _b.t_zero_none(home);" << endl;
	iccos << indent << "  return ES_SUBSUMED;" << endl;
	iccos << indent << "case ES_FAILED:" << endl;
	iccos << indent << "  _b.t_one_none(home);" << endl;
	iccos << indent << "  return ES_SUBSUMED;" << endl;
	iccos << indent << "default:" << endl;
	iccos << indent << "  return ES_FIX;" << endl;
	iccos << indent << "}" << endl;
	--indent;
	iccos << indent << "}" << endl;	
      } else {
      	iccos << indent << "if (_b.one()) return propagatePositive(home);"
	      << endl;
	iccos << indent << "if (_b.zero()) return propagateNegative(home);"
	      << endl;
	iccos << indent << "switch (check(home)) {" << endl;
	iccos << indent << "case ES_SUBSUMED:" << endl;
	iccos << indent << "  _b.t_one_none(home);" << endl;
	iccos << indent << "  return ES_SUBSUMED;" << endl;
	iccos << indent << "case ES_FAILED:" << endl;
	iccos << indent << "  _b.t_zero_none(home);" << endl;
	iccos << indent << "  return ES_SUBSUMED;" << endl;
	iccos << indent << "default:" << endl;
	iccos << indent << "  return ES_FIX;" << endl;
	iccos << indent << "}" << endl;
	--indent;
	iccos << indent << "}" << endl;
      }
      return;	
    }

    iccos << indent << "return propagatePositive(home);" << endl;
    --indent;
    iccos << indent << "}" << endl;
    return;	

  }

  ProjectorCompiler::ProjectorCompiler(ostream& _hhos,
				       ostream& _iccos,
				       const ProjectorPropagatorSpec& _spec)
    : hhos(_hhos), iccos(_iccos), spec(_spec),
      compiletest(false),
      fixpoint(NO_FIX),
      views(SINGLE_VIEW)
  {}

  void 
  ProjectorCompiler::compile(void) {

    if (spec._arity < spec._ps.arity()) {
      /// TODO: Add proper error handling
      cerr << "Arity mismatch\n";
      exit(2);
    }

    header(hhos);
    classdef();
    footer(iccos);

    hhos << endl;

    header(iccos);
    standardMemberFunctions();
    iccos << endl;
    propagation();
    footer(iccos);


  }

}

// STATISTICS: set-prop
