/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *     Mikael Lagerkivst, 2007
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

namespace {

  /** \name %Kakuro specifications
   *
   * Each specification starts with two integers for width and height,
   * followed by entries for vertical constraints, an integer -1
   * (signalling the end of the vertical constraints), entries
   * for the horizontal constraints, and, finally, an integer -1.
   *
   * Each entry consists of four integers:
   *  - the x-coordinate of the hint
   *  - the y-coordinate of the hint
   *  - the number of fields in the respective direction
   *  - the sum of the fields
   *
   * The example are taken from the website of Nikoli (from the free
   * section). Thanks to Nikoli for their great puzzles and their
   * brilliant website: www.nikoli.com.
   *
   * \relates Kakuro
   */
  //@{

  // Easy, Author: Casty
  const int k0[] = {
    // Dimension w x h
    12,10,
    // Vertical constraints
     2, 0, 5,16,     3, 0, 2, 4,     5, 0, 3, 6,     6, 0, 2, 4,
     7, 0, 5,15,    10, 0, 3, 6,    11, 0, 3, 7,     1, 1, 3, 7,
     9, 1, 5,16,     4, 2, 2, 5,     8, 2, 2, 3,     3, 3, 5,16,
     6, 3, 3, 8,     5, 4, 5,15,    10, 4, 5,15,     4, 5, 2, 3,
     8, 5, 2, 4,    11, 5, 3, 7,     1, 6, 3, 6,     2, 6, 3, 7,
     7, 6, 3, 7,     6, 7, 2, 3,     9, 7, 2, 4,    -1,
    // Horizontal constraints
     1, 1, 2, 7,     4, 1, 3, 9,     9, 1, 2, 4,     0, 2, 3, 7,
     4, 2, 3, 7,     8, 2, 3, 6,     0, 3, 2, 3,     3, 3, 2, 4,
     6, 3, 5,16,     0, 4, 4,10,     5, 4, 4,10,     1, 5, 2,10,
     4, 5, 3, 6,     8, 5, 2, 5,     2, 6, 4,10,     7, 6, 4,12,
     0, 7, 5,16,     6, 7, 2, 4,     9, 7, 2, 4,     0, 8, 3, 7,
     4, 8, 3, 8,     8, 8, 3, 6,     0, 9, 2, 3,     4, 9, 3, 7,
     8, 9, 2, 3,    -1
  };

  // Easy, Author: Ogawa Minori
  const int k1[] = {
    // Dimension w x h
    12,10,
    // Vertical constraints
     1, 0, 2, 4,     2, 0, 5,15,     5, 0, 5,18,     6, 0, 2,12,
     7, 0, 3, 8,    10, 0, 3,24,    11, 0, 3,23,     3, 1, 2, 7,
     9, 1, 3,24,     4, 2, 5,16,     8, 2, 5,35,     1, 3, 2,12,
     6, 3, 3,17,     7, 4, 5,34,    10, 4, 5,34,    11, 4, 2,16,
     3, 5, 3, 6,     1, 6, 3, 7,     2, 6, 3, 6,     5, 6, 3,23,
     9, 6, 2,10,     6, 7, 2,14,    11, 7, 2,10,    -1,
    // Horizontal constraints
     0, 1, 2, 3,     4, 1, 3,15,     9, 1, 2,16,     0, 2, 3, 6,
     4, 2, 3, 7,     8, 2, 3,24,     1, 3, 4,11,     6, 3, 5,34,
     0, 4, 2,14,     3, 4, 3,23,     7, 4, 2,14,     0, 5, 2, 7,
     3, 5, 5,15,     9, 5, 2,17,     2, 6, 2, 6,     5, 6, 3,23,
     9, 6, 2,13,     0, 7, 5,16,     6, 7, 4,30,     0, 8, 3, 6,
     4, 8, 3,23,     8, 8, 3, 7,     0, 9, 2, 4,     4, 9, 3,24,
     9, 9, 2,17,    -1
  };

