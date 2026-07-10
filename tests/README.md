# GTUltra Testing Notes

## CI smoke tests

The GitHub Actions workflows run a shared smoke-test script after each build:

- `.github/workflows/build-linux.yml`
- `.github/workflows/build-macos.yml`
- `.github/workflows/build-windows.yml`
- `tests/integration/smoke.sh`

Categories can be selectively disabled with environment variables:

```bash
SKIP_GT2RELOC=1 SKIP_GTULTRA=1 bash tests/integration/smoke.sh mac/ ""
```

Supported toggles:

- `SKIP_BINARIES=1`
- `SKIP_USAGE=1`
- `SKIP_GTULTRA=1`
- `SKIP_GT2RELOC=1`

The script currently covers:

1. **Binary existence checks** — verify that all five expected binaries are produced:
   `gtultra`, `gt2reloc`, `ins2snd2`, `mod2sng2`, `ss2stereo`.
2. **Usage smoke tests** — invoke `gt2reloc`, `ins2snd2` and `mod2sng2` without arguments
   and confirm they print usage and exit with the expected non-zero status.
3. **gtultra startup test** — launch `gtultra -?` with a short timeout to verify it
   starts without crashing (it is an interactive SDL application, so running it
   headlessly may fail display init or hang).
4. **Functional test** — run `gt2reloc` on a fixture `.sng` to pack/convert it into a
   non-empty `.prg`.

## Known tool limitations

### `ss2stereo`

`ss2stereo` is the original GoatTracker v2.76 stereo pattern splitter. It takes a
song and splits long patterns into smaller chunks (default 16 rows), deduplicating
identical patterns to reduce memory usage.

It is **not covered by the functional CI test** because it has a hardcoded limit
(`MAX_SONGLEN` / orderlist length ≤ 254) that is exceeded by modern GTUltra songs.
Running it on current example songs produces:

```text
ERROR: Orderlist-length of 254 exceeded!
```

This is a legacy-tool limitation rather than a bug in GTUltra itself. It should be
fixed in `ss2stereo` before being re-added to the functional smoke tests.

### `gt2reloc`

`gt2reloc` is the command-line packer/relocator. It loads a `.sng` and packs it
into a relocatable binary (`.prg`, `.sid`, or `.bin`). The `.sng` itself is not
relocated; the relocation is applied to the player and packed song data.

It is included in the functional smoke test because it is the most important
non-interactive workload.

Current status: on the tested fixture song it segfaults during packing
(`EXIT: 139` on macOS). This needs to be investigated and fixed; the CI test is
kept blocking so the regression is visible.
