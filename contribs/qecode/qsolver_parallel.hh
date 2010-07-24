/************************************************************ qsolver_parallel.hh
 Copyright (c) 2010 Universite d'Orleans - Jeremie Vautard 
 
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

#include "QCOPPlus.hh"
#include "WorkManager.hh"
#include "Worker.hh"
#include <iostream>
#include <cstdlib>
#include "Strategy.hh"
#include "qecode.hh"


class QECODE_VTABLE_EXPORT QCSP_Parallel_Solver {
	
	private : 
	WorkManager wm;
	QWorker** workers;
	unsigned int nw;
	
	public :
	
	QECODE_EXPORT QCSP_Parallel_Solver(Qcop* sp,WorkComparator* c,unsigned int nbWorkers);
	QECODE_EXPORT Strategy solve();
};