  // Easy, Author: SAKAMOTO, Nobuyuki
  const int k2[] = {
    // Dimension w x h
    12,10,
    // Vertical constraints
     2, 0, 5,15,     3, 0, 2, 3,     7, 0, 3, 7,     8, 0, 4,23,
     9, 0, 2,12,    10, 0, 3,20,    11, 0, 3, 9,     4, 1, 3, 7,
     5, 1, 4,10,     1, 2, 3, 6,     6, 2, 5,15,     9, 3, 2,16,
     3, 4, 2, 3,     7, 4, 4,13,    10, 4, 5,35,    11, 4, 3,23,
     4, 5, 4,11,     8, 5, 3,23,     1, 6, 3,23,     2, 6, 3,14,
     5, 6, 3,11,     3, 7, 2,13,     9, 7, 2,17,    -1,
    // Horizontal constraints
     1, 1, 2, 4,     6, 1, 5,15,     1, 2, 4,11,     6, 2, 5,34,
     0, 3, 2, 3,     3, 3, 5,15,     9, 3, 2,10,     0, 4, 2, 4,
     3, 4, 3, 6,     7, 4, 2,17,     0, 5, 3, 7,     4, 5, 3, 8,
     8, 5, 3,18,     2, 6, 2, 3,     5, 6, 3,11,     9, 6, 2,16,
     0, 7, 2,16,     3, 7, 5,16,     9, 7, 2,17,     0, 8, 5,16,
     6, 8, 4,30,     0, 9, 5,35,     8, 9, 2,17,    -1
  };

  // Easy, Author: country mushroom
  const int k3[] = {
    // Dimension w x h
    12,10,
    // Vertical constraints
     3, 0, 3, 7,     4, 0, 6,21,     7, 0, 4,29,     8, 0, 2,17,
    10, 0, 4,29,    11, 0, 3,23,     2, 1, 3, 6,     6, 1, 2,16,
     9, 1, 4,14,     1, 2, 2, 4,     5, 2, 2, 3,     8, 3, 6,22,
     3, 4, 4,10,     2, 5, 4,11,     5, 5, 4,10,     7, 5, 2,10,
    10, 5, 3,24,    11, 5, 2,16,     1, 6, 3, 7,     6, 6, 2, 9,
     9, 6, 3,23,     4, 7, 2, 4,    -1,
    // Horizontal constraints
     2, 1, 2, 4,     6, 1, 2,17,     9, 1, 2,16,     1, 2, 3, 6,
     5, 2, 6,39,     0, 3, 7,28,     8, 3, 3,24,     0, 4, 2, 3,
     3, 4, 2, 3,     6, 4, 4,20,     2, 5, 2, 9,     7, 5, 2, 4,
     1, 6, 4,10,     6, 6, 2, 3,     9, 6, 2,16,     0, 7, 3, 6,
     4, 7, 7,42,     0, 8, 6,21,     7, 8, 3,21,     0, 9, 2, 4,
     3, 9, 2, 3,     7, 9, 2,16,    -1
  };

  // Medium, Author: Komieda
  const int k4[] = {
    // Dimension w x h
    20,12,
    // Vertical constraints
     3, 0, 3,21,     4, 0, 2, 4,     5, 0, 4,11,     8, 0, 2, 8,
     9, 0, 3, 7,    11, 0, 2, 3,    12, 0, 3, 6,    15, 0, 6,39,
    16, 0, 2, 3,    17, 0, 3,23,     2, 1, 5,15,     6, 1, 4,10,
    10, 1, 4,11,    14, 1, 4,11,    18, 1, 3, 6,     1, 2, 3,24,
     7, 2, 4,14,    13, 2, 2,10,    19, 2, 2,16,     4, 3, 5,18,
     8, 3, 4,10,    11, 3, 4,12,    16, 3, 5,33,     3, 4, 3,23,
     9, 4, 4,29,    12, 4, 4,30,    17, 4, 3,18,     5, 5, 6,38,
    13, 5, 4,29,    18, 5, 5,15,     6, 6, 4,25,    10, 6, 4,12,
    14, 6, 4,28,    19, 6, 3,21,     1, 7, 2, 4,     2, 7, 3, 7,
     7, 7, 2, 7,    15, 7, 4,11,     3, 8, 3,19,     8, 8, 3,24,
    11, 8, 3, 7,    17, 8, 3, 6,     4, 9, 2,16,     9, 9, 2,16,
    12, 9, 2,17,    16, 9, 2, 5,    -1,
    // Horizontal constraints
     2, 1, 3, 7,     7, 1, 2, 4,    10, 1, 2, 4,    14, 1, 3,19,
     1, 2, 5,18,     7, 2, 5,15,    13, 2, 5,16,     0, 3, 3,21,
     4, 3, 3, 6,     8, 3, 2, 3,    11, 3, 4,11,    16, 3, 3,20,
     0, 4, 2,14,     3, 4, 5,15,     9, 4, 2, 3,    12, 4, 4,29,
    17, 4, 2, 8,     0, 5, 4,27,     5, 5, 7,42,    13, 5, 4,12,
     1, 6, 4,12,     6, 6, 3, 8,    10, 6, 3,20,    14, 6, 4,29,
     2, 7, 4,28,     7, 7, 7,28,    15, 7, 4,28,     0, 8, 2, 3,
     3, 8, 4,11,     8, 8, 2,10,    11, 8, 5,35,    17, 8, 2,10,
     0, 9, 3, 6,     4, 9, 4,30,     9, 9, 2, 3,    12, 9, 3,19,
    16, 9, 3, 7,     1,10, 5,34,     7,10, 5,34,    13,10, 5,17,
     2,11, 3,23,     7,11, 2,17,    10,11, 2,10,    14,11, 3, 6,
    -1
  };

