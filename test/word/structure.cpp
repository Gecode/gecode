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

#include <gecode/search.hh>

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

    class DifferentialSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar amount;
      Gecode::WordVar native_result;
      Gecode::WordVar boolean_result;
      DifferentialSpace(unsigned int width)
        : x(*this,width), amount(*this,width),
          native_result(*this,width), boolean_result(*this,width) {}
      DifferentialSpace(DifferentialSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x); amount.update(*this,s.amount);
        native_result.update(*this,s.native_result);
        boolean_result.update(*this,s.boolean_result);
      }
      virtual Gecode::Space* copy(void) {
        return new DifferentialSpace(*this);
      }
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

    /**
     * Constant shifts and rotations enforce bit consistency with direct
     * fixed-width masked propagation.
     */
    class Shift : public Base {
    private:
      enum Op { SHL, LSHR, ASHR, ROL, ROR };

      static Gecode::WordValue mask(unsigned int width) {
        return (width == 64U) ? ~Gecode::WordValue(0) :
          ((Gecode::WordValue(1) << width) - 1);
      }

      static Gecode::WordValue value(Op op, unsigned int width,
                                     Gecode::WordValue x,
                                     unsigned int amount) {
        const Gecode::WordValue m = mask(width);
        switch (op) {
        case SHL:
          return (amount >= width) ? 0 : ((x << amount) & m);
        case LSHR:
          return (amount >= width) ? 0 : (x >> amount);
        case ASHR: {
          const bool sign = (x & (Gecode::WordValue(1) << (width-1))) != 0;
          if (amount >= width)
            return sign ? m : 0;
          return (x >> amount) |
            (sign ? (m & ~(m >> amount)) : 0);
        }
        case ROL:
          amount %= width;
          return (amount == 0U) ? x :
            (((x << amount) | (x >> (width-amount))) & m);
        case ROR:
          amount %= width;
          return (amount == 0U) ? x :
            (((x >> amount) | (x << (width-amount))) & m);
        default:
          GECODE_NEVER;
        }
        return 0;
      }

      static void post(Op op, Gecode::Home home, Gecode::WordVar x,
                       unsigned int amount, Gecode::WordVar result) {
        switch (op) {
        case SHL:  Gecode::shift_left(home,x,amount,result); break;
        case LSHR: Gecode::logical_shift_right(home,x,amount,result); break;
        case ASHR: Gecode::arithmetic_shift_right(home,x,amount,result); break;
        case ROL:  Gecode::rotate_left(home,x,amount,result); break;
        case ROR:  Gecode::rotate_right(home,x,amount,result); break;
        default: GECODE_NEVER;
        }
      }

      static bool assigned_values(void) {
        const unsigned int amounts[] = {0U,2U,3U,4U};
        for (int op=SHL; op<=ROR; op++)
          for (unsigned int ai=0; ai<4; ai++)
            for (Gecode::WordValue x=0; x<8; x++) {
              StructureSpace s(3,3);
              Gecode::dom(s,s.x,x);
              post(static_cast<Op>(op),s,s.x,amounts[ai],s.y);
              if ((s.status() == Gecode::SS_FAILED) || !s.y.assigned() ||
                  (s.y.val() != value(static_cast<Op>(op),3,x,
                                      amounts[ai])))
                return false;
            }
        return true;
      }

      static bool partial_masks(void) {
        StructureSpace left(4,4);
        Gecode::dom(left,left.x,2U,7U);
        Gecode::shift_left(left,left.x,1,left.y);
        if ((left.status() == Gecode::SS_FAILED) ||
            (left.y.lo() != 4U) || (left.y.hi() != 14U)) return false;

        StructureSpace logical(4,4);
        Gecode::dom(logical,logical.y,2U,7U);
        Gecode::logical_shift_right(logical,logical.x,1,logical.y);
        if ((logical.status() == Gecode::SS_FAILED) ||
            ((logical.x.lo() & 4U) == 0)) return false;

        StructureSpace arithmetic(4,4);
        Gecode::dom(arithmetic,arithmetic.y,8U,15U);
        Gecode::arithmetic_shift_right(arithmetic,arithmetic.x,1,
                                       arithmetic.y);
        if ((arithmetic.status() == Gecode::SS_FAILED) ||
            ((arithmetic.x.lo() & 8U) == 0) ||
            ((arithmetic.y.lo() & 12U) != 12U)) return false;

        StructureSpace rotation(4,4);
        Gecode::dom(rotation,rotation.x,1U,7U);
        Gecode::rotate_left(rotation,rotation.x,1,rotation.y);
        return (rotation.status() != Gecode::SS_FAILED) &&
          ((rotation.y.lo() & 2U) != 0);
      }

      static bool constants_and_boundaries(void) {
        StructureSpace s(4,4,4);
        Gecode::shift_left(s,4,9U,4,s.x);
        Gecode::logical_shift_right(s,4,9U,5,s.y);
        Gecode::rotate_left(s,4,9U,5,s.z);
        if ((s.status() == Gecode::SS_FAILED) || (s.x.val() != 0U) ||
            (s.y.val() != 0U) || (s.z.val() != 3U)) return false;

        StructureSpace a(4,4);
        Gecode::arithmetic_shift_right(a,4,9U,4,a.x);
        Gecode::rotate_right(a,4,9U,5,a.y);
        if ((a.status() == Gecode::SS_FAILED) || (a.x.val() != 15U) ||
            (a.y.val() != 12U)) return false;

        StructureSpace one(1,1);
        Gecode::arithmetic_shift_right(one,1,1U,2,one.x);
        if ((one.status() == Gecode::SS_FAILED) || (one.x.val() != 1U))
          return false;

        StructureSpace wide(64,64);
        const Gecode::WordValue high = Gecode::WordValue(1) << 63;
        Gecode::logical_shift_right(wide,64,high,63,wide.x);
        Gecode::arithmetic_shift_right(wide,64,high,64,wide.y);
        return (wide.status() != Gecode::SS_FAILED) &&
          (wide.x.val() == 1U) &&
          (wide.y.val() == ~Gecode::WordValue(0));
      }

      static bool alias_and_lifecycle(void) {
        StructureSpace identity(4,4);
        Gecode::shift_left(identity,identity.x,0,identity.x);
        Gecode::rotate_left(identity,identity.x,4,identity.x);
        Gecode::dom(identity,identity.x,5U);
        if (identity.status() == Gecode::SS_FAILED) return false;

        StructureSpace failed(4,4);
        Gecode::dom(failed,failed.x,1U);
        Gecode::shift_left(failed,failed.x,1,failed.x);
        if (failed.status() != Gecode::SS_FAILED) return false;

        StructureSpace source(4,4);
        Gecode::rotate_right(source,source.x,1,source.y);
        if (source.status() == Gecode::SS_FAILED) return false;
        StructureSpace* clone = static_cast<StructureSpace*>(source.clone());
        Gecode::dom(*clone,clone->x,3U);
        bool ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->y.assigned() && (clone->y.val() == 9U) &&
          !source.y.assigned();
        delete clone;
        if (!ok) return false;

        try {
          StructureSpace mismatch(3,4);
          Gecode::shift_left(mismatch,mismatch.x,1,mismatch.y);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

    public:
      Shift(void) : Base("Word::Structure::Shift") {}
      virtual bool run(void) {
        return assigned_values() && partial_masks() &&
          constants_and_boundaries() && alias_and_lifecycle();
      }
    };

    Shift shift;

    /**
     * Variable shifts use a bounded word-level cube-hull actor. The generic
     * word test supplies the assigned-value oracle and ordinary clone path;
     * these focused checks cover partial amounts and posting aliases.
     */
    class VariableShift : public Test {
    public:
      enum Op { SHL, LSHR, ASHR } op;
    private:

      static Gecode::WordValue word_mask(unsigned int width) {
        return (width == 64U) ? ~Gecode::WordValue(0) :
          ((Gecode::WordValue(1) << width) - 1);
      }

      static Gecode::WordValue evaluate(Op op, unsigned int width,
                                        Gecode::WordValue x,
                                        Gecode::WordValue amount) {
        const Gecode::WordValue m = word_mask(width);
        if (op == SHL)
          return (amount >= width) ? 0 : ((x << amount) & m);
        if (op == LSHR)
          return (amount >= width) ? 0 : (x >> amount);
        const bool sign = (x & (Gecode::WordValue(1) << (width-1))) != 0;
        if (amount >= width)
          return sign ? m : 0;
        return (x >> amount) |
          (sign ? (m & ~(m >> static_cast<unsigned int>(amount))) : 0);
      }

      static void post_op(Op op, Gecode::Home home, Gecode::WordVar x,
                          Gecode::WordVar amount,
                          Gecode::WordVar result) {
        switch (op) {
        case SHL:  Gecode::shift_left(home,x,amount,result); break;
        case LSHR: Gecode::logical_shift_right(home,x,amount,result); break;
        case ASHR: Gecode::arithmetic_shift_right(home,x,amount,result); break;
        default: GECODE_NEVER;
        }
      }

      static bool focused(void) {
        StructureSpace amount(4,4,4);
        Gecode::dom(amount,amount.x,1U);
        Gecode::dom(amount,amount.z,4U);
        Gecode::shift_left(amount,amount.x,amount.y,amount.z);
        if ((amount.status() == Gecode::SS_FAILED) ||
            !amount.y.assigned() || (amount.y.val() != 2U))
          return false;

        StructureSpace partial(4,4,4);
        Gecode::dom(partial,partial.x,8U);
        Gecode::dom(partial,partial.y,0U,1U);
        Gecode::arithmetic_shift_right(
          partial,partial.x,partial.y,partial.z);
        if ((partial.status() == Gecode::SS_FAILED) ||
            (partial.z.lo() != 8U) || (partial.z.hi() != 12U))
          return false;

        StructureSpace over(4,4,4);
        Gecode::dom(over,over.y,4U,7U);
        Gecode::dom(over,over.z,15U);
        Gecode::arithmetic_shift_right(over,over.x,over.y,over.z);
        if ((over.status() == Gecode::SS_FAILED) ||
            ((over.x.lo() & 8U) == 0))
          return false;

        StructureSpace failed(4,4,4);
        Gecode::dom(failed,failed.x,1U);
        Gecode::dom(failed,failed.y,1U);
        Gecode::dom(failed,failed.z,1U);
        Gecode::shift_left(failed,failed.x,failed.y,failed.z);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        StructureSpace same_result(4,4,4);
        Gecode::shift_left(same_result,same_result.x,
                           same_result.y,same_result.x);
        Gecode::dom(same_result,same_result.x,1U);
        if ((same_result.status() == Gecode::SS_FAILED) ||
            !same_result.y.assigned() || (same_result.y.val() != 0U))
          return false;

        StructureSpace same_amount(4,4,4);
        Gecode::logical_shift_right(same_amount,same_amount.x,
                                    same_amount.y,same_amount.y);
        Gecode::dom(same_amount,same_amount.x,0U);
        if ((same_amount.status() == Gecode::SS_FAILED) ||
            !same_amount.y.assigned() || (same_amount.y.val() != 0U))
          return false;

        StructureSpace same_input(4,4,4);
        Gecode::shift_left(same_input,same_input.x,
                           same_input.x,same_input.z);
        Gecode::dom(same_input,same_input.x,1U);
        if ((same_input.status() == Gecode::SS_FAILED) ||
            !same_input.z.assigned() || (same_input.z.val() != 2U))
          return false;

        try {
          StructureSpace mismatch(4,3,4);
          Gecode::shift_left(mismatch,mismatch.x,mismatch.y,mismatch.z);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool boolean_parity(void) {
        const unsigned int width = 3;
        for (int oi=SHL; oi<=ASHR; oi++)
          for (Gecode::WordValue amount=0; amount<8; amount++)
            for (Gecode::WordValue value=0; value<8; value++) {
              const Op op = static_cast<Op>(oi);
              DifferentialSpace s(width);
              Gecode::dom(s,s.x,value);
              Gecode::dom(s,s.amount,amount);
              post_op(op,s,s.x,s.amount,s.native_result);

              // Independently post the fixed-amount shift over Boolean bits;
              // only the input and final result are channelled to words.
              Gecode::BoolVarArray input(s,width,0,1);
              Gecode::BoolVarArray output(s,width,0,1);
              for (unsigned int bit=0; bit<width; bit++) {
                Gecode::channel(s,s.x,bit,input[bit]);
                Gecode::channel(s,s.boolean_result,bit,output[bit]);
                if ((op == SHL) && (amount <= bit))
                  Gecode::rel(s,output[bit],Gecode::IRT_EQ,
                              input[bit-static_cast<unsigned int>(amount)]);
                else if ((op != SHL) && (amount+bit < width))
                  Gecode::rel(s,output[bit],Gecode::IRT_EQ,
                              input[bit+static_cast<unsigned int>(amount)]);
                else if (op == ASHR)
                  Gecode::rel(s,output[bit],Gecode::IRT_EQ,input[width-1]);
                else
                  Gecode::rel(s,output[bit],Gecode::IRT_EQ,0);
              }
              if ((s.status() == Gecode::SS_FAILED) ||
                  !s.native_result.assigned() ||
                  !s.boolean_result.assigned() ||
                  (s.native_result.val() != s.boolean_result.val()))
                return false;
            }
        return true;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class ShiftSpace : public Space {
        public:
          WordVar x;
          WordVar amount;
          WordVar result;
          ShiftSpace(void)
            : x(*this,3,1U,3U), amount(*this,3,0U,1U),
              result(*this,3) {
            shift_left(*this,x,amount,result);
            WordVarArgs decision(2);
            decision[0] = x;
            decision[1] = amount;
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          ShiftSpace(ShiftSpace& s) : Space(s) {
            x.update(*this,s.x);
            amount.update(*this,s.amount);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new ShiftSpace(*this); }
        };

        ShiftSpace* root = new ShiftSpace;
        Search::Options options;
        // Keep only the root clone so later alternatives use replay.
        options.c_d = 64;
        options.a_d = 64;
        DFS<ShiftSpace> dfs(root,options);
        delete root;
        unsigned int seen = 0;
        while (ShiftSpace* solution = dfs.next()) {
          const WordValue x = solution->x.val();
          const WordValue amount = solution->amount.val();
          const WordValue result = solution->result.val();
          const bool ok = (result == (x << amount)) &&
            (PropagatorGroup::all.size(*solution) == 0);
          if (ok)
            seen |= 1U << static_cast<unsigned int>((x << 1) | amount);
          delete solution;
          if (!ok)
            return false;
        }
        return seen == ((1U << 2) | (1U << 3) |
                        (1U << 6) | (1U << 7));
      }

    public:
      VariableShift(Op op0, const std::string& name)
        : Test("Structure::VariableShift::"+name,
               3,Domain(3,0,7)), op(op0) {}

      virtual bool solution(const Assignment& a) const {
        return a[2] == evaluate(op,dom.width(),a[0],a[1]);
      }

      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        post_op(op,home,x[0],x[1],x[2]);
      }

      virtual bool run(void) {
        return Test::run() &&
          ((op != SHL) ||
           (focused() && boolean_parity() && search_recomputation()));
      }
    };

    VariableShift variable_shift_left(VariableShift::SHL,"Left");
    VariableShift variable_logical_right(VariableShift::LSHR,"LogicalRight");
    VariableShift variable_arithmetic_right(
      VariableShift::ASHR,"ArithmeticRight");
  }
}}

// STATISTICS: test-word
