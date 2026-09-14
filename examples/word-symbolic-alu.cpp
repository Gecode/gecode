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

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <vector>

using namespace Gecode;

namespace {
  class ComparisonALU : public Space {
  public:
    WordVar input, output;
    ComparisonALU(unsigned int width, WordValue input_min,
                  WordValue input_max, WordValue output_min,
                  WordValue output_max, bool lsb)
      : input(*this,width,WDT_UNSIGNED,input_min,input_max),
        output(*this,width,WDT_UNSIGNED,output_min,output_max) {
      const WordValue mask=Word::width_mask(width);
      WordVar amount(*this,width,WDT_UNSIGNED,0U,3U);
      WordVar s1(*this,width,WDT_UNSIGNED), addend(*this,width,
        WDT_UNSIGNED,WordValue(0x1d)&mask,WordValue(0x1d)&mask);
      WordVar s2(*this,width,WDT_UNSIGNED), shifted(*this,width,WDT_UNSIGNED),
        xored(*this,width,WDT_UNSIGNED), s3(*this,width,WDT_UNSIGNED),
        incremented(*this,width,WDT_UNSIGNED);
      BoolVar carry(*this,0,1), negative(*this,0,1);
      rel(*this,input,WOT_AND,width,3U,amount);
      shift_left(*this,input,amount,s1);
      add(*this,s1,addend,s2,carry);
      channel(*this,s2,width-1,negative);
      arithmetic_shift_right(*this,s2,amount,shifted);
      rel(*this,s2,WOT_XOR,width,WordValue(0x15)&mask,xored);
      ite(*this,carry,xored,shifted,s3);
      add(*this,s3,width,1U,incremented);
      ite(*this,negative,incremented,s3,output);
      if (lsb) {
        branch(*this,input,WORD_VAL_LSB()); branch(*this,output,WORD_VAL_LSB());
      } else {
        branch(*this,input,WORD_VAL_SPLIT_MIN()); branch(*this,output,WORD_VAL_SPLIT_MIN());
      }
    }
    ComparisonALU(ComparisonALU& s) : Space(s) {
      input.update(*this,s.input); output.update(*this,s.output);
    }
    Space* copy(void) { return new ComparisonALU(*this); }
  };

  int comparison_main(int argc, char* argv[]) {
    if (argc < 11 || ((argc-11)%2 != 0)) return 2;
    const char* search="split-min"; unsigned int batch=1U;
    for (int i=11; i<argc; i+=2) {
      if (!std::strcmp(argv[i],"--search") &&
          (!std::strcmp(argv[i+1],"lsb") || !std::strcmp(argv[i+1],"split-min"))) search=argv[i+1];
      else if (!std::strcmp(argv[i],"--batch"))
        batch=static_cast<unsigned int>(std::strtoul(argv[i+1],nullptr,10));
      else return 2;
    }
    if (batch == 0U) return 2;
    unsigned int width=static_cast<unsigned int>(std::strtoul(argv[2],nullptr,10));
    WordValue imin=std::strtoull(argv[4],nullptr,10), imax=std::strtoull(argv[6],nullptr,10);
    WordValue omin=std::strtoull(argv[8],nullptr,10), omax=std::strtoull(argv[10],nullptr,10);
    std::vector<std::pair<WordValue,WordValue> > rows;
    std::uint64_t total_solutions=0;
    for (unsigned int trial=0; trial<batch; trial++) {
      ComparisonALU* root=new ComparisonALU(width,imin,imax,omin,omax,!std::strcmp(search,"lsb"));
      DFS<ComparisonALU> engine(root); delete root;
      while (ComparisonALU* solution=engine.next()) {
        total_solutions++;
        if (trial == 0U) rows.emplace_back(solution->input.val(),solution->output.val());
        delete solution;
      }
    }
    std::cout << "{\"schema_version\":1,\"semantic_status\":\""
              << (rows.empty() ? "unsat" : "sat") << "\",\"solutions\":" << rows.size()
              << ",\"batch\":" << batch << ",\"batch_solutions\":" << total_solutions
              << ",\"decision_variables\":[\"input\",\"output\"],\"projections\":[";
    for (std::size_t i=0; i<rows.size(); i++) {
      if (i) std::cout << ','; std::cout << '[' << rows[i].first << ',' << rows[i].second << ']';
    }
    std::cout << "]}\n"; return 0;
  }
}

