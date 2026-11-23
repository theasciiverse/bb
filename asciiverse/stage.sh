#!/bin/sh
# Lays out the av archive in stage/: BB with its three songs beside it
# (it looks for them beside its own program file, following av's link),
# the licences, README.md and PATCHES.md.
#   sh asciiverse/stage.sh [bb.exe]     (the Windows build)
set -eu
cd "$(dirname "$0")/.."
rm -rf stage && mkdir -p stage
cp "${1:-bb}" stage/
cp bb.s3m bb2.s3m bb3.s3m stage/
cp COPYING AUTHORS README.md PATCHES.md stage/
cp aalib/COPYING stage/COPYING.AAlib
mm=$(ls -d av-build/*/libmikmod-* | head -n 1)
cp "$mm/COPYING.LIB" stage/COPYING.MikMod
