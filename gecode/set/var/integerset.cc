/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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



#include "gecode/set.hh"

namespace Gecode { namespace Set {

  BndSet::BndSet(Space* home, const IntSet& is)  {
    if (is.size()==0) {
      fst(NULL); lst(NULL); _size = 0;
    } else {
      int n = is.size();
      RangeList* r = static_cast<RangeList*>(home->alloc(sizeof(RangeList)*n));
      fst(r); lst(r+n-1);
      unsigned int s = 0;
      for (int i = n; i--; ) {
        s += is.width(i);
        r[i].min(is.min(i)); r[i].max(is.max(i));
        r[i].prevnext(&r[i-1],&r[i+1]);
      }
      r[0].prev(&r[-1],NULL); 
      r[n-1].next(&r[n],NULL);
      _size = s;
    }
    assert(isConsistent());
  }

  bool
  GLBndSet::include_full(Space* home, int mi, int ma, SetDelta& d) {
    assert(ma >= mi);
    assert(fst() != NULL);
    
    RangeList* p = NULL;
    RangeList* c = fst();
    while (c != NULL) {
      if (c->max() >= mi-1){
        if (c->min() > ma+1) {  //in a hole before c
          _size+=(ma-mi+1);
          d._glbMin = mi;
          d._glbMax = ma;
          RangeList* q=new (home) RangeList(mi,ma,NULL,c);
          if (p==NULL){ //the new range is the first
            assert(c==fst());
            c->prev(NULL,q);
            fst(q);
            return true;
          }
          q->prev(NULL, p);
          p->next(c, q);
          c->prev(p, q);
          return true;
        }
        //if the range starts before c, update c->min and size
        bool result = false;
        if (c->min()>mi) {
          _size+=(c->min()-mi);
          c->min(mi);
          d._glbMin = mi;
          result = true;
        } else {
          d._glbMin = c->max()+1;
        }
        assert(c->min()<=mi);
        assert(c->max()+1>=mi);
        if (c->max() >= ma) { 
          d._glbMax = c->min()-1;
          return result;
        }
        RangeList* qp = p;
        RangeList* q = c;
        //sum up the size of holes eaten
        int prevMax = c->max();
        int growth =0;
        // invariant: q->min()<=ma+1
        while (q->next(qp)!=NULL && q->next(qp)->min()<=ma+1){
          RangeList* nq = q->next(qp);
          qp = q; q = nq;
          growth+=q->min()-prevMax-1;
          prevMax=q->max();
        }
        assert(growth>=0);
        _size+=growth;
        if (q->max() < ma) {
          _size+=ma-q->max();
          d._glbMax = ma;
        } else {
          d._glbMax = q->min()-1;          
        }
        c->max(std::max(ma,q->max()));
        if (c!=q) {
          RangeList* oldCNext = c->next(p);
          assert(oldCNext!=NULL); //q would have stayed c if c was last.
          c->next(oldCNext, q->next(qp));
          if (q->next(qp)==NULL){
            assert(q==lst());
            lst(c);
          } else {
            q->next(qp)->prev(q,c);
          }
          oldCNext->dispose(home,c,q);
        }
        return true;
      }
      RangeList* nc = c->next(p);
      p=c; c=nc;
    }
    //the new range is disjoint from the old domain and we add it as last:
    assert(mi>max()+1);
    RangeList* q = new (home) RangeList(mi, ma, lst(), NULL);
    lst()->next(NULL, q);
    lst(q);
    _size+= q->width();
    d._glbMin = mi;
    d._glbMax = ma;
    return true;
  }

  bool
  LUBndSet::intersect_full(Space* home, int mi, int ma) {
    RangeList* p = NULL;
    RangeList* c = fst();

    assert(c != NULL); // Never intersect with an empty set

    // Skip ranges that are before mi
    while (c != NULL && c->max() < mi) {
      _size -= c->width();
      RangeList *nc = c->next(p);
      p=c; c=nc;
    }
    if (c == NULL) {
      // Delete entire domain
      fst()->dispose(home, NULL, lst());
      fst(NULL); lst(NULL);
      return true;
    }

    bool changed = false;
    if (c != fst()) {
      c->prev(p, NULL);
      fst()->dispose(home, NULL, p);
      fst(c);
      p = NULL;
      changed = true;
    }
    // We have found the first range that intersects with [mi,ma]
    if (mi > c->min()) {
      _size -= mi-c->min();
      c->min(mi);
      changed = true;
    }

    while (c != NULL && c->max() <= ma) {
      RangeList *nc = c->next(p);
      p=c; c=nc;      
    }
    
    if (c == NULL) {
      return changed;
    }

    RangeList* newlst = p;
    if (ma >= c->min()) {
      _size -= c->max() - ma;
      c->max(ma);
      newlst = c;
      RangeList* nc = c->next(p);
      c->next(nc, NULL);
      p=c; c=nc;
    } else {
      if (p != NULL)
        p->next(c, NULL);
    }
    if (c != NULL) {
      RangeList* del = c;
      RangeList* delp = p;
      while (c != NULL) {
        _size -= c->width();
        RangeList* nc = c->next(p);
        p=c; c=nc;
      }
      del->dispose(home, delp, lst());
    }
    lst(newlst);
    if (newlst==NULL)
      fst(NULL);
    return true;
  }

