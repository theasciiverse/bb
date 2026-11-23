#!/bin/sh
# Builds BB, the AA project's text-mode demo, as ./bb (bb.exe on Windows).
#
# Upstream's configure looks for a system AAlib and MikMod; here both are
# built from source into av-build/ and linked in:
#   - AAlib from aalib/ (1.4rc5 with Debian's patches and ours, the copy
#     XaoS uses), compiled with only its curses, stdout and stdin drivers
#     from asciiverse/include/aalib/config.h
#   - libmikmod 3.3.13 (tarball checked against its sha256), without any
#     of its own sound drivers: BB's music goes out through miniaudio
#     (asciiverse/src/av_sound.c), which loads the system's sound library
#     when BB starts and plays on silently without one
# BB itself is compiled from asciiverse/include/bb/aconfig.h in place of
# configure's.
#
#   sh asciiverse/build.sh
#       Linux (glibc, built on the oldest runner): AAlib, MikMod and
#       ncurses linked in, needing only libc and terminfo. Not static
#       musl: miniaudio opens ALSA or PulseAudio with dlopen, which a
#       static musl binary cannot
#   CC_ARCH=arm64 sh asciiverse/build.sh     (or x86_64)
#       macOS, on the runner: the system's ncurses and Core Audio
#   TARGET=windows sh asciiverse/build.sh
#       Windows, cross-built with mingw-w64 (x86_64-w64-mingw32-gcc):
#       AAlib's curses driver on PDCurses 3.9 (wincon), sound on WASAPI
set -eu
MIKMOD=3.3.13
MIKMOD_SHA256=9fc1799f7ea6a95c7c5882de98be85fc7d20ba0a4a6fcacae11c8c6b382bb207
PDCURSES=3.9

root=$(cd "$(dirname "$0")/.." && pwd)
target=${TARGET:-native}
deps="$root/av-build/$target${CC_ARCH:+-$CC_ARCH}"
prefix="$deps/prefix"
mkdir -p "$deps" "$prefix/lib" "$prefix/include"
jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
cflags="-O2${CC_ARCH:+ -arch $CC_ARCH}"
host=""
cc=${CC:-cc}
exe=bb
if [ "$target" = windows ]; then
  host=x86_64-w64-mingw32
  cc=$host-gcc
  exe=bb.exe
elif [ "$(uname)" = Darwin ]; then
  case "${CC_ARCH:-$(uname -m)}" in
    arm64) host=aarch64-apple-darwin ;;
    *) host=x86_64-apple-darwin ;;
  esac
fi

sha256() {
  if command -v sha256sum >/dev/null; then sha256sum "$1"; else shasum -a 256 "$1"; fi
}

# libmikmod: the library and its module loaders, no sound drivers
if [ ! -f "$prefix/lib/libmikmod.a" ]; then
  if [ ! -d "$deps/libmikmod-$MIKMOD" ]; then
    curl -sfL -o "$deps/libmikmod.tar.gz" \
      "https://sourceforge.net/projects/mikmod/files/libmikmod/$MIKMOD/libmikmod-$MIKMOD.tar.gz/download"
    sum=$(sha256 "$deps/libmikmod.tar.gz")
    [ "${sum%% *}" = "$MIKMOD_SHA256" ] || { echo "libmikmod-$MIKMOD.tar.gz: wrong sha256" >&2; exit 1; }
    tar -xzf "$deps/libmikmod.tar.gz" -C "$deps"
  fi
  (cd "$deps/libmikmod-$MIKMOD" && CC="$cc" CFLAGS="$cflags" ./configure --prefix="$prefix" \
     ${host:+--host=$host} --disable-shared --enable-static --disable-alldrv --disable-doc \
     --disable-simd >/dev/null && make -j "$jobs" >/dev/null && make install >/dev/null)
fi