  // Medium, Author: crimson
  const int k5[] = {
    // Dimension w x h
    20,12,
    // Vertical constraints
     1, 0, 2, 3,     2, 0, 5,33,     4, 0, 2, 8,     5, 0, 4,14,
     7, 0, 5,15,     8, 0, 3,19,     9, 0, 2,12,    11, 0, 4,11,
    12, 0, 2, 4,    13, 0, 5,16,    15, 0, 4,11,    16, 0, 2,17,
    18, 0, 5,34,    19, 0, 2,17,     3, 1, 2, 3,    10, 1, 9,45,
    17, 1, 2,16,     6, 2, 3,20,    14, 2, 3,12,     1, 3, 2,13,
     4, 3, 5,33,     9, 3, 3,20,    16, 3, 5,21,    19, 3, 2, 8,
     3, 4, 3,11,     8, 4, 3,11,    12, 4, 3, 7,    17, 4, 3, 8,
    11, 5, 3,23,     1, 6, 2,11,     2, 6, 5,15,     6, 6, 3,23,
     7, 6, 5,27,    13, 6, 5,30,    14, 6, 3, 7,    18, 6, 5,15,
    19, 6, 2, 3,     5, 7, 4,26,     9, 7, 4,27,    15, 7, 4,27,
     3, 8, 2, 7,    12, 8, 3,24,    17, 8, 2,17,     1, 9, 2, 5,
     4, 9, 2, 9,     8, 9, 2, 3,    11, 9, 2,16,    16, 9, 2,16,
    19, 9, 2,10,    -1,
    // Horizontal constraints
     0, 1, 2, 4,     3, 1, 2, 7,     6, 1, 3, 7,    10, 1, 3, 7,
    14, 1, 2,11,    17, 1, 2,16,     0, 2, 5,16,     6, 2, 7,42,
    14, 2, 5,35,     1, 3, 2,10,     4, 3, 4,15,     9, 3, 2, 6,
    12, 3, 3, 7,    16, 3, 2,13,     0, 4, 2,17,     3, 4, 4,29,
     8, 4, 3,19,    12, 4, 4,11,    17, 4, 2, 9,     0, 5, 4,29,
     5, 5, 5,33,    11, 5, 3, 6,    15, 5, 4,29,     2, 6, 2, 4,
     7, 6, 5,16,    15, 6, 2, 4,     0, 7, 4,12,     5, 7, 3,10,
     9, 7, 5,18,    15, 7, 4,12,     0, 8, 2,13,     3, 8, 4,29,
     8, 8, 3,24,    12, 8, 4,23,    17, 8, 2, 5,     1, 9, 2, 6,
     4, 9, 3,24,     8, 9, 2, 4,    11, 9, 4,28,    16, 9, 2,11,
     0,10, 5,15,     6,10, 7,41,    14,10, 5,34,     0,11, 2, 3,
     3,11, 2,17,     6,11, 3,14,    10,11, 3,23,    14,11, 2,11,
    17,11, 2, 4,    -1
  };

