import re
from pathlib import Path

manual_dir = Path('/Users/marcin/Projects/_marcin/GTUltra-PDF/docs/manual')
footer = '[Back to index](index.md)'

# Add footer to section files
for md in sorted(manual_dir.glob('*.md')):
    if md.name in ('index.md', 'WORK.md'):
        continue
    if md.name.startswith('page-'):
        continue
    text = md.read_text()
    # Avoid adding a duplicate footer
    if footer in text:
        continue
    # Ensure trailing newline, then add footer
    if not text.endswith('\n'):
        text += '\n'
    text += '\n' + footer + '\n'
    md.write_text(text)

# Add missing whats-new entries to index.md
index = manual_dir / 'index.md'
index_text = index.read_text()
if '[whats-new-141.md]' not in index_text:
    index_text = re.sub(
        r'(- \[What’s new for 1\.5\.0\?\]\(whats-new-150\.md\)\n)',
        r'\1- [What’s new for 1.4.1?](whats-new-141.md)\n- [What’s new for 1.3.0?](whats-new-130.md)\n',
        index_text
    )
    index.write_text(index_text)
