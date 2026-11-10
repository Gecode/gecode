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
 *
 */

namespace Gecode { namespace Word {

  /**
   * \brief Print the values of \a set as ranges in rank order
   *
   * A range of consecutive ranks prints as its endpoint encodings in
   * hexadecimal, such as {0x3,0x5..0x7}; a signed range may therefore print
   * as 0xe..0x1, and an E2M1 range from -0 to +0.5 as 0x8..0x1. Every word
   * with the same domain type and set prints the same text.
   */
  template<class Char, class Traits>
  void
  print_value_ranges(std::basic_ostream<Char,Traits>& os,
                     WordDomainType domain_type, unsigned int width,
                     const WordValueSet& set) {
    const std::ios_base::fmtflags flags=os.flags();
    const WordValueSet ordered = order_value_set(domain_type,width,set);
    const WordValue end = WordValue(1) << width;
    os << std::hex << '{';
    bool is_first = true;
    for (WordValue first=0; first<end; first++) {
      if (!value_set_contains(ordered,first))
        continue;
      WordValue last = first;
      while ((last+1 < end) && value_set_contains(ordered,last+1))
        last++;
      os << (is_first ? "" : ",") << "0x"
         << encode_rank(domain_type,width,first);
      if (last != first)
        os << "..0x" << encode_rank(domain_type,width,last);
      is_first = false;
      first = last;
    }
    os << '}';
    os.flags(flags);
  }

  /// Return the tag that names the order of a bounded word's interval
  inline const char*
  bounds_tag(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_UNSIGNED:
      return "u";
    case WDT_SIGNED:
      return "s";
    case WDT_FP32:
      return "fp32";
    case WDT_BF16:
      return "bf16";
    case WDT_BINARY16:
      return "binary16";
    case WDT_E4M3FN:
      return "e4m3fn";
    case WDT_E2M1:
      return "e2m1";
    case WDT_CUBE:
      GECODE_NEVER;
      break;
    }
    return "?";
  }

}}

namespace Gecode {
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const WordVar& x) {
    os << x.width() << "'b";
    for (unsigned int i=x.width(); i--;) {
      const WordValue bit = WordValue(1) << i;
      os << ((x.lo() & bit) ? '1' : ((x.hi() & bit) ? '?' : '0'));
    }
    if (x.bounded()) {
      const std::ios_base::fmtflags flags=os.flags();
      os << '[' << Word::bounds_tag(x.domain_type())
         << ":0x" << std::hex << x.minimum() << "..0x" << x.maximum()
         << ']';
      os.flags(flags);
    }
    const Word::WordValueSet holes = Word::value_set_holes(Word::WordView(x));
    if (!Word::is_empty_value_set(holes)) {
      os << '\\';
      Word::print_value_ranges(os,x.domain_type(),x.width(),holes);
    }
    return os;
  }
}

// STATISTICS: word-var
