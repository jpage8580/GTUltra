# GTUltra Manual Cleanup – Work Tracking

## Current Status

- [x] Task 1: Add real Markdown headings to `page-03.md` – `page-23.md`
- [x] Task 2: Fix nested list indentation (`a.`, `b.`, `i.`, `ii.`, etc.)
- [x] Task 3: Clean extraction artifacts (merged blocks, empty marker lines, split sentences)
- [x] Task 4: Add 'Back to index' footer to each section file
- [ ] Task 5: Improve image alt text and verify placement
- [x] Task 6: Restructure manual so pages are joined into correct sections
- [x] Task 7: Stage/commit working tree changes

## Naming Requirements

- Final Markdown section files should use **human-readable / nice names** instead of the auto-slugified placeholders in the table below.
- Images should be renamed to match the section file name with an index suffix: `{section-name}-img-{N}.png` (e.g. `transport-bar-img-1.png`, `stereo-panning-img-2.png`).
- The existing extracted images are `page-NN-img-N.png`; during restructuring they can be renamed to the new `{section-name}-img-{N}` pattern and the Markdown references updated accordingly.

## Section Mapping for Task 6

This table lists the intended section files. Each section starts at the heading shown and
includes the source pages that contain its content, joining pages where a section was split.
Level 4 `####` subheadings under `5. Transport bar` are nested inside `feature-5-transport-bar.md`.

