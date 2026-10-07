#!/bin/sh
# Build upstream Pixman for AmigaOS 3.x as libpixman-1.a.
# 68040 + FPU baseline.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
STOVE=${STOVE:-"$HOME/AmigaChrome/stoves/os32-gcc16"}
P="$STOVE/prefix"
CC=${CC:-"$P/bin/m68k-amigaos-gcc"}
AR=${AR:-"$P/bin/m68k-amigaos-ar"}
CPU=${CPU:-"-m68040 -m68881 -mcrt=nix20"}
CFLAGS="${CFLAGS:-"-O2"} $CPU -D_DEFAULT_SOURCE=1 -DPIXMAN_NO_TLS=1 -DNDEBUG -fno-common -Wall -Wextra -Wno-unused-parameter"
TARBALL="$HERE/tarballs/pixman-0.46.4.tar.gz"
WORK="$HERE/work/pixman-0.46.4"
OUT=${OUT:-"$HERE/out"}
OBJ="$HERE/work/obj-pixman"

[ -f "$TARBALL" ] || "$HERE/fetch-sources.sh"
echo "d09c44ebc3bd5bee7021c79f922fe8fb2fb57f7320f55e97ff9914d2346a591c  $TARBALL" | sha256sum -c - >/dev/null
[ -d "$WORK/pixman" ] || { rm -rf "$WORK"; mkdir -p "$HERE/work"; tar xzf "$TARBALL" -C "$HERE/work"; }
mkdir -p "$OUT/lib" "$OUT/include/pixman-1" "$OBJ"
rm -f "$OBJ"/*.o

sed -e 's/@PIXMAN_VERSION_MAJOR@/0/; s/@PIXMAN_VERSION_MINOR@/46/; s/@PIXMAN_VERSION_MICRO@/4/' \
  "$WORK/pixman/pixman-version.h.in" > "$HERE/config/pixman/pixman-version.h"

SOURCES="
pixman.c
pixman-access.c
pixman-access-accessors.c
pixman-arm.c
pixman-bits-image.c
pixman-combine32.c
pixman-combine-float.c
pixman-conical-gradient.c
pixman-edge.c
pixman-edge-accessors.c
pixman-fast-path.c
pixman-filter.c
pixman-glyph.c
pixman-general.c
pixman-gradient-walker.c
pixman-image.c
pixman-implementation.c
pixman-linear-gradient.c
pixman-matrix.c
pixman-mips.c
pixman-noop.c
pixman-ppc.c
pixman-radial-gradient.c
pixman-region16.c
pixman-region32.c
pixman-region64f.c
pixman-riscv.c
pixman-solid-fill.c
pixman-timer.c
pixman-trap.c
pixman-utils.c
pixman-x86.c
"

for s in $SOURCES; do
  o="$OBJ/${s%.c}.o"
  "$CC" $CFLAGS -DHAVE_CONFIG_H -I"$HERE/config/pixman" -I"$WORK/pixman" -c "$WORK/pixman/$s" -o "$o"
done
rm -f "$OUT/lib/libpixman-1.a"
"$AR" rcs "$OUT/lib/libpixman-1.a" "$OBJ"/*.o
cp "$WORK/pixman/pixman.h" "$HERE/config/pixman/pixman-version.h" "$OUT/include/pixman-1/"
echo "$OUT/lib/libpixman-1.a ($(wc -c < "$OUT/lib/libpixman-1.a") bytes)"
