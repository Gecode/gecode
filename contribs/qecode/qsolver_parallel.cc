/************************************************************ qsolver_parallel.cc
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

#include "qsolver_parallel.hh"

QCSP_Parallel_Solver::QCSP_Parallel_Solver(Qcop* sp,WorkComparator* c,unsigned int nbWorkers) : wm(sp,c) {
	workers = new QWorker*[nbWorkers];
	nw = nbWorkers;
	for (unsigned int i=0;i<nbWorkers;i++) 
	  workers[i] = new QWorker(&wm);
}

Strategy QCSP_Parallel_Solver::solve() {
  for (unsigned int i=0;i<nw;i++) {
		Thread::run(workers[i]);
		//		Thread::sleep(200);
	}
	while (!wm.isFinished()) {
		Thread::sleep(1000);
	}
	return wm.getResult();
}
