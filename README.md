# BB, the portable demo

**AA PRESENTS... BB. A real demo, made of nothing but letters.**

In 1997 four students from Tabor in the Czech Republic made a demo for a
text terminal. The screen fills with a hex dump while it "precalculates
data", then two giant letter Bs fly in from opposite corners, the words
"100 % ANSI C", "PORTABLE DEMO" and "FULL SVGA TEXT MODE" come one by one,
and from there it never lets up: plasma, fire, a tunnel and moiré
patterns, a snowfall, a 3D torus that turns in shaded ASCII, a zoom
through a fractal borrowed from XaoS, greetings to Future Crew, Triton
and Cascada, and photographs of the four authors, each with a deadpan
life story typed out beside it ("1997 - Removed Windows 95 to make space
for aalib"). All of it is drawn in plain characters, with bold and dim
for the brightest and darkest parts, by AAlib, the ASCII art library the
same group wrote. Three Scream Tracker songs by FK play underneath. The
credits fly past in letters a screen high, and at the end you can page
through the AA project's own story.

It runs about seven minutes.

## Watching

- **s** or **Backspace**: on to the next part
- **q** or **Esc**: quit
- In the end text: **Space**, **f**, **j** or **Down** for the next page,
  **b**, **k** or **Up** for the one before, **1**, **2** and **3** to
  change the song

`bb -loop` plays it again and again; `bb 2` starts at the second half and
`bb 3` at the end text. `bb -nosound` plays it without the music. AAlib's
own options work too: `bb --help` lists them (`-extended` for the full
character set, `-nobold`, `-nodim`, `-reverse`).

## Who made it

The AA project: Filip Kupsa (FK, the music), Jan Hubicka (HH, AAlib,
XaoS and most of the effects), Mojmir Svoboda (MS, the 3D engine) and
Kamil Toman (KT, the sound engine and the synchronisation). Released 1997
to 2001 under the GPL. Debian has carried it since 1998; this edition
starts from Debian's last package, 1.3rc1-13, with its patches.

What changed here, and why, is in PATCHES.md; the original README is
README.

## Building

```sh
sh asciiverse/build.sh                     # Linux or macOS (CC_ARCH=arm64 or x86_64 on macOS)
TARGET=windows sh asciiverse/build.sh      # Windows, with mingw-w64
sh asciiverse/stage.sh                     # the archive, in stage/
```

AAlib and libmikmod are built from source and linked in; the music plays
through miniaudio. GPL-2.0-or-later (COPYING); AAlib and libmikmod are
LGPL-2.0-or-later.
