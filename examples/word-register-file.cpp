/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

#include <gecode/driver.hh>
#include <gecode/word.hh>

using namespace Gecode;

/**
 * \brief %Example: Select and accumulate a small Word register file
 *
 * The index is the information-flow control point and is branched before the
 * selected data. Larger register files can make Script copy and recomputation
 * settings worth comparing.
 *
 * \ingroup Example
 */
class WordRegisterFile : public Script {
private:
  /// Register file, selected value, addends, and total
  WordVarArray registers;
  IntVar index;
  WordVar selected;
  WordVar increment0;
  WordVar increment1;
  WordVar total;
public:
  /// Actual model
  WordRegisterFile(const Options& opt)
    : Script(opt), registers(*this,4,8,0,0xffU), index(*this,0,3),
      selected(*this,8), increment0(*this,8,0x05U,0x05U),
      increment1(*this,8,0x07U,0x07U), total(*this,8) {
    const WordValue value[] = {0x11U,0x22U,0x33U,0x44U};
    for (int i=0; i<registers.size(); i++)
      dom(*this,registers[i],value[i]);
    element(*this,registers,index,selected);
    WordVarArgs addend={selected,increment0,increment1};
    add(*this,addend,total);
    dom(*this,total,0x3fU);

    branch(*this,index,INT_VAL_MIN());
    WordVarArgs data={selected,total};
    branch(*this,data,WORD_VAR_NONE(),WORD_VAL_MSB());
    branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordRegisterFile(WordRegisterFile& s) : Script(s) {
    registers.update(*this,s.registers);
    index.update(*this,s.index);
    selected.update(*this,s.selected);
    increment0.update(*this,s.increment0);
    increment1.update(*this,s.increment1);
    total.update(*this,s.total);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordRegisterFile(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tindex = " << index.val()
       << ", selected = 0x" << std::hex << selected.val()
       << ", total = 0x" << total.val() << std::dec << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordRegisterFile
 */
int
main(int argc, char* argv[]) {
  Options opt("WordRegisterFile");
  opt.parse(argc,argv);
  Script::run<WordRegisterFile,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
