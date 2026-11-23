# BB: draft entry for games/ROSTER.md

Written on rover, which cannot reach beast; paste it over BB's entry in
`games/ROSTER.md` there.

---

### BB (`bb`), demos shelf

The AA project's "portable demo" (Filip Kupsa, Jan Hubicka, Mojmir
Svoboda, Kamil Toman; 1997, last 1.3rc1 in 2001), drawn in text through
AAlib with FK's three songs through MikMod. GPL-2.0-or-later. Repo
theasciiverse/bb, branch `asciiverse`, on Debian's packaging repo
(salsa.debian.org/debian/bb).

- **History:** upstream 1.3rc1, Debian's twelve patches one commit each
  (tag `debian/1.3rc1-13-patches`), then ours. `PATCHES.md` lists all.
- **Content:** the end text's personal greetings section and a 2001 joke
  about assassinating Bill G. are out (David, 2026-11-23); the demo is
  otherwise as released.
- **Ours:** AAlib 1.4rc5 in `aalib/` (XaoS's copy, with Debian's patches),
  curses driver; music on by default through a miniaudio driver for
  MikMod (`-nosound` to go without), songs found beside the program, a
  session log in `$XDG_STATE_HOME/bb`. Linux, macOS and Windows
  (PDCurses, WASAPI).
- **Freeze at the second song (fixed 2026-11-23, beast, `5ae26c5`):** the
  audio callback only tries MikMod's lock. Debian bug #761023 is a
  different freeze (MikMod's own PulseAudio driver blocking BB's main
  thread), gone since the miniaudio driver.
- **Fixes from others (2026-11-23, rover):** twelve commits from Michael
  Stroucken's stroucki/bb (2015 signedness and const fixes, 2023 header
  and warning fixes by hand, and his 2026 ASan fixes: overlapping
  `memcpy()` in the message scroller, minilzo's work memory, the torus
  and patnik data freed), each under his name and date. Skipped, with
  reasons in `PATCHES.md`: what 1.3rc1 or Debian already had, the VAX
  build files, the "modded by stroucki" banner, Jookia's mikunix fix (no
  mikunix here) and all of artyfarty/bb-osx (covered or build plumbing).
- **Memory check (2026-11-23, rover):** ASan and UBSan over the whole demo
  at 80x24, 100x30 and 200x60, `bb 2`, `bb 3`, `s` in every part, the
  end text paged and songs switched, a resize mid-part, and with
  sound on a null PipeWire sink. Ten fixes, ours, none changing the
  picture or the sound beyond where a few random stars land: two AAlib integer overflows (the character table,
  checked identical entry by entry, and the dithering random generator),
  an AAlib leak of driver names, negative left shifts in the 3D code,
  integer overflows in the star field, the credits noise and the torus
  ramps, PRICE in the zoomer, one shift in miniaudio, and the last song
  freed on quitting. Clean after them.
- **Tag:** `v1.3rc1-av.2` (after `v1.3rc1-av.1`); the release comes with
  go-public.
- **Preview:** `asciiverse/bb.cast`, a minute at 100x30, no sound.
