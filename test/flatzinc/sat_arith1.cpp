/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2014
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

#include "test/flatzinc.hh"

namespace Test { namespace FlatZinc {

  namespace {
    /// Helper class to create and register tests
    class Create {
    public:

      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("sat_arith1",
"\n\
var bool: v_1;\n\
var bool: v_2;\n\
var bool: v_3;\n\
var bool: v_4;\n\
var bool: v_5;\n\
var bool: v_6;\n\
var bool: v_7;\n\
var bool: v_8;\n\
var bool: v_9;\n\
var bool: v_10;\n\
var bool: v_11;\n\
var bool: v_12;\n\
var bool: v_13;\n\
var bool: v_14;\n\
var bool: v_15;\n\
var bool: v_16;\n\
var bool: v_17;\n\
var bool: v_18;\n\
var bool: v_19;\n\
var bool: v_20;\n\
var bool: v_21;\n\
var bool: v_22;\n\
var bool: v_23;\n\
var bool: v_24;\n\
var bool: v_25;\n\
var bool: v_26;\n\
var bool: v_27;\n\
var bool: v_28;\n\
var bool: v_29;\n\
var bool: v_30;\n\
var bool: v_31;\n\
var bool: v_32;\n\
var bool: v_33;\n\
var bool: v_34;\n\
var bool: v_35;\n\
var bool: v_36;\n\
var bool: v_37;\n\
var bool: v_38;\n\
var bool: v_39;\n\
var bool: v_40;\n\
var bool: v_41;\n\
var bool: v_42;\n\
var bool: v_43;\n\
var bool: v_44;\n\
var bool: v_45;\n\
var bool: v_46;\n\
var bool: v_47;\n\
var bool: v_48;\n\
var bool: v_49;\n\
var bool: v_50;\n\
var bool: v_51;\n\
var bool: v_52;\n\
var bool: v_53;\n\
var bool: v_54;\n\
var bool: v_55;\n\
var bool: v_56;\n\
var bool: v_57;\n\
var bool: v_58;\n\
var bool: v_59;\n\
var bool: v_60;\n\
var bool: v_61;\n\
var bool: v_62;\n\
var bool: v_63;\n\
var bool: v_64;\n\
var bool: v_65;\n\
var bool: v_66;\n\
var bool: v_67;\n\
var bool: v_68;\n\
var bool: v_69;\n\
var bool: v_70;\n\
var bool: v_71;\n\
var bool: v_72;\n\
var bool: v_73;\n\
var bool: v_74;\n\
var bool: v_75;\n\
var bool: v_76;\n\
var bool: v_77;\n\
var bool: v_78;\n\
var bool: v_79;\n\
var bool: v_80;\n\
var bool: v_81;\n\
var bool: v_82;\n\
var bool: v_83;\n\
var bool: v_84;\n\
var bool: v_85;\n\
var bool: v_86;\n\
var bool: v_87;\n\
var bool: v_88;\n\
var bool: v_89;\n\
var bool: v_90;\n\
var bool: v_91;\n\
var bool: v_92;\n\
var bool: v_93;\n\
var bool: v_94;\n\
var bool: v_95;\n\
var bool: v_96;\n\
var bool: v_97;\n\
var bool: v_98;\n\
var bool: v_99;\n\
var bool: v_100;\n\
var bool: v_101;\n\
var bool: v_102;\n\
var bool: v_103;\n\
var bool: v_104;\n\
var bool: v_105;\n\
var bool: v_106;\n\
var bool: v_107;\n\
var bool: v_108;\n\
var bool: v_109;\n\
var bool: v_110;\n\
var bool: v_111;\n\
var bool: v_112;\n\
var bool: v_113;\n\
var bool: v_114;\n\
var bool: v_115;\n\
var bool: v_116;\n\
var bool: v_117;\n\
var bool: v_118;\n\
var bool: v_119;\n\
var bool: v_120;\n\
var bool: v_121;\n\
var bool: v_122;\n\
var bool: v_123;\n\
var bool: v_124;\n\
var bool: v_125;\n\
var bool: v_126;\n\
var bool: v_127;\n\
var bool: v_128;\n\
var bool: v_129;\n\
var bool: v_130;\n\
var bool: v_131;\n\
var bool: v_132;\n\
var bool: v_133;\n\
var bool: v_134;\n\
var bool: v_135;\n\
var bool: v_136;\n\
var bool: v_137;\n\
var bool: v_138;\n\
var bool: v_139;\n\
var bool: v_140;\n\
var bool: v_141;\n\
var bool: v_142;\n\
var bool: v_143;\n\
var bool: v_144;\n\
var bool: v_145;\n\
var bool: v_146;\n\
var bool: v_147;\n\
var bool: v_148;\n\
var bool: v_149;\n\
var bool: v_150;\n\
var bool: v_151;\n\
var bool: v_152;\n\
var bool: v_153;\n\
array [1..8] of var bool: a :: output_array([1..8]) =\n\
	[v_8, v_7, v_6, v_5, v_4, v_3, v_2, v_1];\n\
array [1..8] of var bool: b :: output_array([1..8]) =\n\
	[v_28, v_27, v_26, v_25, v_24, v_23, v_22, v_21];\n\
constraint bool_eq(false, v_4);\n\
constraint bool_eq(false, v_5);\n\
constraint bool_eq(false, v_6);\n\
constraint bool_eq(false, v_7);\n\
constraint bool_eq(false, v_8);\n\
constraint bool_eq(false, v_4);\n\
constraint bool_eq(v_4, v_9);\n\
constraint bool_not(v_4, v_10);\n\
constraint bool_eq(v_11, false);\n\
constraint bool_not(v_3, v_12);\n\
constraint bool_not(v_2, v_13);\n\
constraint bool_eq(v_2, v_14);\n\
constraint bool_not(v_1, v_15);\n\
constraint bool_or(v_9, v_16, true);\n\
constraint bool_and(v_10, v_17, v_16);\n\
constraint bool_or(v_11, v_18, v_17);\n\
constraint bool_and(v_12, v_19, v_18);\n\
constraint bool_or(v_13, v_20, v_19);\n\
constraint bool_and(v_14, v_15, v_20);\n\
constraint bool_eq(false, v_24);\n\
constraint bool_eq(false, v_25);\n\
constraint bool_eq(false, v_26);\n\
constraint bool_eq(false, v_27);\n\
constraint bool_eq(false, v_28);\n\
constraint bool_eq(false, v_24);\n\
constraint bool_eq(v_24, v_29);\n\
constraint bool_not(v_24, v_30);\n\
constraint bool_eq(v_31, false);\n\
constraint bool_not(v_23, v_32);\n\
constraint bool_not(v_22, v_33);\n\
constraint bool_eq(v_22, v_34);\n\
constraint bool_not(v_21, v_35);\n\
constraint bool_or(v_29, v_36, true);\n\
constraint bool_and(v_30, v_37, v_36);\n\
constraint bool_or(v_31, v_38, v_37);\n\
constraint bool_and(v_32, v_39, v_38);\n\
constraint bool_or(v_33, v_40, v_39);\n\
constraint bool_and(v_34, v_35, v_40);\n\
constraint bool_eq(v_28, v_41);\n\
constraint bool_not(v_28, v_42);\n\
constraint bool_eq(v_43, false);\n\
constraint bool_not(v_27, v_44);\n\
constraint bool_eq(v_45, false);\n\
constraint bool_not(v_26, v_46);\n\
constraint bool_eq(v_47, false);\n\
constraint bool_not(v_25, v_48);\n\
constraint bool_eq(v_49, false);\n\
constraint bool_not(v_24, v_50);\n\
constraint bool_eq(v_51, false);\n\
constraint bool_not(v_23, v_52);\n\
constraint bool_eq(v_53, false);\n\
constraint bool_not(v_22, v_54);\n\
constraint bool_not(v_21, v_55);\n\
constraint bool_or(v_41, v_56, false);\n\
constraint bool_and(v_42, v_57, v_56);\n\
constraint bool_or(v_43, v_58, v_57);\n\
constraint bool_and(v_44, v_59, v_58);\n\
constraint bool_or(v_45, v_60, v_59);\n\
constraint bool_and(v_46, v_61, v_60);\n\
constraint bool_or(v_47, v_62, v_61);\n\
constraint bool_and(v_48, v_63, v_62);\n\
constraint bool_or(v_49, v_64, v_63);\n\
constraint bool_and(v_50, v_65, v_64);\n\
constraint bool_or(v_51, v_66, v_65);\n\
constraint bool_and(v_52, v_67, v_66);\n\
constraint bool_or(v_53, v_68, v_67);\n\
constraint bool_and(v_54, v_55, v_68);\n\
constraint bool_eq(v_8, v_69);\n\
constraint bool_not(v_8, v_70);\n\
constraint bool_eq(v_71, false);\n\
constraint bool_not(v_7, v_72);\n\
constraint bool_eq(v_73, false);\n\
constraint bool_not(v_6, v_74);\n\
constraint bool_eq(v_75, false);\n\
constraint bool_not(v_5, v_76);\n\
constraint bool_eq(v_77, false);\n\
constraint bool_not(v_4, v_78);\n\
constraint bool_eq(v_79, false);\n\
constraint bool_not(v_3, v_80);\n\
constraint bool_eq(v_81, false);\n\
constraint bool_not(v_2, v_82);\n\
constraint bool_not(v_1, v_83);\n\
constraint bool_or(v_69, v_84, false);\n\
constraint bool_and(v_70, v_85, v_84);\n\
constraint bool_or(v_71, v_86, v_85);\n\
constraint bool_and(v_72, v_87, v_86);\n\
constraint bool_or(v_73, v_88, v_87);\n\
constraint bool_and(v_74, v_89, v_88);\n\
constraint bool_or(v_75, v_90, v_89);\n\
constraint bool_and(v_76, v_91, v_90);\n\
constraint bool_or(v_77, v_92, v_91);\n\
constraint bool_and(v_78, v_93, v_92);\n\
constraint bool_or(v_79, v_94, v_93);\n\
constraint bool_and(v_80, v_95, v_94);\n\
constraint bool_or(v_81, v_96, v_95);\n\
constraint bool_and(v_82, v_83, v_96);\n\
constraint bool_eq(v_113, false);\n\
constraint bool_xor(v_1, v_21, v_105);\n\
constraint bool_xor(v_105, v_113, v_97);\n\
constraint array_bool_or([v_122, v_130, v_138], v_114);\n\
constraint bool_and(v_1, v_21, v_122);\n\
constraint bool_and(v_1, v_113, v_130);\n\
constraint bool_and(v_21, v_113, v_138);\n\
constraint bool_xor(v_2, v_22, v_106);\n\
constraint bool_xor(v_106, v_114, v_98);\n\
constraint array_bool_or([v_123, v_131, v_139], v_115);\n\
constraint bool_and(v_2, v_22, v_123);\n\
constraint bool_and(v_2, v_114, v_131);\n\
constraint bool_and(v_22, v_114, v_139);\n\
constraint bool_xor(v_3, v_23, v_107);\n\
constraint bool_xor(v_107, v_115, v_99);\n\
constraint array_bool_or([v_124, v_132, v_140], v_116);\n\
constraint bool_and(v_3, v_23, v_124);\n\
constraint bool_and(v_3, v_115, v_132);\n\
constraint bool_and(v_23, v_115, v_140);\n\
constraint bool_xor(v_4, v_24, v_108);\n\
constraint bool_xor(v_108, v_116, v_100);\n\
constraint array_bool_or([v_125, v_133, v_141], v_117);\n\
constraint bool_and(v_4, v_24, v_125);\n\
constraint bool_and(v_4, v_116, v_133);\n\
constraint bool_and(v_24, v_116, v_141);\n\
constraint bool_xor(v_5, v_25, v_109);\n\
constraint bool_xor(v_109, v_117, v_101);\n\
constraint array_bool_or([v_126, v_134, v_142], v_118);\n\
constraint bool_and(v_5, v_25, v_126);\n\
constraint bool_and(v_5, v_117, v_134);\n\
constraint bool_and(v_25, v_117, v_142);\n\
constraint bool_xor(v_6, v_26, v_110);\n\
constraint bool_xor(v_110, v_118, v_102);\n\
constraint array_bool_or([v_127, v_135, v_143], v_119);\n\
constraint bool_and(v_6, v_26, v_127);\n\
constraint bool_and(v_6, v_118, v_135);\n\
constraint bool_and(v_26, v_118, v_143);\n\
constraint bool_xor(v_7, v_27, v_111);\n\
constraint bool_xor(v_111, v_119, v_103);\n\
constraint array_bool_or([v_128, v_136, v_144], v_120);\n\
constraint bool_and(v_7, v_27, v_128);\n\
constraint bool_and(v_7, v_119, v_136);\n\
constraint bool_and(v_27, v_119, v_144);\n\
constraint bool_xor(v_8, v_28, v_112);\n\
constraint bool_xor(v_112, v_120, v_104);\n\
constraint array_bool_or([v_129, v_137, v_145], v_121);\n\
constraint bool_and(v_8, v_28, v_129);\n\
constraint bool_and(v_8, v_120, v_137);\n\
constraint bool_and(v_28, v_120, v_145);\n\
constraint bool_eq(v_120, v_121);\n\
constraint bool_not(v_97, v_146);\n\
constraint bool_eq(v_98, v_147);\n\
constraint bool_not(v_99, v_148);\n\
constraint bool_not(v_100, v_149);\n\
constraint bool_not(v_101, v_150);\n\
constraint bool_not(v_102, v_151);\n\
constraint bool_not(v_103, v_152);\n\
constraint bool_not(v_104, v_153);\n\
constraint array_bool_and([v_146, v_147, v_148, v_149, v_150, v_151, v_152, v_153], true);\n\
solve satisfy;\n\
",
"a = array1d(1..8, [false, false, false, false, false, false, false, true]);\n\
b = array1d(1..8, [false, false, false, false, false, false, false, true]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
