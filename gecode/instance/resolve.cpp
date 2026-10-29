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

#include <gecode/instance/impl.hpp>

namespace Gecode { namespace Instance {

  namespace {

    /// A definition of a name, with the repeats around it
    class Definition {
    public:
      /// The name
      std::string name;
      /// The repeats around the definition, outermost first
      Seq<const Stmt*> repeats;
    };

    /// A repeat that names its records, with the repeats around it
    class List {
    public:
      /// The repeat (see Stmt::list and Stmt::each)
      const Stmt* repeat;
      /// The repeats around it, outermost first
      Seq<const Stmt*> repeats;
    };

    /// A name bound by a comprehension, with the repeat whose records it
    /// stands for (null for the elements of an array)
    class Bound {
    public:
      /// The name
      std::string name;
      /// The repeat
      const Stmt* records;
    };

    /// Test whether \a a is a prefix of \a b
    bool
    is_prefix(const Seq<const Stmt*>& a, const Seq<const Stmt*>& b) {
      if (a.size() > b.size())
        return false;
      for (int k=0; k<a.size(); k++)
        if (a[k] != b[k])
          return false;
      return true;
    }

    /// Test whether \a r is one of \a rs
    bool
    contains(const Seq<const Stmt*>& rs, const Stmt* r) {
      for (const Stmt* x : rs)
        if (x == r)
          return true;
      return false;
    }

    /**
     * \brief Test whether operand \a k of \a e is a whole array
     *
     * The array of a comprehension and the argument of \c size and
     * \c defined are whole arrays: a record name there is not one record.
     */
    bool
    whole_array(const Expr& e, int k) {
      if (e.kind == Expr::Kind::COMPREHENSION)
        return k == 0;
      return (e.kind == Expr::Kind::CALL) && ((e.s == "size") || (e.s == "defined"));
    }

    /**
     * \brief Resolver of the names in the expressions of a spec
     *
     * Finds the expressions that are records or lists of records of a
     * named repeat (see Expr) and the record names of the checks (see
     * Record), and reports their misuse.
     */
    class Resolver {
    public:
      /// Initialize for a spec from \a o
      explicit Resolver(const std::string& o)
        : origin(o), stmt(nullptr) {}
      /// Resolve the expressions in \a b, the body of a spec
      void run(Block& b) {
        define(b);
        check_lists();
        visit(b);
      }
    private:
      /// Where the spec comes from
      std::string origin;
      /// All definitions of names
      Seq<Definition> definitions;
      /// All repeats that name their records
      Seq<List> lists;
      /// The repeats around the statement visited, outermost first
      Seq<const Stmt*> repeats;
      /// The statement visited
      Stmt* stmt;
      /// The names bound by the comprehensions around the expression
      Seq<Bound> bound;

