# ATBL - osobny Arp Table (feature request)

Status: analiza / decyzje otwarte. Branch: `feature/atbl-separate-arp-table`.

## Co to jest ATBL

`ATBL` = **A**rp **T**a**BL**e. Nazwa w konwencji istniejacych stalych w `src/gcommon.h`:
`WTBL` (Wave), `PTBL` (Pulse), `FTBL` (Filter), `STBL` (Speed). Osobna tablica arp = piata tablica `ATBL`.

## Zadanie (od jammera)

Jammer chce ficzer do playera: **osobny wave table i osobny arp table**. Dzis w GoatTracker/GTUltra
wavetable jest wspolny dla waveformow i nut (arpeggia). Cel: rozdzielic to na dwie niezalezne tablice,
zeby arp mial wlasny timing, wlasny loop i byl edytowany/wskazywany niezaleznie od waveformu.

## Jak jest teraz (dlaczego wave i arp sa sklejone)

Wavetable to **jedna tablica z dwiema kolumnami, chodzona jednym licznikiem pozycji na glos**:

- LEWA kolumna (`ltable[WTBL]` -> w playerze `mt_wavetbl`): waveform / komenda `$E0-$FE` / delay `$00-$0F` / skok `$FF`
- PRAWA kolumna (`rtable[WTBL]` -> `mt_notetbl`): nuta (bit7=1 relatywna arp, bit7=0 absolutna, `$00` = brak zmiany)

W playerze `mt_waveexec` (`player.s:946-993`) co ramke czyta **obie kolumny pod tym samym indeksem**
`mt_chnwaveptr,x`. Konsekwencje:

- Krok waveformu i krok arpa **zawsze ida w lockstepie** - ten sam wiersz, to samo tempo.
- Delay siedzi w LEWEJ kolumnie (`$00-$0F`), wiec timing arpa jest podporzadkowany timingowi waveformu.
  Nie zrobisz arpa co 2 tiki na waveformie trzymanym 6 tikow bez recznego rozpisywania wierszy.
- Jest jeden loop point (`$FF`) wspolny dla obu.
- Instrument ma jeden wskaznik startu `ptr[WTBL]` -> `mt_inswaveptr`.

Kluczowa obserwacja techniczna: na poziomie asemblera lewa i prawa kolumna to **juz dwie osobne tablice**
(`mt_wavetbl` / `mt_notetbl`). Nie sa rozdzielone dane, tylko **licznik pozycji i timing**.

## Co znaczy "osobny wave table i arp table"

Prawdziwe rozdzielenie = kazda tablica ma **wlasny licznik pozycji, wlasny delay/timing, wlasny loop,
a instrument wskazuje na obie niezaleznie**. To dodanie 5. tablicy (ATBL) i drugiej maszyny wykonawczej
w playerze.

## Jak przeanalizowano (metoda)

Dwa rownolegle rozpoznania (agent Explore) nad codebase, oba tylko czytajace kod (bez zmian):

1. **Warstwa edytora/aplikacji (C).** Zmapowano struktury danych tablic, edycje/UI, serializacje do `.sng`
   i eksport, powiazanie instrumentu z tablica, oraz wszystkie miejsca zakladajace `MAX_TABLES == 4`.
2. **Warstwa playera (6510 asm).** Zmapowano rutyne wykonawcza wavetable, dispatch lewej kolumny
   (waveform/komenda/delay/skok), obsluge prawej kolumny (arp rel/abs), layout pamieci per-glos,
   inicjalizacje wskaznika przy trigerze nuty. Potwierdzono, ze `src/asm/` to vendorowany toolchain
   asemblera (nie kod playera) - kod playera to pliki `.s` w `src/`.

### Kluczowe pliki i referencje (z map)