  // Hard, Author: Yuichi Saito
  const int k6[] = {
    // Dimension w x h
    20,12,
    // Vertical constraints
     1, 0, 2, 6,     2, 0, 2,16,     4, 0, 3,10,     5, 0, 2,12,
     9, 0, 7,28,    10, 0, 2,12,    11, 0, 3,24,    15, 0, 3,10,
    16, 0, 2,17,    17, 0, 6,22,     3, 1, 3,18,     6, 1, 4,10,
     8, 1, 2, 8,    12, 1, 2,10,    13, 1, 3,18,    18, 1, 3,12,
     7, 2, 4,11,    14, 2, 3, 7,    19, 2, 2, 7,     1, 3, 2, 8,
     2, 3, 3, 7,     5, 3, 4,25,    10, 3, 2, 4,    16, 3, 4,15,
     4, 4, 4,11,    11, 4, 7,42,    12, 4, 2,17,    15, 4, 4,26,
     3, 5, 6,22,     8, 5, 2,16,    13, 5, 4,30,    18, 5, 3,24,
     6, 6, 3, 7,    10, 6, 2, 4,    14, 6, 4,29,    19, 6, 2,14,
     1, 7, 2,16,     2, 7, 3,11,     7, 7, 3,24,    17, 7, 3,21,
     5, 8, 3,23,     8, 8, 2,12,     9, 8, 3,20,    12, 8, 2,17,
    16, 8, 3, 6,     4, 9, 2, 9,    10, 9, 2,16,    15, 9, 2, 9,
    18, 9, 2, 3,    19, 9, 2,12,    -1,
    // Horizontal constraints
     0, 1, 2,10,     3, 1, 2, 5,     8, 1, 3,23,    14, 1, 3,11,
     0, 2, 6,38,     7, 2, 6,39,    14, 2, 4,30,     2, 3, 2,10,
     5, 3, 4,11,    10, 3, 5,18,    16, 3, 3, 7,     0, 4, 3, 7,
     4, 4, 3, 7,     8, 4, 2, 6,    12, 4, 2, 8,    15, 4, 4,11,
     0, 5, 2, 8,     3, 5, 4,14,     8, 5, 4,24,    13, 5, 4,10,
     1, 6, 4,13,     6, 6, 3,14,    10, 6, 3,19,    14, 6, 4,29,
     2, 7, 4,15,     7, 7, 4,14,    12, 7, 4,29,    17, 7, 2,16,
     0, 8, 4,29,     5, 8, 2,13,     9, 8, 2, 8,    12, 8, 3,23,
    16, 8, 3,22,     0, 9, 3,10,     4, 9, 5,32,    10, 9, 4,29,
    15, 9, 2,10,     1,10, 4,14,     6,10, 6,39,    13,10, 6,22,
     2,11, 3,21,     8,11, 3,23,    14,11, 2, 6,    17,11, 2,11,
    -1
  };

