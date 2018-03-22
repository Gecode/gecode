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
        (void) new FlatZincTest("tenpenki::1",
std::string("predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var bool: BOOL____00002 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00003 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00004 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00006 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00007 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00008 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00009 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00010 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00011 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00012 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00013 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00014 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00016 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00017 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00018 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00019 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00020 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00021 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00022 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00023 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00024 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00026 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00027 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00028 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00029 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00030 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00031 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00032 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00033 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00034 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00036 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00037 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00038 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00041 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00042 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00044 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00045 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00049 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00052 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00053 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00054 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00056 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00057 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00058 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00059 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00060 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00061 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00063 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00064 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00072 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00073 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00075 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00076 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00077 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00078 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00080 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00081 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00082 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00083 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00084 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00085 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00087 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00088 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00089 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00090 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00091 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00092 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00094 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00095 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00098 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00099 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00100 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00102 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00103 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00104 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00105 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00106 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00107 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00108 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00109 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00110 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00112 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00113 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00114 :: is_defined_var :: var_is_introduced;\n\
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
var bool: BOOL____00128 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00129 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00130 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00132 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00133 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00134 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00137 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00138 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00140 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00141 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00142 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00143 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00145 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00146 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00147 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00148 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00150 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00151 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00152 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00153 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00154 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00155 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00157 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00158 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00163 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00164 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00166 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00167 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00178 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00179 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00181 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00182 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00186 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00187 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00189 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00190 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00191 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00192 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00194 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00195 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00196 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00197 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00199 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00200 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00201 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00202 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00203 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00204 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00206 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00207 :: is_defined_var :: var_is_introduced;\n\
array [1..25] of var bool: a :: output_array([1..5, 1..5]);\n\
array [1..3] of var 1..5: s____00001;\n\
array [1..2] of var 1..5: s____00040;\n\
array [1..1] of var 1..5: s____00066;\n\
array [1..2] of var 1..5: s____00071;\n\
array [1..3] of var 1..5: s____00097;\n\
array [1..2] of var 1..5: s____00136;\n\
array [1..1] of var 1..5: s____00160;\n\
array [1..1] of var 1..5: s____00170;\n\
array [1..1] of var 1..5: s____00175;\n\
array [1..2] of var 1..5: s____00185;\n\
constraint array_bool_and([BOOL____00006, BOOL____00007], BOOL____00012) :: defines_var(BOOL____00012);\n\
constraint array_bool_and([BOOL____00008, BOOL____00009], BOOL____00013) :: defines_var(BOOL____00013);\n\
constraint array_bool_and([BOOL____00010, BOOL____00011], BOOL____00014) :: defines_var(BOOL____00014);\n\
constraint array_bool_and([BOOL____00016, BOOL____00017], BOOL____00022) :: defines_var(BOOL____00022);\n\
constraint array_bool_and([BOOL____00018, BOOL____00019], BOOL____00023) :: defines_var(BOOL____00023);\n\
constraint array_bool_and([BOOL____00020, BOOL____00021], BOOL____00024) :: defines_var(BOOL____00024);\n\
constraint array_bool_and([BOOL____00026, BOOL____00027], BOOL____00032) :: defines_var(BOOL____00032);\n\
constraint array_bool_and([BOOL____00028, BOOL____00029], BOOL____00033) :: defines_var(BOOL____00033);\n\
constraint array_bool_and([BOOL____00030, BOOL____00031], BOOL____00034) :: defines_var(BOOL____00034);\n\
constraint array_bool_and([BOOL____00044, BOOL____00045], BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint array_bool_and([BOOL____00049, BOOL____00050], BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint array_bool_and([BOOL____00051, BOOL____00052], BOOL____00054) :: defines_var(BOOL____00054);\n\
constraint array_bool_and([BOOL____00056, BOOL____00057], BOOL____00060) :: defines_var(BOOL____00060);\n\
constraint array_bool_and([BOOL____00058, BOOL____00059], BOOL____00061) :: defines_var(BOOL____00061);\n\
constraint array_bool_and([BOOL____00076, BOOL____00077], BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint array_bool_and([BOOL____00080, BOOL____00081], BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint array_bool_and([BOOL____00082, BOOL____00083], BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint array_bool_and([BOOL____00087, BOOL____00088], BOOL____00091) :: defines_var(BOOL____00091);\n\
constraint array_bool_and([BOOL____00089, BOOL____00090], BOOL____00092) :: defines_var(BOOL____00092);\n\
constraint array_bool_and([BOOL____00102, BOOL____00103], BOOL____00108) :: defines_var(BOOL____00108);\n\
constraint array_bool_and([BOOL____00104, BOOL____00105], BOOL____00109) :: defines_var(BOOL____00109);\n\
constraint array_bool_and([BOOL____00106, BOOL____00107], BOOL____00110) :: defines_var(BOOL____00110);\n\
constraint array_bool_and([BOOL____00112, BOOL____00113], BOOL____00118) :: defines_var(BOOL____00118);\n\
constraint array_bool_and([BOOL____00114, BOOL____00115], BOOL____00119) :: defines_var(BOOL____00119);\n\
constraint array_bool_and([BOOL____00116, BOOL____00117], BOOL____00120) :: defines_var(BOOL____00120);\n\
constraint array_bool_and([BOOL____00122, BOOL____00123], BOOL____00128) :: defines_var(BOOL____00128);\n\
constraint array_bool_and([BOOL____00124, BOOL____00125], BOOL____00129) :: defines_var(BOOL____00129);\n\
constraint array_bool_and([BOOL____00126, BOOL____00127], BOOL____00130) :: defines_var(BOOL____00130);\n\
constraint array_bool_and([BOOL____00141, BOOL____00142], BOOL____00143) :: defines_var(BOOL____00143);\n\
constraint array_bool_and([BOOL____00146, BOOL____00147], BOOL____00148) :: defines_var(BOOL____00148);\n\
constraint array_bool_and([BOOL____00150, BOOL____00151], BOOL____00154) :: defines_var(BOOL____00154);\n\
constraint array_bool_and([BOOL____00152, BOOL____00153], BOOL____00155) :: defines_var(BOOL____00155);\n\
constraint array_bool_and([BOOL____00163, BOOL____00164], a[18]);\n\
constraint array_bool_and([BOOL____00166, BOOL____00167], a[19]);\n\
constraint array_bool_and([BOOL____00178, BOOL____00179], a[8]);\n\
constraint array_bool_and([BOOL____00181, BOOL____00182], a[9]);\n\
constraint array_bool_and([BOOL____00189, BOOL____00190], BOOL____00192) :: defines_var(BOOL____00192);\n\
constraint array_bool_and([BOOL____00194, BOOL____00195], BOOL____00197) :: defines_var(BOOL____00197);\n\
constraint array_bool_and([BOOL____00199, BOOL____00200], BOOL____00203) :: defines_var(BOOL____00203);\n\
constraint array_bool_and([BOOL____00201, BOOL____00202], BOOL____00204) :: defines_var(BOOL____00204);\n\
constraint array_bool_or([BOOL____00042, BOOL____00041], a[4]);\n\
constraint array_bool_or([BOOL____00046, BOOL____00047], a[9]);\n\
constraint array_bool_or([BOOL____00053, BOOL____00054], a[14]);\n\
constraint array_bool_or([BOOL____00060, BOOL____00061], a[19]);\n\
constraint array_bool_or([BOOL____00064, BOOL____00063], a[24]);\n\
constraint array_bool_or([BOOL____00073, BOOL____00072], a[2]);\n\
constraint array_bool_or([BOOL____00075, BOOL____00078], a[7]);\n\
constraint array_bool_or([BOOL____00084, BOOL____00085], a[12]);\n\
constraint array_bool_or([BOOL____00091, BOOL____00092], a[17]);\n\
constraint array_bool_or([BOOL____00095, BOOL____00094], a[22]);\n\
constraint array_bool_or([BOOL____00138, BOOL____00137], a[21]);\n\
constraint array_bool_or([BOOL____00140, BOOL____00143], a[22]);\n\
constraint array_bool_or([BOOL____00145, BOOL____00148], a[23]);\n\
constraint array_bool_or([BOOL____00154, BOOL____00155], a[24]);\n\
constraint array_bool_or([BOOL____00158, BOOL____00157], a[25]);\n\
constraint array_bool_or([BOOL____00187, BOOL____00186], a[1]);\n\
constraint array_bool_or([BOOL____00191, BOOL____00192], a[2]);\n\
constraint array_bool_or([BOOL____00196, BOOL____00197], a[3]);\n\
constraint array_bool_or([BOOL____00203, BOOL____00204], a[4]);\n\
constraint array_bool_or([BOOL____00207, BOOL____00206], a[5]);\n\
constraint array_bool_or([BOOL____00004, BOOL____00003, BOOL____00002], a[5]);\n\
constraint array_bool_or([BOOL____00014, BOOL____00012, BOOL____00013], a[10]);\n\
constraint array_bool_or([BOOL____00024, BOOL____00022, BOOL____00023], a[15]);\n\
constraint array_bool_or([BOOL____00034, BOOL____00032, BOOL____00033], a[20]);\n\
constraint array_bool_or([BOOL____00038, BOOL____00037, BOOL____00036], a[25]);\n\
constraint array_bool_or([BOOL____00100, BOOL____00099, BOOL____00098], a[1]);\n")+"\
constraint array_bool_or([BOOL____00110, BOOL____00108, BOOL____00109], a[6]);\n\
constraint array_bool_or([BOOL____00120, BOOL____00118, BOOL____00119], a[11]);\n\
constraint array_bool_or([BOOL____00130, BOOL____00128, BOOL____00129], a[16]);\n\
constraint array_bool_or([BOOL____00134, BOOL____00133, BOOL____00132], a[21]);\n\
constraint bool_eq(a[15], true);\n\
constraint bool_eq(a[23], true);\n\
constraint int_le_reif(s____00001[1], 1, BOOL____00002) :: defines_var(BOOL____00002);\n\
constraint int_le_reif(s____00001[1], 2, BOOL____00006) :: defines_var(BOOL____00006);\n\
constraint int_le_reif(s____00001[1], 3, BOOL____00016) :: defines_var(BOOL____00016);\n\
constraint int_le_reif(s____00001[1], 4, BOOL____00026) :: defines_var(BOOL____00026);\n\
constraint int_le_reif(s____00001[2], 1, BOOL____00003) :: defines_var(BOOL____00003);\n\
constraint int_le_reif(s____00001[2], 2, BOOL____00008) :: defines_var(BOOL____00008);\n\
constraint int_le_reif(s____00001[2], 3, BOOL____00018) :: defines_var(BOOL____00018);\n\
constraint int_le_reif(s____00001[2], 4, BOOL____00028) :: defines_var(BOOL____00028);\n\
constraint int_le_reif(s____00001[3], 1, BOOL____00004) :: defines_var(BOOL____00004);\n\
constraint int_le_reif(s____00001[3], 2, BOOL____00010) :: defines_var(BOOL____00010);\n\
constraint int_le_reif(s____00001[3], 3, BOOL____00020) :: defines_var(BOOL____00020);\n\
constraint int_le_reif(s____00001[3], 4, BOOL____00030) :: defines_var(BOOL____00030);\n\
constraint int_le_reif(s____00040[1], 1, BOOL____00041) :: defines_var(BOOL____00041);\n\
constraint int_le_reif(s____00040[1], 2, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_le_reif(s____00040[1], 3, BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint int_le_reif(s____00040[1], 4, BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint int_le_reif(s____00040[2], 1, BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_le_reif(s____00040[2], 2, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_le_reif(s____00040[2], 3, BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_le_reif(s____00040[2], 4, BOOL____00058) :: defines_var(BOOL____00058);\n\
constraint int_le_reif(s____00066[1], 1, a[3]);\n\
constraint int_le_reif(s____00066[1], 2, a[8]);\n\
constraint int_le_reif(s____00066[1], 3, a[13]);\n\
constraint int_le_reif(s____00066[1], 4, a[18]);\n\
constraint int_le_reif(s____00071[1], 1, BOOL____00072) :: defines_var(BOOL____00072);\n\
constraint int_le_reif(s____00071[1], 2, BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint int_le_reif(s____00071[1], 3, BOOL____00080) :: defines_var(BOOL____00080);\n\
constraint int_le_reif(s____00071[1], 4, BOOL____00087) :: defines_var(BOOL____00087);\n\
constraint int_le_reif(s____00071[2], 1, BOOL____00073) :: defines_var(BOOL____00073);\n\
constraint int_le_reif(s____00071[2], 2, BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_le_reif(s____00071[2], 3, BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint int_le_reif(s____00071[2], 4, BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint int_le_reif(s____00097[1], 1, BOOL____00098) :: defines_var(BOOL____00098);\n\
constraint int_le_reif(s____00097[1], 2, BOOL____00102) :: defines_var(BOOL____00102);\n\
constraint int_le_reif(s____00097[1], 3, BOOL____00112) :: defines_var(BOOL____00112);\n\
constraint int_le_reif(s____00097[1], 4, BOOL____00122) :: defines_var(BOOL____00122);\n\
constraint int_le_reif(s____00097[2], 1, BOOL____00099) :: defines_var(BOOL____00099);\n\
constraint int_le_reif(s____00097[2], 2, BOOL____00104) :: defines_var(BOOL____00104);\n\
constraint int_le_reif(s____00097[2], 3, BOOL____00114) :: defines_var(BOOL____00114);\n\
constraint int_le_reif(s____00097[2], 4, BOOL____00124) :: defines_var(BOOL____00124);\n\
constraint int_le_reif(s____00097[3], 1, BOOL____00100) :: defines_var(BOOL____00100);\n\
constraint int_le_reif(s____00097[3], 2, BOOL____00106) :: defines_var(BOOL____00106);\n\
constraint int_le_reif(s____00097[3], 3, BOOL____00116) :: defines_var(BOOL____00116);\n\
constraint int_le_reif(s____00097[3], 4, BOOL____00126) :: defines_var(BOOL____00126);\n\
constraint int_le_reif(s____00136[1], 1, BOOL____00137) :: defines_var(BOOL____00137);\n\
constraint int_le_reif(s____00136[1], 2, BOOL____00140) :: defines_var(BOOL____00140);\n\
constraint int_le_reif(s____00136[1], 3, BOOL____00145) :: defines_var(BOOL____00145);\n\
constraint int_le_reif(s____00136[1], 4, BOOL____00150) :: defines_var(BOOL____00150);\n\
constraint int_le_reif(s____00136[2], 1, BOOL____00138) :: defines_var(BOOL____00138);\n\
constraint int_le_reif(s____00136[2], 2, BOOL____00141) :: defines_var(BOOL____00141);\n\
constraint int_le_reif(s____00136[2], 3, BOOL____00146) :: defines_var(BOOL____00146);\n\
constraint int_le_reif(s____00136[2], 4, BOOL____00152) :: defines_var(BOOL____00152);\n\
constraint int_le_reif(s____00160[1], 1, a[16]);\n\
constraint int_le_reif(s____00160[1], 2, a[17]);\n\
constraint int_le_reif(s____00160[1], 3, BOOL____00163) :: defines_var(BOOL____00163);\n\
constraint int_le_reif(s____00160[1], 4, BOOL____00166) :: defines_var(BOOL____00166);\n\
constraint int_le_reif(s____00170[1], 1, a[11]);\n\
constraint int_le_reif(s____00170[1], 2, a[12]);\n\
constraint int_le_reif(s____00170[1], 3, a[13]);\n\
constraint int_le_reif(s____00170[1], 4, a[14]);\n\
constraint int_le_reif(s____00175[1], 1, a[6]);\n\
constraint int_le_reif(s____00175[1], 2, a[7]);\n\
constraint int_le_reif(s____00175[1], 3, BOOL____00178) :: defines_var(BOOL____00178);\n\
constraint int_le_reif(s____00175[1], 4, BOOL____00181) :: defines_var(BOOL____00181);\n\
constraint int_le_reif(s____00185[1], 1, BOOL____00186) :: defines_var(BOOL____00186);\n\
constraint int_le_reif(s____00185[1], 2, BOOL____00189) :: defines_var(BOOL____00189);\n\
constraint int_le_reif(s____00185[1], 3, BOOL____00194) :: defines_var(BOOL____00194);\n\
constraint int_le_reif(s____00185[1], 4, BOOL____00199) :: defines_var(BOOL____00199);\n\
constraint int_le_reif(s____00185[2], 1, BOOL____00187) :: defines_var(BOOL____00187);\n\
constraint int_le_reif(s____00185[2], 2, BOOL____00191) :: defines_var(BOOL____00191);\n\
constraint int_le_reif(s____00185[2], 3, BOOL____00196) :: defines_var(BOOL____00196);\n\
constraint int_le_reif(s____00185[2], 4, BOOL____00201) :: defines_var(BOOL____00201);\n\
constraint int_lin_le([1], [s____00040[2]], 4);\n\
constraint int_lin_le([1], [s____00066[1]], 1);\n\
constraint int_lin_le([1], [s____00160[1]], 4);\n\
constraint int_lin_le([1], [s____00170[1]], 1);\n\
constraint int_lin_le([1], [s____00175[1]], 4);\n\
constraint int_lin_le([1], [s____00185[2]], 3);\n\
constraint int_lin_le([1, -1], [s____00001[1], s____00001[2]], -2);\n\
constraint int_lin_le([1, -1], [s____00001[2], s____00001[3]], -2);\n\
constraint int_lin_le([1, -1], [s____00040[1], s____00040[2]], -2);\n\
constraint int_lin_le([1, -1], [s____00071[1], s____00071[2]], -3);\n\
constraint int_lin_le([1, -1], [s____00097[1], s____00097[2]], -2);\n\
constraint int_lin_le([1, -1], [s____00097[2], s____00097[3]], -2);\n\
constraint int_lin_le([1, -1], [s____00136[1], s____00136[2]], -4);\n\
constraint int_lin_le([1, -1], [s____00185[1], s____00185[2]], -2);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -5, BOOL____00036) :: defines_var(BOOL____00036);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -4, BOOL____00027) :: defines_var(BOOL____00027);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -3, BOOL____00017) :: defines_var(BOOL____00017);\n\
constraint int_lin_le_reif([-1], [s____00001[1]], -2, BOOL____00007) :: defines_var(BOOL____00007);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -5, BOOL____00037) :: defines_var(BOOL____00037);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -4, BOOL____00029) :: defines_var(BOOL____00029);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -3, BOOL____00019) :: defines_var(BOOL____00019);\n\
constraint int_lin_le_reif([-1], [s____00001[2]], -2, BOOL____00009) :: defines_var(BOOL____00009);\n\
constraint int_lin_le_reif([-1], [s____00001[3]], -5, BOOL____00038) :: defines_var(BOOL____00038);\n\
constraint int_lin_le_reif([-1], [s____00001[3]], -4, BOOL____00031) :: defines_var(BOOL____00031);\n\
constraint int_lin_le_reif([-1], [s____00001[3]], -3, BOOL____00021) :: defines_var(BOOL____00021);\n\
constraint int_lin_le_reif([-1], [s____00001[3]], -2, BOOL____00011) :: defines_var(BOOL____00011);\n\
constraint int_lin_le_reif([-1], [s____00040[1]], -5, BOOL____00063) :: defines_var(BOOL____00063);\n\
constraint int_lin_le_reif([-1], [s____00040[1]], -4, BOOL____00057) :: defines_var(BOOL____00057);\n\
constraint int_lin_le_reif([-1], [s____00040[1]], -3, BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_lin_le_reif([-1], [s____00040[1]], -2, BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_lin_le_reif([-1], [s____00040[2]], -4, BOOL____00064) :: defines_var(BOOL____00064);\n\
constraint int_lin_le_reif([-1], [s____00040[2]], -3, BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint int_lin_le_reif([-1], [s____00040[2]], -2, BOOL____00052) :: defines_var(BOOL____00052);\n\
constraint int_lin_le_reif([-1], [s____00071[1]], -4, BOOL____00094) :: defines_var(BOOL____00094);\n\
constraint int_lin_le_reif([-1], [s____00071[1]], -3, BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint int_lin_le_reif([-1], [s____00071[1]], -2, BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint int_lin_le_reif([-1], [s____00071[2]], -5, BOOL____00095) :: defines_var(BOOL____00095);\n\
constraint int_lin_le_reif([-1], [s____00071[2]], -4, BOOL____00090) :: defines_var(BOOL____00090);\n\
constraint int_lin_le_reif([-1], [s____00071[2]], -3, BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint int_lin_le_reif([-1], [s____00071[2]], -2, BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_lin_le_reif([-1], [s____00097[1]], -5, BOOL____00132) :: defines_var(BOOL____00132);\n\
constraint int_lin_le_reif([-1], [s____00097[1]], -4, BOOL____00123) :: defines_var(BOOL____00123);\n\
constraint int_lin_le_reif([-1], [s____00097[1]], -3, BOOL____00113) :: defines_var(BOOL____00113);\n\
constraint int_lin_le_reif([-1], [s____00097[1]], -2, BOOL____00103) :: defines_var(BOOL____00103);\n\
constraint int_lin_le_reif([-1], [s____00097[2]], -5, BOOL____00133) :: defines_var(BOOL____00133);\n\
constraint int_lin_le_reif([-1], [s____00097[2]], -4, BOOL____00125) :: defines_var(BOOL____00125);\n\
constraint int_lin_le_reif([-1], [s____00097[2]], -3, BOOL____00115) :: defines_var(BOOL____00115);\n\
constraint int_lin_le_reif([-1], [s____00097[2]], -2, BOOL____00105) :: defines_var(BOOL____00105);\n\
constraint int_lin_le_reif([-1], [s____00097[3]], -5, BOOL____00134) :: defines_var(BOOL____00134);\n\
constraint int_lin_le_reif([-1], [s____00097[3]], -4, BOOL____00127) :: defines_var(BOOL____00127);\n\
constraint int_lin_le_reif([-1], [s____00097[3]], -3, BOOL____00117) :: defines_var(BOOL____00117);\n\
constraint int_lin_le_reif([-1], [s____00097[3]], -2, BOOL____00107) :: defines_var(BOOL____00107);\n\
constraint int_lin_le_reif([-1], [s____00136[1]], -3, BOOL____00157) :: defines_var(BOOL____00157);\n\
constraint int_lin_le_reif([-1], [s____00136[1]], -2, BOOL____00151) :: defines_var(BOOL____00151);\n\
constraint int_lin_le_reif([-1], [s____00136[2]], -5, BOOL____00158) :: defines_var(BOOL____00158);\n\
constraint int_lin_le_reif([-1], [s____00136[2]], -4, BOOL____00153) :: defines_var(BOOL____00153);\n\
constraint int_lin_le_reif([-1], [s____00136[2]], -3, BOOL____00147) :: defines_var(BOOL____00147);\n\
constraint int_lin_le_reif([-1], [s____00136[2]], -2, BOOL____00142) :: defines_var(BOOL____00142);\n\
constraint int_lin_le_reif([-1], [s____00160[1]], -4, a[20]);\n\
constraint int_lin_le_reif([-1], [s____00160[1]], -3, BOOL____00167) :: defines_var(BOOL____00167);\n\
constraint int_lin_le_reif([-1], [s____00160[1]], -2, BOOL____00164) :: defines_var(BOOL____00164);\n\
constraint int_lin_le_reif([-1], [s____00175[1]], -4, a[10]);\n\
constraint int_lin_le_reif([-1], [s____00175[1]], -3, BOOL____00182) :: defines_var(BOOL____00182);\n\
constraint int_lin_le_reif([-1], [s____00175[1]], -2, BOOL____00179) :: defines_var(BOOL____00179);\n\
constraint int_lin_le_reif([-1], [s____00185[1]], -5, BOOL____00206) :: defines_var(BOOL____00206);\n\
constraint int_lin_le_reif([-1], [s____00185[1]], -4, BOOL____00200) :: defines_var(BOOL____00200);\n\
constraint int_lin_le_reif([-1], [s____00185[1]], -3, BOOL____00195) :: defines_var(BOOL____00195);\n\
constraint int_lin_le_reif([-1], [s____00185[1]], -2, BOOL____00190) :: defines_var(BOOL____00190);\n\
constraint int_lin_le_reif([-1], [s____00185[2]], -3, BOOL____00207) :: defines_var(BOOL____00207);\n\
constraint int_lin_le_reif([-1], [s____00185[2]], -2, BOOL____00202) :: defines_var(BOOL____00202);\n\
solve satisfy;\n\
", "a = array2d(1..5, 1..5, [true, false, true, true, true, false, true, true, false, false, true, true, true, true, true, false, false, true, true, false, true, true, true, false, true]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
