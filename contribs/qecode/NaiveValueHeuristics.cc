/*********************************************************[naivevalueheuristics.cc]
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


#include "NaiveValueHeuristics.hh"

int** SmallestValueFirst::subSet(Implicative* home,int var,int* nbRanges) {
    switch (home->type_of_v[var]) {
        case VTYPE_INT : {
            int zeValue = getIntVar(home,whichSpaceOwns(home,var),var)->min();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
        case VTYPE_BOOL : {
            int zeValue = getBoolVar(home,whichSpaceOwns(home,var),var)->min();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;    
    }
    return 0;
}


int** LargestValueFirst::subSet(Implicative* home,int var,int* nbRanges) {
    switch (home->type_of_v[var]) {
        case VTYPE_INT : {
            int zeValue = getIntVar(home,whichSpaceOwns(home,var),var)->max();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
        case VTYPE_BOOL : {
            int zeValue = getBoolVar(home,whichSpaceOwns(home,var),var)->max();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
    }
    return 0;
}

int** MedianValueFirst::subSet(Implicative* home,int var,int* nbRanges) {
    switch (home->type_of_v[var]) {
        case VTYPE_INT : {
            int zeValue = getIntVar(home,whichSpaceOwns(home,var),var)->med();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
        case VTYPE_BOOL : {
            int zeValue = getBoolVar(home,whichSpaceOwns(home,var),var)->med();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
    }
    return 0;
}

int** SmallestHalfFirst::subSet(Implicative* home,int var,int* nbRanges) {
    switch (home->type_of_v[var]) {
        case VTYPE_INT : {
            int zeMin = getIntVar(home,whichSpaceOwns(home,var),var)->min();
            int zeMax = getIntVar(home,whichSpaceOwns(home,var),var)->med();
            int varMax = getIntVar(home,whichSpaceOwns(home,var),var)->max();
            if (zeMax == varMax) zeMax--;
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeMin;
            ret[1][0]=zeMax;
            return ret;
        }
            break;
        case VTYPE_BOOL : {
            int zeValue = getBoolVar(home,whichSpaceOwns(home,var),var)->min();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
    }
    return 0;
}

int** LargestHalfFirst::subSet(Implicative* home,int var,int* nbRanges) {
    switch (home->type_of_v[var]) {
        case VTYPE_INT : {
            int zeMin = getIntVar(home,whichSpaceOwns(home,var),var)->med();
            int zeMax = getIntVar(home,whichSpaceOwns(home,var),var)->max();
            int varMin = getIntVar(home,whichSpaceOwns(home,var),var)->min();
            if (zeMin == varMin) zeMin++;
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeMin;
            ret[1][0]=zeMax;
            return ret;
        }
            break;
        case VTYPE_BOOL : {
            int zeValue = getBoolVar(home,whichSpaceOwns(home,var),var)->max();
            *nbRanges = 1;
            int** ret = new int*[2];
            ret[0]=new int[1];
            ret[1]=new int[1];
            ret[0][0]=zeValue;
            ret[1][0]=zeValue;
            return ret;
        }
            break;
    }
    return 0;
}