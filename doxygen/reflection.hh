/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2008
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

/**
  \page PageReflection The Reflection API

The reflection API allows to get information about all the actors (propagators and branchings) and the variables in a Space. Some use cases for reflection are
  - \b %Serialization, which creates an external, platform-independent 
    representation of a space that can be used to recreate a copy of the 
    original space. This facilitates the development of distributed search 
    engines, allows to store instances of constraint problems, and can be used 
    to transfer a problem instance from one solver to another.

  - \b Visualization of variable domains or the propagators in a space can 
    help with debugging scripts or propagators. Gecode's Gist uses reflection 
    for visualization.

  - \b Heuristics can be based on properties of the constraint graph, which is 
    effectively exposed by the reflection API.

This tutorial is split in two parts:
  - \ref SecReflUsage
  - \ref SecReflAddSupport

For information on how to recreate a Space object from a description obtained by reflection, please refer to the \ref PageSerialization "serialization tutorial".

\section SecReflUsage Using the reflection API

The starting point for reflection in Gecode is a Space. The basic work cycle is to iterate over all the actors in a space using an ActorSpecIter, collecting the variables that the actors reference in a VarMap. The iterator returns an ActorSpec for each actor, and the VarMap contains a VarSpec for each variable. All the specifications in the VarMap can be accessed using a VarMapIter.

\code
Gecode::Space* space = ...
Gecode::Reflection::VarMap vm;
Gecode::Reflection::VarMapIter vmi(vm);
for (Gecode::Reflection::ActorSpecIter si(space, vm); si(); ++si) {
  Gecode::Reflection::ActorSpec aSpec = si.actor();
  doSomethingForAnActor(aSpec);
  for (; vmi(); ++vmi) {
    Gecode::Reflection::VarSpec vSpec = vmi.spec();
    doSomethingForAVariable(vSpec);
  }
}
\endcode

Note that new variables are added to \c vm when the specification for an actor is accessed (using \c si.actor()). The VarMapIter therefore always iterates all those variables that were added to \c vm for the current actor.

We will now go into the details of ActorSpec and VarSpec, and after that discuss the VarMap.

\subsection SecReflUsageSpecs Accessing the specifications

Specifications for actors and variables are similar. For each actor or variable, a specification contains a Symbol (a string) representing its type (accessed as ActorSpec::ati resp. VarSpec::vti), and a term representing its state (e.g. the current variable domain for variables, or the arguments of a propagator). Terms are represented as objects of the class Reflection::Arg and can be summarized by the following grammar:

\verbatim
term ::= INT | VAR | [term,...,term] | [INT,...,INT] | STRING | (term,term)
\endverbatim

The upper-case constructors stand for actual values. Signed integers for \c INT, character pointers for \c STRING, and <em>integers for \c VAR </em>: variables are represented as indices into the VarMap.

The concrete representation of a variable's or actor's state depends on its concrete implementation. The following list will give you an idea of possible encodings:
  - Domains of integer variables are encoded as arrays of integers, where two consecutive integers represent a range in the domain. For example, the domain \f$\{0,1,2,4,5,8\}\f$ would be represented as \c [0,2,4,5,8,8].
  - The arguments of the distinct constraint with offsets are encoded as an array of pairs of integers and variables, like \c [(0,q0),(-1,q1),(-2,q2),(-3,q3)] for the Queens model.
  - The DFA of an extensional constraint that uses a LayeredGraph is encoded into an array of integers, where each transition is represented by three integers, one for the input state, one for the symbol, and one for the output state.

\subsection SecReflUsageVarMap The VarMap

Several propagators can share the same variable, and reflection makes this sharing explicit. The VarMap contains one entry per variable that is referenced by an actor, and it is filled by iterating over the actors. The specification of an actor only contains an integer reference into the VarMap that was used when the specification was created.

\sa
Gecode::Reflection::ActorSpecIter
Gecode::Reflection::ActorSpec
Gecode::Reflection::VarSpec
Gecode::Reflection::VarMap
Gecode::Reflection::VarMapIter
Gecode::Reflection::Arg

\section SecReflAddSupport Adding reflection support to actors and variables

When the ActorSpecIter iterates over the actors of a space, each actor has to deliver its specification through the virtual function Actor::spec. When this function is called, the actor creates an ActorSpec with its <em>actor type identifier</em> (ati) and fills it with a representation of its arguments, encoded into objects of type Reflection::Arg. The specification for the views is delegated to the views or view arrays that the actor uses.

We now go through the steps that are needed for reflection of actors and variables.

\subsection SecReflAddSupportProp Propagators

Propagators implement the virtual function Gecode::Reflection::Actor::spec. It creates an ActorSpec with a proper actor type identifier and arguments. The following example provides reflection for a propagator with an IntView \c x, a ViewArray of IntViews \c y, and an integer constant \c c:

\code
class MyPropagator : public Gecode::Propagator {
protected:
  IntView            x;
  ViewArray<IntView> y;
  int                c;
public:
  virtual Gecode::Reflection::ActorSpec&
  spec(const Space* home, Gecode::Reflection::VarMap& m) const {
    Gecode::Reflection::ActorSpec spec =
      Gecode::Propagator::spec(home, m, "MyPropagator");
    spec << x.spec(home, m);
    spec << y.spec(home, m);
    spec << c;
    return spec;
  }
};
\endcode

For a generic propagator, the types of its views must be part of the actor type identifier. All views have a static function \c type to enable this, and the reflection library contains name mangling functions:

\code
template <class View0, class View1>
class MyGenericPropagator : public Gecode::Propagator {
protected:
  View0            x;
  ViewArray<View1> y;
  int              c;
public:
  virtual Gecode::Reflection::ActorSpec&
  spec(const Space* home, Gecode::Reflection::VarMap& m) const {
    Gecode::Support::Symbol name =
      Gecode::Reflection::mangle<View0,View1>("MyGenericPropagator");
    Gecode::Reflection::ActorSpec spec =
      Gecode::Propagator::spec(home, m, name);
    spec << x.spec(home, m);
    spec << y.spec(home, m);
    spec << c;
    return spec;
  }
};
\endcode

\subsection SecReflAddSupportBranching Branchings

\subsection SecReflAddSupportVar Variables


*/

/**
  \page PageSerialization The Serialization API
*/