      [[noreturn]] void fail(const std::string& m) const {
        fail_at(origin, stmt->line, m);
      }
      /// Add a definition of \a n
      void define(const std::string& n) {
        if (n.empty() || (n == "_"))
          return;
        Definition d;
        d.name = n;
        d.repeats = repeats;
        definitions.push(std::move(d));
      }
      /// Add the definitions of the captures of \a p
      void define(const Pattern& p) {
        for (const Pattern::Item& it : p.items)
          if (it.kind == Pattern::Item::Kind::CAP)
            define(it.text);
      }
      /// Add the definitions and named repeats in \a b
      void define(const Block& b) {
        for (const std::unique_ptr<Stmt>& s : b) {
          switch (s->kind) {
          case Stmt::Kind::READ: case Stmt::Kind::LIST: case Stmt::Kind::LET:
          case Stmt::Kind::PARAM:
            define(s->name);
            break;
          case Stmt::Kind::LINE: case Stmt::Kind::FIND:
            define(s->pattern);
            break;
          case Stmt::Kind::REPEAT:
            define(s->store);
            if (!s->list.empty() || !s->each.empty()) {
              List l;
              l.repeat = s.get();
              l.repeats = repeats;
              lists.push(std::move(l));
            }
            repeats.push(s.get());
            define(s->body);
            repeats.pop();
            break;
          case Stmt::Kind::SECTIONS:
            define(s->store);
            for (const Arm& a : s->arms) {
              define(a.pattern);
              define(a.body);
            }
            break;
          case Stmt::Kind::IF:
            define(s->body);
            define(s->orelse);
            break;
          case Stmt::Kind::INSTANCES:
            define(s->body);
            break;
          default:
            break;
          }
        }
      }
      /// Test whether \a n names the records of a repeat
      bool names_records(const std::string& n) const {
        for (const List& l : lists)
          if ((l.repeat->list == n) || (l.repeat->each == n))
            return true;
        return false;
      }
      /// Check that name \a n of the records of repeat \a i is not used
      /// otherwise
      void check_name(const std::string& n, int i) const {
        const Stmt* r = lists[i].repeat;
        if (n.empty())
          return;
        for (int j=0; j<i; j++)
          if ((lists[j].repeat->list == n) || (lists[j].repeat->each == n))
            fail_at(origin, r->line, "'" + n + "' already names records");
        for (const Definition& d : definitions)
          if (d.name == n)
            fail_at(origin, r->line, "'" + n + "' names records, so it cannot also be defined");
      }
      /// Check that the names of the records of repeats are not used
      /// otherwise
      void check_lists(void) const {
        for (int i=0; i<lists.size(); i++) {
          const Stmt* r = lists[i].repeat;
          check_name(r->list, i);
          if (r->each == r->list)
            fail_at(origin, r->line, "'" + r->each + "' cannot name both the list and each record");
          check_name(r->each, i);
        }
      }
      /// Return the repeat whose list of records is called \a n, or null
      const List* find_list(const std::string& n) const {
        for (const List& l : lists)
          if (l.repeat->list == n)
            return &l;
        return nullptr;
      }
      /// Return the repeat each of whose records is called \a n, or null
      const List* find_each(const std::string& n) const {
        for (const List& l : lists)
          if (l.repeat->each == n)
            return &l;
        return nullptr;
      }
      /// Return the name of the records of repeat \a r, for messages
      static const std::string& records_name(const Stmt* r) {
        return r->list.empty() ? r->each : r->list;
      }
      /// Test whether \a n is bound by a comprehension
      bool is_bound(const std::string& n) const {
        for (const Bound& b : bound)
          if (b.name == n)
            return true;
        return false;
      }
      /// Return the list of records called \a n directly inside repeat
      /// \a r, or null
      const List* nested_list(const Stmt* r, const std::string& n) const {
        const List* m = find_list(n);
        if ((m == nullptr) || !contains(m->repeats, r))
          return nullptr;
        if (m->repeats.back() != r)
          fail("'" + n + "' is inside a repeat without a list name in the records of '" + records_name(r) +
               "'; name that list with 'as' to reach it");
        if (contains(repeats, m->repeat))
          fail("'" + n + "' cannot be used inside its own repeat");
        return m;
      }
      /// Return the names that the records of repeat \a r have, for messages
      std::string fields(const Stmt* r) const {
        Seq<std::string> f;
        for (const Definition& d : definitions)
          if (contains(d.repeats, r))
            f.push(d.name);
        for (const List& l : lists)
          if (!l.repeats.empty() && (l.repeats.back() == r) && !l.repeat->list.empty())
            f.push(l.repeat->list);
        std::string s;
        for (int i=0; i<f.size(); i++) {
          bool seen = false;
          for (int j=0; j<i; j++)
            seen = seen || (f[j] == f[i]);
          if (!seen)
            s += (s.empty() ? "" : ", ") + f[i];
        }
        return s;
      }
      /// Return an example of a field of a record of repeat \a r, through
      /// its list if \a list, for messages
      std::string example(const Stmt* r, bool list) const {
        std::string f = fields(r);
        size_t c = f.find(',');
        std::string x = (list || r->each.empty()) ? r->list + "[0]" : r->each;
        return f.empty() ? x : x + "." + ((c == std::string::npos) ? f : f.substr(0, c));
      }
      /// Return the index of \a r in \a rs, or -1
      static int position(const Seq<const Stmt*>& rs, const Stmt* r) {
        for (int k=0; k<rs.size(); k++)
          if (rs[k] == r)
            return k;
        return -1;
      }
      /// Test whether definition \a d of a field of the records of repeat
      /// \a r is in a repeat around the statement inside \a r, so that its
      /// values for the record are not complete
      bool incomplete(const Definition& d, const Stmt* r) const {
        int common = 0;
        while ((common < d.repeats.size()) && (common < repeats.size()) && (d.repeats[common] == repeats[common]))
          common++;
        return common > position(d.repeats, r) + 1;
      }
      /// Check that \a f is a field of the records of repeat \a r
      void require_field(const Stmt* r, const std::string& f) const {
        bool inside = false, outside = false;
        for (const Definition& d : definitions)
          if (d.name == f) {
            inside = inside || contains(d.repeats, r);
            outside = outside || !contains(d.repeats, r);
          }
        if (!inside) {
          std::string known = fields(r);
          fail("the records of '" + records_name(r) + "' have no field '" + f + "'" +
               (known.empty() ? std::string(" (they have none)") : " (they have " + known + ")"));
        }
        if (outside)
          fail("'" + f + "' is defined inside and outside the repeat of '" + records_name(r) +
               "', so it cannot be a field of its records");
        for (const Definition& d : definitions)
          if ((d.name == f) && incomplete(d, r))
            fail("the field '" + f + "' cannot be used inside the repeat that reads it");
      }
      /// Check that \a e is not a record or a list of records
      void require_value(const Expr& e) const {
        const Stmt* r = e.records;
        if (r == nullptr)
          return;
        if (e.open == 0)
          fail("a record of '" + records_name(r) + "' is not a value; use one of its fields, as in '" +
               example(r, false) + "'");
        fail("'" + r->list + "' is a list of records, not a value; use size(" + r->list +
             "), an index, or a field of a record, as in '" + example(r, true) + "'");
      }

