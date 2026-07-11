# PDF to Markdown conversion handoff

## Plan

- Decompose `docs/archive/GTUltra.pdf` into Markdown files under `docs/manual`.
- Make the first page an index (`docs/manual/index.md`).
- Extract all images from the PDF into `docs/images/manual`.
- Reference images from the Markdown with relative paths (`../images/manual/...`).

## Notes on the source PDF

- `GTUltra.pdf` is 23 pages.
- The first three pages are the index/table of contents. The actual content begins on page 3 with the `What is it?` and `What’s new for 1.5.0?` sections.
- The decision was therefore:
  - `docs/manual/index.md` = combined index from pages 1–3.
  - `docs/manual/page-03.md` to `docs/manual/page-23.md` = the remaining content.

## Conversion script

The extraction was done with a one-off Python script using PyMuPDF (`fitz`). It was run as a `python3 - <<'PY'` heredoc from the repo root.

```python
import fitz
import re
from pathlib import Path

PDF = Path('docs/archive/GTUltra.pdf')
MANUAL_DIR = Path('docs/manual')
IMG_DIR = Path('docs/images/manual')

MANUAL_DIR.mkdir(parents=True, exist_ok=True)
IMG_DIR.mkdir(parents=True, exist_ok=True)

doc = fitz.open(PDF)

marker_prefix = re.compile(r'^([a-z]\.|i{1,3}\.|iv\.|v\.|vi{0,3}\.|ix\.|x\.|xi{0,3}\.|\d+\.|-)(\s+|$)', re.I)


def process_content(block_text):
    lines = [l.strip() for l in block_text.splitlines() if l.strip()]
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        m = marker_prefix.match(line)
        if m:
            marker = m.group(1)
            rest = line[m.end():].strip()
            i += 1
            parts = [rest] if rest else []
            while i < len(lines) and not marker_prefix.match(lines[i]):
                parts.append(lines[i])
                i += 1
            content = ' '.join(parts)
            if marker == '-':
                out.append(f'- {content}')
            else:
                out.append(f'{marker} {content}')
        else:
            out.append(line)
            i += 1
    return '\n'.join(out)


def parse_index(block_text):
    lines = [l.strip() for l in block_text.splitlines() if l.strip()]
    entries = []
    i = 0
    while i < len(lines):
        if re.fullmatch(r'\d+', lines[i]):
            i += 1
            continue
        title = lines[i]
        page = None
        if i + 1 < len(lines) and re.fullmatch(r'\d+', lines[i + 1]):
            page = int(lines[i + 1])
            i += 2
        else:
            i += 1
        entries.append((title, page))
    return entries


def indent_level(title):
    if re.match(r'^[a-z]\.\s', title):
        return 2
    if re.match(r'^(i{1,3}|iv|v|vi{0,3}|ix|x|xi{0,3})\.\s', title, re.I):
        return 3
    if re.match(r'^\d+\.\s', title):
        return 1
    return 0


def image_md(filename):
    return f'![image](../images/manual/{filename})'


# extract and save images, keep per-page image info
page_images = {}
for pnum in range(1, len(doc) + 1):
    page = doc[pnum - 1]
    img_tuples = page.get_images()
    img_infos = page.get_image_info()
    pairs = list(zip(img_tuples, img_infos))
    pairs.sort(key=lambda pair: pair[1]['bbox'][1])
    imgs = []
    for idx, ((xref, *_), info) in enumerate(pairs, 1):
        pix = fitz.Pixmap(doc, xref)
        if pix.n > 4:
            pix = fitz.Pixmap(fitz.csRGB, pix)
        filename = f'page-{pnum:02d}-img-{idx}.png'
        pix.save(IMG_DIR / filename)
        imgs.append({'filename': filename, 'y0': info['bbox'][1]})
    page_images[pnum] = imgs

index_entries = []
index_title = ''
index_subtitle = ''
index_imgs = []
content_pages = {}

for pnum in range(1, len(doc) + 1):
    page = doc[pnum - 1]
    blocks = page.get_text('blocks')
    # skip page number block
    if blocks and blocks[0][4].strip().isdigit():
        blocks = blocks[1:]

    if pnum == 1:
        title_block = blocks[0][4]
        lines = [l.strip() for l in title_block.splitlines() if l.strip()]
        if lines:
            index_title = lines[0]
        if len(lines) > 1:
            index_subtitle = lines[1]
        index_imgs = page_images[pnum]
        for b in blocks[1:]:
            index_entries.extend(parse_index(b[4]))
    elif pnum == 2:
        for b in blocks:
            index_entries.extend(parse_index(b[4]))
    elif pnum == 3:
        if blocks:
            index_entries.extend(parse_index(blocks[0][4]))
            content_blocks = blocks[1:]
        else:
            content_blocks = []
        content_pages[pnum] = content_blocks
    else:
        content_pages[pnum] = blocks

# build index.md
index_lines = []
index_lines.append(f'# {index_title}')
if index_subtitle:
    index_lines.append(index_subtitle)
    index_lines.append('')
for img in sorted(index_imgs, key=lambda x: x['y0']):
    index_lines.append(image_md(img['filename']))
    index_lines.append('')
index_lines.append('## Table of Contents')
index_lines.append('')
for title, page in index_entries:
    page = page or 1
    indent = '  ' * indent_level(title)
    index_lines.append(f'{indent}- [{title}](page-{page:02d}.md)')

(MANUAL_DIR / 'index.md').write_text('\n'.join(index_lines) + '\n')

# build content pages
for pnum, blocks in content_pages.items():
    elements = []
    for b in blocks:
        text = process_content(b[4])
        if text.strip():
            elements.append({'type': 'text', 'y0': b[1], 'text': text})
    for img in page_images.get(pnum, []):
        elements.append({'type': 'image', 'y0': img['y0'], 'filename': img['filename']})
    elements.sort(key=lambda e: (e['y0'], 0 if e['type'] == 'text' else 1))

    out = []
    for e in elements:
        if e['type'] == 'text':
            out.append(e['text'])
        else:
            out.append(image_md(e['filename']))
        out.append('')
    (MANUAL_DIR / f'page-{pnum:02d}.md').write_text('\n'.join(out).strip() + '\n')

print('Wrote files:')
print(' ', (MANUAL_DIR / 'index.md').stat().st_size, 'bytes', MANUAL_DIR / 'index.md')
for p in sorted(MANUAL_DIR.glob('page-*.md')):
    print(' ', p.stat().st_size, 'bytes', p)
print('Images:', len(list(IMG_DIR.glob('*.png'))))
```

