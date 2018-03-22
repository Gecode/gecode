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
        (void) new FlatZincTest("battleships::2",
std::string("predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate count(array [int] of var int: x, var int: y, var int: c);\n\
predicate count_reif(array [int] of var int: x, var int: y, var int: c, var bool: b);\n\
array [1..3] of int: class_sizes = [3, 2, 1];\n\
var bool: BOOL____00042 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00043 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00044 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00045 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00046 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00047 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00048 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00049 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00050 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00051 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00052 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00053 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00054 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00055 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00056 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00057 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00058 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00059 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00060 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00061 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00062 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00063 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00064 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00065 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00066 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00067 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00068 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00069 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00070 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00071 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00072 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00073 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00074 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00075 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00076 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00077 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00078 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00079 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00080 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00081 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00082 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00083 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00084 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00085 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00086 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00087 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00088 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00089 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00090 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00091 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00092 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00093 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00094 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00095 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00096 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00097 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00098 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00099 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00100 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00101 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00102 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00103 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00104 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00105 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00106 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00107 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00108 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00109 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00110 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00111 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00112 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00113 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00114 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00115 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00116 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00117 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00118 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00119 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00120 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00121 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00122 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00123 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00124 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00125 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00126 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00127 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00128 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00129 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00130 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00131 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00132 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00133 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00134 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00135 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00136 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00137 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00138 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00139 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00140 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00141 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00142 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00143 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00144 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00145 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00146 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00147 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00148 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00149 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00150 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00151 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00152 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00153 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00154 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00155 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00156 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00157 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00158 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00159 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00160 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00161 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00162 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00163 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00164 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00165 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00166 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00167 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00168 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00169 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00170 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00171 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00172 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00173 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00174 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00175 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00176 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00177 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00178 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00179 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00180 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00181 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00182 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00183 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00184 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00185 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00186 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00187 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00188 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00189 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00190 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00191 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00192 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00193 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00194 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00195 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00196 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00197 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00198 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00199 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00200 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00201 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00202 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00203 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00204 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00205 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00206 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00207 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00208 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00209 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00210 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00211 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00212 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00213 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00214 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00215 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00216 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00217 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00218 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00219 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00220 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00221 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00222 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00223 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00224 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00225 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00226 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00227 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00228 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00229 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00230 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00231 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00232 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00233 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00234 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00235 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00236 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00237 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00238 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00239 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00240 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00241 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00242 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00243 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00244 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00245 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00246 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00247 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00248 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00249 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00250 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00251 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00252 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00253 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00254 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00255 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00256 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00257 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00258 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00259 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00260 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00261 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00262 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00263 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00264 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00265 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00266 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00267 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00268 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00269 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00270 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00271 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00272 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00273 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00274 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00275 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00276 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00277 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00278 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00279 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00280 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00281 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00282 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00283 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00284 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00285 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00286 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00287 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00288 :: is_defined_var :: var_is_introduced;\n")+
"var bool: BOOL____00289 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00290 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00291 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00292 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00293 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00294 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00295 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00296 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00297 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00298 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00299 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00300 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00301 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00302 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00303 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00304 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00305 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00306 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00307 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00308 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00309 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00310 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00311 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00312 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00313 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00314 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00315 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00316 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00317 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00318 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00319 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00320 :: is_defined_var :: var_is_introduced;\n\
var 0..6: INT____00357 :: is_defined_var :: var_is_introduced;\n\
var 0..6: INT____00358 :: is_defined_var :: var_is_introduced;\n\
var 0..6: INT____00359 :: is_defined_var :: var_is_introduced;\n\
var 0..6: INT____00360 :: is_defined_var :: var_is_introduced;\n\
var 0..6: INT____00361 :: is_defined_var :: var_is_introduced;\n\
var 0..3: X____00001;\n\
var 0..3: X____00002;\n\
var 1..3: X____00003;\n\
var 0..3: X____00004;\n\
var 0..3: X____00005;\n\
var 0..3: X____00006;\n\
var 0..3: X____00007;\n\
var 0..3: X____00008;\n\
var 0..3: X____00009;\n\
var 0..3: X____00010;\n\
var 0..3: X____00011;\n\
var 1..3: X____00012;\n\
var 0..3: X____00013;\n\
var 0..3: X____00014;\n\
var 0..3: X____00015;\n\
var 0..3: X____00016;\n\
var 0..3: X____00017;\n\
var 0..3: X____00018;\n\
var 0..3: X____00019;\n\
var 0..3: X____00020;\n\
var 0..3: X____00021;\n\
var 0..3: X____00022;\n\
var 0..3: X____00023;\n\
var 0..3: X____00024;\n\
var 0..3: X____00025;\n\
var 0..3: X____00026;\n\
var 0..3: X____00027;\n\
var 0..3: X____00028;\n\
var 0..3: X____00029;\n\
var 1..3: X____00030;\n\
var 0..3: X____00031;\n\
var 0..3: X____00032;\n\
var 0..3: X____00033;\n\
var 0..3: X____00034;\n\
var 0..3: X____00035;\n\
var 0..3: X____00036;\n\
var 0..6: X____00037;\n\
var 0..6: X____00038;\n\
var 0..6: X____00039;\n\
var 0..6: X____00040;\n\
var 0..6: X____00041;\n\
array [1..64] of var 0..3: a :: output_array([0..7, 0..7]) = [0, 0, 0, 0, 0, 0, 0, 0, 0, X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, 0, 0, X____00007, X____00008, X____00009, X____00010, X____00011, X____00012, 0, 0, X____00013, X____00014, X____00015, X____00016, X____00017, X____00018, 0, 0, X____00019, X____00020, X____00021, X____00022, X____00023, X____00024, 0, 0, X____00025, X____00026, X____00027, X____00028, X____00029, X____00030, 0, 0, X____00031, X____00032, X____00033, X____00034, X____00035, X____00036, 0, 0, 0, 0, 0, 0, 0, 0, 0];\n\
array [1..36] of var 0..3: a_flat = [X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009, X____00010, X____00011, X____00012, X____00013, X____00014, X____00015, X____00016, X____00017, X____00018, X____00019, X____00020, X____00021, X____00022, X____00023, X____00024, X____00025, X____00026, X____00027, X____00028, X____00029, X____00030, X____00031, X____00032, X____00033, X____00034, X____00035, X____00036];\n\
array [1..6] of var 0..6: col_sums = [2, 0, X____00037, X____00038, X____00039, X____00040];\n\
array [1..6] of var 0..6: row_sums = [2, 2, 1, 1, X____00041, 1];\n\
constraint array_bool_and([BOOL____00042, BOOL____00065], BOOL____00134) :: defines_var(BOOL____00134);\n\
constraint array_bool_and([BOOL____00044, BOOL____00068], BOOL____00136) :: defines_var(BOOL____00136);\n\
constraint array_bool_and([BOOL____00045, BOOL____00071], BOOL____00138) :: defines_var(BOOL____00138);\n\
constraint array_bool_and([BOOL____00046, BOOL____00074], BOOL____00140) :: defines_var(BOOL____00140);\n\
constraint array_bool_and([BOOL____00047, BOOL____00077], BOOL____00142) :: defines_var(BOOL____00142);\n\
constraint array_bool_and([BOOL____00049, BOOL____00055], BOOL____00056) :: defines_var(BOOL____00056);\n\
constraint array_bool_and([BOOL____00049, BOOL____00081], BOOL____00144) :: defines_var(BOOL____00144);\n\
constraint array_bool_and([BOOL____00051, BOOL____00052], BOOL____00053) :: defines_var(BOOL____00053);\n\
constraint array_bool_and([BOOL____00052, BOOL____00058], BOOL____00059) :: defines_var(BOOL____00059);\n\
constraint array_bool_and([BOOL____00052, BOOL____00084], BOOL____00146) :: defines_var(BOOL____00146);\n\
constraint array_bool_and([BOOL____00055, BOOL____00061], BOOL____00062) :: defines_var(BOOL____00062);\n\
constraint array_bool_and([BOOL____00055, BOOL____00087], BOOL____00148) :: defines_var(BOOL____00148);\n\
constraint array_bool_and([BOOL____00058, BOOL____00090], BOOL____00150) :: defines_var(BOOL____00150);\n\
constraint array_bool_and([BOOL____00061, BOOL____00093], BOOL____00152) :: defines_var(BOOL____00152);\n\
constraint array_bool_and([BOOL____00065, BOOL____00071], BOOL____00072) :: defines_var(BOOL____00072);\n"+
"constraint array_bool_and([BOOL____00065, BOOL____00097], BOOL____00154) :: defines_var(BOOL____00154);\n\
constraint array_bool_and([BOOL____00067, BOOL____00068], BOOL____00069) :: defines_var(BOOL____00069);\n\
constraint array_bool_and([BOOL____00068, BOOL____00074], BOOL____00075) :: defines_var(BOOL____00075);\n\
constraint array_bool_and([BOOL____00068, BOOL____00100], BOOL____00156) :: defines_var(BOOL____00156);\n\
constraint array_bool_and([BOOL____00071, BOOL____00077], BOOL____00078) :: defines_var(BOOL____00078);\n\
constraint array_bool_and([BOOL____00071, BOOL____00103], BOOL____00158) :: defines_var(BOOL____00158);\n\
constraint array_bool_and([BOOL____00074, BOOL____00106], BOOL____00160) :: defines_var(BOOL____00160);\n\
constraint array_bool_and([BOOL____00077, BOOL____00109], BOOL____00162) :: defines_var(BOOL____00162);\n\
constraint array_bool_and([BOOL____00081, BOOL____00087], BOOL____00088) :: defines_var(BOOL____00088);\n\
constraint array_bool_and([BOOL____00081, BOOL____00113], BOOL____00164) :: defines_var(BOOL____00164);\n\
constraint array_bool_and([BOOL____00083, BOOL____00084], BOOL____00085) :: defines_var(BOOL____00085);\n\
constraint array_bool_and([BOOL____00084, BOOL____00090], BOOL____00091) :: defines_var(BOOL____00091);\n\
constraint array_bool_and([BOOL____00084, BOOL____00116], BOOL____00166) :: defines_var(BOOL____00166);\n\
constraint array_bool_and([BOOL____00087, BOOL____00093], BOOL____00094) :: defines_var(BOOL____00094);\n\
constraint array_bool_and([BOOL____00087, BOOL____00119], BOOL____00168) :: defines_var(BOOL____00168);\n\
constraint array_bool_and([BOOL____00090, BOOL____00122], BOOL____00170) :: defines_var(BOOL____00170);\n\
constraint array_bool_and([BOOL____00093, BOOL____00125], BOOL____00172) :: defines_var(BOOL____00172);\n\
constraint array_bool_and([BOOL____00097, BOOL____00103], BOOL____00104) :: defines_var(BOOL____00104);\n\
constraint array_bool_and([BOOL____00099, BOOL____00100], BOOL____00101) :: defines_var(BOOL____00101);\n\
constraint array_bool_and([BOOL____00100, BOOL____00106], BOOL____00107) :: defines_var(BOOL____00107);\n\
constraint array_bool_and([BOOL____00103, BOOL____00109], BOOL____00110) :: defines_var(BOOL____00110);\n\
constraint array_bool_and([BOOL____00113, BOOL____00119], BOOL____00120) :: defines_var(BOOL____00120);\n\
constraint array_bool_and([BOOL____00115, BOOL____00116], BOOL____00117) :: defines_var(BOOL____00117);\n\
constraint array_bool_and([BOOL____00116, BOOL____00122], BOOL____00123) :: defines_var(BOOL____00123);\n\
constraint array_bool_and([BOOL____00119, BOOL____00125], BOOL____00126) :: defines_var(BOOL____00126);\n\
constraint array_bool_and([BOOL____00178, BOOL____00042], BOOL____00180) :: defines_var(BOOL____00180);\n\
constraint array_bool_and([BOOL____00178, BOOL____00051], BOOL____00179) :: defines_var(BOOL____00179);\n\
constraint array_bool_and([BOOL____00181, BOOL____00049], BOOL____00182) :: defines_var(BOOL____00182);\n\
constraint array_bool_and([BOOL____00185, BOOL____00052], BOOL____00186) :: defines_var(BOOL____00186);\n\
constraint array_bool_and([BOOL____00189, BOOL____00055], BOOL____00190) :: defines_var(BOOL____00190);\n\
constraint array_bool_and([BOOL____00193, BOOL____00058], BOOL____00194) :: defines_var(BOOL____00194);\n\
constraint array_bool_and([BOOL____00197, BOOL____00061], BOOL____00198) :: defines_var(BOOL____00198);\n\
constraint array_bool_and([BOOL____00199, BOOL____00046], BOOL____00200) :: defines_var(BOOL____00200);\n\
constraint array_bool_and([BOOL____00203, BOOL____00049], BOOL____00204) :: defines_var(BOOL____00204);\n\
constraint array_bool_and([BOOL____00223, BOOL____00058], BOOL____00224) :: defines_var(BOOL____00224);\n\
constraint array_bool_and([BOOL____00227, BOOL____00065], BOOL____00228) :: defines_var(BOOL____00228);\n\
constraint array_bool_and([BOOL____00247, BOOL____00074], BOOL____00248) :: defines_var(BOOL____00248);\n\
constraint array_bool_and([BOOL____00251, BOOL____00081], BOOL____00252) :: defines_var(BOOL____00252);\n\
constraint array_bool_and([BOOL____00271, BOOL____00090], BOOL____00272) :: defines_var(BOOL____00272);\n\
constraint array_bool_and([BOOL____00275, BOOL____00097], BOOL____00276) :: defines_var(BOOL____00276);\n\
constraint array_bool_and([BOOL____00295, BOOL____00106], BOOL____00296) :: defines_var(BOOL____00296);\n\
constraint array_bool_and([BOOL____00297, BOOL____00099], BOOL____00298) :: defines_var(BOOL____00298);\n\
constraint array_bool_and([BOOL____00299, BOOL____00113], BOOL____00300) :: defines_var(BOOL____00300);\n\
constraint array_bool_and([BOOL____00301, BOOL____00097], BOOL____00302) :: defines_var(BOOL____00302);\n\
constraint array_bool_and([BOOL____00305, BOOL____00100], BOOL____00306) :: defines_var(BOOL____00306);\n\
constraint array_bool_and([BOOL____00309, BOOL____00103], BOOL____00310) :: defines_var(BOOL____00310);\n\
constraint array_bool_and([BOOL____00313, BOOL____00106], BOOL____00314) :: defines_var(BOOL____00314);\n\
constraint array_bool_and([BOOL____00317, BOOL____00109], BOOL____00318) :: defines_var(BOOL____00318);\n\
constraint array_bool_and([BOOL____00319, BOOL____00122], BOOL____00320) :: defines_var(BOOL____00320);\n\
constraint array_bool_and([BOOL____00044, BOOL____00183, BOOL____00043], BOOL____00184) :: defines_var(BOOL____00184);\n\
constraint array_bool_and([BOOL____00045, BOOL____00187, BOOL____00042], BOOL____00188) :: defines_var(BOOL____00188);\n\
constraint array_bool_and([BOOL____00046, BOOL____00191, BOOL____00044], BOOL____00192) :: defines_var(BOOL____00192);\n\
constraint array_bool_and([BOOL____00047, BOOL____00195, BOOL____00045], BOOL____00196) :: defines_var(BOOL____00196);\n\
constraint array_bool_and([BOOL____00052, BOOL____00207, BOOL____00051], BOOL____00208) :: defines_var(BOOL____00208);\n\
constraint array_bool_and([BOOL____00055, BOOL____00211, BOOL____00049], BOOL____00212) :: defines_var(BOOL____00212);\n\
constraint array_bool_and([BOOL____00058, BOOL____00215, BOOL____00052], BOOL____00216) :: defines_var(BOOL____00216);\n\
constraint array_bool_and([BOOL____00061, BOOL____00219, BOOL____00055], BOOL____00220) :: defines_var(BOOL____00220);\n\
constraint array_bool_and([BOOL____00065, BOOL____00205, BOOL____00042], BOOL____00206) :: defines_var(BOOL____00206);\n\
constraint array_bool_and([BOOL____00067, BOOL____00201, BOOL____00043], BOOL____00202) :: defines_var(BOOL____00202);\n\
constraint array_bool_and([BOOL____00068, BOOL____00209, BOOL____00044], BOOL____00210) :: defines_var(BOOL____00210);\n\
constraint array_bool_and([BOOL____00068, BOOL____00231, BOOL____00067], BOOL____00232) :: defines_var(BOOL____00232);\n\
constraint array_bool_and([BOOL____00071, BOOL____00213, BOOL____00045], BOOL____00214) :: defines_var(BOOL____00214);\n\
constraint array_bool_and([BOOL____00071, BOOL____00235, BOOL____00065], BOOL____00236) :: defines_var(BOOL____00236);\n\
constraint array_bool_and([BOOL____00074, BOOL____00217, BOOL____00046], BOOL____00218) :: defines_var(BOOL____00218);\n\
constraint array_bool_and([BOOL____00074, BOOL____00239, BOOL____00068], BOOL____00240) :: defines_var(BOOL____00240);\n\
constraint array_bool_and([BOOL____00077, BOOL____00221, BOOL____00047], BOOL____00222) :: defines_var(BOOL____00222);\n\
constraint array_bool_and([BOOL____00077, BOOL____00243, BOOL____00071], BOOL____00244) :: defines_var(BOOL____00244);\n\
constraint array_bool_and([BOOL____00081, BOOL____00229, BOOL____00049], BOOL____00230) :: defines_var(BOOL____00230);\n\
constraint array_bool_and([BOOL____00083, BOOL____00225, BOOL____00051], BOOL____00226) :: defines_var(BOOL____00226);\n\
constraint array_bool_and([BOOL____00084, BOOL____00233, BOOL____00052], BOOL____00234) :: defines_var(BOOL____00234);\n\
constraint array_bool_and([BOOL____00084, BOOL____00255, BOOL____00083], BOOL____00256) :: defines_var(BOOL____00256);\n\
constraint array_bool_and([BOOL____00087, BOOL____00237, BOOL____00055], BOOL____00238) :: defines_var(BOOL____00238);\n\
constraint array_bool_and([BOOL____00087, BOOL____00259, BOOL____00081], BOOL____00260) :: defines_var(BOOL____00260);\n\
constraint array_bool_and([BOOL____00090, BOOL____00241, BOOL____00058], BOOL____00242) :: defines_var(BOOL____00242);\n\
constraint array_bool_and([BOOL____00090, BOOL____00263, BOOL____00084], BOOL____00264) :: defines_var(BOOL____00264);\n\
constraint array_bool_and([BOOL____00093, BOOL____00245, BOOL____00061], BOOL____00246) :: defines_var(BOOL____00246);\n\
constraint array_bool_and([BOOL____00093, BOOL____00267, BOOL____00087], BOOL____00268) :: defines_var(BOOL____00268);\n\
constraint array_bool_and([BOOL____00097, BOOL____00253, BOOL____00065], BOOL____00254) :: defines_var(BOOL____00254);\n\
constraint array_bool_and([BOOL____00099, BOOL____00249, BOOL____00067], BOOL____00250) :: defines_var(BOOL____00250);\n\
constraint array_bool_and([BOOL____00100, BOOL____00257, BOOL____00068], BOOL____00258) :: defines_var(BOOL____00258);\n\
constraint array_bool_and([BOOL____00100, BOOL____00279, BOOL____00099], BOOL____00280) :: defines_var(BOOL____00280);\n\
constraint array_bool_and([BOOL____00103, BOOL____00261, BOOL____00071], BOOL____00262) :: defines_var(BOOL____00262);\n\
constraint array_bool_and([BOOL____00103, BOOL____00283, BOOL____00097], BOOL____00284) :: defines_var(BOOL____00284);\n\
constraint array_bool_and([BOOL____00106, BOOL____00265, BOOL____00074], BOOL____00266) :: defines_var(BOOL____00266);\n\
constraint array_bool_and([BOOL____00106, BOOL____00287, BOOL____00100], BOOL____00288) :: defines_var(BOOL____00288);\n\
constraint array_bool_and([BOOL____00109, BOOL____00269, BOOL____00077], BOOL____00270) :: defines_var(BOOL____00270);\n\
constraint array_bool_and([BOOL____00109, BOOL____00291, BOOL____00103], BOOL____00292) :: defines_var(BOOL____00292);\n\
constraint array_bool_and([BOOL____00113, BOOL____00277, BOOL____00081], BOOL____00278) :: defines_var(BOOL____00278);\n\
constraint array_bool_and([BOOL____00115, BOOL____00273, BOOL____00083], BOOL____00274) :: defines_var(BOOL____00274);\n\
constraint array_bool_and([BOOL____00116, BOOL____00281, BOOL____00084], BOOL____00282) :: defines_var(BOOL____00282);\n\
constraint array_bool_and([BOOL____00116, BOOL____00303, BOOL____00115], BOOL____00304) :: defines_var(BOOL____00304);\n\
constraint array_bool_and([BOOL____00119, BOOL____00285, BOOL____00087], BOOL____00286) :: defines_var(BOOL____00286);\n\
constraint array_bool_and([BOOL____00119, BOOL____00307, BOOL____00113], BOOL____00308) :: defines_var(BOOL____00308);\n\
constraint array_bool_and([BOOL____00122, BOOL____00289, BOOL____00090], BOOL____00290) :: defines_var(BOOL____00290);\n\
constraint array_bool_and([BOOL____00122, BOOL____00311, BOOL____00116], BOOL____00312) :: defines_var(BOOL____00312);\n\
constraint array_bool_and([BOOL____00125, BOOL____00293, BOOL____00093], BOOL____00294) :: defines_var(BOOL____00294);\n\
constraint array_bool_and([BOOL____00125, BOOL____00315, BOOL____00119], BOOL____00316) :: defines_var(BOOL____00316);\n\
constraint array_bool_or([BOOL____00180, BOOL____00179, BOOL____00043], true);\n\
constraint array_bool_or([BOOL____00184, BOOL____00182, BOOL____00042], true);\n\
constraint array_bool_or([BOOL____00188, BOOL____00186, BOOL____00044], true);\n\
constraint array_bool_or([BOOL____00192, BOOL____00190, BOOL____00045], true);\n\
constraint array_bool_or([BOOL____00196, BOOL____00194, BOOL____00046], true);\n\
constraint array_bool_or([BOOL____00200, BOOL____00198, BOOL____00047], true);\n\
constraint array_bool_or([BOOL____00204, BOOL____00202, BOOL____00051], true);\n\
constraint array_bool_or([BOOL____00208, BOOL____00206, BOOL____00049], true);\n\
constraint array_bool_or([BOOL____00212, BOOL____00210, BOOL____00052], true);\n\
constraint array_bool_or([BOOL____00216, BOOL____00214, BOOL____00055], true);\n\
constraint array_bool_or([BOOL____00220, BOOL____00218, BOOL____00058], true);\n\
constraint array_bool_or([BOOL____00224, BOOL____00222, BOOL____00061], true);\n\
constraint array_bool_or([BOOL____00228, BOOL____00226, BOOL____00067], true);\n\
constraint array_bool_or([BOOL____00232, BOOL____00230, BOOL____00065], true);\n\
constraint array_bool_or([BOOL____00236, BOOL____00234, BOOL____00068], true);\n\
constraint array_bool_or([BOOL____00240, BOOL____00238, BOOL____00071], true);\n\
constraint array_bool_or([BOOL____00244, BOOL____00242, BOOL____00074], true);\n\
constraint array_bool_or([BOOL____00248, BOOL____00246, BOOL____00077], true);\n\
constraint array_bool_or([BOOL____00252, BOOL____00250, BOOL____00083], true);\n\
constraint array_bool_or([BOOL____00256, BOOL____00254, BOOL____00081], true);\n\
constraint array_bool_or([BOOL____00260, BOOL____00258, BOOL____00084], true);\n\
constraint array_bool_or([BOOL____00264, BOOL____00262, BOOL____00087], true);\n\
constraint array_bool_or([BOOL____00268, BOOL____00266, BOOL____00090], true);\n\
constraint array_bool_or([BOOL____00272, BOOL____00270, BOOL____00093], true);\n\
constraint array_bool_or([BOOL____00276, BOOL____00274, BOOL____00099], true);\n\
constraint array_bool_or([BOOL____00280, BOOL____00278, BOOL____00097], true);\n\
constraint array_bool_or([BOOL____00284, BOOL____00282, BOOL____00100], true);\n\
constraint array_bool_or([BOOL____00288, BOOL____00286, BOOL____00103], true);\n\
constraint array_bool_or([BOOL____00292, BOOL____00290, BOOL____00106], true);\n\
constraint array_bool_or([BOOL____00296, BOOL____00294, BOOL____00109], true);\n\
constraint array_bool_or([BOOL____00300, BOOL____00298, BOOL____00115], true);\n\
constraint array_bool_or([BOOL____00304, BOOL____00302, BOOL____00113], true);\n\
constraint array_bool_or([BOOL____00308, BOOL____00306, BOOL____00116], true);\n\
constraint array_bool_or([BOOL____00312, BOOL____00310, BOOL____00119], true);\n\
constraint array_bool_or([BOOL____00316, BOOL____00314, BOOL____00122], true);\n\
constraint array_bool_or([BOOL____00320, BOOL____00318, BOOL____00125], true);\n\
constraint bool_le(BOOL____00048, BOOL____00049);\n\
constraint bool_le(BOOL____00050, BOOL____00053);\n\
constraint bool_le(BOOL____00054, BOOL____00056);\n\
constraint bool_le(BOOL____00057, BOOL____00059);\n\
constraint bool_le(BOOL____00060, BOOL____00062);\n\
constraint bool_le(BOOL____00063, BOOL____00058);\n\
constraint bool_le(BOOL____00064, BOOL____00065);\n\
constraint bool_le(BOOL____00066, BOOL____00069);\n\
constraint bool_le(BOOL____00070, BOOL____00072);\n\
constraint bool_le(BOOL____00073, BOOL____00075);\n\
constraint bool_le(BOOL____00076, BOOL____00078);\n\
constraint bool_le(BOOL____00079, BOOL____00074);\n\
constraint bool_le(BOOL____00080, BOOL____00081);\n\
constraint bool_le(BOOL____00082, BOOL____00085);\n\
constraint bool_le(BOOL____00086, BOOL____00088);\n\
constraint bool_le(BOOL____00089, BOOL____00091);\n\
constraint bool_le(BOOL____00092, BOOL____00094);\n\
constraint bool_le(BOOL____00095, BOOL____00090);\n\
constraint bool_le(BOOL____00096, BOOL____00097);\n\
constraint bool_le(BOOL____00098, BOOL____00101);\n\
constraint bool_le(BOOL____00102, BOOL____00104);\n\
constraint bool_le(BOOL____00105, BOOL____00107);\n\
constraint bool_le(BOOL____00108, BOOL____00110);\n\
constraint bool_le(BOOL____00111, BOOL____00106);\n\
constraint bool_le(BOOL____00112, BOOL____00113);\n\
constraint bool_le(BOOL____00114, BOOL____00117);\n\
constraint bool_le(BOOL____00118, BOOL____00120);\n\
constraint bool_le(BOOL____00121, BOOL____00123);\n\
constraint bool_le(BOOL____00124, BOOL____00126);\n\
constraint bool_le(BOOL____00127, BOOL____00122);\n\
constraint bool_le(BOOL____00128, BOOL____00049);\n\
constraint bool_le(BOOL____00129, BOOL____00052);\n\
constraint bool_le(BOOL____00130, BOOL____00055);\n\
constraint bool_le(BOOL____00131, BOOL____00058);\n\
constraint bool_le(BOOL____00132, BOOL____00061);\n\
constraint bool_le(BOOL____00133, BOOL____00134);\n\
constraint bool_le(BOOL____00135, BOOL____00136);\n\
constraint bool_le(BOOL____00137, BOOL____00138);\n\
constraint bool_le(BOOL____00139, BOOL____00140);\n\
constraint bool_le(BOOL____00141, BOOL____00142);\n\
constraint bool_le(BOOL____00143, BOOL____00144);\n\
constraint bool_le(BOOL____00145, BOOL____00146);\n\
constraint bool_le(BOOL____00147, BOOL____00148);\n\
constraint bool_le(BOOL____00149, BOOL____00150);\n\
constraint bool_le(BOOL____00151, BOOL____00152);\n\
constraint bool_le(BOOL____00153, BOOL____00154);\n\
constraint bool_le(BOOL____00155, BOOL____00156);\n\
constraint bool_le(BOOL____00157, BOOL____00158);\n\
constraint bool_le(BOOL____00159, BOOL____00160);\n\
constraint bool_le(BOOL____00161, BOOL____00162);\n\
constraint bool_le(BOOL____00163, BOOL____00164);\n\
constraint bool_le(BOOL____00165, BOOL____00166);\n\
constraint bool_le(BOOL____00167, BOOL____00168);\n\
constraint bool_le(BOOL____00169, BOOL____00170);\n"+
"constraint bool_le(BOOL____00171, BOOL____00172);\n\
constraint bool_le(BOOL____00173, BOOL____00097);\n\
constraint bool_le(BOOL____00174, BOOL____00100);\n\
constraint bool_le(BOOL____00175, BOOL____00103);\n\
constraint bool_le(BOOL____00176, BOOL____00106);\n\
constraint bool_le(BOOL____00177, BOOL____00109);\n\
constraint count([X____00001, X____00002, X____00003, X____00004, X____00005, X____00006], 0, 4);\n\
constraint count([X____00001, X____00007, X____00013, X____00019, X____00025, X____00031], 0, 4);\n\
constraint count([X____00002, X____00008, X____00014, X____00020, X____00026, X____00032], 0, 6);\n\
constraint count([X____00003, X____00009, X____00015, X____00021, X____00027, X____00033], 0, INT____00357);\n\
constraint count([X____00004, X____00010, X____00016, X____00022, X____00028, X____00034], 0, INT____00358);\n\
constraint count([X____00005, X____00011, X____00017, X____00023, X____00029, X____00035], 0, INT____00359);\n\
constraint count([X____00006, X____00012, X____00018, X____00024, X____00030, X____00036], 0, INT____00360);\n\
constraint count([X____00007, X____00008, X____00009, X____00010, X____00011, X____00012], 0, 4);\n\
constraint count([X____00013, X____00014, X____00015, X____00016, X____00017, X____00018], 0, 5);\n\
constraint count([X____00019, X____00020, X____00021, X____00022, X____00023, X____00024], 0, 5);\n\
constraint count([X____00025, X____00026, X____00027, X____00028, X____00029, X____00030], 0, INT____00361);\n\
constraint count([X____00031, X____00032, X____00033, X____00034, X____00035, X____00036], 0, 5);\n\
constraint count(a_flat, 1, 6);\n\
constraint count(a_flat, 2, 3);\n\
constraint count(a_flat, 3, 1);\n\
constraint int_eq_reif(X____00001, 0, BOOL____00043) :: defines_var(BOOL____00043);\n\
constraint int_eq_reif(X____00001, 1, BOOL____00178) :: defines_var(BOOL____00178);\n\
constraint int_eq_reif(X____00002, 0, BOOL____00042) :: defines_var(BOOL____00042);\n\
constraint int_eq_reif(X____00002, 1, BOOL____00183) :: defines_var(BOOL____00183);\n\
constraint int_eq_reif(X____00003, 0, BOOL____00044) :: defines_var(BOOL____00044);\n\
constraint int_eq_reif(X____00003, 1, BOOL____00187) :: defines_var(BOOL____00187);\n\
constraint int_eq_reif(X____00004, 0, BOOL____00045) :: defines_var(BOOL____00045);\n\
constraint int_eq_reif(X____00004, 1, BOOL____00191) :: defines_var(BOOL____00191);\n\
constraint int_eq_reif(X____00005, 0, BOOL____00046) :: defines_var(BOOL____00046);\n\
constraint int_eq_reif(X____00005, 1, BOOL____00195) :: defines_var(BOOL____00195);\n\
constraint int_eq_reif(X____00006, 0, BOOL____00047) :: defines_var(BOOL____00047);\n\
constraint int_eq_reif(X____00006, 1, BOOL____00199) :: defines_var(BOOL____00199);\n\
constraint int_eq_reif(X____00007, 0, BOOL____00051) :: defines_var(BOOL____00051);\n\
constraint int_eq_reif(X____00007, 1, BOOL____00201) :: defines_var(BOOL____00201);\n\
constraint int_eq_reif(X____00008, 0, BOOL____00049) :: defines_var(BOOL____00049);\n\
constraint int_eq_reif(X____00009, 0, BOOL____00052) :: defines_var(BOOL____00052);\n\
constraint int_eq_reif(X____00010, 0, BOOL____00055) :: defines_var(BOOL____00055);\n\
constraint int_eq_reif(X____00011, 0, BOOL____00058) :: defines_var(BOOL____00058);\n\
constraint int_eq_reif(X____00012, 0, BOOL____00061) :: defines_var(BOOL____00061);\n\
constraint int_eq_reif(X____00013, 0, BOOL____00067) :: defines_var(BOOL____00067);\n\
constraint int_eq_reif(X____00013, 1, BOOL____00225) :: defines_var(BOOL____00225);\n\
constraint int_eq_reif(X____00014, 0, BOOL____00065) :: defines_var(BOOL____00065);\n\
constraint int_eq_reif(X____00015, 0, BOOL____00068) :: defines_var(BOOL____00068);\n\
constraint int_eq_reif(X____00016, 0, BOOL____00071) :: defines_var(BOOL____00071);\n\
constraint int_eq_reif(X____00017, 0, BOOL____00074) :: defines_var(BOOL____00074);\n\
constraint int_eq_reif(X____00018, 0, BOOL____00077) :: defines_var(BOOL____00077);\n\
constraint int_eq_reif(X____00019, 0, BOOL____00083) :: defines_var(BOOL____00083);\n\
constraint int_eq_reif(X____00019, 1, BOOL____00249) :: defines_var(BOOL____00249);\n\
constraint int_eq_reif(X____00020, 0, BOOL____00081) :: defines_var(BOOL____00081);\n\
constraint int_eq_reif(X____00021, 0, BOOL____00084) :: defines_var(BOOL____00084);\n\
constraint int_eq_reif(X____00022, 0, BOOL____00087) :: defines_var(BOOL____00087);\n\
constraint int_eq_reif(X____00023, 0, BOOL____00090) :: defines_var(BOOL____00090);\n\
constraint int_eq_reif(X____00024, 0, BOOL____00093) :: defines_var(BOOL____00093);\n\
constraint int_eq_reif(X____00025, 0, BOOL____00099) :: defines_var(BOOL____00099);\n\
constraint int_eq_reif(X____00025, 1, BOOL____00273) :: defines_var(BOOL____00273);\n\
constraint int_eq_reif(X____00026, 0, BOOL____00097) :: defines_var(BOOL____00097);\n\
constraint int_eq_reif(X____00027, 0, BOOL____00100) :: defines_var(BOOL____00100);\n\
constraint int_eq_reif(X____00028, 0, BOOL____00103) :: defines_var(BOOL____00103);\n\
constraint int_eq_reif(X____00029, 0, BOOL____00106) :: defines_var(BOOL____00106);\n\
constraint int_eq_reif(X____00030, 0, BOOL____00109) :: defines_var(BOOL____00109);\n\
constraint int_eq_reif(X____00031, 0, BOOL____00115) :: defines_var(BOOL____00115);\n\
constraint int_eq_reif(X____00031, 1, BOOL____00297) :: defines_var(BOOL____00297);\n\
constraint int_eq_reif(X____00032, 0, BOOL____00113) :: defines_var(BOOL____00113);\n\
constraint int_eq_reif(X____00033, 0, BOOL____00116) :: defines_var(BOOL____00116);\n\
constraint int_eq_reif(X____00034, 0, BOOL____00119) :: defines_var(BOOL____00119);\n\
constraint int_eq_reif(X____00035, 0, BOOL____00122) :: defines_var(BOOL____00122);\n\
constraint int_eq_reif(X____00036, 0, BOOL____00125) :: defines_var(BOOL____00125);\n\
constraint int_lin_eq([-1, -1], [INT____00357, X____00037], -6) :: defines_var(INT____00357);\n\
constraint int_lin_eq([-1, -1], [INT____00358, X____00038], -6) :: defines_var(INT____00358);\n\
constraint int_lin_eq([-1, -1], [INT____00359, X____00039], -6) :: defines_var(INT____00359);\n\
constraint int_lin_eq([-1, -1], [INT____00360, X____00040], -6) :: defines_var(INT____00360);\n\
constraint int_lin_eq([-1, -1], [INT____00361, X____00041], -6) :: defines_var(INT____00361);\n\
constraint int_lin_eq_reif([-1, 1], [X____00001, X____00002], 1, BOOL____00181) :: defines_var(BOOL____00181);\n\
constraint int_lin_eq_reif([-1, 1], [X____00001, X____00007], 1, BOOL____00203) :: defines_var(BOOL____00203);\n\
constraint int_lin_eq_reif([-1, 1], [X____00002, X____00003], 1, BOOL____00185) :: defines_var(BOOL____00185);\n\
constraint int_lin_eq_reif([-1, 1], [X____00002, X____00008], 1, BOOL____00207) :: defines_var(BOOL____00207);\n\
constraint int_lin_eq_reif([-1, 1], [X____00003, X____00004], 1, BOOL____00189) :: defines_var(BOOL____00189);\n\
constraint int_lin_eq_reif([-1, 1], [X____00003, X____00009], 1, BOOL____00211) :: defines_var(BOOL____00211);\n\
constraint int_lin_eq_reif([-1, 1], [X____00004, X____00005], 1, BOOL____00193) :: defines_var(BOOL____00193);\n\
constraint int_lin_eq_reif([-1, 1], [X____00004, X____00010], 1, BOOL____00215) :: defines_var(BOOL____00215);\n\
constraint int_lin_eq_reif([-1, 1], [X____00005, X____00006], 1, BOOL____00197) :: defines_var(BOOL____00197);\n\
constraint int_lin_eq_reif([-1, 1], [X____00005, X____00011], 1, BOOL____00219) :: defines_var(BOOL____00219);\n\
constraint int_lin_eq_reif([-1, 1], [X____00006, X____00012], 1, BOOL____00223) :: defines_var(BOOL____00223);\n\
constraint int_lin_eq_reif([-1, 1], [X____00007, X____00008], 1, BOOL____00205) :: defines_var(BOOL____00205);\n\
constraint int_lin_eq_reif([-1, 1], [X____00007, X____00013], 1, BOOL____00227) :: defines_var(BOOL____00227);\n\
constraint int_lin_eq_reif([-1, 1], [X____00008, X____00009], 1, BOOL____00209) :: defines_var(BOOL____00209);\n\
constraint int_lin_eq_reif([-1, 1], [X____00008, X____00014], 1, BOOL____00231) :: defines_var(BOOL____00231);\n\
constraint int_lin_eq_reif([-1, 1], [X____00009, X____00010], 1, BOOL____00213) :: defines_var(BOOL____00213);\n\
constraint int_lin_eq_reif([-1, 1], [X____00009, X____00015], 1, BOOL____00235) :: defines_var(BOOL____00235);\n\
constraint int_lin_eq_reif([-1, 1], [X____00010, X____00011], 1, BOOL____00217) :: defines_var(BOOL____00217);\n\
constraint int_lin_eq_reif([-1, 1], [X____00010, X____00016], 1, BOOL____00239) :: defines_var(BOOL____00239);\n\
constraint int_lin_eq_reif([-1, 1], [X____00011, X____00012], 1, BOOL____00221) :: defines_var(BOOL____00221);\n\
constraint int_lin_eq_reif([-1, 1], [X____00011, X____00017], 1, BOOL____00243) :: defines_var(BOOL____00243);\n\
constraint int_lin_eq_reif([-1, 1], [X____00012, X____00018], 1, BOOL____00247) :: defines_var(BOOL____00247);\n\
constraint int_lin_eq_reif([-1, 1], [X____00013, X____00014], 1, BOOL____00229) :: defines_var(BOOL____00229);\n\
constraint int_lin_eq_reif([-1, 1], [X____00013, X____00019], 1, BOOL____00251) :: defines_var(BOOL____00251);\n\
constraint int_lin_eq_reif([-1, 1], [X____00014, X____00015], 1, BOOL____00233) :: defines_var(BOOL____00233);\n\
constraint int_lin_eq_reif([-1, 1], [X____00014, X____00020], 1, BOOL____00255) :: defines_var(BOOL____00255);\n\
constraint int_lin_eq_reif([-1, 1], [X____00015, X____00016], 1, BOOL____00237) :: defines_var(BOOL____00237);\n\
constraint int_lin_eq_reif([-1, 1], [X____00015, X____00021], 1, BOOL____00259) :: defines_var(BOOL____00259);\n\
constraint int_lin_eq_reif([-1, 1], [X____00016, X____00017], 1, BOOL____00241) :: defines_var(BOOL____00241);\n\
constraint int_lin_eq_reif([-1, 1], [X____00016, X____00022], 1, BOOL____00263) :: defines_var(BOOL____00263);\n\
constraint int_lin_eq_reif([-1, 1], [X____00017, X____00018], 1, BOOL____00245) :: defines_var(BOOL____00245);\n\
constraint int_lin_eq_reif([-1, 1], [X____00017, X____00023], 1, BOOL____00267) :: defines_var(BOOL____00267);\n\
constraint int_lin_eq_reif([-1, 1], [X____00018, X____00024], 1, BOOL____00271) :: defines_var(BOOL____00271);\n\
constraint int_lin_eq_reif([-1, 1], [X____00019, X____00020], 1, BOOL____00253) :: defines_var(BOOL____00253);\n\
constraint int_lin_eq_reif([-1, 1], [X____00019, X____00025], 1, BOOL____00275) :: defines_var(BOOL____00275);\n\
constraint int_lin_eq_reif([-1, 1], [X____00020, X____00021], 1, BOOL____00257) :: defines_var(BOOL____00257);\n\
constraint int_lin_eq_reif([-1, 1], [X____00020, X____00026], 1, BOOL____00279) :: defines_var(BOOL____00279);\n\
constraint int_lin_eq_reif([-1, 1], [X____00021, X____00022], 1, BOOL____00261) :: defines_var(BOOL____00261);\n\
constraint int_lin_eq_reif([-1, 1], [X____00021, X____00027], 1, BOOL____00283) :: defines_var(BOOL____00283);\n\
constraint int_lin_eq_reif([-1, 1], [X____00022, X____00023], 1, BOOL____00265) :: defines_var(BOOL____00265);\n\
constraint int_lin_eq_reif([-1, 1], [X____00022, X____00028], 1, BOOL____00287) :: defines_var(BOOL____00287);\n\
constraint int_lin_eq_reif([-1, 1], [X____00023, X____00024], 1, BOOL____00269) :: defines_var(BOOL____00269);\n\
constraint int_lin_eq_reif([-1, 1], [X____00023, X____00029], 1, BOOL____00291) :: defines_var(BOOL____00291);\n\
constraint int_lin_eq_reif([-1, 1], [X____00024, X____00030], 1, BOOL____00295) :: defines_var(BOOL____00295);\n\
constraint int_lin_eq_reif([-1, 1], [X____00025, X____00026], 1, BOOL____00277) :: defines_var(BOOL____00277);\n\
constraint int_lin_eq_reif([-1, 1], [X____00025, X____00031], 1, BOOL____00299) :: defines_var(BOOL____00299);\n\
constraint int_lin_eq_reif([-1, 1], [X____00026, X____00027], 1, BOOL____00281) :: defines_var(BOOL____00281);\n\
constraint int_lin_eq_reif([-1, 1], [X____00026, X____00032], 1, BOOL____00303) :: defines_var(BOOL____00303);\n\
constraint int_lin_eq_reif([-1, 1], [X____00027, X____00028], 1, BOOL____00285) :: defines_var(BOOL____00285);\n\
constraint int_lin_eq_reif([-1, 1], [X____00027, X____00033], 1, BOOL____00307) :: defines_var(BOOL____00307);\n\
constraint int_lin_eq_reif([-1, 1], [X____00028, X____00029], 1, BOOL____00289) :: defines_var(BOOL____00289);\n\
constraint int_lin_eq_reif([-1, 1], [X____00028, X____00034], 1, BOOL____00311) :: defines_var(BOOL____00311);\n\
constraint int_lin_eq_reif([-1, 1], [X____00029, X____00030], 1, BOOL____00293) :: defines_var(BOOL____00293);\n\
constraint int_lin_eq_reif([-1, 1], [X____00029, X____00035], 1, BOOL____00315) :: defines_var(BOOL____00315);\n\
constraint int_lin_eq_reif([-1, 1], [X____00030, X____00036], 1, BOOL____00319) :: defines_var(BOOL____00319);\n\
constraint int_lin_eq_reif([-1, 1], [X____00031, X____00032], 1, BOOL____00301) :: defines_var(BOOL____00301);\n\
constraint int_lin_eq_reif([-1, 1], [X____00032, X____00033], 1, BOOL____00305) :: defines_var(BOOL____00305);\n\
constraint int_lin_eq_reif([-1, 1], [X____00033, X____00034], 1, BOOL____00309) :: defines_var(BOOL____00309);\n\
constraint int_lin_eq_reif([-1, 1], [X____00034, X____00035], 1, BOOL____00313) :: defines_var(BOOL____00313);\n\
constraint int_lin_eq_reif([-1, 1], [X____00035, X____00036], 1, BOOL____00317) :: defines_var(BOOL____00317);\n\
constraint int_lt_reif(X____00002, X____00001, BOOL____00128) :: defines_var(BOOL____00128);\n\
constraint int_lt_reif(X____00003, X____00002, BOOL____00129) :: defines_var(BOOL____00129);\n\
constraint int_lt_reif(X____00004, X____00003, BOOL____00130) :: defines_var(BOOL____00130);\n\
constraint int_lt_reif(X____00005, X____00004, BOOL____00131) :: defines_var(BOOL____00131);\n\
constraint int_lt_reif(X____00006, X____00005, BOOL____00132) :: defines_var(BOOL____00132);\n\
constraint int_lt_reif(X____00007, X____00001, BOOL____00048) :: defines_var(BOOL____00048);\n\
constraint int_lt_reif(X____00008, X____00002, BOOL____00050) :: defines_var(BOOL____00050);\n\
constraint int_lt_reif(X____00008, X____00007, BOOL____00133) :: defines_var(BOOL____00133);\n\
constraint int_lt_reif(X____00009, X____00003, BOOL____00054) :: defines_var(BOOL____00054);\n\
constraint int_lt_reif(X____00009, X____00008, BOOL____00135) :: defines_var(BOOL____00135);\n\
constraint int_lt_reif(X____00010, X____00004, BOOL____00057) :: defines_var(BOOL____00057);\n\
constraint int_lt_reif(X____00010, X____00009, BOOL____00137) :: defines_var(BOOL____00137);\n\
constraint int_lt_reif(X____00011, X____00005, BOOL____00060) :: defines_var(BOOL____00060);\n\
constraint int_lt_reif(X____00011, X____00010, BOOL____00139) :: defines_var(BOOL____00139);\n\
constraint int_lt_reif(X____00012, X____00006, BOOL____00063) :: defines_var(BOOL____00063);\n\
constraint int_lt_reif(X____00012, X____00011, BOOL____00141) :: defines_var(BOOL____00141);\n\
constraint int_lt_reif(X____00013, X____00007, BOOL____00064) :: defines_var(BOOL____00064);\n\
constraint int_lt_reif(X____00014, X____00008, BOOL____00066) :: defines_var(BOOL____00066);\n\
constraint int_lt_reif(X____00014, X____00013, BOOL____00143) :: defines_var(BOOL____00143);\n\
constraint int_lt_reif(X____00015, X____00009, BOOL____00070) :: defines_var(BOOL____00070);\n\
constraint int_lt_reif(X____00015, X____00014, BOOL____00145) :: defines_var(BOOL____00145);\n\
constraint int_lt_reif(X____00016, X____00010, BOOL____00073) :: defines_var(BOOL____00073);\n\
constraint int_lt_reif(X____00016, X____00015, BOOL____00147) :: defines_var(BOOL____00147);\n\
constraint int_lt_reif(X____00017, X____00011, BOOL____00076) :: defines_var(BOOL____00076);\n\
constraint int_lt_reif(X____00017, X____00016, BOOL____00149) :: defines_var(BOOL____00149);\n\
constraint int_lt_reif(X____00018, X____00012, BOOL____00079) :: defines_var(BOOL____00079);\n\
constraint int_lt_reif(X____00018, X____00017, BOOL____00151) :: defines_var(BOOL____00151);\n\
constraint int_lt_reif(X____00019, X____00013, BOOL____00080) :: defines_var(BOOL____00080);\n\
constraint int_lt_reif(X____00020, X____00014, BOOL____00082) :: defines_var(BOOL____00082);\n\
constraint int_lt_reif(X____00020, X____00019, BOOL____00153) :: defines_var(BOOL____00153);\n\
constraint int_lt_reif(X____00021, X____00015, BOOL____00086) :: defines_var(BOOL____00086);\n\
constraint int_lt_reif(X____00021, X____00020, BOOL____00155) :: defines_var(BOOL____00155);\n\
constraint int_lt_reif(X____00022, X____00016, BOOL____00089) :: defines_var(BOOL____00089);\n\
constraint int_lt_reif(X____00022, X____00021, BOOL____00157) :: defines_var(BOOL____00157);\n\
constraint int_lt_reif(X____00023, X____00017, BOOL____00092) :: defines_var(BOOL____00092);\n\
constraint int_lt_reif(X____00023, X____00022, BOOL____00159) :: defines_var(BOOL____00159);\n\
constraint int_lt_reif(X____00024, X____00018, BOOL____00095) :: defines_var(BOOL____00095);\n\
constraint int_lt_reif(X____00024, X____00023, BOOL____00161) :: defines_var(BOOL____00161);\n\
constraint int_lt_reif(X____00025, X____00019, BOOL____00096) :: defines_var(BOOL____00096);\n"+
"constraint int_lt_reif(X____00026, X____00020, BOOL____00098) :: defines_var(BOOL____00098);\n\
constraint int_lt_reif(X____00026, X____00025, BOOL____00163) :: defines_var(BOOL____00163);\n\
constraint int_lt_reif(X____00027, X____00021, BOOL____00102) :: defines_var(BOOL____00102);\n\
constraint int_lt_reif(X____00027, X____00026, BOOL____00165) :: defines_var(BOOL____00165);\n\
constraint int_lt_reif(X____00028, X____00022, BOOL____00105) :: defines_var(BOOL____00105);\n\
constraint int_lt_reif(X____00028, X____00027, BOOL____00167) :: defines_var(BOOL____00167);\n\
constraint int_lt_reif(X____00029, X____00023, BOOL____00108) :: defines_var(BOOL____00108);\n\
constraint int_lt_reif(X____00029, X____00028, BOOL____00169) :: defines_var(BOOL____00169);\n\
constraint int_lt_reif(X____00030, X____00024, BOOL____00111) :: defines_var(BOOL____00111);\n\
constraint int_lt_reif(X____00030, X____00029, BOOL____00171) :: defines_var(BOOL____00171);\n\
constraint int_lt_reif(X____00031, X____00025, BOOL____00112) :: defines_var(BOOL____00112);\n\
constraint int_lt_reif(X____00032, X____00026, BOOL____00114) :: defines_var(BOOL____00114);\n\
constraint int_lt_reif(X____00032, X____00031, BOOL____00173) :: defines_var(BOOL____00173);\n\
constraint int_lt_reif(X____00033, X____00027, BOOL____00118) :: defines_var(BOOL____00118);\n\
constraint int_lt_reif(X____00033, X____00032, BOOL____00174) :: defines_var(BOOL____00174);\n\
constraint int_lt_reif(X____00034, X____00028, BOOL____00121) :: defines_var(BOOL____00121);\n\
constraint int_lt_reif(X____00034, X____00033, BOOL____00175) :: defines_var(BOOL____00175);\n\
constraint int_lt_reif(X____00035, X____00029, BOOL____00124) :: defines_var(BOOL____00124);\n\
constraint int_lt_reif(X____00035, X____00034, BOOL____00176) :: defines_var(BOOL____00176);\n\
constraint int_lt_reif(X____00036, X____00030, BOOL____00127) :: defines_var(BOOL____00127);\n\
constraint int_lt_reif(X____00036, X____00035, BOOL____00177) :: defines_var(BOOL____00177);\n\
solve  :: int_search(a_flat, first_fail, indomain_max, complete) satisfy;\n\
", "a = array2d(0..7, 0..7, [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
