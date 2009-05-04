/****   , [ qecode.hh ], 
Copyright (c) 2007 Universite d'Orleans - Jeremie Vautard 

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
 *************************************************************************/

#ifndef __QECODE_HH__
#define __QECODE_HH__

#include "gecode/kernel.hh"

#if !defined(GECODE_STATIC_LIBS) && \
(defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#define QECODE_VTABLE_EXPORT

#ifdef GECODE_BUILD_QECODE
#define QECODE_EXPORT __declspec( dllexport )
#else
#define QECODE_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define QECODE_VTABLE_EXPORT __attribute__ ((visibility("default")))
#define QECODE_EXPORT __attribute__ ((visibility("default")))

#else
#define QECODE_VTABLE_EXPORT
#define QECODE_EXPORT

#endif
#endif

#define QECODE_EXISTENTIAL false
#define QECODE_UNIVERSAL true

#endif
