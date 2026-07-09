#!/bin/bash
# Shared smoke tests for GTUltra builds.
# Usage: smoke-test.sh <binary-prefix> <binary-suffix>
#   prefix: e.g. "linux/", "mac/", "win32/"
#   suffix: e.g. "", ".exe"
set -uo pipefail

PREFIX="${1:-}"
SUFFIX="${2:-}"
EXAMPLE='examples/Linus/$3Space_Beer.sng'

failures=0
passes=0

report() {
  if [ "$1" -eq 0 ]; then
    passes=$((passes + 1))
    echo "PASS: $2"
  else
    failures=$((failures + 1))
    echo "FAIL: $2" >&2
  fi
}

GTULTRA="${PREFIX}gtultra${SUFFIX}"
GT2R="${PREFIX}gt2reloc${SUFFIX}"
SS2="${PREFIX}ss2stereo${SUFFIX}"
INS2="${PREFIX}ins2snd2${SUFFIX}"
MOD2="${PREFIX}mod2sng2${SUFFIX}"

echo "=== Verifying built binaries exist ==="
for bin in "$GTULTRA" "$GT2R" "$SS2" "$INS2" "$MOD2"; do
  if [ -s "$bin" ]; then
    report 0 "binary exists: $bin"
  else
    report 1 "binary missing or empty: $bin"
  fi
done

echo "=== Usage smoke tests (no-args should print usage and exit non-zero) ==="

set +e
"$GT2R"
gt2r_ec=$?
set -e
if [ "$gt2r_ec" -ne 0 ]; then
  report 0 "gt2reloc usage exits non-zero (got $gt2r_ec)"
else
  report 1 "gt2reloc usage exits zero"
fi

set +e
"$INS2"
ins2_ec=$?
set -e
if [ "$ins2_ec" -eq 1 ]; then
  report 0 "ins2snd2 usage exit code 1"
else
  report 1 "ins2snd2 usage unexpected exit code $ins2_ec"
fi

set +e
"$MOD2"
mod2_ec=$?
set -e
if [ "$mod2_ec" -eq 1 ]; then
  report 0 "mod2sng2 usage exit code 1"
else
  report 1 "mod2sng2 usage unexpected exit code $mod2_ec"
fi

echo "=== Functional tests on example song ==="
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

set +e
"$GT2R" "$EXAMPLE" "${TMPDIR}/out.prg"
gt2r_func_ec=$?
set -e
if [ "$gt2r_func_ec" -eq 0 ] && [ -s "${TMPDIR}/out.prg" ]; then
  report 0 "gt2reloc produces non-empty PRG"
else
  report 1 "gt2reloc failed (exit $gt2r_func_ec) or produced empty PRG"
fi

set +e
"$SS2" "$EXAMPLE" "${TMPDIR}/split.sng"
ss2_func_ec=$?
set -e
if [ "$ss2_func_ec" -eq 0 ] && [ -s "${TMPDIR}/split.sng" ]; then
  report 0 "ss2stereo produces non-empty SNG"
else
  report 1 "ss2stereo failed (exit $ss2_func_ec) or produced empty SNG"
fi

echo "=== Summary: $passes passed, $failures failed ==="

if [ "$failures" -ne 0 ]; then
  exit 1
fi

exit 0
