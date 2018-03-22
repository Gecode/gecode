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
        (void) new FlatZincTest("tenpenki::3",
std::string("predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00003 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00005 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00006 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00007 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00008 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00010 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00011 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00012 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00013 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00014 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00015 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00017 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00018 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00019 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00021 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00022 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00024 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00025 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00028 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00029 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00031 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00033 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00034 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00036 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00037 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00039 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00040 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00041 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00043 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00044 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00045 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00048 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00055 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00056 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00058 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00059 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00061 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00062 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00066 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00067 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00069 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00070 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00071 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00072 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00074 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00075 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00076 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00077 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00078 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00079 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00081 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00082 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00083 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00084 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00085 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00086 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00088 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00089 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00094 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00095 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00097 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00098 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00104 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00105 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00107 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00108 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00112 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00113 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00115 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00116 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00117 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00118 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00119 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00120 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00122 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00123 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00124 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00125 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00126 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00127 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00129 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00130 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00131 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00132 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00133 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00134 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00136 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00137 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00140 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00141 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00143 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00144 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00146 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00147 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00148 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00149 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00150 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00151 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00153 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00154 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00155 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00156 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00157 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00158 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00160 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00161 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00164 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00165 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00167 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00168 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00170 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00171 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00172 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00173 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00174 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00175 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00177 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00178 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00179 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00180 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00181 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00182 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00184 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00185 :: is_defined_var :: var_is_introduced;\n\
array [1..25] of var bool: a :: output_array([1..5, 1..5]);\n\
array [1..2] of var 1..5: s____00001;\n\
array [1..2] of var 1..5: s____00027;\n\
array [1..1] of var 1..5: s____00053;\n\
array [1..2] of var 1..5: s____00065;\n\
array [1..1] of var 1..5: s____00091;\n\
array [1..1] of var 1..5: s____00101;\n\
array [1..2] of var 1..5: s____00111;\n\
array [1..2] of var 1..5: s____00139;\n\
array [1..2] of var 1..5: s____00163;\n\
constraint array_bool_and([BOOL____00006, BOOL____00007], BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint array_bool_and([BOOL____00010, BOOL____00011], BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint array_bool_and([BOOL____00012, BOOL____00013], BOOL____00015) :: defines_var(BOOL____00015);\n\
constraint array_bool_and([BOOL____00017, BOOL____00018], BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint array_bool_and([BOOL____00019, BOOL____00020], BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint array_bool_and([BOOL____00032, BOOL____00033], BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint array_bool_and([BOOL____00036, BOOL____00037], BOOL____00040) :: defines_var(BOOL____00040);\n\
constraint array_bool_and([BOOL____00038, BOOL____00039], BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint array_bool_and([BOOL____00043, BOOL____00044], BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint array_bool_and([BOOL____00045, BOOL____00046], BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint array_bool_and([BOOL____00055, BOOL____00056], a[8]);\n\
constraint array_bool_and([BOOL____00058, BOOL____00059], a[13]);\n\
constraint array_bool_and([BOOL____00061, BOOL____00062], a[18]);\n\
constraint array_bool_and([BOOL____00070, BOOL____00071], BOOL____00072) :: defines_var(BOOL____00072);\n\
constraint array_bool_and([BOOL____00074, BOOL____00075], BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint array_bool_and([BOOL____00076, BOOL____00077], BOOL____00079) :: defines_var(BOOL____00079);\n\
constraint array_bool_and([BOOL____00081, BOOL____00082], BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint array_bool_and([BOOL____00083, BOOL____00084], BOOL____00086) :: defines_var(BOOL____00086);\n\
constraint array_bool_and([BOOL____00094, BOOL____00095], a[11]);\n\
constraint array_bool_and([BOOL____00097, BOOL____00098], a[16]);\n\
constraint array_bool_and([BOOL____00104, BOOL____00105], a[23]);\n\
constraint array_bool_and([BOOL____00107, BOOL____00108], a[24]);\n\
constraint array_bool_and([BOOL____00115, BOOL____00116], BOOL____00119) :: defines_var(BOOL____00119);\n\
constraint array_bool_and([BOOL____00117, BOOL____00118], BOOL____00120) :: defines_var(BOOL____00120);\n\
constraint array_bool_and([BOOL____00122, BOOL____00123], BOOL____00126) :: defines_var(BOOL____00126);\n\
constraint array_bool_and([BOOL____00124, BOOL____00125], BOOL____00127) :: defines_var(BOOL____00127);\n\
constraint array_bool_and([BOOL____00129, BOOL____00130], BOOL____00133) :: defines_var(BOOL____00133);\n\
constraint array_bool_and([BOOL____00131, BOOL____00132], BOOL____00134) :: defines_var(BOOL____00134);\n\
constraint array_bool_and([BOOL____00146, BOOL____00147], BOOL____00150) :: defines_var(BOOL____00150);\n\
constraint array_bool_and([BOOL____00148, BOOL____00149], BOOL____00151) :: defines_var(BOOL____00151);\n\
constraint array_bool_and([BOOL____00153, BOOL____00154], BOOL____00157) :: defines_var(BOOL____00157);\n\
constraint array_bool_and([BOOL____00155, BOOL____00156], BOOL____00158) :: defines_var(BOOL____00158);\n\
constraint array_bool_and([BOOL____00170, BOOL____00171], BOOL____00174) :: defines_var(BOOL____00174);\n\
constraint array_bool_and([BOOL____00172, BOOL____00173], BOOL____00175) :: defines_var(BOOL____00175);\n\
constraint array_bool_and([BOOL____00177, BOOL____00178], BOOL____00181) :: defines_var(BOOL____00181);\n\
constraint array_bool_and([BOOL____00179, BOOL____00180], BOOL____00182) :: defines_var(BOOL____00182);\n\
constraint array_bool_or([BOOL____00003, BOOL____00002], a[5]);\n\
constraint array_bool_or([BOOL____00005, BOOL____00008], a[10]);\n\
constraint array_bool_or([BOOL____00014, BOOL____00015], a[15]);\n\
constraint array_bool_or([BOOL____00021, BOOL____00022], a[20]);\n\
constraint array_bool_or([BOOL____00025, BOOL____00024], a[25]);\n\
constraint array_bool_or([BOOL____00029, BOOL____00028], a[4]);\n\
constraint array_bool_or([BOOL____00031, BOOL____00034], a[9]);\n\
constraint array_bool_or([BOOL____00040, BOOL____00041], a[14]);\n\
constraint array_bool_or([BOOL____00047, BOOL____00048], a[19]);\n\
constraint array_bool_or([BOOL____00051, BOOL____00050], a[24]);\n\
constraint array_bool_or([BOOL____00067, BOOL____00066], a[2]);\n\
constraint array_bool_or([BOOL____00069, BOOL____00072], a[7]);\n\
constraint array_bool_or([BOOL____00078, BOOL____00079], a[12]);\n\
constraint array_bool_or([BOOL____00085, BOOL____00086], a[17]);\n\
constraint array_bool_or([BOOL____00089, BOOL____00088], a[22]);\n\
constraint array_bool_or([BOOL____00113, BOOL____00112], a[16]);\n\
constraint array_bool_or([BOOL____00119, BOOL____00120], a[17]);\n\
constraint array_bool_or([BOOL____00126, BOOL____00127], a[18]);\n\
constraint array_bool_or([BOOL____00133, BOOL____00134], a[19]);\n")+"\
constraint array_bool_or([BOOL____00137, BOOL____00136], a[20]);\n\
constraint array_bool_or([BOOL____00141, BOOL____00140], a[6]);\n\
constraint array_bool_or([BOOL____00144, BOOL____00143], a[7]);\n\
constraint array_bool_or([BOOL____00150, BOOL____00151], a[8]);\n\
constraint array_bool_or([BOOL____00157, BOOL____00158], a[9]);\n\
constraint array_bool_or([BOOL____00161, BOOL____00160], a[10]);\n\
constraint array_bool_or([BOOL____00165, BOOL____00164], a[1]);\n\
constraint array_bool_or([BOOL____00168, BOOL____00167], a[2]);\n\
constraint array_bool_or([BOOL____00174, BOOL____00175], a[3]);\n\
constraint array_bool_or([BOOL____00181, BOOL____00182], a[4]);\n\
constraint array_bool_or([BOOL____00185, BOOL____00184], a[5]);\n\
constraint bool_eq(a[11], false);\n\
constraint bool_eq(a[12], false);\n\
constraint bool_eq(a[13], false);\n\
constraint bool_eq(a[14], false);\n\
constraint bool_eq(a[15], false);\n\
constraint int_le_reif(s____00001[1], 1, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_le_reif(s____00001[1], 2, BOOL____00005) :: defines_var(BOOL____00005);\n\
constraint int_le_reif(s____00001[1], 3, BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_le_reif(s____00001[1], 4, BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_le_reif(s____00001[2], 1, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_le_reif(s____00001[2], 2, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_le_reif(s____00001[2], 3, BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint int_le_reif(s____00001[2], 4, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_le_reif(s____00027[1], 1, BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint int_le_reif(s____00027[1], 2, BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_le_reif(s____00027[1], 3, BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_le_reif(s____00027[1], 4, BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_le_reif(s____00027[2], 1, BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_le_reif(s____00027[2], 2, BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint int_le_reif(s____00027[2], 3, BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_le_reif(s____00027[2], 4, BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_le_reif(s____00053[1], 1, a[3]);\n\
constraint int_le_reif(s____00053[1], 2, BOOL____00055) :: defines_var(BOOL____00055);\n\
constraint int_le_reif(s____00053[1], 3, BOOL____00058) :: defines_var(BOOL____00058);\n\
constraint int_le_reif(s____00053[1], 4, BOOL____00061) :: defines_var(BOOL____00061);\n\
constraint int_le_reif(s____00065[1], 1, BOOL____00066) :: defines_var(BOOL____00066);\n\
constraint int_le_reif(s____00065[1], 2, BOOL____00069) :: defines_var(BOOL____00069);\n\
constraint int_le_reif(s____00065[1], 3, BOOL____00074) :: defines_var(BOOL____00074);\n\
constraint int_le_reif(s____00065[1], 4, BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint int_le_reif(s____00065[2], 1, BOOL____00067) :: defines_var(BOOL____00067);\n\
constraint int_le_reif(s____00065[2], 2, BOOL____00070) :: defines_var(BOOL____00070);\n\
constraint int_le_reif(s____00065[2], 3, BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_le_reif(s____00065[2], 4, BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint int_le_reif(s____00091[1], 1, a[1]);\n\
constraint int_le_reif(s____00091[1], 2, a[6]);\n\
constraint int_le_reif(s____00091[1], 3, BOOL____00094) :: defines_var(BOOL____00094);\n\
constraint int_le_reif(s____00091[1], 4, BOOL____00097) :: defines_var(BOOL____00097);\n\
constraint int_le_reif(s____00101[1], 1, a[21]);\n\
constraint int_le_reif(s____00101[1], 2, a[22]);\n\
constraint int_le_reif(s____00101[1], 3, BOOL____00104) :: defines_var(BOOL____00104);\n\
constraint int_le_reif(s____00101[1], 4, BOOL____00107) :: defines_var(BOOL____00107);\n\
constraint int_le_reif(s____00111[1], 1, BOOL____00112) :: defines_var(BOOL____00112);\n\
constraint int_le_reif(s____00111[1], 2, BOOL____00115) :: defines_var(BOOL____00115);\n\
constraint int_le_reif(s____00111[1], 3, BOOL____00122) :: defines_var(BOOL____00122);\n\
constraint int_le_reif(s____00111[1], 4, BOOL____00129) :: defines_var(BOOL____00129);\n\
constraint int_le_reif(s____00111[2], 1, BOOL____00113) :: defines_var(BOOL____00113);\n\
constraint int_le_reif(s____00111[2], 2, BOOL____00117) :: defines_var(BOOL____00117);\n\
constraint int_le_reif(s____00111[2], 3, BOOL____00124) :: defines_var(BOOL____00124);\n\
constraint int_le_reif(s____00111[2], 4, BOOL____00131) :: defines_var(BOOL____00131);\n\
constraint int_le_reif(s____00139[1], 1, BOOL____00140) :: defines_var(BOOL____00140);\n\
constraint int_le_reif(s____00139[1], 2, BOOL____00143) :: defines_var(BOOL____00143);\n\
constraint int_le_reif(s____00139[1], 3, BOOL____00146) :: defines_var(BOOL____00146);\n\
constraint int_le_reif(s____00139[1], 4, BOOL____00153) :: defines_var(BOOL____00153);\n\
constraint int_le_reif(s____00139[2], 1, BOOL____00141) :: defines_var(BOOL____00141);\n\
constraint int_le_reif(s____00139[2], 2, BOOL____00144) :: defines_var(BOOL____00144);\n\
constraint int_le_reif(s____00139[2], 3, BOOL____00148) :: defines_var(BOOL____00148);\n\
constraint int_le_reif(s____00139[2], 4, BOOL____00155) :: defines_var(BOOL____00155);\n\
constraint int_le_reif(s____00163[1], 1, BOOL____00164) :: defines_var(BOOL____00164);\n\
constraint int_le_reif(s____00163[1], 2, BOOL____00167) :: defines_var(BOOL____00167);\n\
constraint int_le_reif(s____00163[1], 3, BOOL____00170) :: defines_var(BOOL____00170);\n\
constraint int_le_reif(s____00163[1], 4, BOOL____00177) :: defines_var(BOOL____00177);\n\
constraint int_le_reif(s____00163[2], 1, BOOL____00165) :: defines_var(BOOL____00165);\n\
constraint int_le_reif(s____00163[2], 2, BOOL____00168) :: defines_var(BOOL____00168);\n\
constraint int_le_reif(s____00163[2], 3, BOOL____00172) :: defines_var(BOOL____00172);\n\
constraint int_le_reif(s____00163[2], 4, BOOL____00179) :: defines_var(BOOL____00179);\n\
constraint int_lin_le([1], [s____00091[1]], 4);\n\
constraint int_lin_le([1], [s____00101[1]], 4);\n\
constraint int_lin_le([1], [s____00139[2]], 4);\n\
constraint int_lin_le([1], [s____00163[2]], 4);\n\
constraint int_lin_le([1, -1], [s____00001[1], s____00001[2]], -3);\n\
constraint int_lin_le([1, -1], [s____00027[1], s____00027[2]], -3);\n\
constraint int_lin_le([1, -1], [s____00065[1], s____00065[2]], -3);\n\
constraint int_lin_le([1, -1], [s____00111[1], s____00111[2]], -2);\n\
constraint int_lin_le([1, -1], [s____00139[1], s____00139[2]], -3);\n\
constraint int_lin_le([1, -1], [s____00163[1], s____00163[2]], -3);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -4, BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -3, BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -2, BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -5, BOOL____00025) :: defines_var(BOOL____00025);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -4, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -3, BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -2, BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_lin_le_reif([-1], [s____00027[1]], -4, BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_lin_le_reif([-1], [s____00027[1]], -3, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_lin_le_reif([-1], [s____00027[1]], -2, BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_lin_le_reif([-1], [s____00027[2]], -5, BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_lin_le_reif([-1], [s____00027[2]], -4, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_lin_le_reif([-1], [s____00027[2]], -3, BOOL____00039) :: defines_var(BOOL____00039);\n\
constraint int_lin_le_reif([-1], [s____00027[2]], -2, BOOL____00033) :: defines_var(BOOL____00033);\n\
constraint int_lin_le_reif([-1], [s____00053[1]], -5, a[23]);\n\
constraint int_lin_le_reif([-1], [s____00053[1]], -4, BOOL____00062) :: defines_var(BOOL____00062);\n\
constraint int_lin_le_reif([-1], [s____00053[1]], -3, BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint int_lin_le_reif([-1], [s____00053[1]], -2, BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint int_lin_le_reif([-1], [s____00065[1]], -4, BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint int_lin_le_reif([-1], [s____00065[1]], -3, BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint int_lin_le_reif([-1], [s____00065[1]], -2, BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint int_lin_le_reif([-1], [s____00065[2]], -5, BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint int_lin_le_reif([-1], [s____00065[2]], -4, BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint int_lin_le_reif([-1], [s____00065[2]], -3, BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_lin_le_reif([-1], [s____00065[2]], -2, BOOL____00071) :: defines_var(BOOL____00071);\n\
constraint int_lin_le_reif([-1], [s____00091[1]], -4, a[21]);\n\
constraint int_lin_le_reif([-1], [s____00091[1]], -3, BOOL____00098) :: defines_var(BOOL____00098);\n\
constraint int_lin_le_reif([-1], [s____00091[1]], -2, BOOL____00095) :: defines_var(BOOL____00095);\n\
constraint int_lin_le_reif([-1], [s____00101[1]], -4, a[25]);\n\
constraint int_lin_le_reif([-1], [s____00101[1]], -3, BOOL____00108) :: defines_var(BOOL____00108);\n\
constraint int_lin_le_reif([-1], [s____00101[1]], -2, BOOL____00105) :: defines_var(BOOL____00105);\n\
constraint int_lin_le_reif([-1], [s____00111[1]], -5, BOOL____00136) :: defines_var(BOOL____00136);\n\
constraint int_lin_le_reif([-1], [s____00111[1]], -4, BOOL____00130) :: defines_var(BOOL____00130);\n\
constraint int_lin_le_reif([-1], [s____00111[1]], -3, BOOL____00123) :: defines_var(BOOL____00123);\n\
constraint int_lin_le_reif([-1], [s____00111[1]], -2, BOOL____00116) :: defines_var(BOOL____00116);\n\
constraint int_lin_le_reif([-1], [s____00111[2]], -5, BOOL____00137) :: defines_var(BOOL____00137);\n\
constraint int_lin_le_reif([-1], [s____00111[2]], -4, BOOL____00132) :: defines_var(BOOL____00132);\n\
constraint int_lin_le_reif([-1], [s____00111[2]], -3, BOOL____00125) :: defines_var(BOOL____00125);\n\
constraint int_lin_le_reif([-1], [s____00111[2]], -2, BOOL____00118) :: defines_var(BOOL____00118);\n\
constraint int_lin_le_reif([-1], [s____00139[1]], -4, BOOL____00160) :: defines_var(BOOL____00160);\n\
constraint int_lin_le_reif([-1], [s____00139[1]], -3, BOOL____00154) :: defines_var(BOOL____00154);\n\
constraint int_lin_le_reif([-1], [s____00139[1]], -2, BOOL____00147) :: defines_var(BOOL____00147);\n\
constraint int_lin_le_reif([-1], [s____00139[2]], -4, BOOL____00161) :: defines_var(BOOL____00161);\n\
constraint int_lin_le_reif([-1], [s____00139[2]], -3, BOOL____00156) :: defines_var(BOOL____00156);\n\
constraint int_lin_le_reif([-1], [s____00139[2]], -2, BOOL____00149) :: defines_var(BOOL____00149);\n\
constraint int_lin_le_reif([-1], [s____00163[1]], -4, BOOL____00184) :: defines_var(BOOL____00184);\n\
constraint int_lin_le_reif([-1], [s____00163[1]], -3, BOOL____00178) :: defines_var(BOOL____00178);\n\
constraint int_lin_le_reif([-1], [s____00163[1]], -2, BOOL____00171) :: defines_var(BOOL____00171);\n\
constraint int_lin_le_reif([-1], [s____00163[2]], -4, BOOL____00185) :: defines_var(BOOL____00185);\n\
constraint int_lin_le_reif([-1], [s____00163[2]], -3, BOOL____00180) :: defines_var(BOOL____00180);\n\
constraint int_lin_le_reif([-1], [s____00163[2]], -2, BOOL____00173) :: defines_var(BOOL____00173);\n\
solve satisfy;\n\
", "a = array2d(1..5, 1..5, [true, true, false, true, true, true, true, false, true, true, false, false, false, false, false, false, true, false, false, true, false, false, true, true, false]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
