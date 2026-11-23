# Asciiverse changes

This is the Asciiverse packaging of **BB**, "the portable demo" by the AA
project (Filip Kupsa, Jan Hubicka, Mojmir Svoboda and Kamil Toman), first
released in 1997, last as 1.3rc1 in 2001: a demo drawn entirely in text
characters through AAlib, with its own music played through MikMod.
README.md says what it is; this file says what changed and why.

## History in this repository

This repository is Debian's packaging repository for bb
(salsa.debian.org/debian/bb, the `debian` remote), with its branches
`master`, `upstream` and `pristine-tar` and its tags unchanged. Ours is the
branch `asciiverse`:

1. **Upstream 1.3rc1** (tag `upstream/1.3rc1`): the AA project's release
   tarball as Debian imported it, `bb_1.3rc1.orig.tar.gz`.
2. **Debian's twelve patches** from bb 1.3rc1-13 (Axel Beckert's upload of
   2022-11-20, the last), one commit each in the order of Debian's
   `series`, each under the name and date of the person and upload that
   brought it in: Uwe Hermann (README, manpage and compiler warnings, 2001;
   the amd64 z-buffer fix by Corey Hickey, Kaare Hviid and Helge
   Kreutzmann, 2005; the gcc 3.4 fix by Karl Chen, 2005), Bhavani Shankar R
   (`-lm`, 2011) and Axel Beckert (music off by default, 2015; typo fixes
   and `strlen`, 2016). Tagged `debian/1.3rc1-13-patches`: Debian's own tag
   `debian/1.3rc1-13` names its packaging commit and stays as it is.
3. **Ours**, below: the content cleanup, AAlib, the sound, the log and
   the packaging.
4. **Fixes from others** since 1997 (the `stroucki` and `artyfarty`
   remotes), each its own commit under its author's name and date; see
   "Fixes from others" below.
5. **Our memory check** with AddressSanitizer and UBSan, one commit per
   fix; see "Memory check" below.

## Content

David's cleanup for a catalog that families browse:

- `8a89006` drops six of the end text's personal greetings (for sex,
  drug, drink and occult lines) and one line of Jan Hubicka's biography,
  a 2001 joke about assassinating Bill G.
- `adb86d4` then drops the rest of the personal greetings section, with its
  line in the end text's contents list: with holes in it the list read
  oddly. These were the authors' notes to friends and idols, not part of
  the demo itself; the end text now goes from "Advanced usage" and "Some
  BB rumours" straight to the AA project's page.

## AAlib, in aalib/

BB draws with AAlib, the ASCII art library by Jan Hubicka, Kamil Toman and
others, which no system has in a form we can link into one program for
Linux, macOS and Windows. It is in `aalib/`, the same copy XaoS uses in
theasciiverse/xaos, brought over with its history:

1. **AAlib 1.4rc5 as released** (2001-04-26), credited to Jan Hubicka,
   from Debian's `aalib_1.4p5.orig.tar.gz`, sha256
   `fbddda9230cf6ee2a4f5706b4b11e2190ae45f5eda1f0409dc4f99b35e0a70ee`.
2. **Debian's patches** from aalib 1.4p5-51.1, one commit each under its
   own author (Gergely Nagy, Jonathan Carter, Sven Joachim, Andrey
   Rakhmatullin), tagged `aalib/debian/1.4p5-51.1`.
