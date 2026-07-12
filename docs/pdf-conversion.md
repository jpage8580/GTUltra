# PDF to Markdown conversion handoff

## Source

- `docs/archive/GTUltra.pdf` — the original 23-page manual PDF.
- `docs/archive/readme.txt`, `readme_gtultra.txt`, `readme_resid.txt`, `readme_sdl.txt`, `RtMidi-README.md` and `readme - OriginalGT2 Documentation.txt` were moved here from the repo root to keep legacy docs alongside the PDF.

## Conversion method

The extraction was a one-off Python script using PyMuPDF (`fitz`). It produced:

- `docs/manual/index.md` (combined pages 1–3, title and TOC, later renamed to `README.md`).
- `docs/manual/page-03.md` to `docs/manual/page-23.md` (one Markdown per PDF content page).
- `docs/images/manual/page-<NN>-img-<N>.png` (images extracted per page, sorted by vertical position).

## Post-processing completed

The following cleanup was applied on top of the raw extraction:

1. Added proper Markdown headings to `page-03.md` – `page-23.md`.
2. Fixed nested list indentation for `a.`, `b.`, `i.`, `ii.`, etc.
3. Cleaned extraction artifacts (merged blocks, empty marker lines, split sentences).
4. Restructured the manual into 63 human-readable section files (e.g. `what-is-it.md`, `transport-bar.md`, `zeropage-sid-playback.md`). Pages that split a section were joined into the correct section file.
5. Renamed `docs/manual/index.md` to `docs/manual/README.md` so GitHub renders it.
6. Added `<<< | index | >>>` previous/index/next navigation footers to each section file.
7. Renamed images to `{section-name}-img-{N}.png` where possible; the section files reference the renamed images. The `page-*.md` intermediate files still reference `page-<NN>-img-<N>.png`.

## Current file structure

```
docs/
├── archive/
│   ├── GTUltra.pdf
│   ├── readme.txt
│   ├── readme_gtultra.txt
│   ├── readme_resid.txt
│   ├── readme_sdl.txt
│   ├── RtMidi-README.md
│   └── readme - OriginalGT2 Documentation.txt
├── images/manual/
│   └── *.png (103 images, both page-NN and section-name names)
├── manual/
│   ├── README.md            # GitHub-rendered index / table of contents
│   ├── WORK.md              # Work tracking
│   ├── <section-name>.md    # 63 final section files
│   └── page-03.md .. page-23.md  # intermediate source-page files
└── pdf-conversion.md        # this file
```

## Notes

- `README.md` includes the `What is it?`, `What’s new` and `Credits` links plus the numbered feature TOC.
- The `page-*.md` intermediate files are preserved as the raw extraction output. They are not linked from `README.md` and can be removed if no longer needed.
- All 103 images under `docs/images/manual/` are referenced by at least one Markdown file; there are no broken or orphan images.
- Image alt text remains generic (`![image](...)`). A task to improve alt text was removed from tracking.
