/*
 *  Main authors:
 *     Claude-Guy Quimper <claude-guy.quimper@ift.ulaval.ca>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Claude-Guy Quimper, 2006
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

#include <gecode/int/inter-distance.hh>
#include <gecode/int/distinct.hh>

#include <algorithm>
#include <limits>
#include <tuple>

namespace Gecode { namespace Int { namespace InterDistance {

    namespace {
        typedef long long int Time;
        typedef std::ptrdiff_t RegionIndex;

        /// Task bounds and ranks in their sorted orders
        struct Task {
            Time min;
            Time max;
            int min_rank;
            int max_rank;
        };

        /// Forbidden starting times (inclusive)
        struct Forbidden {
            Time min;
            Time max;
        };

        /// Nonnegative remainder, including for negative deadlines
        Time
        residue(Time t, Time p) {
            Time q = t % p;
            return (q < 0) ? q + p : q;
        }

        /**
         * \brief Ordered residue groups with region-backed storage
         *
         * Group keys are drawn from the original deadline residues. A shift
         * merges groups into an existing key, but later deadline activation can
         * reinsert a removed key. A Fenwick tree of active-key counts supports
         * insertion, removal, and successor queries in O(log n) time.
         */
        class ResidueGroups {
            Time* keys;
            int* groups;
            int* counts;
            int key_count;
            int active_count;
            int top_bit;

            /// Change the active-key count at rank i
            void
            update(int i, int delta) {
                for (unsigned int j = static_cast<unsigned int>(i) + 1; j <= static_cast<unsigned int>(key_count);
                     j += j & -j)
                    counts[j] += delta;
                active_count += delta;
            }

            /// First active key at or after rank i, or keyCount
            int
            next_active(int i) const {
                int before = 0;
                for (unsigned int j = static_cast<unsigned int>(i); j; j -= j & -j)
                    before += counts[j];
                if (before == active_count)
                    return key_count;

                int ordinal = before + 1;
                int rank = 0;
                for (int bit = top_bit; bit; bit /= 2)
                    if ((bit <= key_count - rank) && (counts[rank + bit] < ordinal)) {
                        rank += bit;
                        ordinal -= counts[rank];
                    }
                return rank;
            }

        public:
            /// Allocate once; repeated distance checks reuse these arrays
            ResidueGroups(Region& r, int n)
                : keys(r.alloc<Time>(n)), groups(r.alloc<int>(n)),
                  counts(r.alloc<int>(static_cast<unsigned int>(n) + 1)), key_count(0), active_count(0), top_bit(0) {}

            /// Prepare the key universe for distance p
            void
            reset(const Time* deadlines, int n, int p) {
                for (int i = 0; i < n; i++)
                    keys[i] = residue(deadlines[i], p);
                std::sort(keys, keys + n);
                key_count = static_cast<int>(std::unique(keys, keys + n) - keys);
                std::fill(groups, groups + key_count, -1);
                std::fill(counts, counts + key_count + 1, 0);
                active_count = 0;
                top_bit = 1;
                while (top_bit <= key_count / 2)
                    top_bit *= 2;
            }

            /// Rank of a key in the fixed universe
            int
            rank(Time key) const {
                int i = static_cast<int>(std::lower_bound(keys, keys + key_count, key) - keys);
                assert((i < key_count) && (keys[i] == key));
                return i;
            }

            /// Key and group at rank i; -1 denotes an inactive group
            Time
            key(int i) const {
                return keys[i];
            }
            int
            group(int i) const {
                return groups[i];
            }

            /// Insert a key or replace its group representative
            void
            set_group(int i, int root) {
                if (groups[i] < 0)
                    update(i, 1);
                groups[i] = root;
            }

            /// Remove an active key
            void
            remove(int i) {
                assert(groups[i] >= 0);
                groups[i] = -1;
                update(i, -1);
            }

            /// First active key, or the end rank
            int
            first_active() const {
                return next_active(0);
            }
            /// First active key greater than key, or the end rank
            int
            active_after(Time key) const {
                return next_active(static_cast<int>(std::upper_bound(keys, keys + key_count, key) - keys));
            }
            /// Whether a successor query returned the end rank
            bool
            at_end(int i) const {
                return i == key_count;
            }
        };

        /**
         * \brief Linear-space feasibility checker of Garey et al. (1981)
         *
         * Algorithm B represents each deadline's critical time as its deadline
         * minus its task load and pseudo-offset. A Fenwick tree maintains loads.
         * A weighted union-find merges deadlines with the same residue modulo p;
         * ordered region-backed arrays locate affected residues. Each
         * deadline is inserted once and each merge removes a residue, giving
         * O(n log n) time. Relevant deadlines are kept in deadline order, with
         * dominated predecessors removed permanently (Lemmas 5 and 6).
         */
        class Feasibility {
            const int n;
            int dc;
            Task* tasks;
            Task** release;
            /// Distinct latest start times; critical() adds p to obtain deadlines
            Time* deadlines;
            int* loads;
            int* successor;
            int* previous;
            int* parent;
            int* size;
            Time* offset;
            ResidueGroups residues;
            /// First remaining relevant deadline at or after i
            int
            find(int i) {
                int root = i;
                while (successor[root] != root)
                    root = successor[root];
                while (successor[i] != i) {
                    int next = successor[i];
                    successor[i] = root;
                    i = next;
                }
                return root;
            }
            /// Number of processed tasks with a deadline at or before i
            int
            load(int i) const {
                int count = 0;
                for (size_t j = static_cast<size_t>(i) + 1; j; j -= j & -j)
                    count += loads[j];
                return count;
            }
            /// Sum weighted links, including the root's shared offset
            Time
            displacement(int i) const {
                if (parent[i] == -1)
                    return 0;
                Time d = offset[i];
                while (parent[i] != i) {
                    i = parent[i];
                    d += offset[i];
                }
                return d;
            }
            /// Merge roots while preserving each deadline's displacement
            int
            merge(int a, int b) {
                if (size[a] < size[b])
                    std::swap(a, b);
                parent[b] = a;
                offset[b] -= offset[a];
                size[a] += size[b];
                return a;
            }
            /// Pseudo-critical time for deadline i
            Time
            critical(int i, Time p) const {
                return deadlines[i] + p - static_cast<Time>(load(i)) * p - displacement(i);
            }

            /// Shift and merge active residues from first up to end into target
            void
            merge_residue_range(int first, Time end, int target, int p) {
                Time a = residues.key(target);
                int root = residues.group(target);
                int i = first;
                while (!residues.at_end(i) && (residues.key(i) < end)) {
                    Time key = residues.key(i);
                    int group = residues.group(i);
                    offset[group] += residue(key - a, p);
                    root = merge(root, group);
                    residues.remove(i);
                    i = residues.active_after(key);
                }
                residues.set_group(target, root);
            }

            /// Move groups in the open residue arc (a,b) into the group at a
            void
            forbid(Time a, Time b, int p) {
                int target = residues.rank(a);
                // The active deadline's group has residue critical(active,p) % p,
                // so activation has already supplied the target key.
                assert(residues.group(target) >= 0);

                // At a wraparound, zero is included precisely when it is below b.
                merge_residue_range(residues.active_after(a), (a < b) ? b : p, target, p);
                if (a >= b)
                    merge_residue_range(residues.first_active(), b, target, p);
            }

        public:
            Feasibility(Region& r, const ViewArray<IntView>& x) : n(x.size()), dc(0), residues(r, x.size()) {
                tasks = r.alloc<Task>(n);
                release = r.alloc<Task*>(n);
                Task** finish = r.alloc<Task*>(n);
                deadlines = r.alloc<Time>(n);
                for (int i = 0; i < n; i++) {
                    tasks[i].min = x[i].min();
                    tasks[i].max = x[i].max();
                    release[i] = finish[i] = tasks + i;
                }
                std::sort(release, release + n, [](const Task* a, const Task* b) {
                    return std::tie(a->min, a->max) < std::tie(b->min, b->max);
                });
                std::sort(finish, finish + n, [](const Task* a, const Task* b) { return a->max < b->max; });
                for (int i = 0; i < n; i++) {
                    if ((i == 0) || (finish[i]->max != deadlines[dc - 1]))
                        deadlines[dc++] = finish[i]->max;
                    finish[i]->max_rank = dc - 1;
                }
                // Include the successor sentinel without overflowing signed int.
                // Region::alloc has an unsigned-int overload on every platform.
                const unsigned int count = static_cast<unsigned int>(dc) + 1;
                loads = r.alloc<int>(count);
                successor = r.alloc<int>(count);
                previous = r.alloc<int>(dc);
                parent = r.alloc<int>(dc);
                size = r.alloc<int>(dc);
                offset = r.alloc<Time>(dc);
            }
            /// Test interval feasibility, reusing the sorted bounds and scratch
            bool
            check(int p) {
                if (p == 0)
                    return true;

                std::fill(loads, loads + dc + 1, 0);
                for (int i = 0; i < dc; i++) {
                    successor[i] = i;
                    previous[i] = i - 1;
                    parent[i] = -1;
                }
                successor[dc] = dc;
                residues.reset(deadlines, dc, p);

                int active = dc;
                int pending = dc - 1;
                for (int i = n; i--;) {
                    int rank = release[i]->max_rank;
                    for (size_t j = static_cast<size_t>(rank) + 1; j <= static_cast<size_t>(dc); j += j & -j)
                        loads[j]++;
                    int d = find(rank);
                    active = std::min(active, d);
                    Time c = critical(d, p);
                    while ((previous[d] >= 0) && (critical(previous[d], p) > c)) {
                        int obsolete = previous[d];
                        successor[obsolete] = d;
                        previous[d] = previous[obsolete];
                        if (active == obsolete)
                            active = d;
                    }

                    // Construct regions after processing all tasks at this release time.
                    if ((i > 0) && (release[i - 1]->min == release[i]->min))
                        continue;
                    c = critical(active, p);
                    Time r = release[i]->min;
                    if (c < r)
                        return false;
                    if (c - p + 1 >= r)
                        continue;

                    // Only deadlines whose back-schedule can reach this region
                    // are activated. Earlier deadlines must retain zero offsets.
                    while ((pending >= 0) && (deadlines[pending] + p >= c)) {
                        parent[pending] = pending;
                        size[pending] = 1;
                        offset[pending] = 0;
                        int key = residues.rank(residue(deadlines[pending], p));
                        int group = residues.group(key);
                        residues.set_group(key, (group < 0) ? pending : merge(group, pending));
                        pending--;
                    }
                    forbid(residue(c - p, p), residue(r, p), p);
                }
                return true;
            }
        };

        /// Adjustment interval in a doubly linked list sorted by minimum
        struct Adjustment {
            Time min;
            Time max;
            RegionIndex previous;
            RegionIndex next;
        };

        /**
         * \brief Scratch data for one direction of bounds filtering
         *
         * The Garey-Johnson-Simons-Tarjan feasibility pass constructs forbidden
         * starting regions. An earliest-completion table and a latest-start
         * vector allow the dominance sweep to generate only quadratically many
         * adjustment intervals. The linked list and union-find propagate changes
         * to bounds without rescanning all variables for each interval.
         * This implements Algorithm 2 and the adjustment data structure in
         * Sections 4.1 and 4.2 of Quimper et al. (2008).
         */
        class Filter {
        private:
            const int n;
            const Time p;
            Task** min_sorted;
            Task** max_sorted;
            Forbidden* forbidden;
            int forbidden_count;
            Time* ect_table;
            Time* lst_table;
            Adjustment* regions;
            RegionIndex region_count;
            RegionIndex* first_region;
            /// Adjustment-list cursor for each live bound-group representative
            RegionIndex* next_region;
            Time* bounds;
            // links[i] > i forwards to a merged group's representative; otherwise
            // i is a live representative and links[i] < i is its predecessor (-1
            // for the first group). Merging preserves this predecessor at the root.
            int* links;
            bool* processed;
            int* leader;

        public:
            /// Bounds loaded by the propagator, then tightened by this sweep
            Task* tasks;
            /// Allocate scratch storage, with no persistent ownership
            Filter(Region& r, int n0, int p0) : n(n0), p(p0), forbidden_count(0), region_count(0) {
                const size_t count = static_cast<size_t>(n);
                const size_t limit = std::numeric_limits<size_t>::max();
                // Region::alloc multiplies unchecked. Reject impossible byte sizes
                // before allocating the quadratic tables or interval list.
                if ((count > limit / sizeof(Time) / (count + 1)) ||
                    (count > (limit / sizeof(Adjustment) - 3) / (count + 3)))
                    throw MemoryExhausted();
                tasks = r.alloc<Task>(n);
                min_sorted = r.alloc<Task*>(n);
                max_sorted = r.alloc<Task*>(n);
                forbidden = r.alloc<Forbidden>(n);
                const size_t entries = count * (count + 1);
                ect_table = static_cast<Time*>(r.ralloc(entries * sizeof(Time)));
                lst_table = r.alloc<Time>(n + 2);
                const size_t capacity = count * (count + 3) / 2 + 3;
                regions = static_cast<Adjustment*>(r.ralloc(capacity * sizeof(Adjustment)));
                first_region = r.alloc<RegionIndex>(n);
                next_region = r.alloc<RegionIndex>(n);
                bounds = r.alloc<Time>(n);
                links = r.alloc<int>(n);
                processed = r.alloc<bool>(n);
                leader = r.alloc<int>(n);
            }
            /// Earliest completion time for q tasks at release time i
            Time
            ect(int i, int q) const {
                return ect_table[static_cast<size_t>(q) * n + i];
            }
            /// Latest start time for q tasks at the current deadline
            Time
            lst(int q) const {
                return lst_table[q];
            }
            /// Find the representative of a merged group of bounds
            int
            find(int i) {
                int j = i;
                while (links[j] > j)
                    j = links[j];
                while (links[i] > i) {
                    int k = links[i];
                    links[i] = j;
                    i = k;
                }
                return j;
            }
            /// Notify bounds of a new or expanded adjustment interval
            int
            notify(RegionIndex r, int vi) {
                assert((vi >= 0) && (links[vi] < vi));
                while ((links[vi] >= 0) && (bounds[links[vi]] >= regions[r].min))
                    vi = links[vi];
                while (regions[r].min <= bounds[vi]) {
                    if (bounds[vi] <= regions[r].max) {
                        while ((vi < n - 1) && (regions[r].max + 1 >= min_sorted[vi + 1]->min)) {
                            int previous = links[vi];
                            links[vi] = vi + 1;
                            vi = find(vi);
                            links[vi] = previous;
                        }
                        if (regions[r].max >= bounds[vi])
                            bounds[vi] = regions[r].max + 1;
                    }
                    if (regions[r].min < regions[next_region[vi]].min)
                        r = next_region[vi];
                    else
                        next_region[vi] = r;
                    r = regions[r].next;
                }
                assert((vi >= 0) && (links[vi] < vi));
                return vi;
            }

        private:
            /// Sort task pointers and record release ranks
            void
            sort_tasks() {
                for (int i = 0; i < n; i++)
                    min_sorted[i] = max_sorted[i] = tasks + i;
                std::sort(min_sorted, min_sorted + n, [](const Task* a, const Task* b) {
                    return std::tie(a->min, a->max) < std::tie(b->min, b->max);
                });
                std::sort(max_sorted, max_sorted + n, [](const Task* a, const Task* b) {
                    return std::tie(a->max, a->min) < std::tie(b->max, b->min);
                });
                for (int i = 0; i < n; i++)
                    min_sorted[i]->min_rank = i;
            }

            /// Construct forbidden start regions, detecting interval infeasibility
            bool
            compute_forbidden_regions(Region& r) {
                Time* deadlines = r.alloc<Time>(n);
                int dc = 0;
                for (int i = 0; i < n; i++) {
                    if ((i == 0) || (max_sorted[i]->max > deadlines[dc - 1]))
                        deadlines[dc++] = max_sorted[i]->max;
                    max_sorted[i]->max_rank = dc - 1;
                }

                Time* times = r.alloc<Time>(dc);
                int* cursor = r.alloc<int>(dc);
                Forbidden* buffer = r.alloc<Forbidden>(n);
                // Indices keep cursor movement within the allocated forbidden-region
                // buffer, including its empty-prefix boundary.
                int last = n - 1;
                for (int i = 0; i < dc; i++) {
                    times[i] = deadlines[i];
                    cursor[i] = last;
                }

                Time min = std::numeric_limits<Time>::max();
                Time new_min = min;
                forbidden_count = 0;
                for (int i = n; i--;) {
                    for (int j = min_sorted[i]->max_rank; j < dc; j++) {
                        times[j] -= p;
                        while ((cursor[j] > last) && (times[j] < buffer[cursor[j]].min))
                            cursor[j]--;
                        if ((cursor[j] > last) && (times[j] <= buffer[cursor[j]].max))
                            times[j] = buffer[cursor[j]--].min - 1;
                        new_min = std::min(new_min, times[j]);
                    }
                    if ((i == 0) || (min_sorted[i - 1]->min < min_sorted[i]->min))
                        min = new_min;
                    if (min < min_sorted[i]->min)
                        return false;
                    if (min - p + 1 < min_sorted[i]->min) {
                        if ((forbidden_count > 0) && (min_sorted[i]->min >= buffer[last + 1].min)) {
                            // Adjacent integer regions must also be merged: skipping one
                            // region must not land on a forbidden start in its neighbour.
                            buffer[last + 1].min = min - p + 1;
                        } else {
                            assert(last >= 0);
                            buffer[last--] = {min - p + 1, min_sorted[i]->min - 1};
                            forbidden_count++;
                        }
                    }
                }
                for (int i = 0; i < forbidden_count; i++)
                    forbidden[i] = buffer[last + 1 + i];
                return true;
            }

            /// Tabulate earliest completions outside the forbidden start regions
            void
            compute_earliest_completions() {
                for (int i = 0; i < n; i++) {
                    ect_table[i] = min_sorted[i]->min;
                    int er = 0;
                    for (int q = 1; q <= n; q++) {
                        Time start = ect(i, q - 1);
                        while ((er < forbidden_count) && (forbidden[er].max < start))
                            er++;
                        if ((er < forbidden_count) && (forbidden[er].min <= start))
                            start = forbidden[er].max + 1;
                        ect_table[static_cast<size_t>(q) * n + i] = start + p;
                    }
                }
            }

            /// Latest starts for one deadline, including the external adjustment
            void
            compute_latest_starts(Time deadline, int task_count) {
                // Keeping only the current deadline's vector avoids a second
                // quadratic table without increasing the sweep's complexity.
                lst_table[0] = deadline;
                int region = forbidden_count - 1;
                for (int t = 1; t <= task_count + 1; t++) {
                    Time start = lst(t - 1) - p;
                    while ((region >= 0) && (forbidden[region].min > start))
                        region--;
                    if ((region >= 0) && (forbidden[region].max >= start))
                        start = forbidden[region].min - 1;
                    lst_table[t] = start;
                }
            }

            /// Initialize the adjustment list and the groups of variable bounds
            void
            initialize_adjustments() {
                const Time infinity = std::numeric_limits<Time>::max();
                regions[0] = {-infinity, -infinity, -1, 1};
                regions[1] = {infinity, infinity, 0, -1};
                region_count = 2;
                for (int i = 0; i < n; i++) {
                    links[i] = i - 1;
                    processed[i] = false;
                    bounds[i] = min_sorted[i]->min;
                    next_region[i] = 0;
                }
            }

            /// Find the predecessor for an adjustment with lower bound min
            RegionIndex
            insertion_point(RegionIndex hint, Time min) const {
                // Intervals are appended in decreasing-minimum runs. Follow previous
                // links between runs, then next links within the ordered list.
                while (((hint + 1 == region_count) ? min : regions[hint + 1].min) >= regions[hint].min) {
                    hint = regions[hint].previous;
                    if (hint < 0)
                        break;
                    if (regions[hint].min <= min) {
                        hint--;
                        break;
                    }
                }
                hint++;
                while (regions[regions[hint].next].min < min)
                    hint = regions[hint].next;
                return hint;
            }

            /// Insert an adjustment after its predecessor and return its index
            RegionIndex
            insert_adjustment(RegionIndex previous, Time min, Time max) {
                RegionIndex next = regions[previous].next;
                assert((regions[previous].min <= min) && (min <= regions[next].min));
                RegionIndex inserted = region_count++;
                regions[inserted] = {min, max, previous, next};
                regions[previous].next = inserted;
                regions[next].previous = inserted;
                return inserted;
            }

            /// Build and apply internal adjustments in deadline order
            void
            apply_internal_adjustments() {
                const Time infinity = std::numeric_limits<Time>::max();
                RegionIndex last_region = 0;
                int min_p = n;
                int max_p = 0;
                for (int it = 0; it < n; it++) {
                    int i = max_sorted[it]->min_rank;
                    processed[i] = true;
                    max_p = std::max(max_p, i);
                    min_p = std::min(min_p, i);

                    // Maintain the dominating blocks before generating their intervals.
                    // These cursors share the sweep's task counts and stay together.
                    int l = leader[i] = i;
                    int kp = 1;
                    while (++l <= max_p)
                        if (processed[l])
                            kp++;
                    l = i;
                    int k = kp;
                    if (i > min_p) {
                        while (!processed[--l]) {
                        }
                        k++;
                        while (l > leader[l]) {
                            while (!processed[--l]) {
                            }
                            k++;
                        }
                        if (ect(i, 1) > ect(l, k - kp + 1)) {
                            l = i;
                            k = kp;
                        }
                    }
                    kp = k;
                    int j = l;
                    while (j < max_p) {
                        while (!processed[++j]) {
                        }
                        if (ect(j, 1) <= ect(l, k - --kp + 1))
                            leader[j] = l;
                        else
                            break;
                    }
                    if ((it < n - 1) && (max_sorted[it]->max == max_sorted[it + 1]->max))
                        continue;

                    compute_latest_starts(max_sorted[it]->max, it + 1);

                    int q = 0;
                    RegionIndex insertion = 0;
                    int vi = n - 1;
                    while (true) {
                        for (int t = q; t < k; t++) {
                            Time min = lst(t + 1) + 1;
                            // The final empty region allows external adjustments to
                            // expand the preceding intervals by one more task.
                            Time max = (l >= 0) ? ect(l, k - t) - 1 : -infinity;
                            if (t > 0) {
                                insertion = insertion_point(insertion, min);
                            } else {
                                insertion = last_region;
                                first_region[i] = last_region = region_count;
                            }
                            vi = notify(insert_adjustment(insertion, min, max), vi);
                        }
                        if (l == -1)
                            break;
                        q = k++;
                        if (l > min_p) {
                            while (!processed[--l]) {
                            }
                            j = leader[l];
                            while (j < l) {
                                while (!processed[--l]) {
                                }
                                k++;
                            }
                        } else {
                            l = -1;
                        }
                    }
                }
            }

            /// Apply external adjustments after reading each task's tightened bound
            void
            apply_external_adjustments() {
                // A variable is filtered before adding external intervals for its
                // deadline, so it is never excluded by an interval it helped create.
                for (int it = 0; it < n; it++) {
                    int i = max_sorted[it]->min_rank;
                    max_sorted[it]->min = bounds[find(i)];
                    if ((it == n - 1) || (max_sorted[it]->max != max_sorted[it + 1]->max)) {
                        int vi = n - 1;
                        Time last_max = regions[first_region[i]].max;
                        for (RegionIndex r = first_region[i] + 1;
                             (r < region_count) && (regions[r].min < regions[r - 1].min); r++) {
                            Time old_max = regions[r].max;
                            regions[r].max = last_max;
                            last_max = old_max;
                            vi = notify(r, vi);
                        }
                    }
                }
            }

        public:
            /// Filter bounds, returning false on interval infeasibility
            bool
            filter(Region& r) {
                sort_tasks();
                if (!compute_forbidden_regions(r))
                    return false;
                compute_earliest_completions();
                initialize_adjustments();
                apply_internal_adjustments();
                apply_external_adjustments();
                return true;
            }
        };

        /// Largest feasible distance in [min,max], or -1 when min is infeasible
        int
        maximum_feasible_distance(Region& region, const ViewArray<IntView>& x, int min, int max) {
            Feasibility feasibility(region, x);
            if (!feasibility.check(min))
                return -1;
            if ((max == min) || feasibility.check(max))
                return max;

            // Quimper et al. (2008), Section 5: feasibility is monotone in p.
            // Keep min feasible and max infeasible without enumerating distances.
            while (max - min > 1) {
                int middle = min + (max - min) / 2;
                if (feasibility.check(middle))
                    min = middle;
                else
                    max = middle;
            }
            return min;
        }

        /// Tighten x bounds in both directions; holes may require another pass
        ExecStatus
        filter_bounds(Space& home, Region& region, ViewArray<IntView>& x, int p) {
            Filter f(region, x.size(), p);
            bool nofix = false;
            for (int direction = 0; direction < 2; direction++) {
                for (int i = 0; i < x.size(); i++) {
                    // Reverse task time: [r,d] becomes [-d,-r]. Variables denote
                    // starting times, while the algorithm's maxima are deadlines.
                    f.tasks[i].min = direction ? -static_cast<Time>(x[i].max()) - p : x[i].min();
                    f.tasks[i].max = direction ? -static_cast<Time>(x[i].min()) : static_cast<Time>(x[i].max()) + p;
                }
                if (!f.filter(region))
                    return ES_FAILED;
                for (int i = 0; i < x.size(); i++) {
                    Time bound = direction ? -f.tasks[i].min - p : f.tasks[i].min;
                    if (direction ? (bound < x[i].min()) : (bound > x[i].max()))
                        return ES_FAILED;
                    ModEvent me =
                        direction ? x[i].lq(home, static_cast<int>(bound)) : x[i].gq(home, static_cast<int>(bound));
                    GECODE_ME_CHECK(me);
                    nofix |= (direction ? x[i].max() : x[i].min()) != bound;
                }
            }
            return nofix ? ES_NOFIX : ES_FIX;
        }

        /// Minimum separation in an assigned tuple, without quadratic tables
        Time
        assigned_distance(Region& r, const ViewArray<IntView>& x) {
            Time* values = r.alloc<Time>(x.size());
            for (int i = 0; i < x.size(); i++)
                values[i] = x[i].val();
            std::sort(values, values + x.size());
            Time gap = std::numeric_limits<Time>::max();
            for (int i = 1; i < x.size(); i++)
                gap = std::min(gap, values[i] - values[i - 1]);
            return gap;
        }
    } // namespace

    template <class PView>
    Bnd<PView>::Bnd(Home home, ViewArray<IntView>& x0, PView p0, bool a)
        : MixNaryOnePropagator<IntView, PC_INT_BND, PView, PC_INT_BND>(home, x0, p0), advanced(a) {}

    template <class PView>
    Bnd<PView>::Bnd(Space& home, Bnd& b)
        : MixNaryOnePropagator<IntView, PC_INT_BND, PView, PC_INT_BND>(home, b), advanced(b.advanced) {}

    template <class PView>
    Propagator*
    Bnd<PView>::copy(Space& home) {
        return new (home) Bnd(home, *this);
    }

    template <class PView>
    PropCost
    Bnd<PView>::cost(const Space&, const ModEventDelta& med) const {
        // ME_INT_DOM is a synthetic event: subscriptions only generate bound
        // or assignment events. It schedules the quadratic stage separately.
        if (IntView::me(med) == ME_INT_DOM)
            return PropCost::quadratic(PropCost::HI, x.size());
        return PropCost::linear(PropCost::HI, x.size());
    }

    template <class PView>
    size_t
    Bnd<PView>::dispose(Space& home) {
        (void)MixNaryOnePropagator<IntView, PC_INT_BND, PView, PC_INT_BND>::dispose(home);
        return sizeof(*this);
    }

    template <class PView>
    ExecStatus
    Bnd<PView>::post(Home home, ViewArray<IntView>& x, PView p, bool advanced) {
        if (x.size() > 1)
            (void)new (home) Bnd(home, x, p, advanced);
        return ES_OK;
    }

    template <class PView>
    ExecStatus
    Bnd<PView>::propagate(Space& home, const ModEventDelta& med) {
        Region region;
        bool assigned = true;
        int min = x[0].min();
        int max = x[0].max();
        for (int i = 0; i < x.size(); i++) {
            min = std::min(min, x[i].min());
            max = std::max(max, x[i].max());
            assigned &= x[i].assigned();
        }

        if (assigned) {
            GECODE_ME_CHECK(y.lq(home, assigned_distance(region, x)));
            return home.ES_SUBSUMED(*this);
        }
        if (y.max() == 0)
            return home.ES_SUBSUMED(*this);
        if (y.assigned() && (y.val() == 1))
            GECODE_REWRITE(*this, Distinct::Bnd<IntView>::post(home(*this), x));

        const int p = y.min();
        if (IntView::me(med) != ME_INT_DOM) {
            Time limit = (static_cast<Time>(max) - min) / (x.size() - 1);
            int upper = static_cast<int>(std::min(limit, static_cast<Time>(y.max())));
            if (upper < p)
                return ES_FAILED;
            upper = maximum_feasible_distance(region, x, p, upper);
            if (upper < 0)
                return ES_FAILED;
            GECODE_ME_CHECK(y.lq(home, upper));
            if (y.max() == 0)
                return home.ES_SUBSUMED(*this);
            if (y.assigned() && (y.val() == 1))
                GECODE_REWRITE(*this, Distinct::Bnd<IntView>::post(home(*this), x));
            if (!advanced || (p == 0))
                return ES_FIX;
            return home.ES_FIX_PARTIAL(*this, IntView::med(ME_INT_DOM));
        }

        // A zero minimum gives every x bound a support; only the basic stage
        // needs to filter the distance. A later increase wakes that stage again.
        if (p == 0)
            return ES_FIX;
        ExecStatus filtered = filter_bounds(home, region, x, p);
        if (filtered == ES_FAILED)
            return ES_FAILED;

        assigned = true;
        for (int i = 0; i < x.size(); i++)
            assigned &= x[i].assigned();
        if (assigned) {
            GECODE_ME_CHECK(y.lq(home, assigned_distance(region, x)));
            return home.ES_SUBSUMED(*this);
        }

        // Holes can move actual bounds beyond the computed bounds, invalidating
        // an interval support for x or the distance. Restart the basic stage.
        return filtered;
    }

    // Export the compiled view specializations for propagator reuse.
    template class Bnd<IntView>;
    template class Bnd<ConstIntView>;

}}} // namespace Gecode::Int::InterDistance

