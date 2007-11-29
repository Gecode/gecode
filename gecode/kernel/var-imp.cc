/*
 *  CAUTION:
 *    This file has been automatically generated.
 *    Do not edit, edit the specification file instead.
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#include "gecode/kernel.hh"

#ifdef GECODE_HAVE_INT_VARS


namespace Gecode { namespace Int {



  /*
   * The variable processor for IntVarImpBase
   *
   */

  void
  IntVarImpBase::Processor::process(Space* home, VarBase* _x) {
    // Process modified variables
    Gecode::Variable<VTI_INT,PC_INT_DOM,IntMeDiff>* x = 
      static_cast<Gecode::Variable<VTI_INT,PC_INT_DOM,IntMeDiff>*>(_x);
    do {
      switch (x->modevent()) {
      case ME_INT_VAL:
        x->process(home);
        break;
      case ME_INT_BND:
        // Conditions: BND DOM 
        x->process(home,PC_INT_BND,PC_INT_DOM,ME_INT_BND);
        break;
      case ME_INT_DOM:
        // Conditions: DOM 
        x->process(home,PC_INT_DOM,PC_INT_DOM,ME_INT_DOM);
        break;
      default: GECODE_NEVER;
      }
      x = x->next();
    } while (x != NULL);
  }


  IntVarImpBase::Processor IntVarImpBase::p;


}}

// STATISTICS: int-var
#endif

#ifdef GECODE_HAVE_INT_VARS


namespace Gecode { namespace Int {



  /*
   * The variable processor for BoolVarImpBase
   *
   */

  void
  BoolVarImpBase::Processor::process(Space* home, VarBase* _x) {
    // Process modified variables
    Gecode::Variable<VTI_BOOL,PC_BOOL_VAL,BoolMeDiff>* x = 
      static_cast<Gecode::Variable<VTI_BOOL,PC_BOOL_VAL,BoolMeDiff>*>(_x);
    do {
      x->process(home); x = x->next();
    } while (x != NULL);
  }


  BoolVarImpBase::Processor BoolVarImpBase::p;


}}

// STATISTICS: int-var
#endif

#ifdef GECODE_HAVE_SET_VARS


namespace Gecode { namespace Set {


