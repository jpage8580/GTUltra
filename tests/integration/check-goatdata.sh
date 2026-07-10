#!/bin/bash
# Verify the generated src/goatdata.c matches the committed reference hash.
#
# The build (make {mac,linux,win}-build -> build-tools) regenerates goatdata.c
# from the packed sources on every run. This check catches two failure modes:
#   (a) non-deterministic / platform-divergent generation, and
#   (b) staleness -- player sources or packed resources changed without the
#       reference hash being updated.
#
# Run from the repository root.
set -uo pipefail

GEN='src/goatdata.c'
REF='tests/goatdata.sha256'

if [ ! -f "$GEN" ]; then
  echo "FAIL: $GEN not found (did the build regenerate it?)" >&2
  exit 1
fi
if [ ! -f "$REF" ]; then
  echo "FAIL: reference $REF not found" >&2
  exit 1
fi

# Portable sha256: linux/msys2 have sha256sum, macOS has shasum.
if command -v sha256sum >/dev/null 2>&1; then
  actual=$(sha256sum "$GEN" | awk '{print $1}')
elif command -v shasum >/dev/null 2>&1; then
  actual=$(shasum -a 256 "$GEN" | awk '{print $1}')
else
  echo "FAIL: no sha256 tool (sha256sum/shasum) available" >&2
  exit 1
fi

expected=$(tr -d '[:space:]' < "$REF")

if [ "$actual" = "$expected" ]; then
  echo "PASS: goatdata.c matches reference ($actual)"
  exit 0
fi

echo "FAIL: goatdata.c hash mismatch" >&2
echo "  expected: $expected" >&2
echo "  actual:   $actual" >&2
echo >&2
echo "If you intentionally changed player sources or packed resources, regenerate" >&2
echo "and update the reference:" >&2
echo "    make build-tools" >&2
echo "    shasum -a 256 src/goatdata.c | awk '{print \$1}' > tests/goatdata.sha256" >&2
echo >&2
echo "Otherwise this indicates non-deterministic or platform-divergent generation" >&2
echo "(see tests/docs/build-determinism.md)." >&2
exit 1