      /// Resolve the expressions of the statements in \a b
      void visit(Block& b) {
        for (std::unique_ptr<Stmt>& s : b) {
          statement(*s);
          bool repeat = (s->kind == Stmt::Kind::REPEAT);
          if (repeat)
            repeats.push(s.get());
          visit(s->body);
          if (repeat)
            repeats.pop();
          visit(s->orelse);
          for (Arm& a : s->arms)
            visit(a.body);
        }
      }
      /// Resolve the expressions of statement \a s
      void statement(Stmt& s) {
        stmt = &s;
        type(s.type);
        pattern(s.pattern);
        for (Arm& a : s.arms)
          pattern(a.pattern);
        for (std::unique_ptr<Expr>& d : s.dims)
          value(*d);
        if (s.expr)
          value(*s.expr);
        if (s.sentinel)
          value(*s.sentinel);
        if (s.offset)
          value(*s.offset);
        if (s.kind == Stmt::Kind::CHECK)
          check_chain(s);
      }
      /// Resolve the bounds of type \a t
      void type(Type& t) {
        if (t.lo)
          value(*t.lo);
        if (t.hi)
          value(*t.hi);
      }
      /// Resolve the bounds of the types of the captures of \a p
      void pattern(Pattern& p) {
        for (Pattern::Item& it : p.items)
          type(it.type);
      }
      /// Test whether the statement visited is a check, so that record
      /// names in it range over their records
      bool lifting(void) const {
        return stmt->kind == Stmt::Kind::CHECK;
      }
      /// Check that the record names of check \a s range over one repeat
      /// and the repeats inside it
      void check_chain(const Stmt& s) const {
        int deepest = 0;
        for (int i=1; i<s.records.size(); i++)
          if (s.records[i].repeats.size() > s.records[deepest].repeats.size())
            deepest = i;
        for (int i=0; i<s.records.size(); i++)
          if (!is_prefix(s.records[i].repeats, s.records[deepest].repeats))
            fail("a check can only range over the records of one repeat, but '" +
                 s.records[std::min(i, deepest)].label + "' and '" + s.records[std::max(i, deepest)].label +
                 "' are defined in different repeats");
      }