# PDCurses for Windows: its wincon port, as a static library
if [ "$target" = windows ] && [ ! -f "$prefix/lib/libpdcurses.a" ]; then
  [ -d "$deps/PDCurses" ] || git clone -q --depth 1 -b "$PDCURSES" https://github.com/wmcbrine/PDCurses "$deps/PDCurses"
  (cd "$deps/PDCurses/wincon" && make -j "$jobs" CC="$cc" WIDE=Y UTF8=Y _w64=1 pdcurses.a >/dev/null)
  cp "$deps/PDCurses/wincon/pdcurses.a" "$prefix/lib/libpdcurses.a"
  cp "$deps/PDCurses/curses.h" "$deps/PDCurses/panel.h" "$prefix/include/"
fi

# AAlib: the library's sources minus the drivers for X11, S-Lang, GPM, the
# Linux console, DOS and OS/2
if [ ! -f "$prefix/lib/libaa.a" ] || [ -n "$(find "$root/aalib/src" "$root/asciiverse/include/aalib" -newer "$prefix/lib/libaa.a" -type f)" ]; then
  rm -rf "$deps/aalib-obj" && mkdir -p "$deps/aalib-obj"
  for f in aafont aafonts aavyhen aamktabl aamem aaprintf aahelp aafastre fontgl font16 \
           font14 fontx16 fontx13 fontx13b fontcour aarec font9 font8 fontline aacurses \
           aacurkbd aacurmou aalib aaflush aarender aaout aain aaedit aaregist aakbdreg \
           aamoureg aasave aastdout aastdin aaparse aaimgwidth aaimgheight aascrwidth \
           aascrheight aammwidth aammheight aaimage aatext aacurrfnt aaattrs aaputpixel \
           aarecfunc; do
    # shellcheck disable=SC2086
    $cc $cflags -w -I"$root/asciiverse/include/aalib" -I"$root/asciiverse/include/compat" \
      -I"$prefix/include" -I"$root/aalib/src" -c "$root/aalib/src/$f.c" -o "$deps/aalib-obj/$f.o"
  done
  rm -f "$prefix/lib/libaa.a"
  ${AR:-$( [ "$target" = windows ] && echo $host-ar || echo ar )} rc "$prefix/lib/libaa.a" "$deps"/aalib-obj/*.o
  ${RANLIB:-$( [ "$target" = windows ] && echo $host-ranlib || echo ranlib )} "$prefix/lib/libaa.a"
  cp "$root/aalib/src/aalib.h" "$prefix/include/"
fi

cd "$root"
srcs="autopilo.c backconv.c bb.c credits.c credits2.c ctrl87.c fk1.c fk2.c fk3.c fk4.c
  formulas.c hh1.c hh2.c hh3.c hh4.c image.c julia.c kt1.c kt2.c kt3.c kt4.c main.c
  messager.c minilzo.c ms1.c ms2.c ms3.c ms4.c print.c scene1.c scene2.c scene3.c
  scene4.c scene5.c scene7.c scene8.c scene9.c tex.c textform.c timers.c uncompfn.c
  zeb.c zoom.c"
case "$target" in
  windows) libs="-static -lpdcurses -lole32 -lwinmm -lm" ;;
  *)
    if [ "$(uname)" = Darwin ]; then
      libs="-lncurses -framework CoreAudio -framework AudioToolbox -framework CoreFoundation -lpthread -lm"
    else
      # ncurses linked in, so only libc is needed; miniaudio dlopens the sound library
      libs="-Wl,-Bstatic -lncursesw -ltinfo -Wl,-Bdynamic -lpthread -ldl -lm"
    fi ;;
esac
mm="$deps/libmikmod-$MIKMOD"
rm -f "$root/$exe"
# shellcheck disable=SC2086
$cc $cflags -w -DMIKMOD_STATIC -DHAVE_CONFIG_H -I"$mm" -I"$mm/include" -I"$prefix/include" \
  -c asciiverse/src/av_sound.c -o "$deps/av_sound.o"
# shellcheck disable=SC2086
$cc $cflags -w -DMIKMOD_STATIC -I"$root/asciiverse/include/bb" -I"$root/asciiverse/include/compat" \
  -I"$root/asciiverse/src" -I"$prefix/include" $srcs asciiverse/src/av_util.c \
  "$deps/av_sound.o" -L"$prefix/lib" -laa -lmikmod $libs -o "$root/$exe"
if [ "$target" = windows ]; then $host-strip "$root/$exe"; else strip "$root/$exe"; fi
echo "built $exe"