| Level | Section Title | Source Pages | Proposed File |
|-------|---------------|--------------|---------------|
| 2 | What is it? | 3 | what-is-it.md |
| 2 | What’s new for 1.5.0? | 3-4 | whats-new-150.md |
| 2 | What’s new for 1.4.1? | 4 | whats-new-141.md |
| 2 | What’s new for 1.3.0? | 4 | whats-new-130.md |
| 2 | Credits: | 5 | credits.md |
| 2 | Features compared to GTStereo | 5 (parent heading) | features-compared.md |
| 3 | 1. Updated display / skinning | 5 | updated-display.md |
| 3 | 2. Undo (ctrl-z) | 5 | undo.md |
| 3 | 3. Instrument Use count (IC) | 5 | instrument-use-count.md |
| 3 | 4. Instrument True Stereo Panning | 6 | instrument-true-stereo-panning.md |
| 3 | 5. Transport bar | 6-7 | transport-bar.md |
| 3 | 6. True Stereo (Editor emulation only) | 8 | true-stereo.md |
| 3 | 7. Stereo Panning | 8-9 | stereo-panning.md |
| 3 | 8. 3,6,9 or 12 channel playback (1-4 SID support) | 9 | multi-channel-playback.md |
| 3 | 9. Song pattern selection | 9 | song-pattern-selection.md |
| 3 | 10. Song playback from anywhere | 9-10 | song-playback.md |
| 3 | 11. F3 = Shift/Space | 10 | f3-shift-space.md |
| 3 | 12. Jam Mode (when not in record mode)- Polyphonic | 10 | jam-mode.md |
| 3 | 13. Displays note and arp chord offsets in Jam Mode | 10 | jam-mode-notes.md |
| 3 | 14. MIDI note input | 10 | midi-note-input.md |
| 3 | 15. Load / Save Screen (F10 / F11) | 10 | load-save-screen.md |
| 3 | 16. Move to the previous / next pattern in a song | 11 | previous-next-pattern.md |
| 3 | 17. Tables separated by colour | 11 | tables-by-colour.md |
| 3 | 18. Auto-Portamento key (SHIFT-Y) | 11 | auto-portamento.md |
| 3 | 19. Displays the overall time of the song | 11 | song-overall-time.md |
| 3 | 20. Quick Save | 11 | quick-save.md |
| 3 | 21. Info line | 12 | info-line.md |
| 3 | 22. The Master Channel | 12 | master-channel.md |
| 3 | 23. F8 = Edit Tables (‘cos Jammer said so) | 12 | f8-edit-tables.md |
| 3 | 24. Filter Information | 12 | filter-information.md |
| 3 | 25. Palette Editor | 12-13 | palette-editor.md |
| 3 | 26. Char Editor | 13-14 | char-editor.md |
| 3 | 27. F2: Changed function (if classic F1-F3 mode disabled) | 14 | f2-changed-function.md |
| 3 | 28. Modify values with mouse | 14 | modify-values-with-mouse.md |
| 3 | 29. Looping | 14 | looping.md |
| 3 | 30. Copy (Ctrl-C) changes | 14 | copy-changes.md |
| 3 | 31. Inter-pattern looping | 14 | inter-pattern-looping.md |
| 3 | 32. Improved ENTER key functionality (moving to tables..) | 14 | improved-enter-key.md |
| 3 | 33. Detailed Table Editing: WaveTable | 15 | wavetable-editing.md |
| 3 | 34. Detailed Table Editing: Pulse Table | 16 | pulse-table-editing.md |
| 3 | 35. Detailed Table Editing: Filter Table | 16-17 | filter-table-editing.md |
| 3 | 36. Waveform editor | 17 | waveform-editor.md |
| 3 | 37. MIDI Port Select | 17 | midi-port-select.md |
| 3 | 38. Ctrl+Left / Ctrl+Right keys to quickly move to previous / next song position | 17 | previous-next-song-position.md |
| 3 | 39. SID export | 18 | sid-export.md |
| 3 | 40. Automatic .sng backup | 18 | automatic-sng-backup.md |
| 3 | 41. Editor information is saved within the .sng file | 18 | editor-info-sng.md |
| 3 | 42. Expanded OrderList View | 18-19 | expanded-orderlist-view.md |
| 3 | 43. Expanded OrderList - Copy / Cut / Paste / Insert | 19 | expanded-orderlist-copy-cut-paste.md |
| 3 | 44. Expanded OrderList - Pasting Transpose Values | 20 | expanded-orderlist-pasting-transpose.md |
| 3 | 45. Expanded OrderList - Setting Transpose values | 20 | expanded-orderlist-setting-transpose.md |
| 3 | 46. Expanded OrderList - Compressed Size | 20 | expanded-orderlist-compressed-size.md |
| 3 | 47. Expanded OrderList - Repeat / End Markers | 20-21 | expanded-orderlist-repeat-end-markers.md |
| 3 | 48. Disable all MIDI | 21 | disable-all-midi.md |
| 3 | 49. Multiple .SNG support | 21 | multiple-sng-support.md |
| 3 | 50. Export to .WAV | 21 | export-to-wav.md |
| 3 | 51. GT2Reloc.exe | 21-22 | gt2reloc.md |
| 3 | 52. Pattern order change when exporting to .SID | 22 | pattern-order-change.md |
| 3 | 53. Auto-Advance modes | 22 | auto-advance-modes.md |
| 3 | 54. Mouse Wheel | 22 | mouse-wheel.md |
| 3 | 55. SIDTracker64 Mode | 22-23 | sidtracker64-mode.md |
| 3 | 56. Drag and Drop to load | 23 | drag-and-drop.md |
| 3 | 57. SID Export - Zeropage SID playback option | 23 | zeropage-sid-playback.md |

## Notes for Task 6

1. **Parent / child structure**: `features-compared.md` is the parent file with the `## Features compared to GTStereo` heading. Each `### 1.` – `### 57.` feature becomes its own file.
2. **`5. Transport bar` subsections**: The `a.` – `p.` (including `i. Record ON / OFF`) `####` subheadings stay inside `feature-5-transport-bar.md`.
3. **Sections that span multiple source pages**: `5. Transport bar`, `7. Stereo Panning`, `10. Song playback from anywhere`, `25. Palette Editor`, `26. Char Editor`, `35. Detailed Table Editing: Filter Table`, `42. Expanded OrderList View`, `47. Expanded OrderList - Repeat / End Markers`, `51. GT2Reloc.exe`, `55. SIDTracker64 Mode`, and `What’s new for 1.5.0?` (pages 3–4).
4. **Index (`docs/manual/README.md`)**: The TOC links will need to point to the new section files. `features-compared.md` can link or list its child feature files.
5. **Images**: Image files are named `page-NN-img-N.png` and referenced with `../images/manual/...`. Relative paths stay valid as long as section files are in `docs/manual/` (or a subdirectory) and image references are updated only if needed.

## Next Immediate Task

Task 5: Improve image alt text and verify placement (in_progress).

## Later Steps

1. Task 5: Improve image alt text and rename images to `{section-name}-img-{N}.png`.