  // Hard, Author: mimic
  const int k7[] = {
    // Dimension w x h
    22,14,
    // Vertical constraints
     1, 0, 3,23,     2, 0, 4,29,     3, 0, 2,16,     7, 0, 2, 7,
     8, 0, 2,10,     9, 0, 5,30,    13, 0, 7,41,    14, 0, 2,16,
    15, 0, 4,29,    17, 0, 2, 3,    18, 0, 6,21,    20, 0, 3, 7,
    21, 0, 2, 4,     4, 1, 5,35,     6, 1, 2, 4,    10, 1, 2,17,
    12, 1, 3,24,    19, 1, 9,45,     5, 2, 3,23,    11, 2, 9,45,
    16, 2, 4,21,     3, 3, 9,45,     7, 3, 5,15,     8, 3, 4,10,
    14, 3, 2,10,    17, 3, 2, 3,     6, 4, 2, 4,    10, 4, 4,30,
    20, 4, 4,11,     2, 5, 4,13,    12, 5, 4,30,    15, 5, 5,35,
    21, 5, 2, 4,     1, 6, 2, 4,     9, 6, 7,41,    14, 6, 4,29,
     4, 7, 6,38,     6, 7, 4,11,    16, 7, 2,16,    18, 7, 5,15,
     5, 8, 2, 9,     8, 8, 2,17,    13, 8, 5,16,    17, 8, 3, 7,
     7, 9, 4,20,    10, 9, 3,23,    20, 9, 4,12,     2,10, 3,23,
    12,10, 2, 6,    16,10, 2, 4,    21,10, 3, 9,     1,11, 2,16,
     5,11, 2,16,     8,11, 2,16,    14,11, 2, 6,    15,11, 2, 4,
    19,11, 2, 4,    -1,
    // Horizontal constraints
     0, 1, 3,23,     6, 1, 3, 8,    12, 1, 3,23,    16, 1, 2, 4,
    19, 1, 2, 4,     0, 2, 4,30,     5, 2, 5,31,    11, 2, 4,29,
    16, 2, 5,15,     0, 3, 2,16,     3, 3, 3,19,     8, 3, 5,32,
    14, 3, 2,17,    17, 3, 3, 8,     1, 4, 4,29,     6, 4, 3, 9,
    10, 4, 9,45,     2, 5, 9,45,    12, 5, 2,17,    15, 5, 5,16,
     1, 6, 3,24,     5, 6, 3, 6,     9, 6, 4,30,    14, 6, 2,16,
    17, 6, 4,11,     0, 7, 3, 7,     6, 7, 9,45,    18, 7, 3, 7,
     0, 8, 4,11,     5, 8, 2, 4,     8, 8, 4,29,    13, 8, 3,23,
    17, 8, 3, 7,     1, 9, 5,16,     7, 9, 2,17,    10, 9, 9,45,
     2,10, 9,45,    12,10, 3,23,    16,10, 4,14,     1,11, 3,24,
     5,11, 2, 6,     8,11, 5,16,    15,11, 3, 7,    19,11, 2, 8,
     0,12, 5,35,     6,12, 4,30,    11,12, 5,15,    17,12, 4,11,
     0,13, 2,17,     3,13, 2,16,     6,13, 3,24,    12,13, 3, 6,
    18,13, 3, 7,    -1
  };

  // Hard, Author: OX
  const int k8[] = {
    // Dimension w x h
    22,14,
    // Vertical constraints
     1, 0, 2, 4,     2, 0, 5,15,     5, 0, 5,16,     6, 0, 2,10,
     7, 0, 3,18,     8, 0, 4,29,    10, 0, 5,16,    11, 0, 2, 6,
    13, 0, 2, 8,    14, 0, 5,16,    15, 0, 6,38,    18, 0, 5,15,
    19, 0, 2, 8,    20, 0, 3, 7,    21, 0, 3, 8,     3, 1, 9,45,
    16, 1, 2, 4,     4, 2, 2, 3,     9, 2, 8,39,    17, 2, 2, 3,
     1, 3, 2, 5,     6, 3, 6,22,    11, 3, 3,22,    13, 3, 8,38,
    19, 3, 9,45,     7, 4, 2, 4,    12, 4, 3,24,    16, 4, 6,38,
    20, 4, 3,24,     4, 5, 2,16,     8, 5, 2,14,    17, 5, 2,16,
    21, 5, 2,11,     1, 6, 2, 4,     2, 6, 3, 8,     5, 6, 2, 7,
    10, 6, 3,18,    14, 6, 2,16,    18, 6, 2,16,     7, 7, 6,22,
    11, 7, 3, 7,    15, 7, 2,15,     4, 8, 5,35,     8, 8, 5,34,
    12, 8, 5,34,    17, 8, 5,34,    20, 8, 5,34,    21, 8, 2, 3,
     5, 9, 2,16,    14, 9, 4,12,    18, 9, 2, 7,     1,10, 3,23,
     2,10, 3,21,     6,10, 2, 9,    15,10, 3,20,     3,11, 2,17,
     9,11, 2, 4,    11,11, 2, 4,    16,11, 2,10,    21,11, 2,16,
    -1,
    // Horizontal constraints
     0, 1, 2, 6,     4, 1, 4,30,     9, 1, 2, 6,    12, 1, 3,10,
    17, 1, 4,12,     0, 2, 3, 8,     4, 2, 4,11,     9, 2, 2, 4,
    12, 2, 4,20,    17, 2, 4,11,     1, 3, 4,12,     6, 3, 4,28,
    13, 3, 5,15,    19, 3, 2, 5,     0, 4, 6,22,     7, 4, 4,28,
    12, 4, 3, 8,    16, 4, 3, 6,     0, 5, 3, 7,     4, 5, 3, 7,
     8, 5, 8,40,    17, 5, 3,22,     2, 6, 2, 8,     5, 6, 4,12,
    10, 6, 3,23,    14, 6, 3,22,    18, 6, 3,22,     0, 7, 6,38,
     7, 7, 3,24,    11, 7, 3,23,    15, 7, 6,39,     0, 8, 3, 6,
     4, 8, 3, 6,     8, 8, 3, 6,    12, 8, 4,29,    17, 8, 2,14,
     1, 9, 3,18,     5, 9, 8,36,    14, 9, 3,22,    18, 9, 3,10,
     2,10, 3,22,     6,10, 3,24,    10,10, 4,10,    15,10, 6,21,
     0,11, 2,16,     3,11, 5,34,    11,11, 4,29,    16,11, 4,30,
     0,12, 4,29,     5,12, 4,12,    10,12, 2,10,    13,12, 4,10,
    18,12, 3,23,     0,13, 4,28,     6,13, 3, 7,    10,13, 2,11,
    13,13, 4,28,    19,13, 2,13,    -1
  };

