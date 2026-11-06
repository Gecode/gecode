/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/set.hh>

namespace Gecode { namespace Set {

    BndSet::BndSet(Space& home, const IntSet& is) {
        if (is.ranges() == 0) {
            fst(nullptr);
            lst(nullptr);
            _size = 0;
        } else {
            int n = is.ranges();
            RangeList* r = home.alloc<RangeList>(n);
            fst(r);
            lst(r + n - 1);
            unsigned int s = 0;
            for (int i = n; i--;) {
                s += is.width(i);
                r[i].min(is.min(i));
                r[i].max(is.max(i));
                r[i].next(&r[i + 1]);
            }
            r[n - 1].next(nullptr);
            _size = s;
        }
        assert(is_consistent());
    }

    bool
    GLBndSet::include_full(Space& home, int mi, int ma, SetDelta& d) {
        assert(ma >= mi);
        assert(fst() != nullptr);

        RangeList* p = nullptr;
        RangeList* c = fst();

        while (c != nullptr) {
            if (c->max() >= mi - 1) {
                if (c->min() > ma + 1) { // in a hole before c
                    _size += (ma - mi + 1);
                    d._glb_min = mi;
                    d._glb_max = ma;
                    RangeList* q = new (home) RangeList(mi, ma, c);
                    if (p == nullptr)
                        // the new range is the first
                        fst(q);
                    else
                        p->next(q);
                    return true;
                }
                // if the range starts before c, update c->min and size
                bool result = false;
                if (c->min() > mi) {
                    _size += (c->min() - mi);
                    c->min(mi);
                    d._glb_min = mi;
                    result = true;
                } else {
                    d._glb_min = c->max() + 1;
                }
                assert(c->min() <= mi);
                assert(c->max() + 1 >= mi);
                if (c->max() >= ma) {
                    d._glb_max = c->min() - 1;
                    return result;
                }
                RangeList* q = c;
                // sum up the size of holes eaten
                int prev_max = c->max();
                int growth = 0;
                // invariant: q->min()<=ma+1
                while (q->next() != nullptr && q->next()->min() <= ma + 1) {
                    q = q->next();
                    growth += q->min() - prev_max - 1;
                    prev_max = q->max();
                }
                assert(growth >= 0);
                _size += growth;
                if (q->max() < ma) {
                    _size += ma - q->max();
                    d._glb_max = ma;
                } else {
                    d._glb_max = q->min() - 1;
                }
                c->max(std::max(ma, q->max()));
                if (c != q) {
                    RangeList* old_c_next = c->next();
                    assert(old_c_next != nullptr); // q would have stayed c if c was last.
                    c->next(q->next());
                    if (q->next() == nullptr) {
                        assert(q == lst());
                        lst(c);
                    }
                    old_c_next->dispose(home, q);
                }
                return true;
            }
            RangeList* nc = c->next();
            p = c;
            c = nc;
        }
        // the new range is disjoint from the old domain and we add it as last:
        assert(mi > max() + 1);
        RangeList* q = new (home) RangeList(mi, ma, nullptr);
        lst()->next(q);
        lst(q);
        _size += q->width();
        d._glb_min = mi;
        d._glb_max = ma;
        return true;
    }

    bool
    LUBndSet::intersect_full(Space& home, int mi, int ma) {
        RangeList* p = nullptr;
        RangeList* c = fst();

        assert(c != nullptr); // Never intersect with an empty set

        // Skip ranges that are before mi
        while (c != nullptr && c->max() < mi) {
            _size -= c->width();
            RangeList* nc = c->next();
            p = c;
            c = nc;
        }
        if (c == nullptr) {
            // Delete entire domain
            fst()->dispose(home, lst());
            fst(nullptr);
            lst(nullptr);
            return true;
        }

        bool changed = false;
        if (c != fst()) {
            fst()->dispose(home, p);
            fst(c);
            p = nullptr;
            changed = true;
        }
        // We have found the first range that intersects with [mi,ma]
        if (mi > c->min()) {
            _size -= mi - c->min();
            c->min(mi);
            changed = true;
        }

        while (c != nullptr && c->max() <= ma) {
            RangeList* nc = c->next();
            p = c;
            c = nc;
        }

        if (c == nullptr)
            return changed;

        RangeList* newlst = p;
        if (ma >= c->min()) {
            _size -= c->max() - ma;
            c->max(ma);
            newlst = c;
            RangeList* nc = c->next();
            c->next(nullptr);
            p = c;
            c = nc;
        } else if (p != nullptr) {
            p->next(nullptr);
        }
        if (c != nullptr) {
            for (RangeList* cc = c; cc != nullptr; cc = cc->next())
                _size -= cc->width();
            c->dispose(home, lst());
        }
        lst(newlst);
        if (newlst == nullptr)
            fst(nullptr);
        return true;
    }

