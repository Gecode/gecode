#!/usr/bin/env python3
"""Check canonical TSPTW tours using exact rational arithmetic, without Gecode."""
import argparse
from fractions import Fraction
import itertools
import json
from pathlib import Path
import re
import sys

INTEGER_MAX = 2147483646  # Gecode::Int::Limits::max


def read_instance(path):
    tokens = " ".join(line.split("#", 1)[0] for line in
                      Path(path).read_text().splitlines()).split()
    if not tokens or not re.fullmatch(r"[0-9]+", tokens[0]):
        raise ValueError("node count must be an integer")
    n = int(tokens[0])
    if n < 2 or n*n > INTEGER_MAX or len(tokens) != 1+n*n+2*n:
        raise ValueError("invalid TSPTW dimensions")
    if any(not re.fullmatch(r"(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)", x)
           for x in tokens[1:]):
        raise ValueError("invalid nonnegative decimal")
    values = [Fraction(x) for x in tokens[1:]]
    # Derive the scale from decimal text and keep values as exact fractions.
    # This checks the native reader without repeating its digit accumulation.
    # Trailing fractional zeroes do not require a larger scale.
    places = max(len(x.rstrip("0").split(".")[1]) if "." in x else 0
                 for x in tokens[1:])
    scale = 10**places
    if scale > INTEGER_MAX or any(x*scale > INTEGER_MAX for x in values):
        raise ValueError("scaled value exceeds Gecode integer limits")
    matrix = [values[i*n:(i+1)*n] for i in range(n)]
    windows = [tuple(values[n*n+2*i:n*n+2*i+2]) for i in range(n)]
    if any(ready > due for ready, due in windows):
        raise ValueError("window ready time exceeds due time")
    if windows[0][0] != 0:
        raise ValueError("nonzero start-depot ready time is unsupported")
    cost_limit = n*max(values[:n*n])
    time_limit = cost_limit+max(due for _, due in windows)
    if time_limit*scale > INTEGER_MAX:
        raise ValueError("tour arithmetic exceeds Gecode integer limits")
    return matrix, windows, scale


def exact_text(value):
    """Render a finite decimal Fraction without rounding."""
    denominator = value.denominator
    places = 0
    while (10**places) % denominator:
        places += 1
    integer = value.numerator * (10**places // denominator)
    if not places:
        return str(integer)
    text = str(integer).zfill(places+1)
    return (text[:-places]+"."+text[-places:]).rstrip("0").rstrip(".")


def check_tour(data, tour, claimed_cost=None):
    # Starting at zero, each visit takes the earliest time allowed by travel
    # and its ready time. Waiting adds time but no cost; include the depot return.
    matrix, windows, scale = data
    n = len(matrix)
    if len(tour) != n-1:
        raise ValueError("tour omits customers or has extra visits")
    if set(tour) != set(range(1, n)):
        raise ValueError("tour has a duplicate or invalid customer")
    time = Fraction(0)
    cost = Fraction(0)
    previous = 0
    visits = []
    for node in [*tour, 0]:
        arrival = time+matrix[previous][node]
        time = max(arrival, windows[node][0])
        cost += matrix[previous][node]
        if time > windows[node][1]:
            raise ValueError("late return" if node == 0 else f"late customer {node}")
        visits.append({"node": node, "arrival": exact_text(arrival),
                       "waiting": exact_text(time-arrival), "time": exact_text(time)})
        previous = node
    if claimed_cost is not None and cost != Fraction(claimed_cost):
        raise ValueError(f"incorrect cost: recomputed {exact_text(cost)}")
    return {"cost": exact_text(cost), "scaled_cost": int(cost*scale),
            "return_time": exact_text(time), "scale": scale, "visits": visits}


def enumerate_tours(data):
    n = len(data[0])
    if n > 10:
        raise ValueError("tiny enumeration is limited to nine customers")
    best = None
    feasible = 0
    for tour in itertools.permutations(range(1, n)):
        try:
            result = check_tour(data, tour)
        except ValueError:
            continue
        feasible += 1
        if best is None or result["scaled_cost"] < best["scaled_cost"]:
            best = {**result, "tour": list(tour)}
    return {"feasible_tours": feasible, "optimum": best, "proof": "exhaustive enumeration"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    check = commands.add_parser("check")
    check.add_argument("instance", type=Path)
    check.add_argument("tour", type=Path, help="whitespace customer IDs; omit both depot visits")
    check.add_argument("--cost", help="exact original-unit arc cost, including return")
    enumerate_command = commands.add_parser("enumerate")
    enumerate_command.add_argument("instance", type=Path)
    args = parser.parse_args()
    try:
        data = read_instance(args.instance)
        if args.command == "check":
            tour = [int(x) for x in args.tour.read_text().split()]
            result = check_tour(data, tour, args.cost)
        else:
            result = enumerate_tours(data)
        print(json.dumps(result))
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