  // Hard, Author: TAKEI Daisuke
  const int k9[] = {
    // Dimension w x h
    22,14,
    // Vertical constraints
     1, 0, 4,10,     2, 0, 4,24,     3, 0, 3, 7,     7, 0, 3, 8,
     8, 0, 2,17,     9, 0, 3,13,    13, 0, 3,22,    14, 0, 2,12,
    15, 0, 3,24,    19, 0, 3,19,    20, 0, 4,28,    21, 0, 4,14,
     4, 1, 5,16,     6, 1, 5,17,    10, 1, 5,32,    12, 1, 5,34,
    16, 1, 5,35,    18, 1, 5,31,     5, 2, 3, 9,    11, 2, 3, 7,
    17, 2, 3, 7,     3, 4, 5,27,     7, 4, 5,16,     9, 4, 5,20,
    13, 4, 5,30,    15, 4, 5,30,    19, 4, 5,26,     1, 5, 3,12,
     2, 5, 3,20,     8, 5, 3,22,    14, 5, 3, 9,    20, 5, 3,10,
    21, 5, 3,20,     4, 7, 5,31,     6, 7, 5,16,    10, 7, 5,17,
    12, 7, 5,32,    16, 7, 5,19,    18, 7, 5,34,     5, 8, 3, 8,
    11, 8, 3,24,    17, 8, 3,24,     1, 9, 4,10,     2, 9, 4,15,
    20, 9, 4,30,    21, 9, 4,12,     3,10, 3,20,     7,10, 3, 6,
     9,10, 3, 9,    13,10, 3, 6,    15,10, 3, 7,    19,10, 3,24,
     8,11, 2, 8,    14,11, 2, 7,    -1,
    // Horizontal constraints
     0, 1, 3, 7,     6, 1, 3,12,    12, 1, 3,23,    18, 1, 3,23,
     0, 2, 4,11,     5, 2, 5,19,    11, 2, 5,33,    17, 2, 4,28,
     0, 3, 7,28,     8, 3, 5,34,    14, 3, 7,29,     0, 4, 2,12,
     3, 4, 3, 7,     9, 4, 3,16,    15, 4, 3,10,    19, 4, 2,10,
     2, 5, 5,18,     8, 5, 5,20,    14, 5, 5,29,     0, 6, 4,24,
     5, 6, 5,35,    11, 6, 5,23,    17, 6, 4,26,     0, 7, 3,23,
     6, 7, 3, 9,    12, 7, 3,10,    18, 7, 3,23,     0, 8, 4,15,
     5, 8, 5,19,    11, 8, 5,33,    17, 8, 4,10,     2, 9, 5,19,
     8, 9, 5,35,    14, 9, 5,31,     0,10, 2, 4,     3,10, 3,10,
     9,10, 3,18,    15,10, 3,24,    19,10, 2,12,     0,11, 7,41,
     8,11, 5,23,    14,11, 7,36,     0,12, 4,14,     5,12, 5,17,
    11,12, 5,15,    17,12, 4,26,     0,13, 3, 7,     6,13, 3, 7,
    12,13, 3, 6,    18,13, 3,23,    -1
  };
  //@}

