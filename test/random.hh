/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_RANDOM_HH__
#define __GECODE_RANDOM_HH__

#define TEST_RAND_MAX (1UL<<31)

unsigned int test_seed();
void test_seed(unsigned int val);

unsigned long test_rand();

#endif /* __GECODE_RANDOM_HH__ */

// STATISTICS: test-int
