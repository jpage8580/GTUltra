#!/bin/bash
# Golden-WAV capture/check for the SID engine migration.
#
# Usage (run from the repository root):
#   tests/integration/golden.sh capture [options]   # render corpus -> write manifest
#   tests/integration/golden.sh check   [options]   # render corpus -> diff vs manifest
#
# Options:
#   --bin PATH        sng2wav binary        (default: auto-detect linux/ or mac/)
#   --corpus FILE     corpus list           (default: tests/golden/corpus.txt)
#   --manifest FILE   sha256 manifest       (default: tests/golden/manifest.sha256)
#
# WHY this is not in the 3-platform CI: reSID float output is byte-identical only for a
# fixed build+arch, so the manifest is per-environment. This is the M1 bit-identical
# gate -- `capture` on the pre-refactor build, `check` on the post-refactor build, same
# machine. The manifest is intentionally gitignored.
set -uo pipefail

MODE="${1:-}"; shift || true
BIN=""
CORPUS="tests/golden/corpus.txt"
MANIFEST="tests/golden/manifest.sha256"

while [ $# -gt 0 ]; do
  case "$1" in
    --bin)      BIN="$2"; shift 2 ;;
    --corpus)   CORPUS="$2"; shift 2 ;;
    --manifest) MANIFEST="$2"; shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

if [ "$MODE" != "capture" ] && [ "$MODE" != "check" ]; then
  echo "usage: golden.sh <capture|check> [--bin P] [--corpus F] [--manifest F]" >&2
  exit 2
fi

# Auto-detect the sng2wav binary if not given.
if [ -z "$BIN" ]; then
  for cand in linux/sng2wav mac/sng2wav win32/sng2wav.exe; do
    [ -x "$cand" ] && BIN="$cand" && break
  done
fi
if [ -z "$BIN" ] || [ ! -x "$BIN" ]; then
  echo "FAIL: sng2wav binary not found (build first, or pass --bin)" >&2
  exit 1
fi
if [ ! -f "$CORPUS" ]; then
  echo "FAIL: corpus $CORPUS not found" >&2
  exit 1
fi

sha256() {
  if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
  else shasum -a 256 "$1" | awk '{print $1}'; fi
}

# Deterministic, headless: no window/audio device.
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
# The suite frees its own allocations; the app globals do not, so silence LSan here.
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

TMPDIR_G="$(mktemp -d)"
trap 'rm -rf "$TMPDIR_G"' EXIT

declare -A CUR   # name -> hash for this run
order=()

while IFS= read -r line || [ -n "$line" ]; do
  line="${line%%#*}"                       # strip comments
  line="$(echo "$line" | sed 's/[[:space:]]*$//')"
  [ -z "$line" ] && continue
  name="$(echo "$line"  | cut -d'|' -f1 | xargs)"
  song="$(echo "$line"  | cut -d'|' -f2 | xargs)"
  args="$(echo "$line"  | cut -d'|' -f3-)"
  if [ -z "$name" ] || [ -z "$song" ]; then
    echo "FAIL: malformed corpus line: $line" >&2; exit 1
  fi
  wav="$TMPDIR_G/$name.wav"
  # shellcheck disable=SC2086
  if ! "$BIN" "$song" "$wav" $args >/dev/null 2>&1; then
    echo "FAIL: render failed: $name ($song $args)" >&2; exit 1
  fi
  CUR["$name"]="$(sha256 "$wav")"
  order+=("$name")
done < "$CORPUS"

if [ "$MODE" = "capture" ]; then
  mkdir -p "$(dirname "$MANIFEST")"
  : > "$MANIFEST"
  for name in "${order[@]}"; do
    printf '%s  %s\n' "${CUR[$name]}" "$name" >> "$MANIFEST"
  done
  echo "captured ${#order[@]} golden hashes -> $MANIFEST ($BIN)"
  cat "$MANIFEST"
  exit 0
fi

# check
if [ ! -f "$MANIFEST" ]; then
  echo "FAIL: manifest $MANIFEST not found -- run 'golden.sh capture' first" >&2
  exit 1
fi
fail=0; pass=0
while read -r hash name; do
  [ -z "${name:-}" ] && continue
  if [ -z "${CUR[$name]+x}" ]; then
    echo "WARN: manifest entry '$name' not in corpus (skipped)" >&2; continue
  fi
  if [ "${CUR[$name]}" = "$hash" ]; then
    pass=$((pass+1)); echo "PASS: $name ($hash)"
  else
    fail=$((fail+1)); echo "FAIL: $name" >&2
    echo "  expected: $hash" >&2
    echo "  actual:   ${CUR[$name]}" >&2
  fi
done < "$MANIFEST"

echo "golden check: $pass passed, $fail failed"
[ "$fail" -eq 0 ] || exit 1
