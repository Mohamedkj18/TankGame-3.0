#!/usr/bin/env bash
# perf_matrix.sh — robust perf sweep for Assignment 3
# Generates: perf_results.csv with stable schema.
# Works in Linux/WSL/macOS (Bash). Use Git Bash/WSL on Windows.

set -euo pipefail

########## CONFIG ##########
SIM="${SIM:-./Simulator/build/simulator}"   # override with SIM=... ./perf_matrix.sh
ALG_DIR="${ALG_DIR:-tests/Algorithms}"
GM_DIR="${GM_DIR:-tests/GameManagers}"
MAP_DIR="${MAP_DIR:-tests/maps}"
THREADS="${THREADS:-1 2 4 8}"                    # override with THREADS="1 4 8"
OUT="${OUT:-perf_results.csv}"
################################

# Make globs with no matches expand to empty array instead of literal pattern
shopt -s nullglob

# Help the simulator dlopen libs
export LD_LIBRARY_PATH="${ALG_DIR}:${GM_DIR}:${LD_LIBRARY_PATH:-}"
export DYLD_LIBRARY_PATH="${ALG_DIR}:${GM_DIR}:${DYLD_LIBRARY_PATH:-}"

# GNU time detection (optional; we can still run without it)
TIME_BIN=""
if command -v gtime >/dev/null 2>&1; then
  TIME_BIN="gtime"
elif command -v /usr/bin/time >/dev/null 2>&1; then
  TIME_BIN="/usr/bin/time"
fi

# Milliseconds timestamp (portable enough on Linux/WSL; mac may lack %3N without coreutils)
now_ms() { date +%s%3N 2>/dev/null || echo $(($(date +%s)*1000)); }
now_iso() { date -u +"%Y-%m-%dT%H:%M:%SZ"; }

# Run a command, measure wall (ms), try RSS via GNU time.
measure_run() {
  # Usage: measure_run <cmd ...>
  local start end wall rss="" rc=0 so="" se=""
  local start_iso end_iso
  start=$(now_ms); start_iso=$(now_iso)

  # temp files
  local _so _se _rss; _so=$(mktemp) _se=$(mktemp) _rss=$(mktemp)

  if [[ -n "$TIME_BIN" ]]; then
    # Capture only time's stderr as RSS (KB), simulator stdout/stderr to files
    if ! { $TIME_BIN -f "%M" "$@" 1>"${_so}" 2>"${_rss}"; }; then
      rc=$?
      # if it failed, stderr might be in _rss (or empty); also capture simulator stderr when time not wrapping it
    fi
    rss=$(tr -d '\r\n' < "${_rss}" || true)
  else
    # No GNU time: just run and time it
    if ! "$@" 1>"${_so}" 2>"${_se}"; then
      rc=$?
    fi
    rss="" # unknown
  fi

  # If TIME_BIN used, simulator stderr went to terminal; grab none.
  # To keep stderr_len meaningful, try to recover something:
  if [[ -s "${_se}" ]]; then
    se=$(cat "${_se}")
  fi
  # Also read stdout (for length only)
  if [[ -s "${_so}" ]]; then
    so=$(cat "${_so}")
  fi

  end=$(now_ms); end_iso=$(now_iso); wall=$((end - start))

  # lengths only (avoid polluting CSV with newlines)
  local so_len se_len
  so_len=${#so}
  se_len=${#se}

  # Cleanup
  rm -f "${_so}" "${_se}" "${_rss}"

  # Print a semicolon-pack (caller parses)
  printf '%s;%s;%s;%s;%s;%s' "$start_iso" "$end_iso" "$wall" "$rc" "${rss}" "${so_len},${se_len}"
}

# CSV header (stable)
echo "mode,threads,gm_name,gm_path,map_name,map_path,alg1_name,alg1_path,alg2_name,alg2_path,start_utc,end_utc,wall_ms,exit_code,max_rss_kb,stdout_len,stderr_len" > "${OUT}"

# Collect inputs
algos=( "${ALG_DIR}"/*.so )
gms=( "${GM_DIR}"/*.so )
maps=( "${MAP_DIR}"/* )

# Sanity checks (non-fatal, but warn)
[[ ${#gms[@]} -eq 0 ]]   && echo "WARN: no game managers under ${GM_DIR}" >&2
[[ ${#maps[@]} -eq 0 ]]  && echo "WARN: no maps under ${MAP_DIR}" >&2
[[ ${#algos[@]} -eq 0 ]] && echo "WARN: no algorithms under ${ALG_DIR}" >&2

echo "== Inputs =="
echo "SIM: $SIM"
echo "GMs: ${#gms[@]}  Maps: ${#maps[@]}  Algos: ${#algos[@]}"
echo "Threads: ${THREADS}"
echo

# ---------- COMPARATIVE ----------
for T in ${THREADS}; do
  for gm in "${gms[@]}"; do
    gm_name=$(basename "$gm")
    for map in "${maps[@]}"; do
      map_name=$(basename "$map")
      for ((i=0; i<${#algos[@]}; ++i)); do
        for ((j=0; j<${#algos[@]}; ++j)); do
          # Skip self-pair (can crash some stacks; spec allows, but we avoid for perf matrix)
          [[ $i -eq $j ]] && continue
          a1="${algos[$i]}" ; a2="${algos[$j]}"
          a1_name=$(basename "$a1"); a2_name=$(basename "$a2")

          echo "[comparative] T=$T GM=$gm_name MAP=$map_name A1=$a1_name A2=$a2_name"
          RES=$(measure_run "$SIM" -comparative \
                 "game_map=$map" \
                 "game_managers_folder=$GM_DIR" \
                 "algorithm1=$a1" \
                 "algorithm2=$a2" \
                 "num_threads=$T" \
                 -verbose)

          # Unpack measure_run: start_iso;end_iso;wall_ms;rc;rss;SOlen,SElen
          IFS=';' read -r start_iso end_iso wall_ms rc rss lens <<< "$RES"
          so_len="${lens%,*}"; se_len="${lens#*,}"

          echo "comparative,$T,$gm_name,$gm,$map_name,$map,$a1_name,$a1,$a2_name,$a2,$start_iso,$end_iso,$wall_ms,$rc,$rss,$so_len,$se_len" >> "${OUT}"
        done
      done
    done
  done
done

# ---------- COMPETITION ----------
for T in ${THREADS}; do
  for gm in "${gms[@]}"; do
    gm_name=$(basename "$gm")
    echo "[competition] T=$T GM=$gm_name (maps=${#maps[@]}, algos=${#algos[@]})"
    RES=$(measure_run "$SIM" -competition \
           "game_maps_folder=$MAP_DIR" \
           "game_manager=$gm" \
           "algorithms_folder=$ALG_DIR" \
           "num_threads=$T" \
           -verbose)
    IFS=';' read -r start_iso end_iso wall_ms rc rss lens <<< "$RES"
    so_len="${lens%,*}"; se_len="${lens#*,}"

    # In competition rows we record the alg/map folder context; leave alg2 empty
    echo "competition,$T,$gm_name,$gm,ALL,$MAP_DIR,ALL,$ALG_DIR,,,$start_iso,$end_iso,$wall_ms,$rc,$rss,$so_len,$se_len" >> "${OUT}"
  done
done

echo
echo "Done. Wrote ${OUT}"
