/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#include "test/word.hh"

namespace Test { namespace Word {

  namespace Structure {

    class StructureSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar y;
      Gecode::WordVar z;
      StructureSpace(unsigned int wx, unsigned int wy, unsigned int wz=1)
        : x(*this,wx), y(*this,wy), z(*this,wz) {}
      StructureSpace(StructureSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
      }
      virtual Gecode::Space* copy(void) { return new StructureSpace(*this); }
    };

    /**
     * Fixed structural actors enforce bit consistency for copied bits and
     * copied bit groups using native word masks.
     */
    class Fixed : public Base {
    private:
      static bool assigned_values(void) {
        for (Gecode::WordValue v=0; v<8; v++) {
          StructureSpace s(3,2);
          Gecode::dom(s,s.x,v); Gecode::extract(s,s.x,1,2,s.y);
          if ((s.status() == Gecode::SS_FAILED) || !s.y.assigned() ||
              (s.y.val() != ((v >> 1) & 3U))) return false;
        }
        for (Gecode::WordValue h=0; h<4; h++)
          for (Gecode::WordValue l=0; l<2; l++) {
            StructureSpace s(2,1,3);
            Gecode::dom(s,s.x,h); Gecode::dom(s,s.y,l);
            Gecode::concat(s,s.x,s.y,s.z);
            if ((s.status() == Gecode::SS_FAILED) || !s.z.assigned() ||
                (s.z.val() != ((h << 1) | l))) return false;
          }
        for (Gecode::WordValue v=0; v<4; v++) {
          StructureSpace r(2,4); Gecode::dom(r,r.x,v);
          Gecode::repeat(r,r.x,2,r.y);
          if ((r.status() == Gecode::SS_FAILED) || !r.y.assigned() ||
              (r.y.val() != (v | (v << 2)))) return false;
          StructureSpace z(2,4); Gecode::dom(z,z.x,v);
          Gecode::zero_extend(z,z.x,4,z.y);
          if ((z.status() == Gecode::SS_FAILED) || !z.y.assigned() ||
              (z.y.val() != v)) return false;
          StructureSpace q(2,4); Gecode::dom(q,q.x,v);
          Gecode::sign_extend(q,q.x,4,q.y);
          Gecode::WordValue sv = ((v & 2U) != 0) ? (v | 12U) : v;
          if ((q.status() == Gecode::SS_FAILED) || !q.y.assigned() ||
              (q.y.val() != sv)) return false;
        }
        return true;
      }

      static bool partial_masks(void) {
        StructureSpace e(4,2);
        Gecode::dom(e,e.x,4U,15U); Gecode::dom(e,e.y,0U,2U);
        Gecode::extract(e,e.x,1,2,e.y);
        if ((e.status() == Gecode::SS_FAILED) || (e.y.lo() != 2U) ||
            (e.y.hi() != 2U) || ((e.x.hi() & 2U) != 0)) return false;
        StructureSpace c(2,2,4);
        Gecode::dom(c,c.x,1U,3U); Gecode::dom(c,c.y,0U,2U);
        Gecode::concat(c,c.x,c.y,c.z);
        if ((c.status() == Gecode::SS_FAILED) || (c.z.lo() != 4U) ||
            (c.z.hi() != 14U)) return false;
        StructureSpace r(2,4); Gecode::dom(r,r.y,1U,15U);
        Gecode::repeat(r,r.x,2,r.y);
        if ((r.status() == Gecode::SS_FAILED) ||
            ((r.x.lo() & 1U) == 0) || ((r.y.lo() & 5U) != 5U)) return false;
        StructureSpace z(2,4); Gecode::zero_extend(z,z.x,4,z.y);
        if ((z.status() == Gecode::SS_FAILED) || ((z.y.hi() & 12U) != 0))
          return false;
        StructureSpace s(2,4); Gecode::dom(s,s.y,4U,15U);
        Gecode::sign_extend(s,s.x,4,s.y);
        return (s.status() != Gecode::SS_FAILED) &&
          ((s.x.lo() & 2U) != 0) && ((s.y.lo() & 12U) == 12U);
      }

      static bool lifecycle(void) {
        StructureSpace constants(2,2,4);
        Gecode::extract(constants,3,6U,1,2,constants.x);
        Gecode::dom(constants,constants.y,1U);
        Gecode::concat(constants,2,2U,constants.y,constants.z);
        if ((constants.status() == Gecode::SS_FAILED) ||
            (constants.x.val() != 3U) || (constants.z.val() != 9U))
          return false;
        StructureSpace ends(1,1);
        Gecode::extract(ends,4,9U,0,1,ends.x);
        Gecode::extract(ends,4,8U,3,1,ends.y);
        if ((ends.status() == Gecode::SS_FAILED) ||
            (ends.x.val() != 1U) || (ends.y.val() != 1U)) return false;
        StructureSpace low(2,1,3); Gecode::dom(low,low.x,2U);
        Gecode::concat(low,low.x,1,1U,low.z);
        if ((low.status() == Gecode::SS_FAILED) || (low.z.val() != 5U))
          return false;
        StructureSpace other(2,4,4);
        Gecode::repeat(other,2,1U,2,other.y);
        Gecode::zero_extend(other,2,3U,4,other.z);
        if ((other.status() == Gecode::SS_FAILED) || (other.y.val() != 5U) ||
            (other.z.val() != 3U)) return false;
        StructureSpace sign(2,4);
        Gecode::sign_extend(sign,2,2U,4,sign.y);
        if ((sign.status() == Gecode::SS_FAILED) || (sign.y.val() != 14U))
          return false;

        StructureSpace alias(2,2,4);
        Gecode::extract(alias,alias.x,0,2,alias.x);
        Gecode::repeat(alias,alias.x,1,alias.x);
        Gecode::zero_extend(alias,alias.x,2,alias.x);
        Gecode::sign_extend(alias,alias.x,2,alias.x);
        Gecode::concat(alias,alias.x,alias.x,alias.z);
        Gecode::dom(alias,alias.x,2U);
        if ((alias.status() == Gecode::SS_FAILED) || (alias.z.val() != 10U))
          return false;

        StructureSpace source(2,4); Gecode::repeat(source,source.x,2,source.y);
        if (source.status() == Gecode::SS_FAILED) return false;
        StructureSpace* clone = static_cast<StructureSpace*>(source.clone());
        Gecode::dom(*clone,clone->x,3U);
        bool ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->y.assigned() && (clone->y.val() == 15U) &&
          !source.y.assigned();
        delete clone;
        return ok;
      }

      static bool invalid_and_failure(void) {
        try { StructureSpace s(3,2); Gecode::extract(s,s.x,2,2,s.y);
          return false; } catch (const Gecode::Word::OutOfLimits&) {}
        try { StructureSpace s(2,3); Gecode::repeat(s,s.x,2,s.y);
          return false; } catch (const Gecode::Word::WidthMismatch&) {}
        try { StructureSpace s(3,2); Gecode::zero_extend(s,s.x,2,s.y);
          return false; } catch (const Gecode::Word::OutOfLimits&) {}
        StructureSpace s(2,4); Gecode::dom(s,s.x,1U); Gecode::dom(s,s.y,0U);
        Gecode::repeat(s,s.x,2,s.y);
        return s.status() == Gecode::SS_FAILED;
      }

    public:
      Fixed(void) : Base("Word::Structure::Fixed") {}
      virtual bool run(void) {
        return assigned_values() && partial_masks() && lifecycle() &&
          invalid_and_failure();
      }
    };

    Fixed fixed;
  }
}}

// STATISTICS: test-word