Edytor / dane:
- `src/gcommon.h`: `WTBL=0 PTBL=1 FTBL=2 STBL=3` (l.21-24), `MAX_TABLES 4` (l.32), `MAX_TABLELEN 255` (l.33),
  `INSTR.ptr[MAX_TABLES]` (l.70), pasma waveformow `WAVEDELAY/WAVESILENT/WAVECMD` (l.59-64).
- `src/gsong.c`: `ltable[MAX_TABLES][MAX_TABLELEN]` (l.20), `rtable[...]` (l.22), tablice `detailedTable*` (l.50-64),
  `SNG_INFO` bufory multi-song (malloc l.2720-2721, memcpy l.2778-2779 / 2851-2852).
- `src/gtable.c`: caly edytor tablic - key handling, kursor (`etnum/etpos/etcolumn/etview[]`),
  insert/delete z fixupem wskaznikow (l.727/809), edycja detailed wave (l.1276-1378), `gettablelen` (l.944).
- `src/gpattern.h`: `EDITOR_INFO` pola kursora tablic (l.75-86).
- `src/gdisplay.c`: render tablic - `displayTables` (l.2264), `displayDetailedWaveTable` (l.2364),
  klasyczny `displayTable` (l.3065).
- `src/greloc.c`: eksport do playera - nazwy labeli LEWA `mt_wavetbl/pulsetimetbl/filttimetbl/speedlefttbl`
  i PRAWA `mt_notetbl/pulsespdtbl/filtspdtbl/speedrighttbl` (l.26-36), `tableused`/`tablemap` (l.43-44),
  petla emisji (l.1653-1783), remap wskaznika instrumentu (l.1258-1263), kodowanie nuty `^0x80` (l.1744-1748).
- Serializacja `.sng`: `savesong` blok tablic `[len:1][ltable:len][rtable:len]` x MAX_TABLES (`gsong.c` l.302-309),
  `loadsong` sciezki `GTS5`/`GTS2`/`GTS!` (l.517-523 / 638-643 / 678+), ident zapisu `GTS5`.
- Petle zakladajace `MAX_TABLES==4` (do przejscia przy dodaniu ATBL): `gcommon.h` 32/70, `gdisplay.c` 3052,
  `ginstr.c` 306, `goattrk2.h` 164, `gpattern.h` 76, `greloc.c` 43/44/861/933/998/1654, `greloc.h` 34/37,
  `gsong.c` (liczne, m.in. 303/390/518/638/2146/2188/2720-2721/2778-2779/2851-2852), `gsong.h` 49/50,
  `gt2reloc.c` 129, `gt2stereo.c` 194/1772/3211, `gtable.c` 135/195/638/645/994/1104/1176, `gundo.c` 95.

Player (6510) - kod identyczny w 8 wariantach, cytaty z `src/player.s`:
- `mt_waveexec` (l.946-993): licznik pozycji `mt_chnwaveptr,x`, delay `mt_chnwavetime,x`,
  biezacy waveform `mt_chnwave,x`; czyta LEWA `mt_wavetbl-1,y` i PRAWA `mt_notetbl-2,y`.
- Dispatch lewej: delay `$00-$0F` (l.951-956, bias +$10 z relokatora), waveform `$10-$DF` (l.966),
  komenda `>=$E0` -> `mt_execwavecmd` (l.963-964/985-987, exec l.1479-1503), skok `$FF` (LOOPWAVE, l.60/969)
  -> nowa pozycja z PRAWEJ kolumny (l.976-978).
- Prawa/nuta: `mt_wavefreq` (l.1071-1089), bit7=1 relatywna (`adc mt_chnnote,x`), bit7=0 absolutna,
  `$00` brak zmiany (l.993).
- Trigger nuty: `mt_insfirstwave-1,y` (l.865), `mt_inswaveptr-1,y` -> `mt_chnwaveptr,x` (l.915-916).
- Zmienne per-glos: `mt_chnwaveptr` (l.1580), `mt_chnwave` (l.1582), `mt_chnwavetime` (l.1620),
  `mt_chnnote` (l.1603), `mt_chnlastnote` (l.1651).