namespace Gecode {

    void
    inter_distance(Home home, const IntVarArgs& x, int p, IntPropLevel ipl) {
        using namespace Int;
        Limits::nonnegative(p, "Int::inter_distance");
        if ((p > 0) && same(x))
            throw ArgumentSame("Int::inter_distance");
        GECODE_POST;
        if ((p == 0) || (x.size() < 2))
            return;
        ViewArray<IntView> xv(home, x);
        if (p == 1) {
            GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home, xv));
        } else {
            GECODE_ES_FAIL(InterDistance::Bnd<ConstIntView>::post(home, xv, ConstIntView(p), ba(ipl) != IPL_BASIC));
        }
    }

    void
    inter_distance(Home home, const IntVarArgs& x, IntVar p, IntPropLevel ipl) {
        using namespace Int;
        if ((x.size() > 1) && (same(x, p) || ((p.max() > 0) && same(x))))
            throw ArgumentSame("Int::inter_distance");
        GECODE_POST;
        IntView pv(p);
        GECODE_ME_FAIL(pv.gq(home, 0));
        if ((pv.max() == 0) || (x.size() < 2))
            return;
        if (pv.assigned()) {
            inter_distance(home, x, pv.val(), ipl);
            return;
        }
        ViewArray<IntView> xv(home, x);
        GECODE_ES_FAIL(InterDistance::Bnd<IntView>::post(home, xv, pv, ba(ipl) != IPL_BASIC));
    }

} // namespace Gecode

// STATISTICS: int-post