      /// Resolve \a e, which must be a value
      void value(Expr& e) {
        resolve(e, false);
        require_value(e);
      }
      /// Resolve \a e, which is a whole array if \a whole
      void resolve(Expr& e, bool whole) {
        switch (e.kind) {
        case Expr::Kind::NAME:
          name(e, whole);
          break;
        case Expr::Kind::FIELD:
          field(e);
          break;
        case Expr::Kind::INDEX:
          index(e);
          break;
        case Expr::Kind::COMPREHENSION:
          comprehension(e);
          break;
        case Expr::Kind::CALL:
          call(e);
          break;
        default:
          for (std::unique_ptr<Expr>& a : e.args)
            value(*a);
          break;
        }
      }
      /// Resolve name \a e
      void name(Expr& e, bool whole) {
        for (int k=bound.size(); k--; )
          if (bound[k].name == e.s) {
            e.records = bound[k].records;
            return;
          }
        if (const List* l = find_list(e.s))
          list(e, *l);
        else if (const List* r = find_each(e.s))
          record_name(e, *r);
        else if (lifting() && !whole)
          e.record = record(e.s);
      }
      /// Return the repeats of \a d that are not around the statement visited
      Seq<const Stmt*> beyond(const Seq<const Stmt*>& d) const {
        int common = 0;
        while ((common < d.size()) && (common < repeats.size()) && (d[common] == repeats[common]))
          common++;
        Seq<const Stmt*> r;
        for (int k=common; k<d.size(); k++)
          r.push(d[k]);
        return r;
      }
      /// Resolve name \a e of the list of records of \a l
      void list(Expr& e, const List& l) {
        e.records = l.repeat;
        e.open = 1;
        if (contains(repeats, l.repeat))
          fail("'" + e.s + "' cannot be used inside its own repeat");
        if (beyond(l.repeats).empty())
          return;
        // One list for each record of the repeat around it
        const std::string& outer = l.repeats.back()->each;
        fail("'" + e.s + "' is a list in each record of the repeat around it; " +
             (outer.empty() ? "name each of these records with 'each' and use its field '" + e.s + "'"
                            : "use '" + outer + "." + e.s + "'"));
      }
      /**
       * \brief Resolve name \a e of each record of \a l
       *
       * Inside the repeat, the name is the current record. Elsewhere, it is
       * a record name: a check ranges over the records.
       */
      void record_name(Expr& e, const List& l) {
        e.records = l.repeat;
        if (contains(repeats, l.repeat))
          return;
        if (!lifting())
          fail("'" + e.s + "' is each record of a repeat, so only checks can use it outside that repeat");
        Seq<const Stmt*> o = beyond(l.repeats);
        o.push(l.repeat);
        e.record = lift(record_starts(l.repeat->id), o, e.s);
      }
      /// Resolve field \a e
      void field(Expr& e) {
        Expr& base = *e.args[0];
        resolve(base, false);
        const Stmt* r = base.records;
        const std::string& f = e.s;
        if (r == nullptr)
          fail("only records have fields such as '." + f + "'; to name each record of a repeat, write "
               "'repeat n as nodes { each node ... }' and use 'node." + f + "' or 'nodes[0]." + f + "'");
        if (base.open > 0)
          fail("'" + r->list + "' is a list of records, so '." + f + "' needs one of them: use '" + r->list + "[i]." +
               f + "', or " + (r->each.empty() ? "name each record with 'each' in the repeat and use its field"
                                               : "'" + r->each + "." + f + "' for each record"));
        if (const List* m = nested_list(r, f)) {
          e.records = m->repeat;
          e.open = 1;
        } else {
          require_field(r, f);
        }
      }
      /// Resolve index expression \a e
      void index(Expr& e) {
        Expr& a = *e.args[0];
        resolve(a, false);
        value(*e.args[1]);
        if (a.records == nullptr)
          return;
        if (a.open == 0)
          fail("a record of '" + records_name(a.records) + "' cannot be indexed; use one of its fields, as in '" +
               example(a.records, false) + "'");
        e.records = a.records;
      }
      /// The repeat whose records are the elements of array \a a, or null
      const Stmt* elements(const Expr& a) const {
        if (a.records != nullptr)
          return a.records;
        if (a.kind == Expr::Kind::FIELD)
          return outermost(a.s, a.args[0]->records);
        if ((a.kind != Expr::Kind::NAME) || is_bound(a.s))
          return nullptr;
        return outermost(a.s, nullptr);
      }
      /**
       * \brief Return the outermost repeat whose records are the rows of
       * name \a n, or null
       *
       * These are the repeats inside \a r if not null, otherwise those
       * not around the statement visited.
       */
      const Stmt* outermost(const std::string& n, const Stmt* r) const {
        const Stmt* o = nullptr;
        bool found = false;
        for (const Definition& d : definitions)
          if (d.name == n) {
            int k = (r == nullptr) ? d.repeats.size() - beyond(d.repeats).size() : position(d.repeats, r) + 1;
            const Stmt* x = (k < d.repeats.size()) ? d.repeats[k] : nullptr;
            if (found && (x != o))
              return nullptr;
            o = x;
            found = true;
          }
        return o;
      }
      /// Resolve comprehension \a e
      void comprehension(Expr& e) {
        if (names_records(e.var))
          fail("'" + e.var + "' is bound by a comprehension but names records");
        Expr& a = *e.args[0];
        resolve(a, true);
        if ((a.records != nullptr) && (a.open == 0))
          fail("a record of '" + records_name(a.records) + "' is not a list; use one of its fields");
        e.elements = elements(a);
        Bound b;
        b.name = e.var;
        b.records = a.records;
        bound.push(b);
        value(*e.args[1]);
        bound.pop();
      }
      /// Resolve call \a e
      void call(Expr& e) {
        for (int k=0; k<e.args.size(); k++) {
          Expr& a = *e.args[k];
          resolve(a, whole_array(e, k));
          bool counted = (e.s == "size") || (e.s == "count") || (e.s == "defined");
          if (!counted || (a.open == 0))
            require_value(a);
        }
      }

