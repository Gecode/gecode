/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Martin Mann <mmann@informatik.uni-freiburg.de>
 *     Sebastian Will <will@informatik.uni-freiburg.de>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *     Martin Mann, 2008
 *     Sebastian Will, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <vector>

namespace Gecode {

  namespace Decomposition {
    /// \brief Wrapper class for views with index
    template <class View>
    class IdxView {
    public:
      /// The view
      View view;
      /// The index
      int idx;
      /// Default constructor
      IdxView(void);
      /// Construct with view \a v and index \a i
      IdxView(View v, int i);

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, IdxView<View>& x);
      //@}
      
      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}
      
    };
  }

  template <class View>
  forceinline
  Decomposition::IdxView<View>::IdxView(void) {}

  template <class View>
  forceinline
  Decomposition::IdxView<View>::IdxView(View v, int i) : view(v), idx(i) {}

  template <class View>
  void
  Decomposition::IdxView<View>::update(Space& home, bool share,
                                       Decomposition::IdxView<View>& v) {
    idx = v.idx;
    view.update(home, share, v.view);
  }

  template <class View>
  Reflection::Arg*
  Decomposition::IdxView<View>::spec(const Space& home,
                                     Reflection::VarMap& vm) const {
    return view.spec(home, vm);
  }

  template <class View>
  Support::Symbol
  Decomposition::IdxView<View>::type(void) {
    return View::type();
  }

  /**
   * \brief Branching for decomposition during search
   *
   * This branching analyzes the constraint graph and decomposes the search
   * if the graph contains more than one connected component.
   *
   * Otherwise, the branching behaves like a normal ViewValBranching.
   *
   */    
  template <class ViewSel, class ValSel>
  class DecomposingViewValBranching : public Branching {
  protected:
    /// Views to branch on, paired with index
    ViewArray<Decomposition::IdxView<typename ViewSel::View> > x;
    /// Unassigned views start at x[start]
    mutable int start;
    /// Flag that signals whether the previous commit performed decomposition
    mutable bool selectFirst;

    /// Constructor for cloning \a b
    DecomposingViewValBranching(Space& home, bool share,
                                DecomposingViewValBranching& b);

  public:
    /// Constructor for creation
    DecomposingViewValBranching(Space& home, 
                                ViewArray<typename ViewSel::View>& x);
    /// Check status of branching, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Return branching description
    virtual const BranchingDesc* description(const Space& home) const;
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space& home, const BranchingDesc* d,
                              unsigned int a);
    /// Perform cloning
    virtual Actor* copy(Space& home, bool share);
    
    /// Return specification for this branching given a variable map \a m
    virtual Reflection::ActorSpec spec(const Space& home,
                                       Reflection::VarMap& m) const;
    // /// Return specification for a branch
    virtual Reflection::BranchingSpec
    branchingSpec(const Space& home, 
                  Reflection::VarMap& m,
                  const BranchingDesc* d) const;
    /// Actor type identifier of this branching
    static Support::Symbol ati(void);
    /// Post branching according to specification
    static void post(Space& home, Reflection::VarMap& m,
                     const Reflection::ActorSpec& spec);
    
  };

  namespace Decomposition {
  
    /// \brief Partition of the constraint graph
    class Partition {
    protected:
      /// Elements of the partition
      int* elements;
      /// Start indices of the components
      int* separators;
      /// Number of components
      int _size;
    public:
      /// Construct empty partition
      Partition(void);
      /// Construct partition
      Partition(int noOfElements, int noOfComponents);
      /// Initialize partition
      GECODE_DDS_EXPORT void init(int noOfElements, int noOfComponents);
      /// Destructor
      GECODE_DDS_EXPORT ~Partition(void);
      /// Return element at position \a i
      const int& operator [](int i) const;
      /// Return element at position \a i
      int& operator [](int i);
      /// Return start index of component \a i
      int component(int i) const;
      /// Return start index of component \a i
      int& component(int i);
      /// Return number of components
      int size(void) const;
      /// Decrement size by 1
      void removeLastComponent(void);
    };
  
    /// \brief Branching description for decomposition
    class GECODE_VTABLE_EXPORT DecompDesc : public BranchingDesc {
    protected:
      /// The variable indices in the order of components
      int* element;
      /// Start index
      const int _start;
      /// Component start indices
      int* component;

    public:
      /// Constructor
      template <class View>
      DecompDesc(const Branching* b, int start,
                 const Partition& p,
                 const ViewArray<Decomposition::IdxView<View> >& x);
      /// Destructor
      GECODE_DDS_EXPORT ~DecompDesc(void);
      /// Return the stored label
      int operator [](int i) const;
      /// Return the start index for component \a alt
      int componentStart(unsigned int alt) const;
      /// Return the start index
      int start(void) const;
      /// Return the significant variables for alternative \a alt
      GECODE_DDS_EXPORT std::vector<int> significantVars(int alt) const;
      /// Return size
      GECODE_DDS_EXPORT virtual size_t size(void) const;
    };

    /// A branching description for singleton components
    class GECODE_VTABLE_EXPORT SingletonDescBase : public BranchingDesc {
    private:
      /// Index of the singleton view
      unsigned int _idx;
      /// Size of the domain of the singleton component
      unsigned int _size;
    public:
      /// Constructor
      SingletonDescBase(const Branching* b, unsigned int alt,
                        unsigned int idx, unsigned int size);
      /// Return index of the singleton view
      unsigned int idx(void) const;
      /// Return size of the domain of the singleton component
      unsigned int domainSize(void) const;
      /// Return size
      GECODE_DDS_EXPORT virtual size_t size(void) const;
    };

    /// Wrapper class for PosValDesc that stores an additional component size
    template <class Val, int alt>
    class GECODE_VTABLE_EXPORT SingletonDesc : public SingletonDescBase {
    private:
      PosValDesc<Val,alt>* _pvd;
    public:
      /// Constructor
      SingletonDesc(const Branching* b, Val v, int start,
                    unsigned int idx, unsigned int size);
      /// Destructor
      ~SingletonDesc(void);
      /// Acces embedded PosValDesc
      PosValDesc<Val,alt>* pvd(void) const;
      /// Return size
      virtual size_t size(void) const;
    };

    /// Compute connected components of the constraint graph
    GECODE_DDS_EXPORT void
    connectedComponents(const Space& home, Reflection::VarMap& vars,
                        Partition& p);

    forceinline
    Partition::Partition(void) : elements(NULL), separators(NULL) {}

    forceinline
    Partition::Partition(int noOfElements, int noOfComponents) {
      init(noOfElements, noOfComponents);
    }

    forceinline const int&
    Partition::operator [](int i) const {
      return elements[i];
    }
    forceinline int&
    Partition::operator [](int i) {
      return elements[i];
    }
    forceinline int
    Partition::component(int i) const {
      return separators[i];
    }
    forceinline int&
    Partition::component(int i) {
      return separators[i];
    }
    forceinline int
    Partition::size(void) const {
      return _size;
    }
    forceinline void
    Partition::removeLastComponent(void) {
      _size--;
    }
  
    forceinline
    unsigned int
    SingletonDescBase::domainSize(void) const { return _size; }

    forceinline
    unsigned int
    SingletonDescBase::idx(void) const { return _idx; }

    forceinline
    SingletonDescBase::SingletonDescBase(const Branching* b, 
                                         unsigned int alt,
                                         unsigned int idx,
                                         unsigned int size)
     : BranchingDesc(b, alt), _idx(idx), _size(size) {}

    template <class View>
    DecompDesc::DecompDesc(const Branching* b, int start,
                           const Partition& p,
                           const ViewArray<Decomposition::IdxView<View> >& x)
      : BranchingDesc(b,p.size()-1), _start(start) {
      element = heap.alloc<int>(x.size()*2);
      component = heap.alloc<int>(p.size());
      for (int i=x.size(); i--;) {
        element[i*2]   = p[i];
        element[i*2+1] = x[start+p[i]].idx;
      }
      for (int i=p.size(); i--;) {
        component[i] = p.component(i);
      }
    }

    forceinline
    int
    DecompDesc::start(void) const { return _start; }

    forceinline
    int
    DecompDesc::operator [](int i) const {
      return element[i*2];
    }
  
    forceinline
    int 
    DecompDesc::componentStart(unsigned int alt) const {
      return component[alt];
    }

    template <class Val, int alt>
    SingletonDesc<Val,alt>::SingletonDesc(const Branching* b, Val v,
                                          int start,
                                          unsigned int idx,
                                          unsigned int size)
      : SingletonDescBase(b,alt,idx,size),
        _pvd(new PosValDesc<Val,alt>(b, 0, start, v)) 
    {}
    
    template <class Val, int alt>
    SingletonDesc<Val,alt>::~SingletonDesc(void) {
      delete _pvd;
    }

    template <class Val, int alt>
    PosValDesc<Val,alt>*
    SingletonDesc<Val,alt>::pvd(void) const { return _pvd; }

    template <class Val, int alt>
    size_t
    SingletonDesc<Val,alt>::size(void) const {
      return sizeof(SingletonDesc<Val,alt>);
    }

  }

  /*
   * Generic branching based on variable/value selection
   *
   */

  template <class ViewSel, class ValSel>
  forceinline
  DecomposingViewValBranching<ViewSel,ValSel>
  ::DecomposingViewValBranching(Space& home, 
                                ViewArray<typename ViewSel::View>& x0)
    : Branching(home), x(home, x0.size()), start(0), selectFirst(false) {
    for (int i=x0.size(); i--;)
      x[i] = Decomposition::IdxView<typename ViewSel::View>(x0[i], i);
  }


  template <class ViewSel, class ValSel>
  forceinline
  DecomposingViewValBranching<ViewSel,ValSel>
  ::DecomposingViewValBranching(Space& home, bool share, 
                                DecomposingViewValBranching& b)
    : Branching(home,share,b), start(b.start), selectFirst(b.selectFirst) {
    x.update(home,share,b.x);
  }

  template <class ViewSel, class ValSel>
  Actor*
  DecomposingViewValBranching<ViewSel,ValSel>
  ::copy(Space& home, bool share) {
    return new (home)
      DecomposingViewValBranching<ViewSel,ValSel>(home,
        share,*this);
  }

  template <class ViewSel, class ValSel>
  bool
  DecomposingViewValBranching<ViewSel,ValSel>
  ::status(const Space&) const {
    for (int i=start; i < x.size(); i++)
      if (!x[i].view.assigned()) {
        start = i;
        return true;
      }
    return false;
  }

  template <class ViewSel, class ValSel>
  const BranchingDesc*
  DecomposingViewValBranching<ViewSel,ValSel>
  ::description(const Space& home) const {

    assert(!x[start].view.assigned());

    if (selectFirst && !x[0].view.assigned()) {
      selectFirst = false;
      ValSel  vl; // For value selection
      if (x.size() == 1) {
        return
          new Decomposition::SingletonDesc<typename ValSel::Val,ValSel::alternatives>(
            this,vl.val(home,x[0].view),
            0, x[0].idx, x[0].view.size());
      } else {
        return new PosValDesc<typename ValSel::Val,ValSel::alternatives>
          (this,0,0,vl.val(home,x[0].view));
      }
    }

    if (x.size()-start > 1) {

      // Compute connected components in the constraint graph
      Reflection::VarMap vm;
      typedef typename ViewVarImpTraits<typename ValSel::View>::VarImp VarImp;
      typedef typename VarImpVarTraits<VarImp>::Var Var;
      for (int i=start; i<x.size(); i++) {
        Var v(x[i].view);
        vm.put(home, v, "", false);
      }

      Decomposition::Partition p;
      Decomposition::connectedComponents(home, vm, p);

      if (p.size() > 2) {
        // do decomposing branching
        Decomposition::Partition outP(x.size()-start, p.size());

        unsigned int noOfElements = x.size()-start;
        int countSep = 0;
        int countElem = 0;
        outP.component(countSep++) = 0;

        ViewSel vs;
        while (noOfElements > 0) {
          // Find selected variable
          ViewSelStatus vss = vs.init(home,x[start+p[0]].view);
          int lastSelect = 0;
          unsigned int i = 0;
          while ((vss != VSS_BEST) && (i<noOfElements)) {
            if (!x[start+p[i]].view.assigned()) {
              vss = vs.select(home,x[start+p[i]].view);
              if (vss == VSS_BETTER)
                lastSelect = i;
            }
            i++;
          }
        
          // Find component of selected variable
          int selectedComponent = -1;
          for (int c=0; c < p.size()-1; c++) {
            if (lastSelect >= p.component(c) &&
                lastSelect < p.component(c+1)) {
              selectedComponent = c;
              break;
            }
          }
          assert(selectedComponent >= 0);
        
          // Copy component of selected variable to outP
          for (int i=p.component(selectedComponent); 
               i<p.component(selectedComponent+1); i++) {
            outP[countElem++] = p[i];
          }
          outP.component(countSep++) = countElem;

          // Remove selected component from p
          int next = p.component(selectedComponent);
          int nextSep = selectedComponent+1;
          for (int c=selectedComponent+1; c<p.size()-1; c++) {
            for (int i=p.component(c); i<p.component(c+1); i++)
              p[next++] = p[i];
            p.component(nextSep++) = next;
          }
          noOfElements = next;
          p.removeLastComponent();
        }

        // create decomposition description
        return new Decomposition::DecompDesc(this, start, outP, x);
      }

    } else if (x[start].view.degree() == 0) {
      assert(x.size() - start == 1);
      ValSel  vl; // For value selection
      return
        new Decomposition::SingletonDesc<typename ValSel::Val,ValSel::alternatives>(
          this,
          vl.val(home,x[start].view),
          start,
          x[start].idx,
          x[start].view.size());
    }

    // do normal branching

    ViewSel vs; // For view selection
    ValSel  vl; // For value selection
    int i = start;
    int b = i++;
    if (vs.init(home,x[b].view) != VSS_BEST)
      for (; i < x.size(); i++) {
        if (!x[i].view.assigned()) {
          switch (vs.select(home,x[i].view)) {
          case VSS_BETTER:
            b=i; break;
          case VSS_BEST:
            b=i;
            return new 
              PosValDesc<typename ValSel::Val,ValSel::alternatives>
              (this,b-start,start,vl.val(home,x[b].view));
          case VSS_TIE:   break;
          case VSS_WORSE:   break;
          default:         GECODE_NEVER;
            }
          }
       }
    return new PosValDesc<typename ValSel::Val,ValSel::alternatives>
      (this,b-start,start,vl.val(home,x[b].view));
  }

  template <class ViewSel, class ValSel>
  ExecStatus
  DecomposingViewValBranching<ViewSel,ValSel>
  ::commit(Space& home, const BranchingDesc* d, unsigned int a) {

    const Decomposition::DecompDesc* dd =
      dynamic_cast<const Decomposition::DecompDesc*>(d);
    if( dd == NULL) { // --> normal handling of PosValDescriptions
      
      const PosValDesc<typename ValSel::Val,ValSel::alternatives>* pvd;
      if (const Decomposition::SingletonDesc<typename ValSel::Val,ValSel::alternatives>* sd =
          dynamic_cast<const Decomposition::SingletonDesc<typename ValSel::Val,ValSel::alternatives>*>(d)) {
        pvd = sd->pvd();
      } else {
        pvd = static_cast<const PosValDesc<typename ValSel::Val,ValSel::alternatives>*>(d); 
      }
      // Eliminate views from x[0] ... x[start-1]
      x.drop_fst(pvd->start()); start = 0;
      ValSel vs;
      return me_failed(vs.tell(home,a,x[pvd->pos()].view,pvd->val()))
          ? ES_FAILED : ES_OK;
    } else {
      // Eliminate views from x[0] ... x[start-1]
      x.drop_fst(dd->start()); start = 0;
      
      // Collect selected component at beginning of x array
      int compSize = dd->componentStart(a+1)-dd->componentStart(a);
      Region r(home);
      Decomposition::IdxView<typename ViewSel::View>* tmp =
        r.alloc<Decomposition::IdxView<typename ViewSel::View> >(compSize);
      for (int i=compSize; i--; ) {
        tmp[i] = x[(*dd)[i+dd->componentStart(a)]];
      }
      for (int i=compSize; i--; ) {
        x[i] = tmp[i];
      }

      // Eliminate all views from x that are not in the selected component
      x.drop_lst(compSize-1);

      // No decomposition possible directly after decomposition
      selectFirst = true;

      return ES_OK;
    }
  }


  template <class ViewSel, class ValSel>
  Reflection::BranchingSpec
  DecomposingViewValBranching<ViewSel,ValSel>
  ::branchingSpec(const Space& home,
                  Reflection::VarMap& m,
                  const BranchingDesc* d) const {
    const PosValDesc<typename ValSel::Val,ValSel::alternatives>* pvd 
      = dynamic_cast<const PosValDesc<typename ValSel::Val,ValSel::alternatives>*>(d);
    if (pvd) {
      Reflection::BranchingSpec bs(3);
      bs[0] = x[pvd->start()+pvd->pos()].spec(home, m);
      bs[1] = Reflection::Arg::newString(ValSel::type());
      bs[2] = Reflection::Arg::newInt(pvd->val());
      return bs;
    } else {
      const Decomposition::DecompDesc* dd =
        dynamic_cast<const Decomposition::DecompDesc*>(d);
      assert(dd);
      Reflection::BranchingSpec bs(dd->alternatives());
      for (unsigned int i=0; i<dd->alternatives(); i++) {
        int compSize = dd->componentStart(i+1)-dd->componentStart(i);
        Reflection::ArrayArg* arg = Reflection::Arg::newArray(compSize);
        int k=0;
        for (int j=0; j<compSize; j++) {
          (*arg)[k++] = x[(*dd)[j+dd->componentStart(i)]].spec(home, m);
        }
        bs[i] = arg;
      }
      return bs;
    }
  }

  template <class ViewSel, class ValSel>
  Support::Symbol
  DecomposingViewValBranching<ViewSel,ValSel>::ati(void) {
    return Reflection::mangle<ViewSel,ValSel>
      ("Gecode::DecomposingViewValBranching");
  }

  template <class ViewSel, class ValSel>
  void
  DecomposingViewValBranching<ViewSel,ValSel>
    ::post(Space& home, Reflection::VarMap& vars,
           const Reflection::ActorSpec& spec) {
    spec.checkArity(1);
    ViewArray<Decomposition::IdxView<typename ViewSel::View> > 
      x(home, vars, spec[0]);
    (void) new (home) 
      DecomposingViewValBranching<ViewSel,ValSel>(home, x);
  }

  template <class ViewSel, class ValSel>
  Reflection::ActorSpec
  DecomposingViewValBranching<ViewSel,ValSel>
  ::spec(const Space& home, Reflection::VarMap& m) const {
    Reflection::ActorSpec s(ati());
    return s << x.spec(home, m);
  }

}

// STATISTICS: dds-any
