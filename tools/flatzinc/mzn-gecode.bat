@echo off

rem
rem  Main authors:
rem     Guido Tack <tack@gecode.org>
rem
rem  Copyright:
rem     Guido Tack, 2012
rem
rem  Last modified:
rem     $Date: 2009-10-26 21:16:18 +0100 (Mon, 26 Oct 2009) $ by $Author: schulte $
rem     $Revision: 9990 $
rem
rem  This file is part of Gecode, the generic constraint
rem  development environment:
rem     http://www.gecode.org
rem
rem  Permission is hereby granted, free of charge, to any person obtaining
rem  a copy of this software and associated documentation files (the
rem  "Software"), to deal in the Software without restriction, including
rem  without limitation the rights to use, copy, modify, merge, publish,
rem  distribute, sublicense, and/or sell copies of the Software, and to
rem  permit persons to whom the Software is furnished to do so, subject to
rem  the following conditions:
rem
rem  The above copyright notice and this permission notice shall be
rem  included in all copies or substantial portions of the Software.
rem
rem  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
rem  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
rem  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
rem  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
rem  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
rem  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
rem  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
rem
rem

setlocal
set FLATZINC_CMD=fz
minizinc -I "%GECODEDIR%\mznlib" %*
