#!/usr/bin/env bash
set -euo pipefail

SIM=./Simulator/build/simulator
ALG_DIR=tests/Algorithms
GM_DIR=tests/GameManagers
MAP_DIR=tests/maps

# Environment for dynamic libs (pick one per OS)
# macOS:
export DYLD_LIBRARY_PATH="$ALG_DIR:$GM_DIR:${DYLD_LIBRARY_PATH:-}"
# Linux:
export LD_LIBRARY_PATH="$ALG_DIR:$GM_DIR:${LD_LIBRARY_PATH:-}"

echo "== Sanity: list inputs =="
echo "Algorithms:"; ls -1 "$ALG_DIR"/*.so
echo "GameManagers:"; ls -1 "$GM_DIR"/*.so
echo "Maps:"; ls -1 "$MAP_DIR"/*

# ---- Comparative on each map with the first two algorithms ----
ALGOS=($(ls -1 "$ALG_DIR"/*.so))
if [ ${#ALGOS[@]} -lt 2 ]; then
  echo "Need at least 2 algorithms in $ALG_DIR" >&2
  exit 1
fi
ALG1="${ALGOS[0]}"
ALG2="${ALGOS[1]}"

echo
echo "== Comparative mode across all maps with GM folder =="
for MAP in "$MAP_DIR"/*; do
  echo "-- comparative on $(basename "$MAP") --"
  "$SIM" -comparative \
    game_map="$MAP" \
    game_managers_folder="$GM_DIR" \
    algorithm1="$ALG1" \
    algorithm2="$ALG2" \
    num_threads=4 -verbose
done

# ---- Competition for each GM across all maps and all algos ----
echo
echo "== Competition mode for each GameManager =="
for GM in "$GM_DIR"/*.so; do
  echo "-- competition using $(basename "$GM") --"
  "$SIM" -competition \
    game_maps_folder="$MAP_DIR" \
    game_manager="$GM" \
    algorithms_folder="$ALG_DIR" \
    num_threads=6 -verbose
done

echo
echo "Done. Outputs should be in:"
echo "  Comparative: $GM_DIR/comparative_results_<time>.txt"
echo "  Competition: $ALG_DIR/competition_<time>.txt"