  /// Array of all examples
  const int* examples[] = {
    &k0[0], &k1[0], &k2[0], &k3[0], &k4[0],
    &k5[0], &k6[0], &k7[0], &k8[0], &k9[0]
  };
  /// Number of examples
  const unsigned int n_examples = sizeof(examples)/sizeof(const int*);


  /** \brief Class for solutions of a distinct-linear constraint problem.
   * \relates Kakuro
   */
  class DistinctLinear : public Space {
  protected:
    /// The variables
    IntVarArray x;
  public:
    /// The actual problem
    DistinctLinear(int n, int s) : x(*this,n,1,9) {
      distinct(*this, x);
      linear(*this, x, IRT_EQ, s);
      branch(*this, x, INT_VAR_NONE, INT_VAL_SPLIT_MIN);
    }
    /// Constructor for cloning \a s
    DistinctLinear(bool share, DistinctLinear& s) : Space(share,s) {
      x.update(*this, share, s.x);
    }
    /// Perform copying during cloning
    virtual Space*
    copy(bool share) {
      return new DistinctLinear(share,*this);
    }
    /// Return a solution
    IntArgs solution(void) const {
      IntArgs s(x.size());
      for (int i=0; i<x.size(); i++)
        s[i]=x[i].val();
      return s;
    }
  };

  /** \brief Generate tuple set for \a n distinct variables with sum \a c
   * \relates Kakuro
   */
  TupleSet generate(int n, int c) {
    // Setup search engine that enumerates all solutions
    DistinctLinear* e = new DistinctLinear(n,c);
    DFS<DistinctLinear> d(e);
    delete e;
    TupleSet ts;
    while (DistinctLinear* s = d.next()) {
      ts.add(s->solution()); delete s;
    }
    ts.finalize();
    return ts;
  }

  /** \brief Class to remember already computed specifications
   * \relates Kakuro
   */
  class Cache {
  private:
    /// Cache entry
    class Entry {
    public:
      int n;       ///< Number of variables
      int c;       ///< Sum of variables
      TupleSet ts; ///< The previously computed tuple set
      Entry* next; ///< The next cache entry
    };
    Entry* cache; ///< Where all entries start
  public:
    /// Initialize cache as empty
    Cache(void) : cache(NULL) {}
    /// Return possibly cached Data for \a n distinct variables with sum \a c
    TupleSet get(int n, int c) {
      for (Entry* e = cache; e != NULL; e = e->next)
        if ((e->n == n) && (e->c == c))
          return e->ts;
      {
        Entry* e = new Entry;
        e->n = n;
        e->c = c;
        e->ts = generate(n,c);
        e->next = cache;
        cache = e;
      }
      return cache->ts;
    }
    /// Delete cache entries
    ~Cache(void) {
      Entry* e = cache;
      while (e != NULL) {
        Entry* d = e;
        e = e->next;
        delete d;
      }
    }
  };

}


/**
 * \brief %Example: %Kakuro
 *
 * Another puzzle in the style of Sudoku.
 *
 * Note that "Modeling and Programming with Gecode" uses this example
 * as a case study.
 *
 * \ingroup Example
 */