- Pliki playera: `src/player{,3,9,12}.s` (1/2/3/4 SID) + `src/altplayer{,3,9,12}.s` (alt write order) = 8 sztuk.

## Zakres zmian dla ATBL (warstwa po warstwie)

1. **Model danych / edytor (C):** `MAX_TABLES 4->5`, nowy enum `ATBL`, `INSTR.ptr[]` rosnie,
   nowy widok/edycja w `gtable.c` + `gdisplay.c`, undo `gundo.c`, ~30 petli `MAX_TABLES` do przejscia.
2. **Format pliku:** bump `GTS5 -> GTS6` (save/load, instrument save/load, bufory multi-song); stare
   loadery (`GTS!/GTS2/GTS3/4/5`) ustawiaja pusty arp table. Lamie kompatybilnosc w przod -> wersjonowanie.
3. **Relocator (`greloc.c`):** nowa para labeli (np. `mt_arptbl`/`mt_arpnotetbl` albo jedna kolumna),
   `tableused`/`tablemap` na 5 tablic, emisja, remap wskaznika instrumentu.
4. **Player 6510:** nowa rutyna `mt_arpexec` (klon `mt_waveexec`, ale steruje tylko czestotliwoscia),
   nowe zmienne per-glos `mt_chnarpptr`/`mt_chnarptime`, nowy `mt_insarpptr`, init przy trigerze nuty.
   Do zrobienia w 8 wariantach; player 4SID (player12) to do 12 glosow -> budzet cykli/pamieci na ramke.

## Decyzje otwarte (do odpowiedzi zanim ruszy implementacja)

### 1. Semantyka: jak ATBL wspolgra z istniejaca kolumna nut w wavetable?
Kolumne nut w wavetable trzeba zostawic dla back-compat starych songow. Opcje:
- **(a) Arp przejmuje nute** - jesli instrument ma wskaznik ATBL, arp rzadzi czestotliwoscia;
  prawa kolumna wavetable ignorowana (poza skokiem `$FF`). Najczystsze, najmniej konfliktow.
- **(b) Arp jako offset on-top** - prawa kolumna wavetable dalej ustala nute bazowa; ATBL dodaje
  transpozycje w swoim wlasnym kroku. Bogatsze, trudniejsze cyklicznie i w UI.
- **(c) Arp tylko gdy nuta=0** - ATBL dziala tylko gdy prawa kolumna wavetable = `$00` w danym wierszu.
  Kompromis, semantyka metna.

### 2. Ktore warianty playera obejmuje ficzer?
- Wszystkie 8 (`player{,3,9,12}.s` + `altplayer*.s`) - pelne 1/2/3/4SID + alt write-order.
- Tylko 2SID base (`player.s` + `altplayer.s`) - szybka droga do dzialajacego prototypu.
- Tylko `player.s` - jeden wariant, czysty prototyp do oceny brzmienia/UX.

### 3. Nastepny krok?
- Pelny dokument projektowy + plan PR (format ATBL, kodowanie delay/loop, layout playera, budzet cykli,
  plan migracji formatu) - zero kodu na razie.
- Prototyp od razu wg wybranej semantyki i zakresu playerow.
- Najpierw research fela: czego dokladnie chce jammer (przyklady z innych trackerow, np. SID-Wizard
  separate arp) zanim zamrozimy design.

### Dodatkowe pytania techniczne do rozstrzygniecia w designie
- Format wiersza ATBL: klonowac ksztalt 2-kolumnowy (LEWA=delay/loop/komenda, PRAWA=nuta) czy prostszy?
- Czy ATBL ma miec wlasne komendy (`$E0-$FE`) czy tylko nuty + delay + loop?
- Czy skok/loop `$FF` w ATBL ma dzialac tak samo jak w wavetable?
- Budzet cykli na 4SID/12 glosow: czy druga rutyna per-glos miesci sie w ramce rastra?