/** \brief Options for the constructed symbolic ALU example */
class WordSymbolicALUOptions : public Options {
private:
  /// Number of ALU steps
  Driver::UnsignedIntOption _steps;
  /// Word width
  Driver::UnsignedIntOption _width;
  /// Word domain representation
  Driver::StringOption _domain;
public:
  enum Domain { DOMAIN_CUBE, DOMAIN_UNSIGNED, DOMAIN_SIGNED };
  /// Initialize options
  WordSymbolicALUOptions(const char* n)
    : Options(n), _steps("steps","number of constructed ALU steps",8),
      _width("width","word width",18),
      _domain("word-domain","word domain representation",DOMAIN_CUBE) {
    add(_steps); add(_width); add(_domain);
    _domain.add(DOMAIN_CUBE,"cube","independently known bits");
    _domain.add(DOMAIN_UNSIGNED,"unsigned","unsigned ranked interval and cube");
    _domain.add(DOMAIN_SIGNED,"signed","signed ranked interval and cube");
  }
  /// Return number of steps
  unsigned int steps(void) const { return _steps.value(); }
  /// Return word width
  unsigned int width(void) const { return _width.value(); }
  /// Return selected Word domain representation
  WordDomainType domain_type(void) const {
    switch (_domain.value()) {
    case DOMAIN_UNSIGNED: return WDT_UNSIGNED;
    case DOMAIN_SIGNED: return WDT_SIGNED;
    default: return WDT_CUBE;
    }
  }
};

/**
 * \brief %Example: Constructed symbolic register and ALU path
 *
 * This deterministic model is inspired by symbolic-execution workloads; it
 * is not a copied benchmark instance.  A symbolic input passes through
 * rotate/add, rotate/XOR, and rotate/AND-plus-constant instructions.  The low
 * two output bits are fixed to zero.  Use \c -steps \c 12 \c -width \c 22
 * for the larger profiling configuration.
 *
 * \ingroup Example
 */
class WordSymbolicALU : public Script {
private:
  /// Symbolic input register
  WordVar input;
  /// Final output register
  WordVar output;
public:
  /// Actual model
  WordSymbolicALU(const WordSymbolicALUOptions& opt)
    : Script(opt), input(*this,opt.width(),opt.domain_type()),
      output(*this,opt.width(),opt.domain_type()) {
    const WordValue mask = (opt.width() == 64) ? ~WordValue(0) :
                           (WordValue(1) << opt.width())-1;
    WordVarArray reg(*this,opt.steps()+1,opt.width(),opt.domain_type());
    dom(*this,reg[0],WordValue(0x2a55aU)&mask);
    for (unsigned int i=0; i<opt.steps(); i++) {
      WordVar rotated(*this,opt.width(),opt.domain_type());
      WordVar next(*this,opt.width(),opt.domain_type());
      rotate_left(*this,reg[i],i%(opt.width()-1)+1,rotated);
      if (i%3 == 0) {
        add(*this,rotated,input,next);
      } else if (i%3 == 1) {
        rel(*this,rotated,WOT_XOR,input,next);
      } else {
        WordVar selected(*this,opt.width(),opt.domain_type());
        rel(*this,rotated,WOT_AND,input,selected);
        add(*this,selected,opt.width(),WordValue(0x12345U+i)&mask,next);
      }
      rel(*this,reg[i+1],WRT_EQ,next);
    }
    rel(*this,output,WRT_EQ,reg[opt.steps()]);
    dom(*this,output,0,mask & ~WordValue(3));
    branch(*this,input,(opt.domain_type() == WDT_CUBE)
           ? WORD_VAL_MSB() : WORD_VAL_SPLIT_MIN());
  }
  /// Constructor for cloning \a s
  WordSymbolicALU(WordSymbolicALU& s) : Script(s) {
    input.update(*this,s.input);
    output.update(*this,s.output);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordSymbolicALU(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tinput = 0x" << std::hex << input.val()
       << ", output = 0x" << output.val() << std::dec << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordSymbolicALU
 */
int
main(int argc, char* argv[]) {
  if ((argc > 1) && (std::strcmp(argv[1],"--comparison-width") == 0))
    return comparison_main(argc,argv);
  WordSymbolicALUOptions opt("WordSymbolicALU");
  opt.parse(argc,argv);
  if ((opt.steps() == 0) || (opt.steps() > 64)) {
    std::cerr << "-steps must be between 1 and 64" << std::endl;
    return 1;
  }
  if ((opt.width() < 2) || (opt.width() > 64)) {
    std::cerr << "-width must be between 2 and 64" << std::endl;
    return 1;
  }
  Script::run<WordSymbolicALU,DFS,WordSymbolicALUOptions>(opt);
  return 0;
}

// STATISTICS: example-any