      /**
       * \brief Return the index of record name \a n among the record names
       * of the check visited, or -1 if it is not one
       *
       * A record name is defined inside repeats that are not around the
       * check, and only there.
       */
      int record(const std::string& n) {
        bool found = false;
        Seq<const Stmt*> outside;
        for (const Definition& d : definitions) {
          if (d.name != n)
            continue;
          Seq<const Stmt*> o = beyond(d.repeats);
          if (found && !(is_prefix(o, outside) && is_prefix(outside, o)))
            fail("'" + n + "' is defined in several repeats, or inside and outside a repeat, so "
                 "a check here cannot range over its records");
          outside = std::move(o);
          found = true;
        }
        if (outside.empty())
          return -1;
        return lift(n, outside, n);
      }
      /// Return the index of the record name for the values of \a n in the
      /// records of \a rs among those of the check visited, adding it if
      /// needed; \a label is its name in messages
      int lift(const std::string& n, const Seq<const Stmt*>& rs, const std::string& label) {
        Seq<Record>& records = stmt->records;
        for (int i=0; i<records.size(); i++)
          if ((records[i].name == n) && (records[i].repeats.size() == rs.size()))
            return i;
        Record r;
        r.name = n;
        r.label = label;
        r.repeats = rs;
        r.starts = record_starts(rs.back()->id);
        records.push(std::move(r));
        return records.size()-1;
      }
    };

  }

  void
  resolve(Block& b, const std::string& origin) {
    Resolver(origin).run(b);
  }

}}

// STATISTICS: instance-any