class Kakuro : public Script {
protected:
  const int w;   ///< Width of board
  const int h;   ///< Height of board
  IntVarArray f; ///< Variables for fields of board
public:
  /// Model variants
  enum {
    MODEL_DECOMPOSE,///< Decompose constraints
    MODEL_COMBINE,  ///< Combine distinct and linear constraint
  };
  /// Init the variable \a x if necessary
  IntVar init(IntVar& x) {
    if (x.min() == 0)
      x = IntVar(*this,1,9);
    return x;
  }
  /// Post a distinct-linear constraint on variables \a x with sum \a c
  void distinctlinear(Cache& dc, const IntVarArgs& x, int c,
                      const SizeOptions& opt) {
    int n=x.size();
    if (opt.model() == MODEL_DECOMPOSE) {
      if (n < 8)
        linear(*this, x, IRT_EQ, c, opt.icl());
      else if (n == 8)
        rel(*this, x, IRT_NQ, 9*(9+1)/2 - c);
      distinct(*this, x, opt.icl());
    } else {
      switch (n) {
      case 0:
        return;
      case 1:
        rel(*this, x[0], IRT_EQ, c);
        return;
      case 8:
        // Prune the single missing digit
        rel(*this, x, IRT_NQ, 9*(9+1)/2 - c);
        break;
      case 9:
        break;
      default:
        if (c == n*(n+1)/2) {
          // sum has unique decomposition: 1 + ... + n
          rel(*this, x, IRT_LQ, n);
        } else if (c == n*(n+1)/2 + 1) {
          // sum has unique decomposition: 1 + ... + n-1 + n+1
          rel(*this, x, IRT_LQ, n+1);
          rel(*this, x, IRT_NQ, n);
        } else if (c == 9*(9+1)/2 - (9-n)*(9-n+1)/2) {
          // sum has unique decomposition: (9-n+1) + (9-n+2) + ... + 9
          rel(*this, x, IRT_GQ, 9-n+1);
        } else if (c == 9*(9+1)/2 - (9-n)*(9-n+1)/2 + 1) {
          // sum has unique decomposition: (9-n) + (9-n+2) + ... + 9
          rel(*this, x, IRT_GQ, 9-n);
          rel(*this, x, IRT_NQ, 9-n+1);
        } else {
          extensional(*this, x, dc.get(n,c));
          return;
        }
      }
      distinct(*this, x, opt.icl());
    }
  }
  /// The actual problem
  Kakuro(const SizeOptions& opt)
    : w(examples[opt.size()][0]),  h(examples[opt.size()][1]),
      f(*this,w*h) {
    IntVar black(*this,0,0);
    // Initialize all fields as black (unused). Only if a field
    // is actually used in a constraint, create a fresh variable
    // for it (done via init).
    for (int i=w*h; i--; )
      f[i] = black;

    // Cache of already computed tuple sets
    Cache cache;

    // Matrix for accessing board fields
    Matrix<IntVarArray> b(f,w,h);
    // Access to hints
    const int* k = &examples[opt.size()][2];

    // Process vertical hints
    while (*k >= 0) {
      int x=*k++; int y=*k++; int n=*k++; int s=*k++;
      IntVarArgs col(n);
      for (int i=n; i--; )
        col[i]=init(b(x,y+i+1));
      distinctlinear(cache,col,s,opt);
    }
    k++;

    // Process horizontal hints
    while (*k >= 0) {
      int x=*k++; int y=*k++; int n=*k++; int s=*k++;
      IntVarArgs row(n);
      for (int i=n; i--; )
        row[i]=init(b(x+i+1,y));
      distinctlinear(cache,row,s,opt);
    }
    branch(*this, f, INT_VAR_SIZE_AFC_MIN, INT_VAL_SPLIT_MIN);
  }
  /// Constructor for cloning \a s
  Kakuro(bool share, Kakuro& s) : Script(share,s), w(s.w), h(s.h) {
    f.update(*this, share, s.f);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Kakuro(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> b(f,w,h);
    for (int y=0; y<h; y++) {
      os << '\t';
      for (int x=0; x<w; x++)
        if (b(x,y).min() == 0)
          os << ". ";
        else
          os << b(x,y) << ' ';
      os << std::endl;
    }
  }
};


/** \brief Main-function
 *  \relates Kakuro
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Kakuro");
  opt.model(Kakuro::MODEL_COMBINE);
  opt.model(Kakuro::MODEL_DECOMPOSE,
                  "decompose","decompose distinct and linear constraints");
  opt.model(Kakuro::MODEL_COMBINE,
                  "combine","combine distinct and linear constraints");
  opt.icl(ICL_DOM);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Script::run<Kakuro,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