## Generated files

Markdown files (`docs/manual/`):

- `index.md` (combined index, title page and linked table of contents)
- `page-03.md` to `page-23.md` (content)

Images (`docs/images/manual/`):

- 52 PNG files extracted from the PDF.
- Named `page-<NN>-img-<N>.png` (sorted by vertical position on the page).

## Verification notes

- `index.md` was rendered and the linked TOC matches the PDF’s index.
- Content pages were spot-checked (`page-03.md`, `page-06.md`, `page-07.md`) and images are embedded in reading order.
- Images are referenced with `../images/manual/<filename>` from the `docs/manual` directory.

## Known caveats / next agent notes

- The content pages are mostly plain text with list markers preserved. No explicit Markdown heading levels (`##`/`###`) were added to section titles.
- List indentation is flat; original PDF indentation is not preserved.
- Some pages have an empty `iv.` item (e.g. `page-06.md`) where the PDF had a placeholder; a small `iv.` line remains.
- Page 3 is split: `page-03.md` begins with the `What is it?` and `What’s new for 1.5.0?` content.
- The original PDF is still in `docs/archive/GTUltra.pdf` and was not modified.

## Handover

Next agent: the conversion is done but needs formatting/polish. Remaining work:

1. **Format content pages** — add proper Markdown heading levels (`##`, `###`) to section titles (`What is it?`, `What’s new for 1.4.1?`, `Credits:`, `Features compared to GTStereo`, `5. Transport bar`, etc.). Current content pages are mostly plain text.
2. **Fix list indentation** — restore PDF indentation/nesting for `a.`, `b.`, `i.`, `ii.`, etc. Currently all list items are flat.
3. **Clean extraction artifacts** — fix merged text blocks (e.g. `a. See  True Stereo s ection` and `5. Transport bar` appearing in the `4. Instrument True Stereo Panning` block) and remove empty marker lines like `iv.` and `q.`.
4. **Add page navigation** — previous/next links between `page-03.md` … `page-23.md`.
5. **Improve image references** — replace generic `![image]` alt text with meaningful descriptions; verify images are placed correctly for each section.
6. **Decide on final file structure** — current choice: `index.md` (pages 1–3 index) + `page-03.md` … `page-23.md` (content). Could be changed to one Markdown per PDF page, or reorganized further.
7. **Add a manual overview** — optional `docs/manual/README.md` linking from `index.md` and/or update root `README.md` to point to the manual.
8. **Optional: automate regeneration** — keep/commit the conversion script or a `Makefile` target if the PDF will be updated.
