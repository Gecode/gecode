/*
 *  Main authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Samuel Gagnon, 2018
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

#ifdef GECODE_HAS_CBS

namespace Gecode { namespace Int { namespace Branch {

    template <class View>
    forceinline void
    CBSBrancher<View>::VarIdToPos::init() {
        assert(object() == nullptr);
        object(new VarIdToPosO());
    }

    template <class View>
    forceinline bool
    CBSBrancher<View>::VarIdToPos::is_in(unsigned int var_id) const {
        auto* hm = &static_cast<VarIdToPosO*>(object())->_var_id_to_pos;
        return hm->find(var_id) != hm->end();
    }

    template <class View>
    forceinline int
    CBSBrancher<View>::VarIdToPos::operator[](unsigned int i) const {
        return static_cast<VarIdToPosO*>(object())->_var_id_to_pos.at(i);
    }

    template <class View>
    forceinline void
    CBSBrancher<View>::VarIdToPos::insert(unsigned int var_id, unsigned int pos) {
        static_cast<VarIdToPosO*>(object())->_var_id_to_pos.insert(std::make_pair(var_id, pos));
    }

    template <class View>
    CBSBrancher<View>::CBSBrancher(Home home, ViewArray<View>& x0)
        : Brancher(home), x(x0),
          log_prop(typename decltype(log_prop)::size_type(), typename decltype(log_prop)::hasher(),
                   typename decltype(log_prop)::key_equal(), typename decltype(log_prop)::allocator_type(home)) {
        home.notice(*this, AP_DISPOSE);
        var_id_to_pos.init();
        for (int i = 0; i < x.size(); i++)
            var_id_to_pos.insert(x[i].id(), i);
    }

    template <class View>
    forceinline void
    CBSBrancher<View>::post(Home home, ViewArray<View>& x) {
        (void)new (home) CBSBrancher(home, x);
    }

    template <class View>
    Actor*
    CBSBrancher<View>::copy(Space& home) {
        return new (home) CBSBrancher(home, *this);
    }

    template <class View>
    forceinline size_t
    CBSBrancher<View>::dispose(Space& home) {
        home.ignore(*this, AP_DISPOSE);
        var_id_to_pos.~VarIdToPos();
        (void)Brancher::dispose(home);
        return sizeof(*this);
    }

    template <class View>
    CBSBrancher<View>::CBSBrancher(Space& home, CBSBrancher& b)
        : Brancher(home, b), var_id_to_pos(b.var_id_to_pos),
          log_prop(b.log_prop.begin(), b.log_prop.end(), typename decltype(log_prop)::size_type(),
                   typename decltype(log_prop)::hasher(), typename decltype(log_prop)::key_equal(),
                   typename decltype(log_prop)::allocator_type(home)) {
        x.update(home, b.x);
    }

    template <class View>
    bool
    CBSBrancher<View>::status(const Space& home) const {
        for (Propagators p(home, PropagatorGroup::all); p(); ++p) {
            // Sum of domains of all variable in propagator
            unsigned int domsum;
            // Same, but for variables that are also in this brancher.
            unsigned int domsum_b;

            // If the propagator doesn't support counting-based search, domsum and
            // domsum_b are going to be equal to 0.
            p.propagator().domain_size_sum([this](unsigned int var_id) { return in_brancher(var_id); }, domsum,
                                           domsum_b);

            if (domsum_b > 0)
                return true;
        }

        return false;
    }

    template <class View>
    forceinline bool
    CBSBrancher<View>::in_brancher(unsigned int var_id) const {
        return var_id_to_pos.is_in(var_id);
    }

    template <class View>
    const Choice*
    CBSBrancher<View>::choice(Space& home) {
        // Structure for keeping the maximum solution density assignment
        struct {
            unsigned int var_id;
            int val;
            double dens;
        } max_sd{0, 0, -1};

        // Lambda we pass to propagators via solution_distribution to query solution densities
        auto send_marginal = [this](unsigned int prop_id, unsigned int var_id, int val, double dens) {
            if (log_prop[prop_id].dens < dens) {
                log_prop[prop_id].var_id = var_id;
                log_prop[prop_id].val = val;
                log_prop[prop_id].dens = dens;
            }
        };

        for (auto& kv : log_prop)
            kv.second.visited = false;

        for (Propagators p(home, PropagatorGroup::all); p(); ++p) {
            unsigned int prop_id = p.propagator().id();
            unsigned int domsum;
            unsigned int domsum_b;

            p.propagator().domain_size_sum([this](unsigned int var_id) { return in_brancher(var_id); }, domsum,
                                           domsum_b);

            // If the propagator doesn't share any unasigned variables with this
            // brancher, we continue and it will be deleted from the log afterwards.
            if (domsum_b == 0)
                continue;

            // New propagators can be created as we solve the problem. If this is the
            // case we create a new entry in the log.
            if (log_prop.find(prop_id) == log_prop.end())
                log_prop.insert(std::make_pair(prop_id, PropInfo{0, 0, 0, -1, true}));
            else
                log_prop[prop_id].visited = true;

            // If the domain size sum of all variables in the propagator has changed
            // since the last time we called this function, we need to recompute
            // solution densities. Otherwise, we can reuse them.
            if (log_prop[prop_id].domsum != domsum) {
                log_prop[prop_id].dens = -1;
                // Solution density computation
                p.propagator().solution_distribution(home, send_marginal);
                log_prop[prop_id].domsum = domsum;
            }
        }

        // We delete unvisited propagators from the log and look for the highest
        // solution density across all propagators.
        for (auto i = log_prop.begin(); i != log_prop.end();) {
            if (!i->second.visited) {
                i = log_prop.erase(i);
            } else {
                const PropInfo& info = i->second;
                if (info.dens > max_sd.dens)
                    max_sd = {info.var_id, info.val, info.dens};
                ++i;
            }
        }

        assert(max_sd.dens != -1);
        assert(!x[var_id_to_pos[max_sd.var_id]].assigned());
        return new PosValChoice<int>(*this, 2, var_id_to_pos[max_sd.var_id], max_sd.val);
    }

    template <class View>
    forceinline const Choice*
    CBSBrancher<View>::choice(const Space&, Archive& e) {
        int pos, val;
        e >> pos >> val;
        return new PosValChoice<int>(*this, 2, pos, val);
    }

    template <class View>
    forceinline ExecStatus
    CBSBrancher<View>::commit(Space& home, const Choice& c, unsigned int a) {
        const auto& pvc = static_cast<const PosValChoice<int>&>(c);
        int pos = pvc.pos().pos;
        int val = pvc.val();
        if (a == 0)
            return me_failed(x[pos].eq(home, val)) ? ES_FAILED : ES_OK;
        else
            return me_failed(x[pos].nq(home, val)) ? ES_FAILED : ES_OK;
    }

    template <class View>
    forceinline void
    CBSBrancher<View>::print(const Space&, const Choice& c, unsigned int a, std::ostream& o) const {
        const auto& pvc = static_cast<const PosValChoice<int>&>(c);
        int pos = pvc.pos().pos, val = pvc.val();
        if (a == 0)
            o << "x[" << pos << "] = " << val;
        else
            o << "x[" << pos << "] != " << val;
    }

}}} // namespace Gecode::Int::Branch

#endif

// STATISTICS: int-branch
