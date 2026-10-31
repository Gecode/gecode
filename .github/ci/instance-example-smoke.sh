#!/usr/bin/env bash
# Run the examples that read instance files on tiny files, so that their
# embedded reader specs are parsed and used.
set -euo pipefail

bin_dir="${1:?usage: instance-example-smoke.sh <bin-dir>}"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

expect() {
  local pattern="$1"
  shift
  local out
  out="$("$@" 2>&1)" || { echo "$out"; echo "failed: $*"; exit 1; }
  if ! grep -q -- "$pattern" <<<"$out"; then
    echo "$out"
    echo "missing '$pattern' in output of: $*"
    exit 1
  fi
}

# Like expect, but the command must fail
expect_error() {
  local pattern="$1"
  shift
  local out
  if out="$("$@" 2>&1)"; then
    echo "$out"
    echo "unexpected success: $*"
    exit 1
  fi
  if ! grep -q -- "$pattern" <<<"$out"; then
    echo "$out"
    echo "missing '$pattern' in output of: $*"
    exit 1
  fi
}

cat > "$work_dir/sat.cnf" <<'EOF'
c x1 or x2, not x1
p cnf 2 2
1 2 0
-1 0
EOF
expect "solutions: *1" "$bin_dir/sat" "$work_dir/sat.cnf"

cat > "$work_dir/tsp.tsp" <<'EOF'
NAME : square
TYPE : TSP
DIMENSION : 4
EDGE_WEIGHT_TYPE : EUC_2D
NODE_COORD_SECTION
1 0 0
2 0 10
3 10 10
4 10 0
EOF
expect "Cost: 40" "$bin_dir/tsp" "$work_dir/tsp.tsp"

cat > "$work_dir/tsptw.txt" <<'EOF'
3
0 1 2
1 0 1
2 1 0
0 100
0 100
0 100
EOF
expect "Travel time: 4" "$bin_dir/tsptw" "$work_dir/tsptw.txt"

# The spec checks the time windows
cat > "$work_dir/tsptw-window.txt" <<'EOF'
2
0 1
1 0
0 10
5 4
EOF
expect_error "the time window of a node is invalid" "$bin_dir/tsptw" "$work_dir/tsptw-window.txt"

cat > "$work_dir/job-shop.txt" <<'EOF'
# two jobs on two machines
2 2
0 3 1 2
1 2 0 1
EOF
expect "makespan: 5" "$bin_dir/job-shop" -probes 0 "$work_dir/job-shop.txt"

# The spec checks that the machines exist
cat > "$work_dir/job-shop-machine.txt" <<'EOF'
2 2
0 3 2 2
1 2 0 1
EOF
expect_error "expected an int in 0..1, found '2'" "$bin_dir/job-shop" -probes 0 "$work_dir/job-shop-machine.txt"

cat > "$work_dir/sudoku.txt" <<'EOF'
# size=4 box=2x2
1...
..1.
.1..
...1
EOF
expect "solutions: *1" "$bin_dir/sudoku" "$work_dir/sudoku.txt"

cat > "$work_dir/nonogram.non" <<'EOF'
catalogue "smoke"
width 2
height 2
rows
2
1
columns
2
1
EOF
expect "solutions: *1" "$bin_dir/nonogram" "$work_dir/nonogram.non"
