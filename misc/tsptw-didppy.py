#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["didppy==0.11.1"]
# ///
"""Exact travel-cost TSPTW reference; single-threaded DIDPPy CAASDy (A*) / CABS."""
import argparse
import importlib.metadata
import importlib.util
import json
from pathlib import Path
import time

START = time.perf_counter()
import didppy as dp

spec = importlib.util.spec_from_file_location('checker', Path(__file__).with_name('tsptw-check.py'))
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('instance', type=Path)
    parser.add_argument('--engine', choices=['astar', 'cabs'], required=True)
    parser.add_argument('--time-ms', type=int, required=True)
    args = parser.parse_args()
    data = checker.read_instance(args.instance)
    matrix, windows, scale = data
    arcs = [[int(v*scale) for v in row] for row in matrix]
    ready = [int(a*scale) for a, _ in windows]
    due = [int(b*scale) for _, b in windows]
    n = len(arcs)
    model = dp.Model()
    customers = model.add_object_type(number=n)
    unvisited = model.add_set_var(object_type=customers, target=list(range(1, n)))
    location = model.add_element_var(object_type=customers, target=0)
    clock = model.add_int_resource_var(target=0, less_is_better=True)
    travel = model.add_int_table(arcs)
    for j in range(1, n):
        arrival = dp.max(clock + travel[location, j], ready[j])
        model.add_transition(dp.Transition(name=str(j),
            cost=travel[location, j] + dp.IntExpr.state_cost(),
            preconditions=[unvisited.contains(j), arrival <= due[j]],
            effects=[(unvisited, unvisited.remove(j)), (location, j), (clock, arrival)]))
    model.add_transition(dp.Transition(name='return',
        cost=travel[location, 0] + dp.IntExpr.state_cost(),
        preconditions=[unvisited.is_empty(), location != 0, clock + travel[location, 0] <= due[0]],
        effects=[(location, 0), (clock, clock + travel[location, 0])]))
    model.add_base_case([unvisited.is_empty(), location == 0])
    # The notebook's direct-arc residual deadline constraint is unsound for
    # nonmetric input. Only actual visits and the explicit return check time.
    incoming = model.add_int_table([min(arcs[i][j] for i in range(n) if i != j) for j in range(n)])
    outgoing = model.add_int_table([min(arcs[i][j] for j in range(n) if i != j) for i in range(n)])
    # Every unfinished tour needs one departure from its current location and
    # all remaining customers, plus one arrival at each customer and the depot.
    unfinished = ~(unvisited.is_empty() & (location == 0))
    model.add_dual_bound(unfinished.if_then_else(incoming[unvisited] + incoming[0], 0))
    model.add_dual_bound(unfinished.if_then_else(outgoing[unvisited] + outgoing[location], 0))
    preprocess_ms = (time.perf_counter()-START)*1000
    solver_type = dp.CAASDy if args.engine == 'astar' else dp.CABS
    search_start = time.perf_counter()
    solver = solver_type(model, quiet=True, time_limit=max(0.000001, args.time_ms/1000-preprocess_ms/1000))
    incumbent = None
    while True:
        solution, terminated = solver.search_next()
        if solution.cost is not None and (incumbent is None or solution.cost < incumbent['scaled_cost']):
            tour = [int(t.name) for t in solution.transitions if t.name != 'return']
            value = checker.check_tour(data, tour)
            if value['scaled_cost'] != solution.cost:
                raise ValueError('DIDPPy objective differs from checked arc sum')
            incumbent = {k: value[k] for k in ('cost', 'scaled_cost', 'scale', 'return_time')}
            incumbent['tour'] = tour
            print(json.dumps(dict(type='solution', elapsed_ms=(time.perf_counter()-START)*1000, **incumbent)), flush=True)
        if terminated:
            break
    termination = 'optimal' if solution.is_optimal else 'infeasible' if solution.is_infeasible else 'stopped'
    print(json.dumps(dict(type='summary', engine=args.engine, formulation='didppy',
        version=importlib.metadata.version('didppy'), termination=termination, incumbent=incumbent,
        elapsed_ms=(time.perf_counter()-START)*1000, preprocessing_ms=preprocess_ms,
        search_ms=(time.perf_counter()-search_start)*1000,
        statistics=dict(node=solution.expanded, generated=solution.generated))), flush=True)


if __name__ == '__main__':
    main()