  /*
   * Modification event difference for Set-variable implementations
   *
   */
  const Gecode::ModEvent SetMeDiff::med[ME_SET_CBB+1][ME_SET_CBB+1] = {
    {
      ME_SET_NONE ^ ME_SET_NONE, // [ME_SET_NONE][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_NONE][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CARD, // [ME_SET_NONE][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_LUB, // [ME_SET_NONE][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_GLB, // [ME_SET_NONE][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_BB, // [ME_SET_NONE][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CLUB, // [ME_SET_NONE][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CGLB, // [ME_SET_NONE][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_NONE][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_VAL, // [ME_SET_VAL][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_VAL  // [ME_SET_VAL][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_CARD, // [ME_SET_CARD][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_CARD][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CARD, // [ME_SET_CARD][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_CLUB, // [ME_SET_CARD][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_CGLB, // [ME_SET_CARD][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_CBB, // [ME_SET_CARD][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CLUB, // [ME_SET_CARD][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CGLB, // [ME_SET_CARD][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_CARD][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_LUB, // [ME_SET_LUB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_LUB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CLUB, // [ME_SET_LUB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_LUB, // [ME_SET_LUB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_BB, // [ME_SET_LUB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_BB, // [ME_SET_LUB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CLUB, // [ME_SET_LUB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CBB, // [ME_SET_LUB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_LUB][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_GLB, // [ME_SET_GLB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_GLB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CGLB, // [ME_SET_GLB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_BB, // [ME_SET_GLB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_GLB, // [ME_SET_GLB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_BB, // [ME_SET_GLB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CBB, // [ME_SET_GLB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CGLB, // [ME_SET_GLB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_GLB][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_BB, // [ME_SET_BB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_BB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CBB, // [ME_SET_BB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_BB, // [ME_SET_BB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_BB, // [ME_SET_BB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_BB, // [ME_SET_BB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CBB, // [ME_SET_BB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CBB, // [ME_SET_BB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_BB][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_CLUB, // [ME_SET_CLUB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_CLUB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CLUB, // [ME_SET_CLUB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_CLUB, // [ME_SET_CLUB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_CBB, // [ME_SET_CLUB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_CBB, // [ME_SET_CLUB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CLUB, // [ME_SET_CLUB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CBB, // [ME_SET_CLUB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_CLUB][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_CGLB, // [ME_SET_CGLB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_CGLB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CGLB, // [ME_SET_CGLB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_CBB, // [ME_SET_CGLB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_CGLB, // [ME_SET_CGLB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_CBB, // [ME_SET_CGLB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CBB, // [ME_SET_CGLB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CGLB, // [ME_SET_CGLB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_CGLB][ME_SET_CBB]
    },
    {
      ME_SET_NONE ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_NONE]
      ME_SET_VAL ^ ME_SET_VAL, // [ME_SET_CBB][ME_SET_VAL]
      ME_SET_CARD ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_CARD]
      ME_SET_LUB ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_LUB]
      ME_SET_GLB ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_GLB]
      ME_SET_BB ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_BB]
      ME_SET_CLUB ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_CLUB]
      ME_SET_CGLB ^ ME_SET_CBB, // [ME_SET_CBB][ME_SET_CGLB]
      ME_SET_CBB ^ ME_SET_CBB  // [ME_SET_CBB][ME_SET_CBB]
    }
  };


  /*
   * The variable processor for SetVarImpBase
   *
   */

  void
  SetVarImpBase::Processor::process(Space* home, VarBase* _x) {
    // Process modified variables
    Gecode::Variable<VTI_SET,PC_SET_ANY,SetMeDiff>* x = 
      static_cast<Gecode::Variable<VTI_SET,PC_SET_ANY,SetMeDiff>*>(_x);
    do {
      switch (x->modevent()) {
      case ME_SET_VAL:
        x->process(home);
        break;
      case ME_SET_CARD:
        // Conditions: CARD CLUB CGLB ANY 
        x->process(home,PC_SET_CARD,PC_SET_ANY,ME_SET_CARD);
        break;
      case ME_SET_LUB:
        // Conditions: CLUB ANY 
        x->process(home,PC_SET_CLUB,PC_SET_CLUB,ME_SET_LUB);
        x->process(home,PC_SET_ANY,PC_SET_ANY,ME_SET_LUB);
        break;
      case ME_SET_GLB:
        // Conditions: CGLB ANY 
        x->process(home,PC_SET_CGLB,PC_SET_ANY,ME_SET_GLB);
        break;
      case ME_SET_BB:
        // Conditions: CLUB CGLB ANY 
        x->process(home,PC_SET_CLUB,PC_SET_ANY,ME_SET_BB);
        break;
      case ME_SET_CLUB:
        // Conditions: CARD CLUB CGLB ANY 
        x->process(home,PC_SET_CARD,PC_SET_ANY,ME_SET_CLUB);
        break;
      case ME_SET_CGLB:
        // Conditions: CARD CLUB CGLB ANY 
        x->process(home,PC_SET_CARD,PC_SET_ANY,ME_SET_CGLB);
        break;
      case ME_SET_CBB:
        // Conditions: CARD CLUB CGLB ANY 
        x->process(home,PC_SET_CARD,PC_SET_ANY,ME_SET_CBB);
        break;
      default: GECODE_NEVER;
      }
      x = x->next();
    } while (x != NULL);
  }


  SetVarImpBase::Processor SetVarImpBase::p;


}}

// STATISTICS: set-var
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS


namespace Gecode { namespace CpltSet {



  /*
   * The variable processor for CpltSetVarImpBase
   *
   */

  void
  CpltSetVarImpBase::Processor::process(Space* home, VarBase* _x) {
    // Process modified variables
    Gecode::Variable<VTI_CPLTSET,PC_CPLTSET_DOM,CpltSetMeDiff>* x = 
      static_cast<Gecode::Variable<VTI_CPLTSET,PC_CPLTSET_DOM,CpltSetMeDiff>*>(_x);
    do {
      switch (x->modevent()) {
      case ME_CPLTSET_VAL:
        x->process(home);
        break;
      case ME_CPLTSET_DOM:
        // Conditions: DOM 
        x->process(home,PC_CPLTSET_DOM,PC_CPLTSET_DOM,ME_CPLTSET_DOM);
        break;
      default: GECODE_NEVER;
      }
      x = x->next();
    } while (x != NULL);
  }


  CpltSetVarImpBase::Processor CpltSetVarImpBase::p;


}}

// STATISTICS: cpltset-var
#endif