  bool
  LUBndSet::exclude_full(Space* home, int mi, int ma, SetDelta& d) {
    assert(ma >= mi);
    assert(mi <= max() && ma >= min() &&
           (mi > min() || ma < max()));
    bool result=false;

    RangeList* p = NULL;
    RangeList* c = fst();
    d._lubMin = Limits::Set::int_max+1;
    while (c != NULL) {
      if (c->max() >= mi){
        if (c->min() > ma) { return result; } //in a hole

        if (c->min()<mi && c->max() > ma) {  //Range split:
          RangeList* q =
            new (home) RangeList(ma+1,c->max(),c,c->next(p));
          c->max(mi-1);
          if (c != lst()) {
            c->next(p)->prev(c,q);
          } else {
            lst(q);
          }
          c->next(c->next(p),q);
          _size -= (ma-mi+1);
          d._lubMin = mi;
          d._lubMax = ma;
          return true;
        }

        if (c->max() > ma) { //start of range clipped, end remains
          d._lubMin = std::min(d._lubMin, c->min());
          d._lubMax = ma;
          _size-=(ma-c->min()+1);
          c->min(ma+1);
          return true;
        }

        if (c->min() < mi) { //end of range clipped, start remains
          _size-=(c->max()-mi+1);
          d._lubMin = mi;
          d._lubMax = c->max();
          c->max(mi-1);
          result=true;
        } else { //range *c destroyed
          d._lubMin = c->min();
          _size-=c->width();
          RangeList *cendp = p;
          RangeList *cend = c;
          while ((cend->next(cendp)!=NULL) && (cend->next(cendp)->max()<=ma)){
            RangeList *ncend = cend->next(cendp);
            cendp=cend; cend=ncend;
            _size-=cend->width();
          }
          d._lubMax = cend->max();
          if (fst()==c) {
            fst(cend->next(cendp));
          } else {
            p->next(c, cend->next(cendp));
          }
          if (lst()==cend) {
            lst(p);
          } else {
            cend->next(cendp)->prev(cend,p);
          }
          RangeList* nc=cend->next(cendp); //performs loop step!
          c->dispose(home,p,cend);
          p=cend;
          c=nc;
          if (c!=NULL && c->min() <= ma ) {
            //start of range clipped, end remains
            _size-=(ma-c->min()+1);
            c->min(ma+1);
            d._lubMax = ma;
          }
          return true;
        }
      }
      RangeList *nc = c->next(p);
      p=c; c=nc;
    }
    return result;
  }

#ifndef NDEBUG
  using namespace Gecode::Int;
#endif

  bool
  BndSet::isConsistent(void) const {
#ifndef NDEBUG
    if (fst()==NULL) {
      if (lst()!=NULL || size()!=0) {
        std::cerr<<"Strange empty set.\n";
        return false;
      } else return true;
    }

    if (fst()!=NULL && lst()==NULL) {
      std::cerr<<"First is not null, last is.\n";
      return false;
    }

    RangeList *p=NULL;
    RangeList *c=fst();

    int min = c->min();
    int max = c->max();

    if (max<min) {
      std::cerr << "Single range list twisted: ("<<min<<","<<max<<")" ;
      return false;
    }

    RangeList *nc=c->next(p);
    p=c; c=nc;
    while (c) {
      if (max<min) {
        std::cerr << "1";
        return false;
      }
      if ((max+1)>=c->min()) {
        std::cerr << "2";
        return false;
      }
      if (c->next(p)==NULL && c!=lst()) {
        std::cerr << "3";
        return false;
      }

      if (c->next(p)!=NULL && c->next(p)->prev(c->next(p)->next(c))!=c) {
        std::cerr<<"Prev of next not self.\n";
        ((RangeList *)NULL)->min();
        return false;
      }

      min = c->min();
      max = c->max();
      
      nc=c->next(p);
      p=c; c=nc;
    }
#endif
    return true;
  }


}}

// STATISTICS: set-var

