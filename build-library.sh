#!/bin/sh
# Build pixman.library 1.0 for AmigaOS 3.x.
# Baseline: 68040 + FPU.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
STOVE=${STOVE:-"$HOME/AmigaChrome/stoves/os32-gcc16"}
P="$STOVE/prefix"
CC=${CC:-"$P/bin/m68k-amigaos-gcc"}
OUT=${OUT:-"$HERE/out"}

"$HERE/build-static.sh"
mkdir -p "$OUT/lib" "$OUT/include/libraries" "$OUT/include/proto" "$OUT/include/inline"
cp "$HERE/include/libraries/pixman.h" "$OUT/include/libraries/"
cp "$HERE/include/proto/pixman.h" "$OUT/include/proto/"
cp "$HERE/include/inline/pixman.h" "$OUT/include/inline/"

"$CC" -m68040 -m68881 -mcrt=nix20 -O2 -fomit-frame-pointer -fno-toplevel-reorder \
  -fno-builtin -Wall -Wextra -Werror -Wno-unused-parameter \
  -nostartfiles -I"$HERE/include" -I"$OUT/include" -I"$OUT/include/pixman-1" \
  -o "$OUT/lib/pixman.library" "$HERE/library/pixman_lib.c" "$HERE/library/pixman_runtime.c" \
  "$OUT/lib/libpixman-1.a" -lamiga -lm -lgcc \
  -Wl,-Map,"$OUT/lib/pixman.library.map"

echo "$OUT/lib/pixman.library ($(wc -c < "$OUT/lib/pixman.library") bytes)"
