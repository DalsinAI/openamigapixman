#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
STOVE=${STOVE:-"$HOME/AmigaChrome/stoves/os32-gcc16"}
CC=${CC:-"$STOVE/prefix/bin/m68k-amigaos-gcc"}
mkdir -p "$HERE/out/tests"
"$CC" -m68040 -m68881 -O2 -Wall -Wextra -Werror -noixemul \
  -I"$HERE/include" -I"$HERE/out/include" -I"$HERE/out/include/pixman-1" \
  -o "$HERE/out/tests/PixBench" "$HERE/bench/pixbench.c"
echo "$HERE/out/tests/PixBench ($(wc -c < "$HERE/out/tests/PixBench") bytes)"
sha256sum "$HERE/out/tests/PixBench"
