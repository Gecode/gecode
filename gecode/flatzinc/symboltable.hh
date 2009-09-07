/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#ifndef __FLATZINC_SYMBOL_TABLE_HH__
#define __FLATZINC_SYMBOL_TABLE_HH__

#include <map>
#include <vector>

namespace Gecode { namespace FlatZinc {

  template <class Val>
  class SymbolTable {
  private:
    std::map<std::string,std::pair<Val,bool> > m;
  public:
    void put(const std::string& key, const Val& val, bool ext=false);
    bool get(const std::string& key, Val& val) const;
    std::vector<std::pair<std::string, Val> > toVec(bool all) const;
  };

  template <class Val>
  void
  SymbolTable<Val>::put(const std::string& key, const Val& val, bool ext) {
    m[key] = std::pair<Val,bool>(val,ext);
  }

  template <class Val>
  bool
  SymbolTable<Val>::get(const std::string& key, Val& val) const {
    typename std::map<std::string,std::pair<Val,bool> >::const_iterator i = 
      m.find(key);
    if (i == m.end())
      return false;
    val = i->second.first;
    return true;
  }

  template <class Val>
  std::vector<std::pair<std::string, Val> >
  SymbolTable<Val>::toVec(bool all) const {
    typename std::map<std::string,std::pair<Val,bool> >::const_iterator i = 
      m.begin();
    std::vector<std::pair<std::string, Val> > ret;
    for (; i!=m.end(); i++) {
      if (!all) {
        if (i->second.second) {
          ret.push_back(std::pair<std::string, Val>(i->first, i->second.first));
        }
      } else
        ret.push_back(std::pair<std::string, Val>(i->first, i->second.first));
    }
    return ret;
  }

}}
#endif

// STATISTICS: flatzinc-any
