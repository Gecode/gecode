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
        (void) new FlatZincTest("battleships::10",
std::string("predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate count(array [int] of var int: x, var int: y, var int: c);\n\
predicate count_reif(array [int] of var int: x, var int: y, var int: c, var bool: b);\n\
array [1..4] of int: class_sizes = [4, 3, 2, 1];\n\
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
var bool: BOOL____00288 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00289 :: is_defined_var :: var_is_introduced;\n\
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
var bool: BOOL____00321 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00322 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00323 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00324 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00325 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00326 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00327 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00328 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00329 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00330 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00331 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00332 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00333 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00334 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00335 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00336 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00337 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00338 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00339 :: is_defined_var :: var_is_introduced;\n")+
"var bool: BOOL____00340 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00341 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00342 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00343 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00344 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00345 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00346 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00347 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00348 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00349 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00350 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00351 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00352 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00353 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00354 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00355 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00356 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00357 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00358 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00359 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00360 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00361 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00362 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00363 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00364 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00365 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00366 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00367 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00368 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00369 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00370 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00371 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00372 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00373 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00374 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00375 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00376 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00377 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00378 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00379 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00380 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00381 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00382 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00383 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00384 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00385 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00386 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00387 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00388 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00389 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00390 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00391 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00392 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00393 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00394 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00395 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00396 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00397 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00398 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00399 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00400 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00401 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00402 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00403 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00404 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00405 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00406 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00407 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00408 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00409 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00410 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00411 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00412 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00413 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00414 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00415 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00416 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00417 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00418 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00419 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00420 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00421 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00422 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00423 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00424 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00425 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00426 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00427 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00428 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00429 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00430 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00431 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00432 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00433 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00434 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00435 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00436 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00437 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00438 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00439 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00440 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00441 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00442 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00443 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00444 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00445 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00446 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00447 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00448 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00449 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00450 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00451 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00452 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00453 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00454 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00455 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00456 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00457 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00458 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00459 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00460 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00461 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00462 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00463 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00464 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00465 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00466 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00467 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00468 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00469 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00470 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00471 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00472 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00473 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00474 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00475 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00476 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00477 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00478 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00479 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00480 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00481 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00482 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00483 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00484 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00485 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00486 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00487 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00488 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00489 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00490 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00491 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00492 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00493 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00494 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00495 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00496 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00497 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00498 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00499 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00500 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00501 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00502 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00503 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00504 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00505 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00506 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00507 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00508 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00509 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00510 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00511 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00512 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00513 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00514 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00515 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00516 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00517 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00518 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00519 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00520 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00521 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00522 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00523 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00524 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00525 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00526 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00527 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00528 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00529 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00530 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00531 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00532 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00533 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00534 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00535 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00536 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00537 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00538 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00539 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00540 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00541 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00542 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00543 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00544 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00545 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00546 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00547 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00548 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00549 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00550 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00551 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00552 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00553 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00554 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00555 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00556 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00557 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00558 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00559 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00560 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00561 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00562 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00563 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00564 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00565 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00566 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00567 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00568 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00569 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00570 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00571 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00572 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00573 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00574 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00575 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00576 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00577 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00578 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00579 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00580 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00581 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00582 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00583 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00584 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00585 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00586 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00587 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00588 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00589 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00590 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00591 :: is_defined_var :: var_is_introduced;\n"+
"var bool: BOOL____00592 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00593 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00594 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00595 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00596 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00597 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00598 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00599 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00600 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00601 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00602 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00603 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00604 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00605 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00606 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00607 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00608 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00609 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00610 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00611 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00612 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00613 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00614 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00615 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00616 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00617 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00618 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00619 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00620 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00621 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00622 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00623 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00624 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00625 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00626 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00627 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00628 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00629 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00630 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00631 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00632 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00633 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00634 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00635 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00636 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00637 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00638 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00639 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00640 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00641 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00642 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00643 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00644 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00645 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00646 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00647 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00648 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00649 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00650 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00651 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00652 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00653 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00654 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00655 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00656 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00657 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00658 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00659 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00660 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00661 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00662 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00663 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00664 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00665 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00666 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00667 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00668 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00669 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00670 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00671 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00672 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00673 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00674 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00675 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00676 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00677 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00678 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00679 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00680 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00681 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00682 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00683 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00684 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00685 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00686 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00687 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00688 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00689 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00690 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00691 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00692 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00693 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00694 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00695 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00696 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00697 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00698 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00699 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00700 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00701 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00702 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00703 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00704 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00705 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00706 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00707 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00708 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00709 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00710 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00711 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00712 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00713 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00714 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00715 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00716 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00717 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00718 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00719 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00720 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00721 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00722 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00723 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00724 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00725 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00726 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00727 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00728 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00729 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00730 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00731 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00732 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00733 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00734 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00735 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00736 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00737 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00738 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00739 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00740 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00741 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00742 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00743 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00744 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00745 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00746 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00747 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00748 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00749 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00750 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00751 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00752 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00753 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00754 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00755 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00756 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00757 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00758 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00759 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00760 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00761 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00762 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00763 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00764 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00765 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00766 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00767 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00768 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00769 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00770 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00771 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00772 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00773 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00774 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00775 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00776 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00777 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00778 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00779 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00780 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00781 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00782 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00783 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00784 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00785 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00786 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00787 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00788 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00789 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00790 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00791 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00792 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00793 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00794 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00795 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00796 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00797 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00798 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00799 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00800 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00801 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00802 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00803 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00804 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00805 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00806 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00807 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00808 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00809 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00810 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00811 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00812 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00813 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00814 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00815 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00816 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00817 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00818 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00819 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00820 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00821 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00822 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00823 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00824 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00825 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00826 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00827 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00828 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00829 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00830 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00831 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00832 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00833 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00834 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00835 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00836 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00837 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00838 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00839 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00840 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00841 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00842 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00843 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00844 :: is_defined_var :: var_is_introduced;\n"+
"var bool: BOOL____00845 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00846 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00847 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00848 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00849 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00850 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00851 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00852 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00853 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00854 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00855 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00856 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00857 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00858 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00859 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00860 :: is_defined_var :: var_is_introduced;\n\
var bool: BOOL____00861 :: is_defined_var :: var_is_introduced;\n\
var 0..4: X____00001;\n\
var 0..4: X____00002;\n\
var 0..4: X____00003;\n\
var 0..4: X____00004;\n\
var 0..4: X____00005;\n\
var 0..4: X____00006;\n\
var 0..4: X____00007;\n\
var 0..4: X____00008;\n\
var 0..4: X____00009;\n\
var 0..4: X____00010;\n\
var 0..4: X____00011;\n\
var 0..4: X____00012;\n\
var 0..4: X____00013;\n\
var 0..4: X____00014;\n\
var 0..4: X____00015;\n\
var 0..4: X____00016;\n\
var 0..4: X____00017;\n\
var 2..4: X____00018;\n\
var 0..4: X____00019;\n\
var 0..4: X____00020;\n\
var 0..4: X____00021;\n\
var 0..4: X____00022;\n\
var 0..4: X____00023;\n\
var 0..4: X____00024;\n\
var 0..4: X____00025;\n\
var 0..4: X____00026;\n\
var 0..4: X____00027;\n\
var 0..4: X____00028;\n\
var 0..4: X____00029;\n\
var 0..4: X____00030;\n\
var 0..4: X____00031;\n\
var 0..4: X____00032;\n\
var 0..4: X____00033;\n\
var 0..4: X____00034;\n\
var 0..4: X____00035;\n\
var 0..4: X____00036;\n\
var 0..4: X____00037;\n\
var 0..4: X____00038;\n\
var 0..4: X____00039;\n\
var 0..4: X____00040;\n\
var 0..4: X____00041;\n\
var 0..4: X____00042;\n\
var 0..4: X____00043;\n\
var 0..4: X____00044;\n\
var 0..4: X____00045;\n\
var 0..4: X____00046;\n\
var 0..4: X____00047;\n\
var 0..4: X____00048;\n\
var 0..4: X____00049;\n\
var 0..4: X____00050;\n\
var 0..4: X____00051;\n\
var 0..4: X____00052;\n\
var 0..4: X____00053;\n\
var 0..4: X____00054;\n\
var 0..4: X____00055;\n\
var 0..4: X____00056;\n\
var 0..4: X____00057;\n\
var 0..4: X____00058;\n\
var 0..4: X____00059;\n\
var 0..4: X____00060;\n\
var 0..4: X____00061;\n\
var 0..4: X____00062;\n\
var 0..4: X____00063;\n\
var 0..4: X____00064;\n\
var 0..4: X____00065;\n\
var 0..4: X____00066;\n\
var 0..4: X____00067;\n\
var 0..4: X____00068;\n\
var 0..4: X____00069;\n\
var 0..4: X____00070;\n\
var 0..4: X____00071;\n\
var 0..4: X____00072;\n\
var 0..4: X____00073;\n\
var 0..4: X____00074;\n\
var 0..4: X____00075;\n\
var 0..4: X____00076;\n\
var 0..4: X____00077;\n\
var 0..4: X____00078;\n\
var 0..4: X____00079;\n\
var 0..4: X____00080;\n\
var 0..4: X____00081;\n\
var 0..4: X____00082;\n\
var 0..4: X____00083;\n\
var 0..4: X____00084;\n\
var 0..4: X____00085;\n\
var 0..4: X____00086;\n\
var 0..4: X____00087;\n\
var 0..4: X____00088;\n\
var 0..4: X____00089;\n\
var 0..4: X____00090;\n\
var 0..4: X____00091;\n\
var 0..4: X____00092;\n\
var 0..4: X____00093;\n\
var 0..4: X____00094;\n\
var 0..4: X____00095;\n\
array [1..144] of var 0..4: a :: output_array([0..11, 0..11]) = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009, X____00010, 0, 0, X____00011, X____00012, X____00013, X____00014, X____00015, X____00016, X____00017, 0, X____00018, 0, 0, 0, 0, X____00019, X____00020, X____00021, X____00022, X____00023, X____00024, X____00025, 0, X____00026, 0, 0, X____00027, X____00028, X____00029, X____00030, X____00031, X____00032, X____00033, X____00034, X____00035, X____00036, 0, 0, X____00037, X____00038, X____00039, X____00040, X____00041, X____00042, X____00043, X____00044, X____00045, X____00046, 0, 0, X____00047, X____00048, X____00049, X____00050, X____00051, X____00052, X____00053, X____00054, X____00055, X____00056, 0, 0, X____00057, X____00058, X____00059, X____00060, X____00061, X____00062, X____00063, X____00064, X____00065, X____00066, 0, 0, X____00067, X____00068, X____00069, 0, X____00070, X____00071, X____00072, X____00073, X____00074, X____00075, 0, 0, X____00076, X____00077, X____00078, X____00079, X____00080, X____00081, X____00082, X____00083, X____00084, X____00085, 0, 0, X____00086, X____00087, X____00088, X____00089, X____00090, X____00091, X____00092, X____00093, X____00094, X____00095, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];\n\
array [1..100] of var 0..4: a_flat = [X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009, X____00010, X____00011, X____00012, X____00013, X____00014, X____00015, X____00016, X____00017, 0, X____00018, 0, 0, X____00019, X____00020, X____00021, X____00022, X____00023, X____00024, X____00025, 0, X____00026, X____00027, X____00028, X____00029, X____00030, X____00031, X____00032, X____00033, X____00034, X____00035, X____00036, X____00037, X____00038, X____00039, X____00040, X____00041, X____00042, X____00043, X____00044, X____00045, X____00046, X____00047, X____00048, X____00049, X____00050, X____00051, X____00052, X____00053, X____00054, X____00055, X____00056, X____00057, X____00058, X____00059, X____00060, X____00061, X____00062, X____00063, X____00064, X____00065, X____00066, X____00067, X____00068, X____00069, 0, X____00070, X____00071, X____00072, X____00073, X____00074, X____00075, X____00076, X____00077, X____00078, X____00079, X____00080, X____00081, X____00082, X____00083, X____00084, X____00085, X____00086, X____00087, X____00088, X____00089, X____00090, X____00091, X____00092, X____00093, X____00094, X____00095];\n\
array [1..10] of var 0..4: col_sums = [1, 4, 1, 2, 3, 1, 3, 0, 4, 1];\n\
array [1..10] of var 0..6: row_sums = [2, 1, 5, 0, 0, 0, 1, 6, 1, 4];\n\
constraint array_bool_and([BOOL____00096, BOOL____00131], BOOL____00350) :: defines_var(BOOL____00350);\n\
constraint array_bool_and([BOOL____00098, BOOL____00133], BOOL____00352) :: defines_var(BOOL____00352);\n\
constraint array_bool_and([BOOL____00099, BOOL____00135], BOOL____00354) :: defines_var(BOOL____00354);\n\
constraint array_bool_and([BOOL____00100, BOOL____00138], BOOL____00356) :: defines_var(BOOL____00356);\n\
constraint array_bool_and([BOOL____00101, BOOL____00141], BOOL____00358) :: defines_var(BOOL____00358);\n\
constraint array_bool_and([BOOL____00102, BOOL____00144], BOOL____00360) :: defines_var(BOOL____00360);\n\
constraint array_bool_and([BOOL____00103, BOOL____00147], BOOL____00362) :: defines_var(BOOL____00362);\n\
constraint array_bool_and([BOOL____00105, BOOL____00150], BOOL____00363) :: defines_var(BOOL____00363);\n\
constraint array_bool_and([BOOL____00107, BOOL____00113], BOOL____00114) :: defines_var(BOOL____00114);\n\
constraint array_bool_and([BOOL____00109, BOOL____00110], BOOL____00111) :: defines_var(BOOL____00111);\n\
constraint array_bool_and([BOOL____00110, BOOL____00116], BOOL____00117) :: defines_var(BOOL____00117);\n\
constraint array_bool_and([BOOL____00110, BOOL____00155], BOOL____00365) :: defines_var(BOOL____00365);\n\
constraint array_bool_and([BOOL____00113, BOOL____00119], BOOL____00120) :: defines_var(BOOL____00120);\n\
constraint array_bool_and([BOOL____00113, BOOL____00158], BOOL____00367) :: defines_var(BOOL____00367);\n\
constraint array_bool_and([BOOL____00116, BOOL____00122], BOOL____00123) :: defines_var(BOOL____00123);\n\
constraint array_bool_and([BOOL____00116, BOOL____00161], BOOL____00369) :: defines_var(BOOL____00369);\n\
constraint array_bool_and([BOOL____00119, BOOL____00164], BOOL____00371) :: defines_var(BOOL____00371);\n\
constraint array_bool_and([BOOL____00122, BOOL____00126], BOOL____00127) :: defines_var(BOOL____00127);\n\
constraint array_bool_and([BOOL____00122, BOOL____00167], BOOL____00373) :: defines_var(BOOL____00373);\n\
constraint array_bool_and([BOOL____00126, BOOL____00173], BOOL____00376) :: defines_var(BOOL____00376);\n\
constraint array_bool_and([BOOL____00131, BOOL____00135], BOOL____00136) :: defines_var(BOOL____00136);\n\
constraint array_bool_and([BOOL____00131, BOOL____00178], BOOL____00378) :: defines_var(BOOL____00378);\n\
constraint array_bool_and([BOOL____00133, BOOL____00138], BOOL____00139) :: defines_var(BOOL____00139);\n\
constraint array_bool_and([BOOL____00133, BOOL____00181], BOOL____00380) :: defines_var(BOOL____00380);\n\
constraint array_bool_and([BOOL____00135, BOOL____00141], BOOL____00142) :: defines_var(BOOL____00142);\n\
constraint array_bool_and([BOOL____00135, BOOL____00184], BOOL____00382) :: defines_var(BOOL____00382);\n\
constraint array_bool_and([BOOL____00138, BOOL____00144], BOOL____00145) :: defines_var(BOOL____00145);\n\
constraint array_bool_and([BOOL____00138, BOOL____00187], BOOL____00384) :: defines_var(BOOL____00384);\n\
constraint array_bool_and([BOOL____00141, BOOL____00147], BOOL____00148) :: defines_var(BOOL____00148);\n\
constraint array_bool_and([BOOL____00141, BOOL____00190], BOOL____00386) :: defines_var(BOOL____00386);\n\
constraint array_bool_and([BOOL____00144, BOOL____00193], BOOL____00388) :: defines_var(BOOL____00388);\n\
constraint array_bool_and([BOOL____00147, BOOL____00150], BOOL____00151) :: defines_var(BOOL____00151);\n\
constraint array_bool_and([BOOL____00147, BOOL____00196], BOOL____00390) :: defines_var(BOOL____00390);\n\
constraint array_bool_and([BOOL____00150, BOOL____00202], BOOL____00393) :: defines_var(BOOL____00393);\n\
constraint array_bool_and([BOOL____00152, BOOL____00158], BOOL____00159) :: defines_var(BOOL____00159);\n\
constraint array_bool_and([BOOL____00152, BOOL____00206], BOOL____00395) :: defines_var(BOOL____00395);\n\
constraint array_bool_and([BOOL____00154, BOOL____00155], BOOL____00156) :: defines_var(BOOL____00156);\n\
constraint array_bool_and([BOOL____00155, BOOL____00161], BOOL____00162) :: defines_var(BOOL____00162);\n\
constraint array_bool_and([BOOL____00155, BOOL____00209], BOOL____00397) :: defines_var(BOOL____00397);\n\
constraint array_bool_and([BOOL____00158, BOOL____00164], BOOL____00165) :: defines_var(BOOL____00165);\n\
constraint array_bool_and([BOOL____00158, BOOL____00212], BOOL____00399) :: defines_var(BOOL____00399);\n\
constraint array_bool_and([BOOL____00161, BOOL____00167], BOOL____00168) :: defines_var(BOOL____00168);\n\
constraint array_bool_and([BOOL____00161, BOOL____00215], BOOL____00401) :: defines_var(BOOL____00401);\n\
constraint array_bool_and([BOOL____00164, BOOL____00170], BOOL____00171) :: defines_var(BOOL____00171);\n\
constraint array_bool_and([BOOL____00164, BOOL____00218], BOOL____00403) :: defines_var(BOOL____00403);\n\
constraint array_bool_and([BOOL____00167, BOOL____00173], BOOL____00174) :: defines_var(BOOL____00174);\n\
constraint array_bool_and([BOOL____00167, BOOL____00221], BOOL____00405) :: defines_var(BOOL____00405);\n\
constraint array_bool_and([BOOL____00170, BOOL____00224], BOOL____00407) :: defines_var(BOOL____00407);\n\
constraint array_bool_and([BOOL____00173, BOOL____00227], BOOL____00409) :: defines_var(BOOL____00409);\n\
constraint array_bool_and([BOOL____00175, BOOL____00230], BOOL____00411) :: defines_var(BOOL____00411);\n\
constraint array_bool_and([BOOL____00178, BOOL____00184], BOOL____00185) :: defines_var(BOOL____00185);\n\
constraint array_bool_and([BOOL____00178, BOOL____00234], BOOL____00413) :: defines_var(BOOL____00413);\n\
constraint array_bool_and([BOOL____00180, BOOL____00181], BOOL____00182) :: defines_var(BOOL____00182);\n\
constraint array_bool_and([BOOL____00181, BOOL____00187], BOOL____00188) :: defines_var(BOOL____00188);\n\
constraint array_bool_and([BOOL____00181, BOOL____00237], BOOL____00415) :: defines_var(BOOL____00415);\n\
constraint array_bool_and([BOOL____00184, BOOL____00190], BOOL____00191) :: defines_var(BOOL____00191);\n\
constraint array_bool_and([BOOL____00184, BOOL____00240], BOOL____00417) :: defines_var(BOOL____00417);\n\
constraint array_bool_and([BOOL____00187, BOOL____00193], BOOL____00194) :: defines_var(BOOL____00194);\n\
constraint array_bool_and([BOOL____00187, BOOL____00243], BOOL____00419) :: defines_var(BOOL____00419);\n\
constraint array_bool_and([BOOL____00190, BOOL____00196], BOOL____00197) :: defines_var(BOOL____00197);\n\
constraint array_bool_and([BOOL____00190, BOOL____00246], BOOL____00421) :: defines_var(BOOL____00421);\n\
constraint array_bool_and([BOOL____00193, BOOL____00199], BOOL____00200) :: defines_var(BOOL____00200);\n\
constraint array_bool_and([BOOL____00193, BOOL____00249], BOOL____00423) :: defines_var(BOOL____00423);\n\
constraint array_bool_and([BOOL____00196, BOOL____00202], BOOL____00203) :: defines_var(BOOL____00203);\n\
constraint array_bool_and([BOOL____00196, BOOL____00252], BOOL____00425) :: defines_var(BOOL____00425);\n\
constraint array_bool_and([BOOL____00199, BOOL____00255], BOOL____00427) :: defines_var(BOOL____00427);\n\
constraint array_bool_and([BOOL____00202, BOOL____00258], BOOL____00429) :: defines_var(BOOL____00429);\n\
constraint array_bool_and([BOOL____00206, BOOL____00212], BOOL____00213) :: defines_var(BOOL____00213);\n\
constraint array_bool_and([BOOL____00206, BOOL____00262], BOOL____00431) :: defines_var(BOOL____00431);\n\
constraint array_bool_and([BOOL____00208, BOOL____00209], BOOL____00210) :: defines_var(BOOL____00210);\n\
constraint array_bool_and([BOOL____00209, BOOL____00215], BOOL____00216) :: defines_var(BOOL____00216);\n\
constraint array_bool_and([BOOL____00209, BOOL____00265], BOOL____00433) :: defines_var(BOOL____00433);\n\
constraint array_bool_and([BOOL____00212, BOOL____00218], BOOL____00219) :: defines_var(BOOL____00219);\n\
constraint array_bool_and([BOOL____00215, BOOL____00221], BOOL____00222) :: defines_var(BOOL____00222);\n\
constraint array_bool_and([BOOL____00215, BOOL____00269], BOOL____00436) :: defines_var(BOOL____00436);\n\
constraint array_bool_and([BOOL____00218, BOOL____00224], BOOL____00225) :: defines_var(BOOL____00225);\n\
constraint array_bool_and([BOOL____00218, BOOL____00272], BOOL____00438) :: defines_var(BOOL____00438);\n\
constraint array_bool_and([BOOL____00221, BOOL____00227], BOOL____00228) :: defines_var(BOOL____00228);\n\
constraint array_bool_and([BOOL____00221, BOOL____00274], BOOL____00440) :: defines_var(BOOL____00440);\n\
constraint array_bool_and([BOOL____00224, BOOL____00230], BOOL____00231) :: defines_var(BOOL____00231);\n\
constraint array_bool_and([BOOL____00224, BOOL____00277], BOOL____00442) :: defines_var(BOOL____00442);\n\
constraint array_bool_and([BOOL____00227, BOOL____00280], BOOL____00444) :: defines_var(BOOL____00444);\n\
constraint array_bool_and([BOOL____00230, BOOL____00283], BOOL____00446) :: defines_var(BOOL____00446);\n\
constraint array_bool_and([BOOL____00234, BOOL____00240], BOOL____00241) :: defines_var(BOOL____00241);\n\
constraint array_bool_and([BOOL____00234, BOOL____00287], BOOL____00448) :: defines_var(BOOL____00448);\n\
constraint array_bool_and([BOOL____00236, BOOL____00237], BOOL____00238) :: defines_var(BOOL____00238);\n\
constraint array_bool_and([BOOL____00237, BOOL____00243], BOOL____00244) :: defines_var(BOOL____00244);\n\
constraint array_bool_and([BOOL____00237, BOOL____00290], BOOL____00450) :: defines_var(BOOL____00450);\n\
constraint array_bool_and([BOOL____00240, BOOL____00246], BOOL____00247) :: defines_var(BOOL____00247);\n"+
"constraint array_bool_and([BOOL____00240, BOOL____00293], BOOL____00452) :: defines_var(BOOL____00452);\n\
constraint array_bool_and([BOOL____00243, BOOL____00249], BOOL____00250) :: defines_var(BOOL____00250);\n\
constraint array_bool_and([BOOL____00246, BOOL____00252], BOOL____00253) :: defines_var(BOOL____00253);\n\
constraint array_bool_and([BOOL____00246, BOOL____00297], BOOL____00454) :: defines_var(BOOL____00454);\n\
constraint array_bool_and([BOOL____00249, BOOL____00255], BOOL____00256) :: defines_var(BOOL____00256);\n\
constraint array_bool_and([BOOL____00249, BOOL____00300], BOOL____00456) :: defines_var(BOOL____00456);\n\
constraint array_bool_and([BOOL____00252, BOOL____00258], BOOL____00259) :: defines_var(BOOL____00259);\n\
constraint array_bool_and([BOOL____00252, BOOL____00303], BOOL____00458) :: defines_var(BOOL____00458);\n\
constraint array_bool_and([BOOL____00255, BOOL____00306], BOOL____00460) :: defines_var(BOOL____00460);\n\
constraint array_bool_and([BOOL____00258, BOOL____00309], BOOL____00462) :: defines_var(BOOL____00462);\n\
constraint array_bool_and([BOOL____00262, BOOL____00313], BOOL____00464) :: defines_var(BOOL____00464);\n\
constraint array_bool_and([BOOL____00264, BOOL____00265], BOOL____00266) :: defines_var(BOOL____00266);\n\
constraint array_bool_and([BOOL____00265, BOOL____00269], BOOL____00270) :: defines_var(BOOL____00270);\n\
constraint array_bool_and([BOOL____00265, BOOL____00316], BOOL____00466) :: defines_var(BOOL____00466);\n\
constraint array_bool_and([BOOL____00269, BOOL____00274], BOOL____00275) :: defines_var(BOOL____00275);\n\
constraint array_bool_and([BOOL____00269, BOOL____00322], BOOL____00469) :: defines_var(BOOL____00469);\n\
constraint array_bool_and([BOOL____00272, BOOL____00277], BOOL____00278) :: defines_var(BOOL____00278);\n\
constraint array_bool_and([BOOL____00272, BOOL____00325], BOOL____00471) :: defines_var(BOOL____00471);\n\
constraint array_bool_and([BOOL____00274, BOOL____00280], BOOL____00281) :: defines_var(BOOL____00281);\n\
constraint array_bool_and([BOOL____00274, BOOL____00328], BOOL____00473) :: defines_var(BOOL____00473);\n\
constraint array_bool_and([BOOL____00277, BOOL____00283], BOOL____00284) :: defines_var(BOOL____00284);\n\
constraint array_bool_and([BOOL____00277, BOOL____00331], BOOL____00475) :: defines_var(BOOL____00475);\n\
constraint array_bool_and([BOOL____00280, BOOL____00334], BOOL____00477) :: defines_var(BOOL____00477);\n\
constraint array_bool_and([BOOL____00283, BOOL____00337], BOOL____00479) :: defines_var(BOOL____00479);\n\
constraint array_bool_and([BOOL____00287, BOOL____00293], BOOL____00294) :: defines_var(BOOL____00294);\n\
constraint array_bool_and([BOOL____00289, BOOL____00290], BOOL____00291) :: defines_var(BOOL____00291);\n\
constraint array_bool_and([BOOL____00293, BOOL____00297], BOOL____00298) :: defines_var(BOOL____00298);\n\
constraint array_bool_and([BOOL____00295, BOOL____00300], BOOL____00301) :: defines_var(BOOL____00301);\n\
constraint array_bool_and([BOOL____00297, BOOL____00303], BOOL____00304) :: defines_var(BOOL____00304);\n\
constraint array_bool_and([BOOL____00300, BOOL____00306], BOOL____00307) :: defines_var(BOOL____00307);\n\
constraint array_bool_and([BOOL____00303, BOOL____00309], BOOL____00310) :: defines_var(BOOL____00310);\n\
constraint array_bool_and([BOOL____00313, BOOL____00319], BOOL____00320) :: defines_var(BOOL____00320);\n\
constraint array_bool_and([BOOL____00315, BOOL____00316], BOOL____00317) :: defines_var(BOOL____00317);\n\
constraint array_bool_and([BOOL____00316, BOOL____00322], BOOL____00323) :: defines_var(BOOL____00323);\n\
constraint array_bool_and([BOOL____00319, BOOL____00325], BOOL____00326) :: defines_var(BOOL____00326);\n\
constraint array_bool_and([BOOL____00322, BOOL____00328], BOOL____00329) :: defines_var(BOOL____00329);\n\
constraint array_bool_and([BOOL____00325, BOOL____00331], BOOL____00332) :: defines_var(BOOL____00332);\n\
constraint array_bool_and([BOOL____00328, BOOL____00334], BOOL____00335) :: defines_var(BOOL____00335);\n\
constraint array_bool_and([BOOL____00331, BOOL____00337], BOOL____00338) :: defines_var(BOOL____00338);\n\
constraint array_bool_and([BOOL____00489, BOOL____00096], BOOL____00491) :: defines_var(BOOL____00491);\n\
constraint array_bool_and([BOOL____00489, BOOL____00109], BOOL____00490) :: defines_var(BOOL____00490);\n\
constraint array_bool_and([BOOL____00492, BOOL____00107], BOOL____00493) :: defines_var(BOOL____00493);\n\
constraint array_bool_and([BOOL____00496, BOOL____00110], BOOL____00497) :: defines_var(BOOL____00497);\n\
constraint array_bool_and([BOOL____00500, BOOL____00113], BOOL____00501) :: defines_var(BOOL____00501);\n\
constraint array_bool_and([BOOL____00504, BOOL____00116], BOOL____00505) :: defines_var(BOOL____00505);\n\
constraint array_bool_and([BOOL____00508, BOOL____00119], BOOL____00509) :: defines_var(BOOL____00509);\n\
constraint array_bool_and([BOOL____00512, BOOL____00122], BOOL____00513) :: defines_var(BOOL____00513);\n\
constraint array_bool_and([BOOL____00519, BOOL____00126], BOOL____00520) :: defines_var(BOOL____00520);\n\
constraint array_bool_and([BOOL____00524, BOOL____00104], BOOL____00525) :: defines_var(BOOL____00525);\n\
constraint array_bool_and([BOOL____00526, BOOL____00097], BOOL____00527) :: defines_var(BOOL____00527);\n\
constraint array_bool_and([BOOL____00528, BOOL____00107], BOOL____00529) :: defines_var(BOOL____00529);\n\
constraint array_bool_and([BOOL____00552, BOOL____00119], BOOL____00553) :: defines_var(BOOL____00553);\n\
constraint array_bool_and([BOOL____00554, BOOL____00104], BOOL____00555) :: defines_var(BOOL____00555);\n\
constraint array_bool_and([BOOL____00559, BOOL____00133], BOOL____00560) :: defines_var(BOOL____00560);\n\
constraint array_bool_and([BOOL____00581, BOOL____00170], BOOL____00582) :: defines_var(BOOL____00582);\n\
constraint array_bool_and([BOOL____00583, BOOL____00144], BOOL____00584) :: defines_var(BOOL____00584);\n\
constraint array_bool_and([BOOL____00585, BOOL____00175], BOOL____00586) :: defines_var(BOOL____00586);\n\
constraint array_bool_and([BOOL____00587, BOOL____00152], BOOL____00589) :: defines_var(BOOL____00589);\n\
constraint array_bool_and([BOOL____00587, BOOL____00180], BOOL____00588) :: defines_var(BOOL____00588);\n\
constraint array_bool_and([BOOL____00618, BOOL____00199], BOOL____00619) :: defines_var(BOOL____00619);\n\
constraint array_bool_and([BOOL____00624, BOOL____00173], BOOL____00625) :: defines_var(BOOL____00625);\n\
constraint array_bool_and([BOOL____00628, BOOL____00178], BOOL____00629) :: defines_var(BOOL____00629);\n\
constraint array_bool_and([BOOL____00664, BOOL____00199], BOOL____00665) :: defines_var(BOOL____00665);\n\
constraint array_bool_and([BOOL____00668, BOOL____00206], BOOL____00669) :: defines_var(BOOL____00669);\n\
constraint array_bool_and([BOOL____00704, BOOL____00227], BOOL____00705) :: defines_var(BOOL____00705);\n\
constraint array_bool_and([BOOL____00708, BOOL____00234], BOOL____00709) :: defines_var(BOOL____00709);\n\
constraint array_bool_and([BOOL____00718, BOOL____00212], BOOL____00719) :: defines_var(BOOL____00719);\n\
constraint array_bool_and([BOOL____00744, BOOL____00255], BOOL____00745) :: defines_var(BOOL____00745);\n\
constraint array_bool_and([BOOL____00748, BOOL____00262], BOOL____00749) :: defines_var(BOOL____00749);\n\
constraint array_bool_and([BOOL____00756, BOOL____00262], BOOL____00757) :: defines_var(BOOL____00757);\n\
constraint array_bool_and([BOOL____00760, BOOL____00272], BOOL____00761) :: defines_var(BOOL____00761);\n\
constraint array_bool_and([BOOL____00780, BOOL____00280], BOOL____00781) :: defines_var(BOOL____00781);\n\
constraint array_bool_and([BOOL____00784, BOOL____00287], BOOL____00785) :: defines_var(BOOL____00785);\n\
constraint array_bool_and([BOOL____00794, BOOL____00319], BOOL____00795) :: defines_var(BOOL____00795);\n\
constraint array_bool_and([BOOL____00820, BOOL____00306], BOOL____00821) :: defines_var(BOOL____00821);\n\
constraint array_bool_and([BOOL____00822, BOOL____00289], BOOL____00823) :: defines_var(BOOL____00823);\n\
constraint array_bool_and([BOOL____00824, BOOL____00313], BOOL____00825) :: defines_var(BOOL____00825);\n\
constraint array_bool_and([BOOL____00826, BOOL____00287], BOOL____00827) :: defines_var(BOOL____00827);\n\
constraint array_bool_and([BOOL____00830, BOOL____00290], BOOL____00831) :: defines_var(BOOL____00831);\n\
constraint array_bool_and([BOOL____00834, BOOL____00293], BOOL____00835) :: defines_var(BOOL____00835);\n\
constraint array_bool_and([BOOL____00838, BOOL____00295], BOOL____00839) :: defines_var(BOOL____00839);\n\
constraint array_bool_and([BOOL____00842, BOOL____00297], BOOL____00843) :: defines_var(BOOL____00843);\n\
constraint array_bool_and([BOOL____00846, BOOL____00300], BOOL____00847) :: defines_var(BOOL____00847);\n\
constraint array_bool_and([BOOL____00850, BOOL____00303], BOOL____00851) :: defines_var(BOOL____00851);\n\
constraint array_bool_and([BOOL____00854, BOOL____00306], BOOL____00855) :: defines_var(BOOL____00855);\n\
constraint array_bool_and([BOOL____00858, BOOL____00309], BOOL____00859) :: defines_var(BOOL____00859);\n\
constraint array_bool_and([BOOL____00860, BOOL____00334], BOOL____00861) :: defines_var(BOOL____00861);\n\
constraint array_bool_and([BOOL____00098, BOOL____00494, BOOL____00097], BOOL____00495) :: defines_var(BOOL____00495);\n\
constraint array_bool_and([BOOL____00099, BOOL____00498, BOOL____00096], BOOL____00499) :: defines_var(BOOL____00499);\n\
constraint array_bool_and([BOOL____00100, BOOL____00502, BOOL____00098], BOOL____00503) :: defines_var(BOOL____00503);\n\
constraint array_bool_and([BOOL____00101, BOOL____00506, BOOL____00099], BOOL____00507) :: defines_var(BOOL____00507);\n\
constraint array_bool_and([BOOL____00102, BOOL____00510, BOOL____00100], BOOL____00511) :: defines_var(BOOL____00511);\n\
constraint array_bool_and([BOOL____00103, BOOL____00514, BOOL____00101], BOOL____00515) :: defines_var(BOOL____00515);\n\
constraint array_bool_and([BOOL____00104, BOOL____00517, BOOL____00102], BOOL____00518) :: defines_var(BOOL____00518);\n\
constraint array_bool_and([BOOL____00105, BOOL____00521, BOOL____00103], BOOL____00522) :: defines_var(BOOL____00522);\n\
constraint array_bool_and([BOOL____00110, BOOL____00532, BOOL____00109], BOOL____00533) :: defines_var(BOOL____00533);\n\
constraint array_bool_and([BOOL____00113, BOOL____00536, BOOL____00107], BOOL____00537) :: defines_var(BOOL____00537);\n\
constraint array_bool_and([BOOL____00116, BOOL____00540, BOOL____00110], BOOL____00541) :: defines_var(BOOL____00541);\n\
constraint array_bool_and([BOOL____00119, BOOL____00544, BOOL____00113], BOOL____00545) :: defines_var(BOOL____00545);\n\
constraint array_bool_and([BOOL____00122, BOOL____00548, BOOL____00116], BOOL____00549) :: defines_var(BOOL____00549);\n\
constraint array_bool_and([BOOL____00131, BOOL____00530, BOOL____00096], BOOL____00531) :: defines_var(BOOL____00531);\n\
constraint array_bool_and([BOOL____00133, BOOL____00534, BOOL____00098], BOOL____00535) :: defines_var(BOOL____00535);\n\
constraint array_bool_and([BOOL____00135, BOOL____00538, BOOL____00099], BOOL____00539) :: defines_var(BOOL____00539);\n\
constraint array_bool_and([BOOL____00135, BOOL____00563, BOOL____00131], BOOL____00564) :: defines_var(BOOL____00564);\n\
constraint array_bool_and([BOOL____00138, BOOL____00542, BOOL____00100], BOOL____00543) :: defines_var(BOOL____00543);\n\
constraint array_bool_and([BOOL____00138, BOOL____00567, BOOL____00133], BOOL____00568) :: defines_var(BOOL____00568);\n\
constraint array_bool_and([BOOL____00141, BOOL____00546, BOOL____00101], BOOL____00547) :: defines_var(BOOL____00547);\n\
constraint array_bool_and([BOOL____00141, BOOL____00571, BOOL____00135], BOOL____00572) :: defines_var(BOOL____00572);\n\
constraint array_bool_and([BOOL____00144, BOOL____00550, BOOL____00102], BOOL____00551) :: defines_var(BOOL____00551);\n\
constraint array_bool_and([BOOL____00144, BOOL____00575, BOOL____00138], BOOL____00576) :: defines_var(BOOL____00576);\n\
constraint array_bool_and([BOOL____00147, BOOL____00579, BOOL____00141], BOOL____00580) :: defines_var(BOOL____00580);\n\
constraint array_bool_and([BOOL____00152, BOOL____00557, BOOL____00107], BOOL____00558) :: defines_var(BOOL____00558);\n\
constraint array_bool_and([BOOL____00155, BOOL____00561, BOOL____00110], BOOL____00562) :: defines_var(BOOL____00562);\n\
constraint array_bool_and([BOOL____00155, BOOL____00592, BOOL____00154], BOOL____00593) :: defines_var(BOOL____00593);\n\
constraint array_bool_and([BOOL____00158, BOOL____00565, BOOL____00113], BOOL____00566) :: defines_var(BOOL____00566);\n\
constraint array_bool_and([BOOL____00158, BOOL____00596, BOOL____00152], BOOL____00597) :: defines_var(BOOL____00597);\n\
constraint array_bool_and([BOOL____00161, BOOL____00569, BOOL____00116], BOOL____00570) :: defines_var(BOOL____00570);\n\
constraint array_bool_and([BOOL____00161, BOOL____00600, BOOL____00155], BOOL____00601) :: defines_var(BOOL____00601);\n\
constraint array_bool_and([BOOL____00164, BOOL____00573, BOOL____00119], BOOL____00574) :: defines_var(BOOL____00574);\n\
constraint array_bool_and([BOOL____00164, BOOL____00604, BOOL____00158], BOOL____00605) :: defines_var(BOOL____00605);\n\
constraint array_bool_and([BOOL____00167, BOOL____00577, BOOL____00122], BOOL____00578) :: defines_var(BOOL____00578);\n\
constraint array_bool_and([BOOL____00167, BOOL____00608, BOOL____00161], BOOL____00609) :: defines_var(BOOL____00609);\n\
constraint array_bool_and([BOOL____00170, BOOL____00612, BOOL____00164], BOOL____00613) :: defines_var(BOOL____00613);\n\
constraint array_bool_and([BOOL____00173, BOOL____00616, BOOL____00167], BOOL____00617) :: defines_var(BOOL____00617);\n\
constraint array_bool_and([BOOL____00175, BOOL____00620, BOOL____00170], BOOL____00621) :: defines_var(BOOL____00621);\n\
constraint array_bool_and([BOOL____00178, BOOL____00590, BOOL____00131], BOOL____00591) :: defines_var(BOOL____00591);\n\
constraint array_bool_and([BOOL____00181, BOOL____00594, BOOL____00133], BOOL____00595) :: defines_var(BOOL____00595);\n\
constraint array_bool_and([BOOL____00181, BOOL____00632, BOOL____00180], BOOL____00633) :: defines_var(BOOL____00633);\n\
constraint array_bool_and([BOOL____00184, BOOL____00598, BOOL____00135], BOOL____00599) :: defines_var(BOOL____00599);\n\
constraint array_bool_and([BOOL____00184, BOOL____00636, BOOL____00178], BOOL____00637) :: defines_var(BOOL____00637);\n\
constraint array_bool_and([BOOL____00187, BOOL____00602, BOOL____00138], BOOL____00603) :: defines_var(BOOL____00603);\n\
constraint array_bool_and([BOOL____00187, BOOL____00640, BOOL____00181], BOOL____00641) :: defines_var(BOOL____00641);\n\
constraint array_bool_and([BOOL____00190, BOOL____00606, BOOL____00141], BOOL____00607) :: defines_var(BOOL____00607);\n\
constraint array_bool_and([BOOL____00190, BOOL____00644, BOOL____00184], BOOL____00645) :: defines_var(BOOL____00645);\n\
constraint array_bool_and([BOOL____00193, BOOL____00610, BOOL____00144], BOOL____00611) :: defines_var(BOOL____00611);\n\
constraint array_bool_and([BOOL____00193, BOOL____00648, BOOL____00187], BOOL____00649) :: defines_var(BOOL____00649);\n\
constraint array_bool_and([BOOL____00196, BOOL____00614, BOOL____00147], BOOL____00615) :: defines_var(BOOL____00615);\n\
constraint array_bool_and([BOOL____00196, BOOL____00652, BOOL____00190], BOOL____00653) :: defines_var(BOOL____00653);\n\
constraint array_bool_and([BOOL____00199, BOOL____00656, BOOL____00193], BOOL____00657) :: defines_var(BOOL____00657);\n\
constraint array_bool_and([BOOL____00202, BOOL____00622, BOOL____00150], BOOL____00623) :: defines_var(BOOL____00623);\n\
constraint array_bool_and([BOOL____00202, BOOL____00660, BOOL____00196], BOOL____00661) :: defines_var(BOOL____00661);\n\
constraint array_bool_and([BOOL____00206, BOOL____00630, BOOL____00152], BOOL____00631) :: defines_var(BOOL____00631);\n\
constraint array_bool_and([BOOL____00208, BOOL____00626, BOOL____00154], BOOL____00627) :: defines_var(BOOL____00627);\n\
constraint array_bool_and([BOOL____00209, BOOL____00634, BOOL____00155], BOOL____00635) :: defines_var(BOOL____00635);\n\
constraint array_bool_and([BOOL____00209, BOOL____00672, BOOL____00208], BOOL____00673) :: defines_var(BOOL____00673);\n"+
"constraint array_bool_and([BOOL____00212, BOOL____00638, BOOL____00158], BOOL____00639) :: defines_var(BOOL____00639);\n\
constraint array_bool_and([BOOL____00212, BOOL____00676, BOOL____00206], BOOL____00677) :: defines_var(BOOL____00677);\n\
constraint array_bool_and([BOOL____00215, BOOL____00642, BOOL____00161], BOOL____00643) :: defines_var(BOOL____00643);\n\
constraint array_bool_and([BOOL____00215, BOOL____00680, BOOL____00209], BOOL____00681) :: defines_var(BOOL____00681);\n\
constraint array_bool_and([BOOL____00218, BOOL____00646, BOOL____00164], BOOL____00647) :: defines_var(BOOL____00647);\n\
constraint array_bool_and([BOOL____00218, BOOL____00684, BOOL____00212], BOOL____00685) :: defines_var(BOOL____00685);\n\
constraint array_bool_and([BOOL____00221, BOOL____00650, BOOL____00167], BOOL____00651) :: defines_var(BOOL____00651);\n\
constraint array_bool_and([BOOL____00221, BOOL____00688, BOOL____00215], BOOL____00689) :: defines_var(BOOL____00689);\n\
constraint array_bool_and([BOOL____00224, BOOL____00654, BOOL____00170], BOOL____00655) :: defines_var(BOOL____00655);\n\
constraint array_bool_and([BOOL____00224, BOOL____00692, BOOL____00218], BOOL____00693) :: defines_var(BOOL____00693);\n\
constraint array_bool_and([BOOL____00227, BOOL____00658, BOOL____00173], BOOL____00659) :: defines_var(BOOL____00659);\n\
constraint array_bool_and([BOOL____00227, BOOL____00696, BOOL____00221], BOOL____00697) :: defines_var(BOOL____00697);\n\
constraint array_bool_and([BOOL____00230, BOOL____00662, BOOL____00175], BOOL____00663) :: defines_var(BOOL____00663);\n\
constraint array_bool_and([BOOL____00230, BOOL____00700, BOOL____00224], BOOL____00701) :: defines_var(BOOL____00701);\n\
constraint array_bool_and([BOOL____00234, BOOL____00670, BOOL____00178], BOOL____00671) :: defines_var(BOOL____00671);\n\
constraint array_bool_and([BOOL____00236, BOOL____00666, BOOL____00180], BOOL____00667) :: defines_var(BOOL____00667);\n\
constraint array_bool_and([BOOL____00237, BOOL____00674, BOOL____00181], BOOL____00675) :: defines_var(BOOL____00675);\n\
constraint array_bool_and([BOOL____00237, BOOL____00712, BOOL____00236], BOOL____00713) :: defines_var(BOOL____00713);\n\
constraint array_bool_and([BOOL____00240, BOOL____00678, BOOL____00184], BOOL____00679) :: defines_var(BOOL____00679);\n\
constraint array_bool_and([BOOL____00240, BOOL____00716, BOOL____00234], BOOL____00717) :: defines_var(BOOL____00717);\n\
constraint array_bool_and([BOOL____00243, BOOL____00682, BOOL____00187], BOOL____00683) :: defines_var(BOOL____00683);\n\
constraint array_bool_and([BOOL____00243, BOOL____00720, BOOL____00237], BOOL____00721) :: defines_var(BOOL____00721);\n\
constraint array_bool_and([BOOL____00246, BOOL____00686, BOOL____00190], BOOL____00687) :: defines_var(BOOL____00687);\n\
constraint array_bool_and([BOOL____00246, BOOL____00724, BOOL____00240], BOOL____00725) :: defines_var(BOOL____00725);\n\
constraint array_bool_and([BOOL____00249, BOOL____00690, BOOL____00193], BOOL____00691) :: defines_var(BOOL____00691);\n\
constraint array_bool_and([BOOL____00249, BOOL____00728, BOOL____00243], BOOL____00729) :: defines_var(BOOL____00729);\n\
constraint array_bool_and([BOOL____00252, BOOL____00694, BOOL____00196], BOOL____00695) :: defines_var(BOOL____00695);\n\
constraint array_bool_and([BOOL____00252, BOOL____00732, BOOL____00246], BOOL____00733) :: defines_var(BOOL____00733);\n\
constraint array_bool_and([BOOL____00255, BOOL____00698, BOOL____00199], BOOL____00699) :: defines_var(BOOL____00699);\n\
constraint array_bool_and([BOOL____00255, BOOL____00736, BOOL____00249], BOOL____00737) :: defines_var(BOOL____00737);\n\
constraint array_bool_and([BOOL____00258, BOOL____00702, BOOL____00202], BOOL____00703) :: defines_var(BOOL____00703);\n\
constraint array_bool_and([BOOL____00258, BOOL____00740, BOOL____00252], BOOL____00741) :: defines_var(BOOL____00741);\n\
constraint array_bool_and([BOOL____00262, BOOL____00710, BOOL____00206], BOOL____00711) :: defines_var(BOOL____00711);\n\
constraint array_bool_and([BOOL____00264, BOOL____00706, BOOL____00208], BOOL____00707) :: defines_var(BOOL____00707);\n\
constraint array_bool_and([BOOL____00265, BOOL____00714, BOOL____00209], BOOL____00715) :: defines_var(BOOL____00715);\n\
constraint array_bool_and([BOOL____00265, BOOL____00752, BOOL____00264], BOOL____00753) :: defines_var(BOOL____00753);\n\
constraint array_bool_and([BOOL____00269, BOOL____00722, BOOL____00215], BOOL____00723) :: defines_var(BOOL____00723);\n\
constraint array_bool_and([BOOL____00272, BOOL____00726, BOOL____00218], BOOL____00727) :: defines_var(BOOL____00727);\n\
constraint array_bool_and([BOOL____00274, BOOL____00730, BOOL____00221], BOOL____00731) :: defines_var(BOOL____00731);\n\
constraint array_bool_and([BOOL____00274, BOOL____00764, BOOL____00269], BOOL____00765) :: defines_var(BOOL____00765);\n\
constraint array_bool_and([BOOL____00277, BOOL____00734, BOOL____00224], BOOL____00735) :: defines_var(BOOL____00735);\n\
constraint array_bool_and([BOOL____00277, BOOL____00768, BOOL____00272], BOOL____00769) :: defines_var(BOOL____00769);\n\
constraint array_bool_and([BOOL____00280, BOOL____00738, BOOL____00227], BOOL____00739) :: defines_var(BOOL____00739);\n\
constraint array_bool_and([BOOL____00280, BOOL____00772, BOOL____00274], BOOL____00773) :: defines_var(BOOL____00773);\n\
constraint array_bool_and([BOOL____00283, BOOL____00742, BOOL____00230], BOOL____00743) :: defines_var(BOOL____00743);\n\
constraint array_bool_and([BOOL____00283, BOOL____00776, BOOL____00277], BOOL____00777) :: defines_var(BOOL____00777);\n\
constraint array_bool_and([BOOL____00287, BOOL____00750, BOOL____00234], BOOL____00751) :: defines_var(BOOL____00751);\n\
constraint array_bool_and([BOOL____00289, BOOL____00746, BOOL____00236], BOOL____00747) :: defines_var(BOOL____00747);\n\
constraint array_bool_and([BOOL____00290, BOOL____00754, BOOL____00237], BOOL____00755) :: defines_var(BOOL____00755);\n\
constraint array_bool_and([BOOL____00290, BOOL____00788, BOOL____00289], BOOL____00789) :: defines_var(BOOL____00789);\n\
constraint array_bool_and([BOOL____00293, BOOL____00792, BOOL____00287], BOOL____00793) :: defines_var(BOOL____00793);\n\
constraint array_bool_and([BOOL____00295, BOOL____00758, BOOL____00243], BOOL____00759) :: defines_var(BOOL____00759);\n\
constraint array_bool_and([BOOL____00295, BOOL____00796, BOOL____00290], BOOL____00797) :: defines_var(BOOL____00797);\n\
constraint array_bool_and([BOOL____00297, BOOL____00762, BOOL____00246], BOOL____00763) :: defines_var(BOOL____00763);\n\
constraint array_bool_and([BOOL____00297, BOOL____00800, BOOL____00293], BOOL____00801) :: defines_var(BOOL____00801);\n\
constraint array_bool_and([BOOL____00300, BOOL____00766, BOOL____00249], BOOL____00767) :: defines_var(BOOL____00767);\n\
constraint array_bool_and([BOOL____00300, BOOL____00804, BOOL____00295], BOOL____00805) :: defines_var(BOOL____00805);\n\
constraint array_bool_and([BOOL____00303, BOOL____00770, BOOL____00252], BOOL____00771) :: defines_var(BOOL____00771);\n\
constraint array_bool_and([BOOL____00303, BOOL____00808, BOOL____00297], BOOL____00809) :: defines_var(BOOL____00809);\n\
constraint array_bool_and([BOOL____00306, BOOL____00774, BOOL____00255], BOOL____00775) :: defines_var(BOOL____00775);\n\
constraint array_bool_and([BOOL____00306, BOOL____00812, BOOL____00300], BOOL____00813) :: defines_var(BOOL____00813);\n\
constraint array_bool_and([BOOL____00309, BOOL____00778, BOOL____00258], BOOL____00779) :: defines_var(BOOL____00779);\n\
constraint array_bool_and([BOOL____00309, BOOL____00816, BOOL____00303], BOOL____00817) :: defines_var(BOOL____00817);\n\
constraint array_bool_and([BOOL____00313, BOOL____00786, BOOL____00262], BOOL____00787) :: defines_var(BOOL____00787);\n\
constraint array_bool_and([BOOL____00315, BOOL____00782, BOOL____00264], BOOL____00783) :: defines_var(BOOL____00783);\n\
constraint array_bool_and([BOOL____00316, BOOL____00790, BOOL____00265], BOOL____00791) :: defines_var(BOOL____00791);\n\
constraint array_bool_and([BOOL____00316, BOOL____00828, BOOL____00315], BOOL____00829) :: defines_var(BOOL____00829);\n\
constraint array_bool_and([BOOL____00319, BOOL____00832, BOOL____00313], BOOL____00833) :: defines_var(BOOL____00833);\n\
constraint array_bool_and([BOOL____00322, BOOL____00798, BOOL____00269], BOOL____00799) :: defines_var(BOOL____00799);\n\
constraint array_bool_and([BOOL____00322, BOOL____00836, BOOL____00316], BOOL____00837) :: defines_var(BOOL____00837);\n\
constraint array_bool_and([BOOL____00325, BOOL____00802, BOOL____00272], BOOL____00803) :: defines_var(BOOL____00803);\n\
constraint array_bool_and([BOOL____00325, BOOL____00840, BOOL____00319], BOOL____00841) :: defines_var(BOOL____00841);\n\
constraint array_bool_and([BOOL____00328, BOOL____00806, BOOL____00274], BOOL____00807) :: defines_var(BOOL____00807);\n\
constraint array_bool_and([BOOL____00328, BOOL____00844, BOOL____00322], BOOL____00845) :: defines_var(BOOL____00845);\n\
constraint array_bool_and([BOOL____00331, BOOL____00810, BOOL____00277], BOOL____00811) :: defines_var(BOOL____00811);\n\
constraint array_bool_and([BOOL____00331, BOOL____00848, BOOL____00325], BOOL____00849) :: defines_var(BOOL____00849);\n\
constraint array_bool_and([BOOL____00334, BOOL____00814, BOOL____00280], BOOL____00815) :: defines_var(BOOL____00815);\n\
constraint array_bool_and([BOOL____00334, BOOL____00852, BOOL____00328], BOOL____00853) :: defines_var(BOOL____00853);\n\
constraint array_bool_and([BOOL____00337, BOOL____00818, BOOL____00283], BOOL____00819) :: defines_var(BOOL____00819);\n\
constraint array_bool_and([BOOL____00337, BOOL____00856, BOOL____00331], BOOL____00857) :: defines_var(BOOL____00857);\n\
constraint array_bool_or([BOOL____00491, BOOL____00490, BOOL____00097], true);\n\
constraint array_bool_or([BOOL____00495, BOOL____00493, BOOL____00096], true);\n\
constraint array_bool_or([BOOL____00499, BOOL____00497, BOOL____00098], true);\n\
constraint array_bool_or([BOOL____00503, BOOL____00501, BOOL____00099], true);\n\
constraint array_bool_or([BOOL____00507, BOOL____00505, BOOL____00100], true);\n\
constraint array_bool_or([BOOL____00511, BOOL____00509, BOOL____00101], true);\n\
constraint array_bool_or([BOOL____00515, BOOL____00513, BOOL____00102], true);\n\
constraint array_bool_or([BOOL____00518, BOOL____00103, BOOL____00516], true);\n\
constraint array_bool_or([BOOL____00522, BOOL____00520, BOOL____00104], true);\n\
constraint array_bool_or([BOOL____00525, BOOL____00105, BOOL____00523], true);\n\
constraint array_bool_or([BOOL____00529, BOOL____00527, BOOL____00109], true);\n\
constraint array_bool_or([BOOL____00533, BOOL____00531, BOOL____00107], true);\n\
constraint array_bool_or([BOOL____00537, BOOL____00535, BOOL____00110], true);\n\
constraint array_bool_or([BOOL____00541, BOOL____00539, BOOL____00113], true);\n\
constraint array_bool_or([BOOL____00545, BOOL____00543, BOOL____00116], true);\n\
constraint array_bool_or([BOOL____00549, BOOL____00547, BOOL____00119], true);\n\
constraint array_bool_or([BOOL____00553, BOOL____00551, BOOL____00122], true);\n\
constraint array_bool_or([BOOL____00556, BOOL____00555, BOOL____00126], true);\n\
constraint array_bool_or([BOOL____00560, BOOL____00558, BOOL____00131], true);\n\
constraint array_bool_or([BOOL____00564, BOOL____00562, BOOL____00133], true);\n\
constraint array_bool_or([BOOL____00568, BOOL____00566, BOOL____00135], true);\n\
constraint array_bool_or([BOOL____00572, BOOL____00570, BOOL____00138], true);\n\
constraint array_bool_or([BOOL____00576, BOOL____00574, BOOL____00141], true);\n\
constraint array_bool_or([BOOL____00580, BOOL____00578, BOOL____00144], true);\n\
constraint array_bool_or([BOOL____00584, BOOL____00582, BOOL____00147], true);\n\
constraint array_bool_or([BOOL____00585, BOOL____00586, BOOL____00150], true);\n\
constraint array_bool_or([BOOL____00589, BOOL____00588, BOOL____00154], true);\n\
constraint array_bool_or([BOOL____00593, BOOL____00591, BOOL____00152], true);\n\
constraint array_bool_or([BOOL____00597, BOOL____00595, BOOL____00155], true);\n\
constraint array_bool_or([BOOL____00601, BOOL____00599, BOOL____00158], true);\n\
constraint array_bool_or([BOOL____00605, BOOL____00603, BOOL____00161], true);\n\
constraint array_bool_or([BOOL____00609, BOOL____00607, BOOL____00164], true);\n\
constraint array_bool_or([BOOL____00613, BOOL____00611, BOOL____00167], true);\n\
constraint array_bool_or([BOOL____00617, BOOL____00615, BOOL____00170], true);\n\
constraint array_bool_or([BOOL____00621, BOOL____00619, BOOL____00173], true);\n\
constraint array_bool_or([BOOL____00625, BOOL____00623, BOOL____00175], true);\n\
constraint array_bool_or([BOOL____00629, BOOL____00627, BOOL____00180], true);\n\
constraint array_bool_or([BOOL____00633, BOOL____00631, BOOL____00178], true);\n\
constraint array_bool_or([BOOL____00637, BOOL____00635, BOOL____00181], true);\n\
constraint array_bool_or([BOOL____00641, BOOL____00639, BOOL____00184], true);\n\
constraint array_bool_or([BOOL____00645, BOOL____00643, BOOL____00187], true);\n\
constraint array_bool_or([BOOL____00649, BOOL____00647, BOOL____00190], true);\n\
constraint array_bool_or([BOOL____00653, BOOL____00651, BOOL____00193], true);\n\
constraint array_bool_or([BOOL____00657, BOOL____00655, BOOL____00196], true);\n\
constraint array_bool_or([BOOL____00661, BOOL____00659, BOOL____00199], true);\n\
constraint array_bool_or([BOOL____00665, BOOL____00663, BOOL____00202], true);\n\
constraint array_bool_or([BOOL____00669, BOOL____00667, BOOL____00208], true);\n\
constraint array_bool_or([BOOL____00673, BOOL____00671, BOOL____00206], true);\n\
constraint array_bool_or([BOOL____00677, BOOL____00675, BOOL____00209], true);\n\
constraint array_bool_or([BOOL____00681, BOOL____00679, BOOL____00212], true);\n\
constraint array_bool_or([BOOL____00685, BOOL____00683, BOOL____00215], true);\n\
constraint array_bool_or([BOOL____00689, BOOL____00687, BOOL____00218], true);\n\
constraint array_bool_or([BOOL____00693, BOOL____00691, BOOL____00221], true);\n\
constraint array_bool_or([BOOL____00697, BOOL____00695, BOOL____00224], true);\n\
constraint array_bool_or([BOOL____00701, BOOL____00699, BOOL____00227], true);\n\
constraint array_bool_or([BOOL____00705, BOOL____00703, BOOL____00230], true);\n\
constraint array_bool_or([BOOL____00709, BOOL____00707, BOOL____00236], true);\n\
constraint array_bool_or([BOOL____00713, BOOL____00711, BOOL____00234], true);\n\
constraint array_bool_or([BOOL____00717, BOOL____00715, BOOL____00237], true);\n\
constraint array_bool_or([BOOL____00721, BOOL____00719, BOOL____00240], true);\n\
constraint array_bool_or([BOOL____00725, BOOL____00723, BOOL____00243], true);\n\
constraint array_bool_or([BOOL____00729, BOOL____00727, BOOL____00246], true);\n\
constraint array_bool_or([BOOL____00733, BOOL____00731, BOOL____00249], true);\n\
constraint array_bool_or([BOOL____00737, BOOL____00735, BOOL____00252], true);\n\
constraint array_bool_or([BOOL____00741, BOOL____00739, BOOL____00255], true);\n\
constraint array_bool_or([BOOL____00745, BOOL____00743, BOOL____00258], true);\n\
constraint array_bool_or([BOOL____00749, BOOL____00747, BOOL____00264], true);\n\
constraint array_bool_or([BOOL____00753, BOOL____00751, BOOL____00262], true);\n\
constraint array_bool_or([BOOL____00757, BOOL____00755, BOOL____00265], true);\n\
constraint array_bool_or([BOOL____00761, BOOL____00759, BOOL____00269], true);\n\
constraint array_bool_or([BOOL____00765, BOOL____00763, BOOL____00272], true);\n\
constraint array_bool_or([BOOL____00769, BOOL____00767, BOOL____00274], true);\n\
constraint array_bool_or([BOOL____00773, BOOL____00771, BOOL____00277], true);\n\
constraint array_bool_or([BOOL____00777, BOOL____00775, BOOL____00280], true);\n\
constraint array_bool_or([BOOL____00781, BOOL____00779, BOOL____00283], true);\n\
constraint array_bool_or([BOOL____00785, BOOL____00783, BOOL____00289], true);\n\
constraint array_bool_or([BOOL____00789, BOOL____00787, BOOL____00287], true);\n\
constraint array_bool_or([BOOL____00793, BOOL____00791, BOOL____00290], true);\n\
constraint array_bool_or([BOOL____00797, BOOL____00795, BOOL____00293], true);\n\
constraint array_bool_or([BOOL____00801, BOOL____00799, BOOL____00295], true);\n\
constraint array_bool_or([BOOL____00805, BOOL____00803, BOOL____00297], true);\n\
constraint array_bool_or([BOOL____00809, BOOL____00807, BOOL____00300], true);\n\
constraint array_bool_or([BOOL____00813, BOOL____00811, BOOL____00303], true);\n\
constraint array_bool_or([BOOL____00817, BOOL____00815, BOOL____00306], true);\n\
constraint array_bool_or([BOOL____00821, BOOL____00819, BOOL____00309], true);\n"+
"constraint array_bool_or([BOOL____00825, BOOL____00823, BOOL____00315], true);\n\
constraint array_bool_or([BOOL____00829, BOOL____00827, BOOL____00313], true);\n\
constraint array_bool_or([BOOL____00833, BOOL____00831, BOOL____00316], true);\n\
constraint array_bool_or([BOOL____00837, BOOL____00835, BOOL____00319], true);\n\
constraint array_bool_or([BOOL____00841, BOOL____00839, BOOL____00322], true);\n\
constraint array_bool_or([BOOL____00845, BOOL____00843, BOOL____00325], true);\n\
constraint array_bool_or([BOOL____00849, BOOL____00847, BOOL____00328], true);\n\
constraint array_bool_or([BOOL____00853, BOOL____00851, BOOL____00331], true);\n\
constraint array_bool_or([BOOL____00857, BOOL____00855, BOOL____00334], true);\n\
constraint array_bool_or([BOOL____00861, BOOL____00859, BOOL____00337], true);\n\
constraint bool_le(BOOL____00106, BOOL____00107);\n\
constraint bool_le(BOOL____00108, BOOL____00111);\n\
constraint bool_le(BOOL____00112, BOOL____00114);\n\
constraint bool_le(BOOL____00115, BOOL____00117);\n\
constraint bool_le(BOOL____00118, BOOL____00120);\n\
constraint bool_le(BOOL____00121, BOOL____00123);\n\
constraint bool_le(BOOL____00124, BOOL____00119);\n\
constraint bool_le(BOOL____00125, BOOL____00127);\n\
constraint bool_le(BOOL____00129, BOOL____00126);\n\
constraint bool_le(BOOL____00130, BOOL____00131);\n\
constraint bool_le(BOOL____00132, BOOL____00133);\n\
constraint bool_le(BOOL____00134, BOOL____00136);\n\
constraint bool_le(BOOL____00137, BOOL____00139);\n\
constraint bool_le(BOOL____00140, BOOL____00142);\n\
constraint bool_le(BOOL____00143, BOOL____00145);\n\
constraint bool_le(BOOL____00146, BOOL____00148);\n\
constraint bool_le(BOOL____00149, BOOL____00151);\n\
constraint bool_le(BOOL____00149, BOOL____00363);\n\
constraint bool_le(BOOL____00153, BOOL____00156);\n\
constraint bool_le(BOOL____00157, BOOL____00159);\n\
constraint bool_le(BOOL____00160, BOOL____00162);\n\
constraint bool_le(BOOL____00163, BOOL____00165);\n\
constraint bool_le(BOOL____00166, BOOL____00168);\n\
constraint bool_le(BOOL____00169, BOOL____00171);\n\
constraint bool_le(BOOL____00172, BOOL____00174);\n\
constraint bool_le(BOOL____00176, BOOL____00173);\n\
constraint bool_le(BOOL____00177, BOOL____00178);\n\
constraint bool_le(BOOL____00179, BOOL____00182);\n\
constraint bool_le(BOOL____00183, BOOL____00185);\n\
constraint bool_le(BOOL____00186, BOOL____00188);\n\
constraint bool_le(BOOL____00189, BOOL____00191);\n\
constraint bool_le(BOOL____00192, BOOL____00194);\n\
constraint bool_le(BOOL____00195, BOOL____00197);\n\
constraint bool_le(BOOL____00198, BOOL____00200);\n\
constraint bool_le(BOOL____00201, BOOL____00203);\n\
constraint bool_le(BOOL____00204, BOOL____00199);\n\
constraint bool_le(BOOL____00205, BOOL____00206);\n\
constraint bool_le(BOOL____00207, BOOL____00210);\n\
constraint bool_le(BOOL____00211, BOOL____00213);\n\
constraint bool_le(BOOL____00214, BOOL____00216);\n\
constraint bool_le(BOOL____00217, BOOL____00219);\n\
constraint bool_le(BOOL____00220, BOOL____00222);\n\
constraint bool_le(BOOL____00223, BOOL____00225);\n\
constraint bool_le(BOOL____00226, BOOL____00228);\n\
constraint bool_le(BOOL____00229, BOOL____00231);\n\
constraint bool_le(BOOL____00232, BOOL____00227);\n\
constraint bool_le(BOOL____00233, BOOL____00234);\n\
constraint bool_le(BOOL____00235, BOOL____00238);\n\
constraint bool_le(BOOL____00239, BOOL____00241);\n\
constraint bool_le(BOOL____00242, BOOL____00244);\n\
constraint bool_le(BOOL____00245, BOOL____00247);\n\
constraint bool_le(BOOL____00248, BOOL____00250);\n\
constraint bool_le(BOOL____00251, BOOL____00253);\n\
constraint bool_le(BOOL____00254, BOOL____00256);\n\
constraint bool_le(BOOL____00257, BOOL____00259);\n\
constraint bool_le(BOOL____00260, BOOL____00255);\n\
constraint bool_le(BOOL____00261, BOOL____00262);\n\
constraint bool_le(BOOL____00263, BOOL____00266);\n\
constraint bool_le(BOOL____00267, BOOL____00262);\n\
constraint bool_le(BOOL____00268, BOOL____00270);\n\
constraint bool_le(BOOL____00271, BOOL____00272);\n\
constraint bool_le(BOOL____00273, BOOL____00275);\n\
constraint bool_le(BOOL____00276, BOOL____00278);\n\
constraint bool_le(BOOL____00279, BOOL____00281);\n\
constraint bool_le(BOOL____00282, BOOL____00284);\n\
constraint bool_le(BOOL____00285, BOOL____00280);\n\
constraint bool_le(BOOL____00286, BOOL____00287);\n\
constraint bool_le(BOOL____00288, BOOL____00291);\n\
constraint bool_le(BOOL____00292, BOOL____00294);\n\
constraint bool_le(BOOL____00296, BOOL____00298);\n\
constraint bool_le(BOOL____00299, BOOL____00301);\n\
constraint bool_le(BOOL____00302, BOOL____00304);\n\
constraint bool_le(BOOL____00305, BOOL____00307);\n\
constraint bool_le(BOOL____00308, BOOL____00310);\n\
constraint bool_le(BOOL____00311, BOOL____00306);\n\
constraint bool_le(BOOL____00312, BOOL____00313);\n\
constraint bool_le(BOOL____00314, BOOL____00317);\n\
constraint bool_le(BOOL____00318, BOOL____00320);\n\
constraint bool_le(BOOL____00321, BOOL____00323);\n\
constraint bool_le(BOOL____00324, BOOL____00326);\n\
constraint bool_le(BOOL____00327, BOOL____00329);\n\
constraint bool_le(BOOL____00330, BOOL____00332);\n\
constraint bool_le(BOOL____00333, BOOL____00335);\n\
constraint bool_le(BOOL____00336, BOOL____00338);\n\
constraint bool_le(BOOL____00339, BOOL____00334);\n\
constraint bool_le(BOOL____00340, BOOL____00107);\n\
constraint bool_le(BOOL____00341, BOOL____00110);\n\
constraint bool_le(BOOL____00342, BOOL____00113);\n\
constraint bool_le(BOOL____00343, BOOL____00116);\n\
constraint bool_le(BOOL____00344, BOOL____00119);\n\
constraint bool_le(BOOL____00345, BOOL____00122);\n\
constraint bool_le(BOOL____00347, BOOL____00126);\n\
constraint bool_le(BOOL____00349, BOOL____00350);\n\
constraint bool_le(BOOL____00351, BOOL____00352);\n\
constraint bool_le(BOOL____00353, BOOL____00354);\n\
constraint bool_le(BOOL____00355, BOOL____00356);\n\
constraint bool_le(BOOL____00357, BOOL____00358);\n\
constraint bool_le(BOOL____00359, BOOL____00360);\n\
constraint bool_le(BOOL____00361, BOOL____00362);\n\
constraint bool_le(BOOL____00364, BOOL____00365);\n\
constraint bool_le(BOOL____00366, BOOL____00367);\n\
constraint bool_le(BOOL____00368, BOOL____00369);\n\
constraint bool_le(BOOL____00370, BOOL____00371);\n\
constraint bool_le(BOOL____00372, BOOL____00373);\n\
constraint bool_le(BOOL____00374, BOOL____00170);\n\
constraint bool_le(BOOL____00375, BOOL____00376);\n\
constraint bool_le(BOOL____00377, BOOL____00378);\n\
constraint bool_le(BOOL____00379, BOOL____00380);\n\
constraint bool_le(BOOL____00381, BOOL____00382);\n\
constraint bool_le(BOOL____00383, BOOL____00384);\n\
constraint bool_le(BOOL____00385, BOOL____00386);\n\
constraint bool_le(BOOL____00387, BOOL____00388);\n\
constraint bool_le(BOOL____00389, BOOL____00390);\n\
constraint bool_le(BOOL____00391, BOOL____00199);\n\
constraint bool_le(BOOL____00392, BOOL____00393);\n\
constraint bool_le(BOOL____00394, BOOL____00395);\n\
constraint bool_le(BOOL____00396, BOOL____00397);\n\
constraint bool_le(BOOL____00398, BOOL____00399);\n\
constraint bool_le(BOOL____00400, BOOL____00401);\n\
constraint bool_le(BOOL____00402, BOOL____00403);\n\
constraint bool_le(BOOL____00404, BOOL____00405);\n\
constraint bool_le(BOOL____00406, BOOL____00407);\n\
constraint bool_le(BOOL____00408, BOOL____00409);\n\
constraint bool_le(BOOL____00410, BOOL____00411);\n\
constraint bool_le(BOOL____00412, BOOL____00413);\n\
constraint bool_le(BOOL____00414, BOOL____00415);\n\
constraint bool_le(BOOL____00416, BOOL____00417);\n\
constraint bool_le(BOOL____00418, BOOL____00419);\n\
constraint bool_le(BOOL____00420, BOOL____00421);\n\
constraint bool_le(BOOL____00422, BOOL____00423);\n\
constraint bool_le(BOOL____00424, BOOL____00425);\n\
constraint bool_le(BOOL____00426, BOOL____00427);\n\
constraint bool_le(BOOL____00428, BOOL____00429);\n\
constraint bool_le(BOOL____00430, BOOL____00431);\n\
constraint bool_le(BOOL____00432, BOOL____00433);\n\
constraint bool_le(BOOL____00434, BOOL____00212);\n\
constraint bool_le(BOOL____00435, BOOL____00436);\n\
constraint bool_le(BOOL____00437, BOOL____00438);\n\
constraint bool_le(BOOL____00439, BOOL____00440);\n\
constraint bool_le(BOOL____00441, BOOL____00442);\n\
constraint bool_le(BOOL____00443, BOOL____00444);\n\
constraint bool_le(BOOL____00445, BOOL____00446);\n\
constraint bool_le(BOOL____00447, BOOL____00448);\n\
constraint bool_le(BOOL____00449, BOOL____00450);\n\
constraint bool_le(BOOL____00451, BOOL____00452);\n\
constraint bool_le(BOOL____00453, BOOL____00454);\n\
constraint bool_le(BOOL____00455, BOOL____00456);\n\
constraint bool_le(BOOL____00457, BOOL____00458);\n\
constraint bool_le(BOOL____00459, BOOL____00460);\n\
constraint bool_le(BOOL____00461, BOOL____00462);\n\
constraint bool_le(BOOL____00463, BOOL____00464);\n\
constraint bool_le(BOOL____00465, BOOL____00466);\n\
constraint bool_le(BOOL____00467, BOOL____00319);\n\
constraint bool_le(BOOL____00468, BOOL____00469);\n\
constraint bool_le(BOOL____00470, BOOL____00471);\n\
constraint bool_le(BOOL____00472, BOOL____00473);\n\
constraint bool_le(BOOL____00474, BOOL____00475);\n\
constraint bool_le(BOOL____00476, BOOL____00477);\n\
constraint bool_le(BOOL____00478, BOOL____00479);\n\
constraint bool_le(BOOL____00480, BOOL____00287);\n\
constraint bool_le(BOOL____00481, BOOL____00290);\n\
constraint bool_le(BOOL____00482, BOOL____00293);\n\
constraint bool_le(BOOL____00483, BOOL____00295);\n\
constraint bool_le(BOOL____00484, BOOL____00297);\n\
constraint bool_le(BOOL____00485, BOOL____00300);\n\
constraint bool_le(BOOL____00486, BOOL____00303);\n\
constraint bool_le(BOOL____00487, BOOL____00306);\n\
constraint bool_le(BOOL____00488, BOOL____00309);\n\
constraint count([0, X____00019, X____00020, X____00021, X____00022, X____00023, X____00024, X____00025, 0, X____00026], 0, 5);\n\
constraint count([X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009, X____00010], 0, 8);\n\
constraint count([X____00001, X____00011, 0, X____00027, X____00037, X____00047, X____00057, X____00067, X____00076, X____00086], 0, 9);\n\
constraint count([X____00002, X____00012, X____00019, X____00028, X____00038, X____00048, X____00058, X____00068, X____00077, X____00087], 0, 6);\n\
constraint count([X____00003, X____00013, X____00020, X____00029, X____00039, X____00049, X____00059, X____00069, X____00078, X____00088], 0, 9);\n\
constraint count([X____00004, X____00014, X____00021, X____00030, X____00040, X____00050, X____00060, 0, X____00079, X____00089], 0, 8);\n\
constraint count([X____00005, X____00015, X____00022, X____00031, X____00041, X____00051, X____00061, X____00070, X____00080, X____00090], 0, 7);\n\
constraint count([X____00006, X____00016, X____00023, X____00032, X____00042, X____00052, X____00062, X____00071, X____00081, X____00091], 0, 9);\n\
constraint count([X____00007, X____00017, X____00024, X____00033, X____00043, X____00053, X____00063, X____00072, X____00082, X____00092], 0, 7);\n\
constraint count([X____00008, 0, X____00025, X____00034, X____00044, X____00054, X____00064, X____00073, X____00083, X____00093], 0, 10);\n\
constraint count([X____00009, X____00018, 0, X____00035, X____00045, X____00055, X____00065, X____00074, X____00084, X____00094], 0, 6);\n\
constraint count([X____00010, 0, X____00026, X____00036, X____00046, X____00056, X____00066, X____00075, X____00085, X____00095], 0, 9);\n\
constraint count([X____00011, X____00012, X____00013, X____00014, X____00015, X____00016, X____00017, 0, X____00018, 0], 0, 9);\n\
constraint count([X____00027, X____00028, X____00029, X____00030, X____00031, X____00032, X____00033, X____00034, X____00035, X____00036], 0, 10);\n\
constraint count([X____00037, X____00038, X____00039, X____00040, X____00041, X____00042, X____00043, X____00044, X____00045, X____00046], 0, 10);\n\
constraint count([X____00047, X____00048, X____00049, X____00050, X____00051, X____00052, X____00053, X____00054, X____00055, X____00056], 0, 10);\n\
constraint count([X____00057, X____00058, X____00059, X____00060, X____00061, X____00062, X____00063, X____00064, X____00065, X____00066], 0, 9);\n\
constraint count([X____00067, X____00068, X____00069, 0, X____00070, X____00071, X____00072, X____00073, X____00074, X____00075], 0, 4);\n\
constraint count([X____00076, X____00077, X____00078, X____00079, X____00080, X____00081, X____00082, X____00083, X____00084, X____00085], 0, 9);\n\
constraint count([X____00086, X____00087, X____00088, X____00089, X____00090, X____00091, X____00092, X____00093, X____00094, X____00095], 0, 6);\n\
constraint count(a_flat, 1, 10);\n\
constraint count(a_flat, 2, 6);\n\
constraint count(a_flat, 3, 3);\n\
constraint count(a_flat, 4, 1);\n\
constraint int_eq_reif(X____00001, 0, BOOL____00097) :: defines_var(BOOL____00097);\n\
constraint int_eq_reif(X____00001, 1, BOOL____00489) :: defines_var(BOOL____00489);\n\
constraint int_eq_reif(X____00002, 0, BOOL____00096) :: defines_var(BOOL____00096);\n\
constraint int_eq_reif(X____00002, 1, BOOL____00494) :: defines_var(BOOL____00494);\n\
constraint int_eq_reif(X____00003, 0, BOOL____00098) :: defines_var(BOOL____00098);\n\
constraint int_eq_reif(X____00003, 1, BOOL____00498) :: defines_var(BOOL____00498);\n\
constraint int_eq_reif(X____00004, 0, BOOL____00099) :: defines_var(BOOL____00099);\n\
constraint int_eq_reif(X____00004, 1, BOOL____00502) :: defines_var(BOOL____00502);\n\
constraint int_eq_reif(X____00005, 0, BOOL____00100) :: defines_var(BOOL____00100);\n\
constraint int_eq_reif(X____00005, 1, BOOL____00506) :: defines_var(BOOL____00506);\n\
constraint int_eq_reif(X____00006, 0, BOOL____00101) :: defines_var(BOOL____00101);\n\
constraint int_eq_reif(X____00006, 1, BOOL____00510) :: defines_var(BOOL____00510);\n\
constraint int_eq_reif(X____00007, 0, BOOL____00102) :: defines_var(BOOL____00102);\n\
constraint int_eq_reif(X____00007, 1, BOOL____00514) :: defines_var(BOOL____00514);\n\
constraint int_eq_reif(X____00008, 0, BOOL____00103) :: defines_var(BOOL____00103);\n\
constraint int_eq_reif(X____00008, 1, BOOL____00517) :: defines_var(BOOL____00517);\n\
constraint int_eq_reif(X____00009, 0, BOOL____00104) :: defines_var(BOOL____00104);\n\
constraint int_eq_reif(X____00009, 1, BOOL____00521) :: defines_var(BOOL____00521);\n\
constraint int_eq_reif(X____00010, 0, BOOL____00105) :: defines_var(BOOL____00105);\n\
constraint int_eq_reif(X____00010, 1, BOOL____00524) :: defines_var(BOOL____00524);\n\
constraint int_eq_reif(X____00011, 0, BOOL____00109) :: defines_var(BOOL____00109);\n\
constraint int_eq_reif(X____00011, 1, BOOL____00526) :: defines_var(BOOL____00526);\n\
constraint int_eq_reif(X____00012, 0, BOOL____00107) :: defines_var(BOOL____00107);\n\
constraint int_eq_reif(X____00013, 0, BOOL____00110) :: defines_var(BOOL____00110);\n\
constraint int_eq_reif(X____00014, 0, BOOL____00113) :: defines_var(BOOL____00113);\n\
constraint int_eq_reif(X____00015, 0, BOOL____00116) :: defines_var(BOOL____00116);\n\
constraint int_eq_reif(X____00016, 0, BOOL____00119) :: defines_var(BOOL____00119);\n\
constraint int_eq_reif(X____00017, 0, BOOL____00122) :: defines_var(BOOL____00122);\n\
constraint int_eq_reif(X____00018, 0, BOOL____00126) :: defines_var(BOOL____00126);\n\
constraint int_eq_reif(X____00018, 1, BOOL____00554) :: defines_var(BOOL____00554);\n\
constraint int_eq_reif(X____00019, 0, BOOL____00131) :: defines_var(BOOL____00131);\n\
constraint int_eq_reif(X____00019, 1, BOOL____00557) :: defines_var(BOOL____00557);\n\
constraint int_eq_reif(X____00020, 0, BOOL____00133) :: defines_var(BOOL____00133);\n\
constraint int_eq_reif(X____00021, 0, BOOL____00135) :: defines_var(BOOL____00135);\n\
constraint int_eq_reif(X____00022, 0, BOOL____00138) :: defines_var(BOOL____00138);\n\
constraint int_eq_reif(X____00023, 0, BOOL____00141) :: defines_var(BOOL____00141);\n\
constraint int_eq_reif(X____00024, 0, BOOL____00144) :: defines_var(BOOL____00144);\n\
constraint int_eq_reif(X____00025, 0, BOOL____00147) :: defines_var(BOOL____00147);\n\
constraint int_eq_reif(X____00025, 1, BOOL____00583) :: defines_var(BOOL____00583);\n\
constraint int_eq_reif(X____00026, 0, BOOL____00150) :: defines_var(BOOL____00150);\n\
constraint int_eq_reif(X____00026, 1, BOOL____00585) :: defines_var(BOOL____00585);\n\
constraint int_eq_reif(X____00027, 0, BOOL____00154) :: defines_var(BOOL____00154);\n\
constraint int_eq_reif(X____00027, 1, BOOL____00587) :: defines_var(BOOL____00587);\n\
constraint int_eq_reif(X____00028, 0, BOOL____00152) :: defines_var(BOOL____00152);\n\
constraint int_eq_reif(X____00029, 0, BOOL____00155) :: defines_var(BOOL____00155);\n\
constraint int_eq_reif(X____00030, 0, BOOL____00158) :: defines_var(BOOL____00158);\n\
constraint int_eq_reif(X____00031, 0, BOOL____00161) :: defines_var(BOOL____00161);\n\
constraint int_eq_reif(X____00032, 0, BOOL____00164) :: defines_var(BOOL____00164);\n"+
"constraint int_eq_reif(X____00033, 0, BOOL____00167) :: defines_var(BOOL____00167);\n\
constraint int_eq_reif(X____00034, 0, BOOL____00170) :: defines_var(BOOL____00170);\n\
constraint int_eq_reif(X____00035, 0, BOOL____00173) :: defines_var(BOOL____00173);\n\
constraint int_eq_reif(X____00035, 1, BOOL____00620) :: defines_var(BOOL____00620);\n\
constraint int_eq_reif(X____00036, 0, BOOL____00175) :: defines_var(BOOL____00175);\n\
constraint int_eq_reif(X____00037, 0, BOOL____00180) :: defines_var(BOOL____00180);\n\
constraint int_eq_reif(X____00037, 1, BOOL____00626) :: defines_var(BOOL____00626);\n\
constraint int_eq_reif(X____00038, 0, BOOL____00178) :: defines_var(BOOL____00178);\n\
constraint int_eq_reif(X____00039, 0, BOOL____00181) :: defines_var(BOOL____00181);\n\
constraint int_eq_reif(X____00040, 0, BOOL____00184) :: defines_var(BOOL____00184);\n\
constraint int_eq_reif(X____00041, 0, BOOL____00187) :: defines_var(BOOL____00187);\n\
constraint int_eq_reif(X____00042, 0, BOOL____00190) :: defines_var(BOOL____00190);\n\
constraint int_eq_reif(X____00043, 0, BOOL____00193) :: defines_var(BOOL____00193);\n\
constraint int_eq_reif(X____00044, 0, BOOL____00196) :: defines_var(BOOL____00196);\n\
constraint int_eq_reif(X____00045, 0, BOOL____00199) :: defines_var(BOOL____00199);\n\
constraint int_eq_reif(X____00046, 0, BOOL____00202) :: defines_var(BOOL____00202);\n\
constraint int_eq_reif(X____00047, 0, BOOL____00208) :: defines_var(BOOL____00208);\n\
constraint int_eq_reif(X____00047, 1, BOOL____00666) :: defines_var(BOOL____00666);\n\
constraint int_eq_reif(X____00048, 0, BOOL____00206) :: defines_var(BOOL____00206);\n\
constraint int_eq_reif(X____00049, 0, BOOL____00209) :: defines_var(BOOL____00209);\n\
constraint int_eq_reif(X____00050, 0, BOOL____00212) :: defines_var(BOOL____00212);\n\
constraint int_eq_reif(X____00051, 0, BOOL____00215) :: defines_var(BOOL____00215);\n\
constraint int_eq_reif(X____00052, 0, BOOL____00218) :: defines_var(BOOL____00218);\n\
constraint int_eq_reif(X____00053, 0, BOOL____00221) :: defines_var(BOOL____00221);\n\
constraint int_eq_reif(X____00054, 0, BOOL____00224) :: defines_var(BOOL____00224);\n\
constraint int_eq_reif(X____00055, 0, BOOL____00227) :: defines_var(BOOL____00227);\n\
constraint int_eq_reif(X____00056, 0, BOOL____00230) :: defines_var(BOOL____00230);\n\
constraint int_eq_reif(X____00057, 0, BOOL____00236) :: defines_var(BOOL____00236);\n\
constraint int_eq_reif(X____00057, 1, BOOL____00706) :: defines_var(BOOL____00706);\n\
constraint int_eq_reif(X____00058, 0, BOOL____00234) :: defines_var(BOOL____00234);\n\
constraint int_eq_reif(X____00059, 0, BOOL____00237) :: defines_var(BOOL____00237);\n\
constraint int_eq_reif(X____00060, 0, BOOL____00240) :: defines_var(BOOL____00240);\n\
constraint int_eq_reif(X____00061, 0, BOOL____00243) :: defines_var(BOOL____00243);\n\
constraint int_eq_reif(X____00062, 0, BOOL____00246) :: defines_var(BOOL____00246);\n\
constraint int_eq_reif(X____00063, 0, BOOL____00249) :: defines_var(BOOL____00249);\n\
constraint int_eq_reif(X____00064, 0, BOOL____00252) :: defines_var(BOOL____00252);\n\
constraint int_eq_reif(X____00065, 0, BOOL____00255) :: defines_var(BOOL____00255);\n\
constraint int_eq_reif(X____00066, 0, BOOL____00258) :: defines_var(BOOL____00258);\n\
constraint int_eq_reif(X____00067, 0, BOOL____00264) :: defines_var(BOOL____00264);\n\
constraint int_eq_reif(X____00067, 1, BOOL____00746) :: defines_var(BOOL____00746);\n\
constraint int_eq_reif(X____00068, 0, BOOL____00262) :: defines_var(BOOL____00262);\n\
constraint int_eq_reif(X____00069, 0, BOOL____00265) :: defines_var(BOOL____00265);\n\
constraint int_eq_reif(X____00070, 0, BOOL____00269) :: defines_var(BOOL____00269);\n\
constraint int_eq_reif(X____00070, 1, BOOL____00758) :: defines_var(BOOL____00758);\n\
constraint int_eq_reif(X____00071, 0, BOOL____00272) :: defines_var(BOOL____00272);\n\
constraint int_eq_reif(X____00072, 0, BOOL____00274) :: defines_var(BOOL____00274);\n\
constraint int_eq_reif(X____00073, 0, BOOL____00277) :: defines_var(BOOL____00277);\n\
constraint int_eq_reif(X____00074, 0, BOOL____00280) :: defines_var(BOOL____00280);\n\
constraint int_eq_reif(X____00075, 0, BOOL____00283) :: defines_var(BOOL____00283);\n\
constraint int_eq_reif(X____00076, 0, BOOL____00289) :: defines_var(BOOL____00289);\n\
constraint int_eq_reif(X____00076, 1, BOOL____00782) :: defines_var(BOOL____00782);\n\
constraint int_eq_reif(X____00077, 0, BOOL____00287) :: defines_var(BOOL____00287);\n\
constraint int_eq_reif(X____00078, 0, BOOL____00290) :: defines_var(BOOL____00290);\n\
constraint int_eq_reif(X____00079, 0, BOOL____00293) :: defines_var(BOOL____00293);\n\
constraint int_eq_reif(X____00079, 1, BOOL____00796) :: defines_var(BOOL____00796);\n\
constraint int_eq_reif(X____00080, 0, BOOL____00295) :: defines_var(BOOL____00295);\n\
constraint int_eq_reif(X____00081, 0, BOOL____00297) :: defines_var(BOOL____00297);\n\
constraint int_eq_reif(X____00082, 0, BOOL____00300) :: defines_var(BOOL____00300);\n\
constraint int_eq_reif(X____00083, 0, BOOL____00303) :: defines_var(BOOL____00303);\n\
constraint int_eq_reif(X____00084, 0, BOOL____00306) :: defines_var(BOOL____00306);\n\
constraint int_eq_reif(X____00085, 0, BOOL____00309) :: defines_var(BOOL____00309);\n\
constraint int_eq_reif(X____00086, 0, BOOL____00315) :: defines_var(BOOL____00315);\n\
constraint int_eq_reif(X____00086, 1, BOOL____00822) :: defines_var(BOOL____00822);\n\
constraint int_eq_reif(X____00087, 0, BOOL____00313) :: defines_var(BOOL____00313);\n\
constraint int_eq_reif(X____00088, 0, BOOL____00316) :: defines_var(BOOL____00316);\n\
constraint int_eq_reif(X____00089, 0, BOOL____00319) :: defines_var(BOOL____00319);\n\
constraint int_eq_reif(X____00090, 0, BOOL____00322) :: defines_var(BOOL____00322);\n\
constraint int_eq_reif(X____00091, 0, BOOL____00325) :: defines_var(BOOL____00325);\n\
constraint int_eq_reif(X____00092, 0, BOOL____00328) :: defines_var(BOOL____00328);\n\
constraint int_eq_reif(X____00093, 0, BOOL____00331) :: defines_var(BOOL____00331);\n\
constraint int_eq_reif(X____00094, 0, BOOL____00334) :: defines_var(BOOL____00334);\n\
constraint int_eq_reif(X____00095, 0, BOOL____00337) :: defines_var(BOOL____00337);\n\
constraint int_le_reif(1, X____00008, BOOL____00125) :: defines_var(BOOL____00125);\n\
constraint int_le_reif(1, X____00010, BOOL____00129) :: defines_var(BOOL____00129);\n\
constraint int_le_reif(1, X____00011, BOOL____00130) :: defines_var(BOOL____00130);\n\
constraint int_le_reif(1, X____00017, BOOL____00361) :: defines_var(BOOL____00361);\n\
constraint int_le_reif(1, X____00018, BOOL____00149) :: defines_var(BOOL____00149);\n\
constraint int_le_reif(1, X____00025, BOOL____00375) :: defines_var(BOOL____00375);\n\
constraint int_le_reif(1, X____00060, BOOL____00268) :: defines_var(BOOL____00268);\n\
constraint int_le_reif(1, X____00069, BOOL____00451) :: defines_var(BOOL____00451);\n\
constraint int_lin_eq_reif([-1, 1], [X____00001, X____00002], 1, BOOL____00492) :: defines_var(BOOL____00492);\n\
constraint int_lin_eq_reif([-1, 1], [X____00001, X____00011], 1, BOOL____00528) :: defines_var(BOOL____00528);\n\
constraint int_lin_eq_reif([-1, 1], [X____00002, X____00003], 1, BOOL____00496) :: defines_var(BOOL____00496);\n\
constraint int_lin_eq_reif([-1, 1], [X____00002, X____00012], 1, BOOL____00532) :: defines_var(BOOL____00532);\n\
constraint int_lin_eq_reif([-1, 1], [X____00003, X____00004], 1, BOOL____00500) :: defines_var(BOOL____00500);\n\
constraint int_lin_eq_reif([-1, 1], [X____00003, X____00013], 1, BOOL____00536) :: defines_var(BOOL____00536);\n\
constraint int_lin_eq_reif([-1, 1], [X____00004, X____00005], 1, BOOL____00504) :: defines_var(BOOL____00504);\n\
constraint int_lin_eq_reif([-1, 1], [X____00004, X____00014], 1, BOOL____00540) :: defines_var(BOOL____00540);\n\
constraint int_lin_eq_reif([-1, 1], [X____00005, X____00006], 1, BOOL____00508) :: defines_var(BOOL____00508);\n\
constraint int_lin_eq_reif([-1, 1], [X____00005, X____00015], 1, BOOL____00544) :: defines_var(BOOL____00544);\n\
constraint int_lin_eq_reif([-1, 1], [X____00006, X____00007], 1, BOOL____00512) :: defines_var(BOOL____00512);\n\
constraint int_lin_eq_reif([-1, 1], [X____00006, X____00016], 1, BOOL____00548) :: defines_var(BOOL____00548);\n\
constraint int_lin_eq_reif([-1, 1], [X____00007, X____00008], 1, BOOL____00516) :: defines_var(BOOL____00516);\n\
constraint int_lin_eq_reif([-1, 1], [X____00007, X____00017], 1, BOOL____00552) :: defines_var(BOOL____00552);\n\
constraint int_lin_eq_reif([-1, 1], [X____00008, X____00009], 1, BOOL____00519) :: defines_var(BOOL____00519);\n\
constraint int_lin_eq_reif([-1, 1], [X____00009, X____00010], 1, BOOL____00523) :: defines_var(BOOL____00523);\n\
constraint int_lin_eq_reif([-1, 1], [X____00009, X____00018], 1, BOOL____00556) :: defines_var(BOOL____00556);\n\
constraint int_lin_eq_reif([-1, 1], [X____00011, X____00012], 1, BOOL____00530) :: defines_var(BOOL____00530);\n\
constraint int_lin_eq_reif([-1, 1], [X____00012, X____00013], 1, BOOL____00534) :: defines_var(BOOL____00534);\n\
constraint int_lin_eq_reif([-1, 1], [X____00012, X____00019], 1, BOOL____00559) :: defines_var(BOOL____00559);\n\
constraint int_lin_eq_reif([-1, 1], [X____00013, X____00014], 1, BOOL____00538) :: defines_var(BOOL____00538);\n\
constraint int_lin_eq_reif([-1, 1], [X____00013, X____00020], 1, BOOL____00563) :: defines_var(BOOL____00563);\n\
constraint int_lin_eq_reif([-1, 1], [X____00014, X____00015], 1, BOOL____00542) :: defines_var(BOOL____00542);\n\
constraint int_lin_eq_reif([-1, 1], [X____00014, X____00021], 1, BOOL____00567) :: defines_var(BOOL____00567);\n\
constraint int_lin_eq_reif([-1, 1], [X____00015, X____00016], 1, BOOL____00546) :: defines_var(BOOL____00546);\n\
constraint int_lin_eq_reif([-1, 1], [X____00015, X____00022], 1, BOOL____00571) :: defines_var(BOOL____00571);\n\
constraint int_lin_eq_reif([-1, 1], [X____00016, X____00017], 1, BOOL____00550) :: defines_var(BOOL____00550);\n\
constraint int_lin_eq_reif([-1, 1], [X____00016, X____00023], 1, BOOL____00575) :: defines_var(BOOL____00575);\n\
constraint int_lin_eq_reif([-1, 1], [X____00017, X____00024], 1, BOOL____00579) :: defines_var(BOOL____00579);\n\
constraint int_lin_eq_reif([-1, 1], [X____00019, X____00020], 1, BOOL____00561) :: defines_var(BOOL____00561);\n\
constraint int_lin_eq_reif([-1, 1], [X____00019, X____00028], 1, BOOL____00592) :: defines_var(BOOL____00592);\n\
constraint int_lin_eq_reif([-1, 1], [X____00020, X____00021], 1, BOOL____00565) :: defines_var(BOOL____00565);\n\
constraint int_lin_eq_reif([-1, 1], [X____00020, X____00029], 1, BOOL____00596) :: defines_var(BOOL____00596);\n\
constraint int_lin_eq_reif([-1, 1], [X____00021, X____00022], 1, BOOL____00569) :: defines_var(BOOL____00569);\n\
constraint int_lin_eq_reif([-1, 1], [X____00021, X____00030], 1, BOOL____00600) :: defines_var(BOOL____00600);\n\
constraint int_lin_eq_reif([-1, 1], [X____00022, X____00023], 1, BOOL____00573) :: defines_var(BOOL____00573);\n\
constraint int_lin_eq_reif([-1, 1], [X____00022, X____00031], 1, BOOL____00604) :: defines_var(BOOL____00604);\n\
constraint int_lin_eq_reif([-1, 1], [X____00023, X____00024], 1, BOOL____00577) :: defines_var(BOOL____00577);\n\
constraint int_lin_eq_reif([-1, 1], [X____00023, X____00032], 1, BOOL____00608) :: defines_var(BOOL____00608);\n\
constraint int_lin_eq_reif([-1, 1], [X____00024, X____00025], 1, BOOL____00581) :: defines_var(BOOL____00581);\n\
constraint int_lin_eq_reif([-1, 1], [X____00024, X____00033], 1, BOOL____00612) :: defines_var(BOOL____00612);\n\
constraint int_lin_eq_reif([-1, 1], [X____00025, X____00034], 1, BOOL____00616) :: defines_var(BOOL____00616);\n\
constraint int_lin_eq_reif([-1, 1], [X____00026, X____00036], 1, BOOL____00624) :: defines_var(BOOL____00624);\n\
constraint int_lin_eq_reif([-1, 1], [X____00027, X____00028], 1, BOOL____00590) :: defines_var(BOOL____00590);\n\
constraint int_lin_eq_reif([-1, 1], [X____00027, X____00037], 1, BOOL____00628) :: defines_var(BOOL____00628);\n\
constraint int_lin_eq_reif([-1, 1], [X____00028, X____00029], 1, BOOL____00594) :: defines_var(BOOL____00594);\n\
constraint int_lin_eq_reif([-1, 1], [X____00028, X____00038], 1, BOOL____00632) :: defines_var(BOOL____00632);\n\
constraint int_lin_eq_reif([-1, 1], [X____00029, X____00030], 1, BOOL____00598) :: defines_var(BOOL____00598);\n\
constraint int_lin_eq_reif([-1, 1], [X____00029, X____00039], 1, BOOL____00636) :: defines_var(BOOL____00636);\n\
constraint int_lin_eq_reif([-1, 1], [X____00030, X____00031], 1, BOOL____00602) :: defines_var(BOOL____00602);\n\
constraint int_lin_eq_reif([-1, 1], [X____00030, X____00040], 1, BOOL____00640) :: defines_var(BOOL____00640);\n\
constraint int_lin_eq_reif([-1, 1], [X____00031, X____00032], 1, BOOL____00606) :: defines_var(BOOL____00606);\n\
constraint int_lin_eq_reif([-1, 1], [X____00031, X____00041], 1, BOOL____00644) :: defines_var(BOOL____00644);\n\
constraint int_lin_eq_reif([-1, 1], [X____00032, X____00033], 1, BOOL____00610) :: defines_var(BOOL____00610);\n\
constraint int_lin_eq_reif([-1, 1], [X____00032, X____00042], 1, BOOL____00648) :: defines_var(BOOL____00648);\n\
constraint int_lin_eq_reif([-1, 1], [X____00033, X____00034], 1, BOOL____00614) :: defines_var(BOOL____00614);\n\
constraint int_lin_eq_reif([-1, 1], [X____00033, X____00043], 1, BOOL____00652) :: defines_var(BOOL____00652);\n\
constraint int_lin_eq_reif([-1, 1], [X____00034, X____00035], 1, BOOL____00618) :: defines_var(BOOL____00618);\n\
constraint int_lin_eq_reif([-1, 1], [X____00034, X____00044], 1, BOOL____00656) :: defines_var(BOOL____00656);\n\
constraint int_lin_eq_reif([-1, 1], [X____00035, X____00036], 1, BOOL____00622) :: defines_var(BOOL____00622);\n\
constraint int_lin_eq_reif([-1, 1], [X____00035, X____00045], 1, BOOL____00660) :: defines_var(BOOL____00660);\n\
constraint int_lin_eq_reif([-1, 1], [X____00036, X____00046], 1, BOOL____00664) :: defines_var(BOOL____00664);\n\
constraint int_lin_eq_reif([-1, 1], [X____00037, X____00038], 1, BOOL____00630) :: defines_var(BOOL____00630);\n\
constraint int_lin_eq_reif([-1, 1], [X____00037, X____00047], 1, BOOL____00668) :: defines_var(BOOL____00668);\n\
constraint int_lin_eq_reif([-1, 1], [X____00038, X____00039], 1, BOOL____00634) :: defines_var(BOOL____00634);\n\
constraint int_lin_eq_reif([-1, 1], [X____00038, X____00048], 1, BOOL____00672) :: defines_var(BOOL____00672);\n\
constraint int_lin_eq_reif([-1, 1], [X____00039, X____00040], 1, BOOL____00638) :: defines_var(BOOL____00638);\n\
constraint int_lin_eq_reif([-1, 1], [X____00039, X____00049], 1, BOOL____00676) :: defines_var(BOOL____00676);\n\
constraint int_lin_eq_reif([-1, 1], [X____00040, X____00041], 1, BOOL____00642) :: defines_var(BOOL____00642);\n\
constraint int_lin_eq_reif([-1, 1], [X____00040, X____00050], 1, BOOL____00680) :: defines_var(BOOL____00680);\n\
constraint int_lin_eq_reif([-1, 1], [X____00041, X____00042], 1, BOOL____00646) :: defines_var(BOOL____00646);\n\
constraint int_lin_eq_reif([-1, 1], [X____00041, X____00051], 1, BOOL____00684) :: defines_var(BOOL____00684);\n\
constraint int_lin_eq_reif([-1, 1], [X____00042, X____00043], 1, BOOL____00650) :: defines_var(BOOL____00650);\n\
constraint int_lin_eq_reif([-1, 1], [X____00042, X____00052], 1, BOOL____00688) :: defines_var(BOOL____00688);\n\
constraint int_lin_eq_reif([-1, 1], [X____00043, X____00044], 1, BOOL____00654) :: defines_var(BOOL____00654);\n\
constraint int_lin_eq_reif([-1, 1], [X____00043, X____00053], 1, BOOL____00692) :: defines_var(BOOL____00692);\n\
constraint int_lin_eq_reif([-1, 1], [X____00044, X____00045], 1, BOOL____00658) :: defines_var(BOOL____00658);\n\
constraint int_lin_eq_reif([-1, 1], [X____00044, X____00054], 1, BOOL____00696) :: defines_var(BOOL____00696);\n\
constraint int_lin_eq_reif([-1, 1], [X____00045, X____00046], 1, BOOL____00662) :: defines_var(BOOL____00662);\n\
constraint int_lin_eq_reif([-1, 1], [X____00045, X____00055], 1, BOOL____00700) :: defines_var(BOOL____00700);\n"+
"constraint int_lin_eq_reif([-1, 1], [X____00046, X____00056], 1, BOOL____00704) :: defines_var(BOOL____00704);\n\
constraint int_lin_eq_reif([-1, 1], [X____00047, X____00048], 1, BOOL____00670) :: defines_var(BOOL____00670);\n\
constraint int_lin_eq_reif([-1, 1], [X____00047, X____00057], 1, BOOL____00708) :: defines_var(BOOL____00708);\n\
constraint int_lin_eq_reif([-1, 1], [X____00048, X____00049], 1, BOOL____00674) :: defines_var(BOOL____00674);\n\
constraint int_lin_eq_reif([-1, 1], [X____00048, X____00058], 1, BOOL____00712) :: defines_var(BOOL____00712);\n\
constraint int_lin_eq_reif([-1, 1], [X____00049, X____00050], 1, BOOL____00678) :: defines_var(BOOL____00678);\n\
constraint int_lin_eq_reif([-1, 1], [X____00049, X____00059], 1, BOOL____00716) :: defines_var(BOOL____00716);\n\
constraint int_lin_eq_reif([-1, 1], [X____00050, X____00051], 1, BOOL____00682) :: defines_var(BOOL____00682);\n\
constraint int_lin_eq_reif([-1, 1], [X____00050, X____00060], 1, BOOL____00720) :: defines_var(BOOL____00720);\n\
constraint int_lin_eq_reif([-1, 1], [X____00051, X____00052], 1, BOOL____00686) :: defines_var(BOOL____00686);\n\
constraint int_lin_eq_reif([-1, 1], [X____00051, X____00061], 1, BOOL____00724) :: defines_var(BOOL____00724);\n\
constraint int_lin_eq_reif([-1, 1], [X____00052, X____00053], 1, BOOL____00690) :: defines_var(BOOL____00690);\n\
constraint int_lin_eq_reif([-1, 1], [X____00052, X____00062], 1, BOOL____00728) :: defines_var(BOOL____00728);\n\
constraint int_lin_eq_reif([-1, 1], [X____00053, X____00054], 1, BOOL____00694) :: defines_var(BOOL____00694);\n\
constraint int_lin_eq_reif([-1, 1], [X____00053, X____00063], 1, BOOL____00732) :: defines_var(BOOL____00732);\n\
constraint int_lin_eq_reif([-1, 1], [X____00054, X____00055], 1, BOOL____00698) :: defines_var(BOOL____00698);\n\
constraint int_lin_eq_reif([-1, 1], [X____00054, X____00064], 1, BOOL____00736) :: defines_var(BOOL____00736);\n\
constraint int_lin_eq_reif([-1, 1], [X____00055, X____00056], 1, BOOL____00702) :: defines_var(BOOL____00702);\n\
constraint int_lin_eq_reif([-1, 1], [X____00055, X____00065], 1, BOOL____00740) :: defines_var(BOOL____00740);\n\
constraint int_lin_eq_reif([-1, 1], [X____00056, X____00066], 1, BOOL____00744) :: defines_var(BOOL____00744);\n\
constraint int_lin_eq_reif([-1, 1], [X____00057, X____00058], 1, BOOL____00710) :: defines_var(BOOL____00710);\n\
constraint int_lin_eq_reif([-1, 1], [X____00057, X____00067], 1, BOOL____00748) :: defines_var(BOOL____00748);\n\
constraint int_lin_eq_reif([-1, 1], [X____00058, X____00059], 1, BOOL____00714) :: defines_var(BOOL____00714);\n\
constraint int_lin_eq_reif([-1, 1], [X____00058, X____00068], 1, BOOL____00752) :: defines_var(BOOL____00752);\n\
constraint int_lin_eq_reif([-1, 1], [X____00059, X____00060], 1, BOOL____00718) :: defines_var(BOOL____00718);\n\
constraint int_lin_eq_reif([-1, 1], [X____00059, X____00069], 1, BOOL____00756) :: defines_var(BOOL____00756);\n\
constraint int_lin_eq_reif([-1, 1], [X____00060, X____00061], 1, BOOL____00722) :: defines_var(BOOL____00722);\n\
constraint int_lin_eq_reif([-1, 1], [X____00061, X____00062], 1, BOOL____00726) :: defines_var(BOOL____00726);\n\
constraint int_lin_eq_reif([-1, 1], [X____00061, X____00070], 1, BOOL____00760) :: defines_var(BOOL____00760);\n\
constraint int_lin_eq_reif([-1, 1], [X____00062, X____00063], 1, BOOL____00730) :: defines_var(BOOL____00730);\n\
constraint int_lin_eq_reif([-1, 1], [X____00062, X____00071], 1, BOOL____00764) :: defines_var(BOOL____00764);\n\
constraint int_lin_eq_reif([-1, 1], [X____00063, X____00064], 1, BOOL____00734) :: defines_var(BOOL____00734);\n\
constraint int_lin_eq_reif([-1, 1], [X____00063, X____00072], 1, BOOL____00768) :: defines_var(BOOL____00768);\n\
constraint int_lin_eq_reif([-1, 1], [X____00064, X____00065], 1, BOOL____00738) :: defines_var(BOOL____00738);\n\
constraint int_lin_eq_reif([-1, 1], [X____00064, X____00073], 1, BOOL____00772) :: defines_var(BOOL____00772);\n\
constraint int_lin_eq_reif([-1, 1], [X____00065, X____00066], 1, BOOL____00742) :: defines_var(BOOL____00742);\n\
constraint int_lin_eq_reif([-1, 1], [X____00065, X____00074], 1, BOOL____00776) :: defines_var(BOOL____00776);\n\
constraint int_lin_eq_reif([-1, 1], [X____00066, X____00075], 1, BOOL____00780) :: defines_var(BOOL____00780);\n\
constraint int_lin_eq_reif([-1, 1], [X____00067, X____00068], 1, BOOL____00750) :: defines_var(BOOL____00750);\n\
constraint int_lin_eq_reif([-1, 1], [X____00067, X____00076], 1, BOOL____00784) :: defines_var(BOOL____00784);\n\
constraint int_lin_eq_reif([-1, 1], [X____00068, X____00069], 1, BOOL____00754) :: defines_var(BOOL____00754);\n\
constraint int_lin_eq_reif([-1, 1], [X____00068, X____00077], 1, BOOL____00788) :: defines_var(BOOL____00788);\n\
constraint int_lin_eq_reif([-1, 1], [X____00069, X____00078], 1, BOOL____00792) :: defines_var(BOOL____00792);\n\
constraint int_lin_eq_reif([-1, 1], [X____00070, X____00071], 1, BOOL____00762) :: defines_var(BOOL____00762);\n\
constraint int_lin_eq_reif([-1, 1], [X____00070, X____00080], 1, BOOL____00800) :: defines_var(BOOL____00800);\n\
constraint int_lin_eq_reif([-1, 1], [X____00071, X____00072], 1, BOOL____00766) :: defines_var(BOOL____00766);\n\
constraint int_lin_eq_reif([-1, 1], [X____00071, X____00081], 1, BOOL____00804) :: defines_var(BOOL____00804);\n\
constraint int_lin_eq_reif([-1, 1], [X____00072, X____00073], 1, BOOL____00770) :: defines_var(BOOL____00770);\n\
constraint int_lin_eq_reif([-1, 1], [X____00072, X____00082], 1, BOOL____00808) :: defines_var(BOOL____00808);\n\
constraint int_lin_eq_reif([-1, 1], [X____00073, X____00074], 1, BOOL____00774) :: defines_var(BOOL____00774);\n\
constraint int_lin_eq_reif([-1, 1], [X____00073, X____00083], 1, BOOL____00812) :: defines_var(BOOL____00812);\n\
constraint int_lin_eq_reif([-1, 1], [X____00074, X____00075], 1, BOOL____00778) :: defines_var(BOOL____00778);\n\
constraint int_lin_eq_reif([-1, 1], [X____00074, X____00084], 1, BOOL____00816) :: defines_var(BOOL____00816);\n\
constraint int_lin_eq_reif([-1, 1], [X____00075, X____00085], 1, BOOL____00820) :: defines_var(BOOL____00820);\n\
constraint int_lin_eq_reif([-1, 1], [X____00076, X____00077], 1, BOOL____00786) :: defines_var(BOOL____00786);\n\
constraint int_lin_eq_reif([-1, 1], [X____00076, X____00086], 1, BOOL____00824) :: defines_var(BOOL____00824);\n\
constraint int_lin_eq_reif([-1, 1], [X____00077, X____00078], 1, BOOL____00790) :: defines_var(BOOL____00790);\n\
constraint int_lin_eq_reif([-1, 1], [X____00077, X____00087], 1, BOOL____00828) :: defines_var(BOOL____00828);\n\
constraint int_lin_eq_reif([-1, 1], [X____00078, X____00079], 1, BOOL____00794) :: defines_var(BOOL____00794);\n\
constraint int_lin_eq_reif([-1, 1], [X____00078, X____00088], 1, BOOL____00832) :: defines_var(BOOL____00832);\n\
constraint int_lin_eq_reif([-1, 1], [X____00079, X____00080], 1, BOOL____00798) :: defines_var(BOOL____00798);\n\
constraint int_lin_eq_reif([-1, 1], [X____00079, X____00089], 1, BOOL____00836) :: defines_var(BOOL____00836);\n\
constraint int_lin_eq_reif([-1, 1], [X____00080, X____00081], 1, BOOL____00802) :: defines_var(BOOL____00802);\n\
constraint int_lin_eq_reif([-1, 1], [X____00080, X____00090], 1, BOOL____00840) :: defines_var(BOOL____00840);\n\
constraint int_lin_eq_reif([-1, 1], [X____00081, X____00082], 1, BOOL____00806) :: defines_var(BOOL____00806);\n\
constraint int_lin_eq_reif([-1, 1], [X____00081, X____00091], 1, BOOL____00844) :: defines_var(BOOL____00844);\n\
constraint int_lin_eq_reif([-1, 1], [X____00082, X____00083], 1, BOOL____00810) :: defines_var(BOOL____00810);\n\
constraint int_lin_eq_reif([-1, 1], [X____00082, X____00092], 1, BOOL____00848) :: defines_var(BOOL____00848);\n\
constraint int_lin_eq_reif([-1, 1], [X____00083, X____00084], 1, BOOL____00814) :: defines_var(BOOL____00814);\n\
constraint int_lin_eq_reif([-1, 1], [X____00083, X____00093], 1, BOOL____00852) :: defines_var(BOOL____00852);\n\
constraint int_lin_eq_reif([-1, 1], [X____00084, X____00085], 1, BOOL____00818) :: defines_var(BOOL____00818);\n\
constraint int_lin_eq_reif([-1, 1], [X____00084, X____00094], 1, BOOL____00856) :: defines_var(BOOL____00856);\n\
constraint int_lin_eq_reif([-1, 1], [X____00085, X____00095], 1, BOOL____00860) :: defines_var(BOOL____00860);\n\
constraint int_lin_eq_reif([-1, 1], [X____00086, X____00087], 1, BOOL____00826) :: defines_var(BOOL____00826);\n\
constraint int_lin_eq_reif([-1, 1], [X____00087, X____00088], 1, BOOL____00830) :: defines_var(BOOL____00830);\n\
constraint int_lin_eq_reif([-1, 1], [X____00088, X____00089], 1, BOOL____00834) :: defines_var(BOOL____00834);\n\
constraint int_lin_eq_reif([-1, 1], [X____00089, X____00090], 1, BOOL____00838) :: defines_var(BOOL____00838);\n\
constraint int_lin_eq_reif([-1, 1], [X____00090, X____00091], 1, BOOL____00842) :: defines_var(BOOL____00842);\n\
constraint int_lin_eq_reif([-1, 1], [X____00091, X____00092], 1, BOOL____00846) :: defines_var(BOOL____00846);\n\
constraint int_lin_eq_reif([-1, 1], [X____00092, X____00093], 1, BOOL____00850) :: defines_var(BOOL____00850);\n\
constraint int_lin_eq_reif([-1, 1], [X____00093, X____00094], 1, BOOL____00854) :: defines_var(BOOL____00854);\n\
constraint int_lin_eq_reif([-1, 1], [X____00094, X____00095], 1, BOOL____00858) :: defines_var(BOOL____00858);\n\
constraint int_lt_reif(X____00002, X____00001, BOOL____00340) :: defines_var(BOOL____00340);\n\
constraint int_lt_reif(X____00003, X____00002, BOOL____00341) :: defines_var(BOOL____00341);\n\
constraint int_lt_reif(X____00004, X____00003, BOOL____00342) :: defines_var(BOOL____00342);\n\
constraint int_lt_reif(X____00005, X____00004, BOOL____00343) :: defines_var(BOOL____00343);\n\
constraint int_lt_reif(X____00006, X____00005, BOOL____00344) :: defines_var(BOOL____00344);\n\
constraint int_lt_reif(X____00007, X____00006, BOOL____00345) :: defines_var(BOOL____00345);\n\
constraint int_lt_reif(X____00008, X____00007, BOOL____00346) :: defines_var(BOOL____00346);\n\
constraint int_lt_reif(X____00009, X____00008, BOOL____00347) :: defines_var(BOOL____00347);\n\
constraint int_lt_reif(X____00010, X____00009, BOOL____00348) :: defines_var(BOOL____00348);\n\
constraint int_lt_reif(X____00011, X____00001, BOOL____00106) :: defines_var(BOOL____00106);\n\
constraint int_lt_reif(X____00012, X____00002, BOOL____00108) :: defines_var(BOOL____00108);\n\
constraint int_lt_reif(X____00012, X____00011, BOOL____00349) :: defines_var(BOOL____00349);\n\
constraint int_lt_reif(X____00013, X____00003, BOOL____00112) :: defines_var(BOOL____00112);\n\
constraint int_lt_reif(X____00013, X____00012, BOOL____00351) :: defines_var(BOOL____00351);\n\
constraint int_lt_reif(X____00014, X____00004, BOOL____00115) :: defines_var(BOOL____00115);\n\
constraint int_lt_reif(X____00014, X____00013, BOOL____00353) :: defines_var(BOOL____00353);\n\
constraint int_lt_reif(X____00015, X____00005, BOOL____00118) :: defines_var(BOOL____00118);\n\
constraint int_lt_reif(X____00015, X____00014, BOOL____00355) :: defines_var(BOOL____00355);\n\
constraint int_lt_reif(X____00016, X____00006, BOOL____00121) :: defines_var(BOOL____00121);\n\
constraint int_lt_reif(X____00016, X____00015, BOOL____00357) :: defines_var(BOOL____00357);\n\
constraint int_lt_reif(X____00017, X____00007, BOOL____00124) :: defines_var(BOOL____00124);\n\
constraint int_lt_reif(X____00017, X____00016, BOOL____00359) :: defines_var(BOOL____00359);\n\
constraint int_lt_reif(X____00018, X____00009, BOOL____00128) :: defines_var(BOOL____00128);\n\
constraint int_lt_reif(X____00019, X____00012, BOOL____00132) :: defines_var(BOOL____00132);\n\
constraint int_lt_reif(X____00020, X____00013, BOOL____00134) :: defines_var(BOOL____00134);\n\
constraint int_lt_reif(X____00020, X____00019, BOOL____00364) :: defines_var(BOOL____00364);\n\
constraint int_lt_reif(X____00021, X____00014, BOOL____00137) :: defines_var(BOOL____00137);\n\
constraint int_lt_reif(X____00021, X____00020, BOOL____00366) :: defines_var(BOOL____00366);\n\
constraint int_lt_reif(X____00022, X____00015, BOOL____00140) :: defines_var(BOOL____00140);\n\
constraint int_lt_reif(X____00022, X____00021, BOOL____00368) :: defines_var(BOOL____00368);\n\
constraint int_lt_reif(X____00023, X____00016, BOOL____00143) :: defines_var(BOOL____00143);\n\
constraint int_lt_reif(X____00023, X____00022, BOOL____00370) :: defines_var(BOOL____00370);\n\
constraint int_lt_reif(X____00024, X____00017, BOOL____00146) :: defines_var(BOOL____00146);\n\
constraint int_lt_reif(X____00024, X____00023, BOOL____00372) :: defines_var(BOOL____00372);\n\
constraint int_lt_reif(X____00025, X____00024, BOOL____00374) :: defines_var(BOOL____00374);\n\
constraint int_lt_reif(X____00028, X____00019, BOOL____00153) :: defines_var(BOOL____00153);\n\
constraint int_lt_reif(X____00028, X____00027, BOOL____00377) :: defines_var(BOOL____00377);\n\
constraint int_lt_reif(X____00029, X____00020, BOOL____00157) :: defines_var(BOOL____00157);\n\
constraint int_lt_reif(X____00029, X____00028, BOOL____00379) :: defines_var(BOOL____00379);\n\
constraint int_lt_reif(X____00030, X____00021, BOOL____00160) :: defines_var(BOOL____00160);\n\
constraint int_lt_reif(X____00030, X____00029, BOOL____00381) :: defines_var(BOOL____00381);\n\
constraint int_lt_reif(X____00031, X____00022, BOOL____00163) :: defines_var(BOOL____00163);\n\
constraint int_lt_reif(X____00031, X____00030, BOOL____00383) :: defines_var(BOOL____00383);\n\
constraint int_lt_reif(X____00032, X____00023, BOOL____00166) :: defines_var(BOOL____00166);\n\
constraint int_lt_reif(X____00032, X____00031, BOOL____00385) :: defines_var(BOOL____00385);\n\
constraint int_lt_reif(X____00033, X____00024, BOOL____00169) :: defines_var(BOOL____00169);\n\
constraint int_lt_reif(X____00033, X____00032, BOOL____00387) :: defines_var(BOOL____00387);\n\
constraint int_lt_reif(X____00034, X____00025, BOOL____00172) :: defines_var(BOOL____00172);\n\
constraint int_lt_reif(X____00034, X____00033, BOOL____00389) :: defines_var(BOOL____00389);\n\
constraint int_lt_reif(X____00035, X____00034, BOOL____00391) :: defines_var(BOOL____00391);\n\
constraint int_lt_reif(X____00036, X____00026, BOOL____00176) :: defines_var(BOOL____00176);\n\
constraint int_lt_reif(X____00036, X____00035, BOOL____00392) :: defines_var(BOOL____00392);\n\
constraint int_lt_reif(X____00037, X____00027, BOOL____00177) :: defines_var(BOOL____00177);\n\
constraint int_lt_reif(X____00038, X____00028, BOOL____00179) :: defines_var(BOOL____00179);\n\
constraint int_lt_reif(X____00038, X____00037, BOOL____00394) :: defines_var(BOOL____00394);\n\
constraint int_lt_reif(X____00039, X____00029, BOOL____00183) :: defines_var(BOOL____00183);\n\
constraint int_lt_reif(X____00039, X____00038, BOOL____00396) :: defines_var(BOOL____00396);\n\
constraint int_lt_reif(X____00040, X____00030, BOOL____00186) :: defines_var(BOOL____00186);\n\
constraint int_lt_reif(X____00040, X____00039, BOOL____00398) :: defines_var(BOOL____00398);\n\
constraint int_lt_reif(X____00041, X____00031, BOOL____00189) :: defines_var(BOOL____00189);\n\
constraint int_lt_reif(X____00041, X____00040, BOOL____00400) :: defines_var(BOOL____00400);\n\
constraint int_lt_reif(X____00042, X____00032, BOOL____00192) :: defines_var(BOOL____00192);\n\
constraint int_lt_reif(X____00042, X____00041, BOOL____00402) :: defines_var(BOOL____00402);\n\
constraint int_lt_reif(X____00043, X____00033, BOOL____00195) :: defines_var(BOOL____00195);\n\
constraint int_lt_reif(X____00043, X____00042, BOOL____00404) :: defines_var(BOOL____00404);\n\
constraint int_lt_reif(X____00044, X____00034, BOOL____00198) :: defines_var(BOOL____00198);\n\
constraint int_lt_reif(X____00044, X____00043, BOOL____00406) :: defines_var(BOOL____00406);\n\
constraint int_lt_reif(X____00045, X____00035, BOOL____00201) :: defines_var(BOOL____00201);\n\
constraint int_lt_reif(X____00045, X____00044, BOOL____00408) :: defines_var(BOOL____00408);\n\
constraint int_lt_reif(X____00046, X____00036, BOOL____00204) :: defines_var(BOOL____00204);\n\
constraint int_lt_reif(X____00046, X____00045, BOOL____00410) :: defines_var(BOOL____00410);\n\
constraint int_lt_reif(X____00047, X____00037, BOOL____00205) :: defines_var(BOOL____00205);\n\
constraint int_lt_reif(X____00048, X____00038, BOOL____00207) :: defines_var(BOOL____00207);\n"+
"constraint int_lt_reif(X____00048, X____00047, BOOL____00412) :: defines_var(BOOL____00412);\n\
constraint int_lt_reif(X____00049, X____00039, BOOL____00211) :: defines_var(BOOL____00211);\n\
constraint int_lt_reif(X____00049, X____00048, BOOL____00414) :: defines_var(BOOL____00414);\n\
constraint int_lt_reif(X____00050, X____00040, BOOL____00214) :: defines_var(BOOL____00214);\n\
constraint int_lt_reif(X____00050, X____00049, BOOL____00416) :: defines_var(BOOL____00416);\n\
constraint int_lt_reif(X____00051, X____00041, BOOL____00217) :: defines_var(BOOL____00217);\n\
constraint int_lt_reif(X____00051, X____00050, BOOL____00418) :: defines_var(BOOL____00418);\n\
constraint int_lt_reif(X____00052, X____00042, BOOL____00220) :: defines_var(BOOL____00220);\n\
constraint int_lt_reif(X____00052, X____00051, BOOL____00420) :: defines_var(BOOL____00420);\n\
constraint int_lt_reif(X____00053, X____00043, BOOL____00223) :: defines_var(BOOL____00223);\n\
constraint int_lt_reif(X____00053, X____00052, BOOL____00422) :: defines_var(BOOL____00422);\n\
constraint int_lt_reif(X____00054, X____00044, BOOL____00226) :: defines_var(BOOL____00226);\n\
constraint int_lt_reif(X____00054, X____00053, BOOL____00424) :: defines_var(BOOL____00424);\n\
constraint int_lt_reif(X____00055, X____00045, BOOL____00229) :: defines_var(BOOL____00229);\n\
constraint int_lt_reif(X____00055, X____00054, BOOL____00426) :: defines_var(BOOL____00426);\n\
constraint int_lt_reif(X____00056, X____00046, BOOL____00232) :: defines_var(BOOL____00232);\n\
constraint int_lt_reif(X____00056, X____00055, BOOL____00428) :: defines_var(BOOL____00428);\n\
constraint int_lt_reif(X____00057, X____00047, BOOL____00233) :: defines_var(BOOL____00233);\n\
constraint int_lt_reif(X____00058, X____00048, BOOL____00235) :: defines_var(BOOL____00235);\n\
constraint int_lt_reif(X____00058, X____00057, BOOL____00430) :: defines_var(BOOL____00430);\n\
constraint int_lt_reif(X____00059, X____00049, BOOL____00239) :: defines_var(BOOL____00239);\n\
constraint int_lt_reif(X____00059, X____00058, BOOL____00432) :: defines_var(BOOL____00432);\n\
constraint int_lt_reif(X____00060, X____00050, BOOL____00242) :: defines_var(BOOL____00242);\n\
constraint int_lt_reif(X____00060, X____00059, BOOL____00434) :: defines_var(BOOL____00434);\n\
constraint int_lt_reif(X____00061, X____00051, BOOL____00245) :: defines_var(BOOL____00245);\n\
constraint int_lt_reif(X____00061, X____00060, BOOL____00435) :: defines_var(BOOL____00435);\n\
constraint int_lt_reif(X____00062, X____00052, BOOL____00248) :: defines_var(BOOL____00248);\n\
constraint int_lt_reif(X____00062, X____00061, BOOL____00437) :: defines_var(BOOL____00437);\n\
constraint int_lt_reif(X____00063, X____00053, BOOL____00251) :: defines_var(BOOL____00251);\n\
constraint int_lt_reif(X____00063, X____00062, BOOL____00439) :: defines_var(BOOL____00439);\n\
constraint int_lt_reif(X____00064, X____00054, BOOL____00254) :: defines_var(BOOL____00254);\n\
constraint int_lt_reif(X____00064, X____00063, BOOL____00441) :: defines_var(BOOL____00441);\n\
constraint int_lt_reif(X____00065, X____00055, BOOL____00257) :: defines_var(BOOL____00257);\n\
constraint int_lt_reif(X____00065, X____00064, BOOL____00443) :: defines_var(BOOL____00443);\n\
constraint int_lt_reif(X____00066, X____00056, BOOL____00260) :: defines_var(BOOL____00260);\n\
constraint int_lt_reif(X____00066, X____00065, BOOL____00445) :: defines_var(BOOL____00445);\n\
constraint int_lt_reif(X____00067, X____00057, BOOL____00261) :: defines_var(BOOL____00261);\n\
constraint int_lt_reif(X____00068, X____00058, BOOL____00263) :: defines_var(BOOL____00263);\n\
constraint int_lt_reif(X____00068, X____00067, BOOL____00447) :: defines_var(BOOL____00447);\n\
constraint int_lt_reif(X____00069, X____00059, BOOL____00267) :: defines_var(BOOL____00267);\n\
constraint int_lt_reif(X____00069, X____00068, BOOL____00449) :: defines_var(BOOL____00449);\n\
constraint int_lt_reif(X____00070, X____00061, BOOL____00271) :: defines_var(BOOL____00271);\n\
constraint int_lt_reif(X____00071, X____00062, BOOL____00273) :: defines_var(BOOL____00273);\n\
constraint int_lt_reif(X____00071, X____00070, BOOL____00453) :: defines_var(BOOL____00453);\n\
constraint int_lt_reif(X____00072, X____00063, BOOL____00276) :: defines_var(BOOL____00276);\n\
constraint int_lt_reif(X____00072, X____00071, BOOL____00455) :: defines_var(BOOL____00455);\n\
constraint int_lt_reif(X____00073, X____00064, BOOL____00279) :: defines_var(BOOL____00279);\n\
constraint int_lt_reif(X____00073, X____00072, BOOL____00457) :: defines_var(BOOL____00457);\n\
constraint int_lt_reif(X____00074, X____00065, BOOL____00282) :: defines_var(BOOL____00282);\n\
constraint int_lt_reif(X____00074, X____00073, BOOL____00459) :: defines_var(BOOL____00459);\n\
constraint int_lt_reif(X____00075, X____00066, BOOL____00285) :: defines_var(BOOL____00285);\n\
constraint int_lt_reif(X____00075, X____00074, BOOL____00461) :: defines_var(BOOL____00461);\n\
constraint int_lt_reif(X____00076, X____00067, BOOL____00286) :: defines_var(BOOL____00286);\n\
constraint int_lt_reif(X____00077, X____00068, BOOL____00288) :: defines_var(BOOL____00288);\n\
constraint int_lt_reif(X____00077, X____00076, BOOL____00463) :: defines_var(BOOL____00463);\n\
constraint int_lt_reif(X____00078, X____00069, BOOL____00292) :: defines_var(BOOL____00292);\n\
constraint int_lt_reif(X____00078, X____00077, BOOL____00465) :: defines_var(BOOL____00465);\n\
constraint int_lt_reif(X____00079, X____00078, BOOL____00467) :: defines_var(BOOL____00467);\n\
constraint int_lt_reif(X____00080, X____00070, BOOL____00296) :: defines_var(BOOL____00296);\n\
constraint int_lt_reif(X____00080, X____00079, BOOL____00468) :: defines_var(BOOL____00468);\n\
constraint int_lt_reif(X____00081, X____00071, BOOL____00299) :: defines_var(BOOL____00299);\n\
constraint int_lt_reif(X____00081, X____00080, BOOL____00470) :: defines_var(BOOL____00470);\n\
constraint int_lt_reif(X____00082, X____00072, BOOL____00302) :: defines_var(BOOL____00302);\n\
constraint int_lt_reif(X____00082, X____00081, BOOL____00472) :: defines_var(BOOL____00472);\n\
constraint int_lt_reif(X____00083, X____00073, BOOL____00305) :: defines_var(BOOL____00305);\n\
constraint int_lt_reif(X____00083, X____00082, BOOL____00474) :: defines_var(BOOL____00474);\n\
constraint int_lt_reif(X____00084, X____00074, BOOL____00308) :: defines_var(BOOL____00308);\n\
constraint int_lt_reif(X____00084, X____00083, BOOL____00476) :: defines_var(BOOL____00476);\n\
constraint int_lt_reif(X____00085, X____00075, BOOL____00311) :: defines_var(BOOL____00311);\n\
constraint int_lt_reif(X____00085, X____00084, BOOL____00478) :: defines_var(BOOL____00478);\n\
constraint int_lt_reif(X____00086, X____00076, BOOL____00312) :: defines_var(BOOL____00312);\n\
constraint int_lt_reif(X____00087, X____00077, BOOL____00314) :: defines_var(BOOL____00314);\n\
constraint int_lt_reif(X____00087, X____00086, BOOL____00480) :: defines_var(BOOL____00480);\n\
constraint int_lt_reif(X____00088, X____00078, BOOL____00318) :: defines_var(BOOL____00318);\n\
constraint int_lt_reif(X____00088, X____00087, BOOL____00481) :: defines_var(BOOL____00481);\n\
constraint int_lt_reif(X____00089, X____00079, BOOL____00321) :: defines_var(BOOL____00321);\n\
constraint int_lt_reif(X____00089, X____00088, BOOL____00482) :: defines_var(BOOL____00482);\n\
constraint int_lt_reif(X____00090, X____00080, BOOL____00324) :: defines_var(BOOL____00324);\n\
constraint int_lt_reif(X____00090, X____00089, BOOL____00483) :: defines_var(BOOL____00483);\n\
constraint int_lt_reif(X____00091, X____00081, BOOL____00327) :: defines_var(BOOL____00327);\n\
constraint int_lt_reif(X____00091, X____00090, BOOL____00484) :: defines_var(BOOL____00484);\n\
constraint int_lt_reif(X____00092, X____00082, BOOL____00330) :: defines_var(BOOL____00330);\n\
constraint int_lt_reif(X____00092, X____00091, BOOL____00485) :: defines_var(BOOL____00485);\n\
constraint int_lt_reif(X____00093, X____00083, BOOL____00333) :: defines_var(BOOL____00333);\n\
constraint int_lt_reif(X____00093, X____00092, BOOL____00486) :: defines_var(BOOL____00486);\n\
constraint int_lt_reif(X____00094, X____00084, BOOL____00336) :: defines_var(BOOL____00336);\n\
constraint int_lt_reif(X____00094, X____00093, BOOL____00487) :: defines_var(BOOL____00487);\n\
constraint int_lt_reif(X____00095, X____00085, BOOL____00339) :: defines_var(BOOL____00339);\n\
constraint int_lt_reif(X____00095, X____00094, BOOL____00488) :: defines_var(BOOL____00488);\n\
solve  :: int_search(a_flat, first_fail, indomain_max, complete) satisfy;\n\
", "a = array2d(0..11, 0..11, [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 2, 3, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
