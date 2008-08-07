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

#ifndef GECODE_SERIALIZATION_JAVASCRIPT_HH
#define GECODE_SERIALIZATION_JAVASCRIPT_HH

#include <QtScript>
#include <iostream>
#include "gecode/kernel.hh"
#include "gecode/serialization.hh"

namespace Gecode { namespace Serialization {

  /// \brief Wrapper class to export a Space to JavaScript
  class GECODE_SERIALIZATION_EXPORT GJSSpace
   : public QObject, public QScriptable {
    Q_OBJECT
  public:
    QScriptValue varProto;
    QScriptValue pairProto;
    Gecode::Space* s;
    Gecode::Reflection::VarMap vm;
    Gecode::Reflection::Unreflector d;
    GJSSpace(QScriptEngine*, Gecode::Space& s0);
    Gecode::Reflection::Arg* scriptValToArg(QScriptValue v);
  public Q_SLOTS:
    void constraint(const QString& name, QScriptValue args);
    QScriptValue variable(const QString& vti, QScriptValue args);
    QScriptValue pair(QScriptValue a, QScriptValue b);
  };
}}

#endif

// STATISTICS: serialization-any