3. **XaoS's fixes**: the curses driver gets the width and height the right
   way round (Debian's patch for ncurses' opaque `WINDOW` passed
   `getmaxyx()` the columns first, so a 100x30 terminal was taken for 30
   columns by 100 rows; Debian's own libaa1 has the same bug), and two
   mouse changes BB does not use.
4. **Builds on Windows**: AAlib's stdin keyboard driver waits on stdin with
   `select()`, which Windows has only for sockets, and it is the one driver
   AAlib always registers; on Windows it is left out, and BB uses the
   curses keyboard driver on PDCurses there.

## Code

- **The music starts with the demo.** BB asked "Music?" and then showed
  MikMod's settings table before the demo; Debian made the answer default
  to no, because MikMod's drivers held up the picture under PulseAudio
  (Debian bug #761023). Now the music plays from the start at 44100 Hz,
  16-bit stereo with interpolation, and `-nosound` (or `--nosound`) plays
  the demo without it.
- **The music goes out through miniaudio.** A new MikMod driver,
  `asciiverse/src/av_sound.c`, hands MikMod's software mixer to miniaudio
  0.11.25 (`asciiverse/src/miniaudio.h`, public domain or MIT-0), which
  pulls the samples from its own thread, as MikMod's SDL driver does, so
  the picture never waits for the sound. miniaudio opens the system's sound
  library when BB starts (ALSA, PulseAudio or JACK on Linux, Core Audio on
  macOS, WASAPI on Windows); without one BB says so and plays on silently.
  `AV_AUDIO_DUMP=<file>` plays to no device and writes the stream to the
  file (raw 16-bit stereo, 44100 Hz), to check the music without
  listening. The audio thread never waits for MikMod's lock: MikMod holds
  it while it starts or stops the output (`Player_Start` and `Player_Free`
  when BB changes songs, `MikMod_Exit` on quitting), and starting or
  stopping waits for the audio thread, so a waiting callback froze BB on
  PulseAudio at the change to the second song. When the lock is taken the
  callback plays one 20 ms period of silence instead.
- **Debian bug #761023 is a different freeze with the same face.** Its
  log (2014 to 2024, merged with #123150 from 2001) describes Debian's bb
  on MikMod's own PulseAudio driver: the music plays on while the picture
  stops, and Samuel Bronson found that `timestuff()` never returns. There
  BB itself pushed the sound, `update_sound()` calling `MikMod_Update()`
  from a 10 ms timer on the main thread, and PulseAudio's blocking writes
  made each call take longer than its interval; BB's timer code calls a
  late handler again until it catches up (`tl_process_group()`, as Torbjörn
  Andersson pointed out in 2015), so it never got back to drawing. One
  thread, no lock. The miniaudio driver ended that: `MikMod_Update()` has
  nothing to push and the audio thread pulls. The freeze `5ae26c5` fixes
  is a lock-order deadlock between that audio thread and the main thread,
  which only our driver has; both threads stop, music included. So both
  are fixed at their cause, not worked around, and the music is on by
  default again.
- **The music is found beside the program.** BB looked in a folder fixed
  when it was built (`SOUNDDIR`, /usr/share/bb in Debian), then in the
  current folder. It now looks beside its own program file first
  (`asciiverse/src/av_util.c`), so an install works wherever it is.
- **A session log.** BB keeps no saves or settings; it writes `bb.log`,
  new each run, to `$XDG_STATE_HOME/bb` (`~/.local/state/bb` by default,
  `%LOCALAPPDATA%\bb` on Windows): the AAlib driver and screen size, the
  sound system found, each song, the keys that skip a part or quit.

The drawing is AAlib's curses driver, as BB always had it in a terminal:
plain characters with bold and dim, fitted to the terminal at every part
of the demo. Apart from the content cleanup above, nothing in the demo
itself changed.

## Fixes from others

People kept BB building after 2001 in their own trees. Every commit in
them was read against ours (upstream 1.3rc1 with Debian's twelve
patches); what applies is here, one commit each under its author's name
and date, with git's "cherry picked from" line naming the original.

From **Michael Stroucken**, github.com/stroucki/bb (GPL-2.0, built on bb
1.2):

- `fd4a318` (2015-08-21) credits2, image: unsigned pixel pointers, and the
  LZO decompressor's size as unsigned, as it expects.
- `98f2f8f` (2015-08-21) messager, scene4: unsigned screen backups and
  fire buffer pointer.
- `0df3417` (2015-08-21) bb.h: `decompressimg()` declared as returning
  unsigned bytes.
- `c69b484` (2015-08-21) bb.h: font data and `fastscale()` unsigned. By
  hand: 1.3rc1 already has its `const`.
- `81656c3` (2015-08-21) scene7: the fractal buffers unsigned.
- `39bde3b` (2015-08-21) print: unsigned glyph data, and `pscale()` returns
  at once when a letter comes out zero wide or zero tall, instead of
  scaling by nothing.
- `cc3fcc0` (2015-08-21) scene2: the title effects work out the part's
  time and the screen size once per frame rather than in every
  expression; the same picture.
- `09b131a` (2015-08-21) zoom: `%lu` for the sizes in its out of memory
  messages, unsigned buffers for `make_context()`, and a note that PRICE
  is undefined. By hand: Debian's zoom.c.patch already braces the
  ambiguous else.
- `c6e3636` (2015-08-21) scene5: its texts are `char *`.
- `fdc60bf` (2015-08-21) zoom.h: `make_context()` takes unsigned buffers.
- `167ff4d` (2023-10-12) "Avoid compiler warnings", by hand: `<stdlib.h>`
  in place of the non-standard `<malloc.h>` everywhere, two unused
  variables. Left out of it: the `mikunix/` files (1.3rc1 has none), the
  bb.c `<time.h>` and julia.c variables (Debian's patches did both), the
  timers.c globals (1.3rc1 already guards them), and its new PRICE macro,
  which drops the parentheses and no longer squares (`3bcc25f` fixes PRICE
  instead).
- `5512c4d` (2026-03-11) "Address memory problems reported by compiler
  and libasan": the message text scrolls line by line instead of with one
  `memcpy()` over overlapping memory (garbled on a modern libc),
  minilzo's work memory starts zeroed, and the torus and the patnik free
  their 3D data.

Skipped, with the reason:

- stroucki `ef51e52`, "Import from bb-1.2.tar.gz": the 1.2 base, not a fix.
- stroucki `134a264`, backconv struct params from artyfarty: already in
  1.3rc1.
- stroucki `29e3d1c`, "tex: correct memset size": Debian's tex.c.patch
  (`cc2355a`), the same one-line fix.
- stroucki `2731458`, the NetBSD/VAX makefile, and `1dcc17c`, config.log
  removal and a VAX aconfig.h: build plumbing; `asciiverse/build.sh`
  builds BB.
- stroucki `fdeabad`, textform: 1.3rc1 already has the line endings and
  the SourceForge address it brought in; the rest is a "modded by
  stroucki" banner and contact addresses of the 1.2 text.
- stroucki `13a5223`, uncompfn `const`: already in 1.3rc1.
- stroucki `5226e20`, `extern` for the context, params and font globals in
  scene1 and tex: 1.3rc1 already defines them only in bb.c.
- stroucki `10b060d` (Jookia, 2023-08-06), "mikunix: Use stdint for type
  widths (fixes glitchy audio)", and its merge `610616f`: a fix to the old
  MikMod copy in bb 1.2's `mikunix/`, where 64-bit `long` made MikMod's
  32-bit types 64 bits wide. 1.3rc1 has no copy; ours is libmikmod
  3.3.13, whose `mikmod.h` already picks 32-bit types on 64-bit systems,
  and the sound goes through `asciiverse/src/av_sound.c`.
- artyfarty/bb-osx (Dmitry Guzhov and contributors, 2013 to 2023): its
  first commit is 1.3rc1 with `<malloc.h>` removed for macOS, which
  `167ff4d` covers (and `asciiverse/include/compat/malloc.h` already did
  for the build); "Fixed data type size issue" (superandroidtron,
  2014-10-28) is the same z-buffer fix as Debian's tex.c.patch; "Fixed
  bugs related to missing headers" (Suyash Joshi, 2022-01-01) adds
  `<stdlib.h>` where artyfarty took out `<malloc.h>`, covered by
  `167ff4d`, and `<time.h>` in bb.c, covered by Debian's bb.c.patch; the
  rest is the Xcode and Homebrew build, a generated configure and READMEs.
- denisse-dev/bb (2018): bb 1.2 exactly as released.
- deepin-community/bb (2023): Debian's 1.3rc1-13 packaging, the same
  twelve patches.
- coolwanglu/bb.js, RobSis/bbee and TSVM-Project/tsvm-bb: ports to
  JavaScript and TSVM, not code for this tree.

## Memory check

After the fixes above, BB was built a second time with
`-fsanitize=address,undefined -fno-omit-frame-pointer -g` (AAlib, BB and
our sound driver; the release build and its flags are unchanged) and
played under the 4 GB memory cap:

- the whole demo with `-nosound` at 80x24, from the start to the end
  text, which was paged with Space, Up and Down, its songs switched with
  1, 2 and 3, and quit with `q`;
- `bb 2` at 200x60 and `bb 3` at 100x30;
- `s` pressed in every part: once per song through the whole demo at
  100x30, and single presses at staggered moments in five more runs at
  80x24, 100x30 and 200x60, `bb 2` among them, so the skip lands in each
  scene;
- a resize mid-part, 100x30 to 130x40, 80x24, 200x60 and back while the
  demo played;
- with sound on a silent PipeWire sink through PulseAudio: the whole demo
  at 100x30 and again at 80x24, every song change included, `bb 3` at
  200x60 with the songs switched, and `bb 2` quit mid-part.

What it found, one commit each, all ours; apart from where a few stars
of the random star field land, none changes the picture or the sound:

- `5760aab` aalib: `aa_mktable()` assigned `p1` twice in one expression
  and overflowed an int. Rewritten as GCC has always compiled it; the
  character tables were compared entry by entry with the old code's for
  every font, attribute set and dim and bold setting, and are identical.
- `22d9735` aalib: the dithering random generator overflowed an int; now
  unsigned, the same numbers.
- `6fbf249` tex.c: negative coordinates shifted left in the torus and the
  patnik; now multiplied.
- `3bcc25f` zoom.c: PRICE wrote and read `ftmp` with no sequence point
  (GCC's warning, and Michael Stroucken's note); now the square written
  out.
- `7c0478d` credits.c: star positions overflowed an int for stars far off
  to the side; now 64-bit, and those stars are thrown away as the code
  meant.
- `13662ed` aalib: the driver names left in AAlib's recommendation lists
  were never freed (a leak in every run).
- `499d422` credits2.c and `3d79a3e` scene5.c: brightness, colour and
  noise ramps overflowed an int when a part ran ahead of the music's
  schedule; now unsigned, the same numbers.
- `e2a97f5` miniaudio: 16-bit samples were widened to 32-bit by a left
  shift, undefined for negative samples; now multiplied (marked in
  `miniaudio.h` as ours).
- `a3bb5d8` main.c: the song playing at the end was never freed.

After these the sanitizers report nothing over the same runs, and the
release build plays the whole demo with its music on the same sink, to
the end text and its songs, and quits with `q` within a second at ten
points through the demo and in the end text. One more
thing they turned up in a separate check of AAlib's tables, outside what
BB uses: two of AAlib's fonts, "X8x16" and "vgagl", hold fewer than 256
characters, and AAlib reads past them if one is chosen with `-font`. BB
uses AAlib's standard VGA 8x16 font ("vga16"), which is whole, and the
two fonts are left as they are.

## Build

`asciiverse/build.sh` replaces upstream's configure, which looks for a
system AAlib and MikMod. libmikmod 3.3.13 (sha256
`9fc1799f7ea6a95c7c5882de98be85fc7d20ba0a4a6fcacae11c8c6b382bb207`, the
same tarball as Debian's) is built without any of its own sound drivers,
AAlib from `aalib/` with only its curses, stdout and stdin drivers
(`asciiverse/include/aalib/config.h`), and BB from
`asciiverse/include/bb/aconfig.h` in place of configure's; all linked in.

- **Linux** (x86_64, aarch64): built on Ubuntu 22.04 with ncurses linked
  in, so it needs only glibc 2.35 or newer and terminfo. Not static musl:
  miniaudio opens the sound library with `dlopen`, which a static musl
  program cannot.
- **macOS** (arm64, x86_64): the system's ncurses and Core Audio.
- **Windows** (x86_64): cross-built with mingw-w64, AAlib's curses driver
  on PDCurses 3.9 (wincon), sound through WASAPI.

## Licence

BB is GPL-2.0-or-later (COPYING); AAlib (`aalib/COPYING`, shipped as
COPYING.AAlib) and libmikmod (shipped as COPYING.MikMod) are
LGPL-2.0-or-later, miniaudio public domain or MIT-0, ncurses and PDCurses
under their own permissive licences. The three songs are FK's (Filip
Kupsa), part of the GPL release; BB's credits name him for the music.

## Packaging (new files)

- `asciiverse/build.sh`, `asciiverse/include/`, `asciiverse/src/`: the
  build above, the sound driver and the log.
- `asciiverse/stage.sh`: the archive: BB with `bb.s3m`, `bb2.s3m` and
  `bb3.s3m` beside it, COPYING, COPYING.AAlib, COPYING.MikMod, AUTHORS,
  README.md and this file.
- `asciiverse/nfpm.yaml`: .deb, .rpm and Arch packages, BB and its songs in
  `/usr/lib/bb`, linked from `/usr/bin/bb`; they conflict with the
  distributions' own `bb`.
- `.github/workflows/release.yml`: Linux, macOS and Windows, a smoke test
  that plays the start of the demo with the music written to a file, the
  packages and SHA256SUMS, on our tags (`v1.3rc1-av.N`).
- `asciiverse/record-preview.py`, `asciiverse/bb.cast`: the catalog
  preview, 100x30 at 25 frames a second, without sound.
- `README.md` is ours; upstream's README is unchanged.
