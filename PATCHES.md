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
3. **Ours**, below.

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