    bool
    LUBndSet::exclude_full(Space& home, int mi, int ma, SetDelta& d) {
        assert(ma >= mi);
        assert(mi <= max() && ma >= min() && (mi > min() || ma < max()));
        bool result = false;

        RangeList* p = nullptr;
        RangeList* c = fst();
        d._lub_min = Limits::max + 1;
        while (c != nullptr) {
            if (c->max() >= mi) {
                if (c->min() > ma) {
                    return result;
                } // in a hole

                if (c->min() < mi && c->max() > ma) { // Range split:
                    RangeList* q = new (home) RangeList(ma + 1, c->max(), c->next());
                    c->max(mi - 1);
                    if (c == lst()) {
                        lst(q);
                    }
                    c->next(q);
                    _size -= (ma - mi + 1);
                    d._lub_min = mi;
                    d._lub_max = ma;
                    return true;
                }

                if (c->max() > ma) { // start of range clipped, end remains
                    d._lub_min = std::min(d._lub_min, c->min());
                    d._lub_max = ma;
                    _size -= (ma - c->min() + 1);
                    c->min(ma + 1);
                    return true;
                }

                if (c->min() < mi) { // end of range clipped, start remains
                    _size -= (c->max() - mi + 1);
                    d._lub_min = mi;
                    d._lub_max = c->max();
                    c->max(mi - 1);
                    result = true;
                } else { // range *c destroyed
                    d._lub_min = c->min();
                    _size -= c->width();
                    RangeList* cend = c;
                    while ((cend->next() != nullptr) && (cend->next()->max() <= ma)) {
                        cend = cend->next();
                        _size -= cend->width();
                    }
                    d._lub_max = cend->max();
                    if (fst() == c) {
                        fst(cend->next());
                    } else {
                        p->next(cend->next());
                    }
                    if (lst() == cend) {
                        lst(p);
                    }
                    RangeList* nc = cend->next(); // performs loop step!
                    c->dispose(home, cend);
                    p = cend;
                    c = nc;
                    if (c != nullptr && c->min() <= ma) {
                        // start of range clipped, end remains
                        _size -= (ma - c->min() + 1);
                        c->min(ma + 1);
                        d._lub_max = ma;
                    }
                    return true;
                }
            }
            RangeList* nc = c->next();
            p = c;
            c = nc;
        }
        return result;
    }

#ifndef NDEBUG
    using namespace Gecode::Int;
#endif

    bool
    BndSet::is_consistent() const {
#ifndef NDEBUG
        if (fst() == nullptr) {
            if (lst() != nullptr || !empty()) {
                std::cerr << "Strange empty set.\n";
                return false;
            } else
                return true;
        }

        if (fst() != nullptr && lst() == nullptr) {
            std::cerr << "First is not null, last is.\n";
            return false;
        }

        RangeList* c = fst();

        int min = c->min();
        int max = c->max();

        if (max < min) {
            std::cerr << "Single range list twisted: (" << min << "," << max << ")";
            return false;
        }

        RangeList* nc = c->next();
        c = nc;
        while (c) {
            if (max < min) {
                std::cerr << "1";
                return false;
            }
            if ((max + 1) >= c->min()) {
                std::cerr << "2";
                return false;
            }
            if (c->next() == nullptr && c != lst()) {
                std::cerr << "3";
                return false;
            }

            min = c->min();
            max = c->max();

            nc = c->next();
            c = nc;
        }
#endif
        return true;
    }

}} // namespace Gecode::Set

// STATISTICS: set-var
