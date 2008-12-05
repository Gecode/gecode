/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
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

namespace Gecode { 

  namespace CpltSet {

    /**
     * \defgroup TaskActorCpltSetView CpltSet views
     *
     * CpltSet propagators and branchings compute with CpltSet views.
     * CpltSet views provide views on CpltSet variable implementations and 
     * integer variable implementations.
     * \ingroup TaskActorCpltSet
     */

    /**
     * \brief %CpltSet view for %CpltSet variables
     * \ingroup TaskActorCpltSetView
     */
    class CpltSetView : public VarViewBase<CpltSetVarImp> {
    protected:
      using VarViewBase<CpltSetVarImp>::varimp;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      CpltSetView(void);
      /// Initialize from set variable \a x
      CpltSetView(const CpltSetVar& x);
      /// Initialize from set variable \a x
      CpltSetView(CpltSetVarImp* x);
      //@}

      /// \name Value access
      //@{
      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;
      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}

      /// \name Bdd information 
      //@{
      /// Return the initial minimum of the least upper bound
      int initialLubMin(void) const;
      /// Return the initial maximum of the least upper bound
      int initialLubMax(void) const;
      /** \brief Return the number of bdd variables allocated for this 
        * variable (initialLubMax-initialLubMin) */
      unsigned int tableWidth(void) const;
      /** \brief Return the offset in the global bdd table where the 
        * variable's domain starts */
      unsigned int offset(void) const;
      /** \brief Return bdd for the \a i -th element of this variable 
        * (counting from initialLubMin) */
      bdd element(int i) const;
      /** \brief Return negated bdd for the \a i -th element of this variable 
        * (counting from initialLubMin) */
      bdd elementNeg(int i) const;
      /// Return the bdd representing the current domain
      bdd dom(void) const;
      //@}


      /// \name Domain update by value
      //@{
      /** 
       * \brief Restrict cardinality to be greater than or equal to \a l
       *        and to be less than or equal to \a u.
       */
      ModEvent cardinality(Space& home, int l, int u);
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);

      /// Include all elements in range \f$[a..b]\f$ in greatest lower bound
      ModEvent include(Space& home, int i,int j);
      /// Include elements \a i in greatest lower bound
      ModEvent include(Space& home, int i); 

      /// Exclude all elements in range \f$[a..b]\f$ from least upper bound 
      ModEvent exclude(Space& home, int i,int j);
      /// Exclude element \a i from least upper bound 
      ModEvent exclude(Space& home, int i);

      /// Restrict domain values to be different from singleton set \f$ \{v\} \f$
      ModEvent nq(Space& home, int v);
      /// Restrict domain values to be different from set \f$ [a,b] \f$
      ModEvent nq(Space& home, int a, int b);

      /// Restrict domain to be equal to the singleton set \f$ \{v\} \f$
      ModEvent eq(Space& home, int v);
      /// Restrict domain to be equal to the set \f$ [a,b] \f$
      ModEvent eq(Space& home, int a, int b);
       
      /// Restrict least upper bound to contain at most all elements in the range \f$ [a,b] \f$
      ModEvent intersect(Space& home,int i,int j);
      /// Restrict least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);

      /// Restrict domain values to be a solution to the logical formula described by the bdd \a d
      ModEvent intersect(Space& home, bdd& d);   
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include set described by range list \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);  
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
      /// Restrict domain to be equal to the set described by the range sequence \a i
      template <class I> ModEvent eqI(Space& home, I& i);
      /// Restrict domain values to be different from set described by the range sequence \a i
      template <class I> ModEvent nqI(Space& home, I& i);
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, CpltSetView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      //@}
    };

    /**
     * \brief Print set variable view
     * \relates Gecode::CpltSet::CpltSetView
     */
    GECODE_CPLTSET_EXPORT std::ostream&
    operator <<(std::ostream&, const Gecode::CpltSet::CpltSetView& x);
  }


  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for CpltSetView.
   * \ingroup TaskActorCpltSetView
   */
  template<>
  class ViewVarImpTraits<CpltSet::CpltSetView> {
  public:
    /// The variable type of a CpltSetView
    typedef CpltSet::CpltSetVarImp VarImp;
  };

}

#include <gecode/cpltset/var/cpltset.hpp>
#include <gecode/cpltset/view/cpltset.hpp>

// STATISTICS: cpltset-var
