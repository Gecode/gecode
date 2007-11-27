/*****************************************************[extensivecomparator.hh]
Copyright (c) 2007, Universite d'Orleans - Jeremie Vautard, Marco Benedetti,
Arnaud Lallouet.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/
#ifndef __QECODE_EXTENSIVECOMPARATOR__
#define __QECODE_EXTENSIVECOMPARATOR__
#include "qecode.hh"
/** \brief A simple integer comparator.
* This class compares two integer between them, according to the score they are given. Scores are stored in an array and can be modified every time.
 */
class QECODE_VTABLE_EXPORT ExtensiveComparator {
  int size;  
  int* scores; 

public : 
  /** Constructor for an extensive Comparator. 
   *  @param the_size This comparator will compare integers from 0 to this parameter.
   *  @param the_scores The array of scores.
   */
  QECODE_EXPORT ExtensiveComparator(int the_size, int* the_scores);

  /** \brief The method to compare two integers.
  *  This compares the scores of a and b. Returns true if the first argument has a smaller score than b.
   */
  QECODE_EXPORT bool operator() (int a, int b);
};

#endif
