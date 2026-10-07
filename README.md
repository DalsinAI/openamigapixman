# OpenAmigaPixman

AmigaOS 3.x packaging and acceleration boundary for upstream Pixman, targeting AmigaChrome's 68040 + FPU machine profile.

The project keeps upstream Pixman semantics as the correctness reference while exposing a compact `pixman.library` ABI for Amiga software. The same ABI is intended to host progressively faster 68040, AC090/AC_MAGIC and OpenGPU implementations without pushing machine-specific acceleration into WebKit or applications.

## Baseline

- Upstream Pixman: 0.46.4
- Target: m68k AmigaOS 3.x
- CPU baseline: 68040 + FPU
- Endianness: big-endian
- TLS: disabled
- Library version: 0.1

## Build

```sh
./fetch-sources.sh
./build-library.sh
./build-test.sh
./build-bench.sh
```

`build-library.sh` builds the upstream static library and then links `out/lib/pixman.library`. Generated outputs, fetched tarballs and unpacked upstream sources are intentionally excluded from Git.

## AC090's native helpers (SPD-20)

`AC_HELPERS=1` routes copies and fills through amigachrome-guest's `common/amiga/ac_helpers`, as openamigaimage's build does. These are magic functions: AmigaChrome's AC090 runs them as host code, and a real Amiga runs them as 68k code written for the 68020 and 68040. `AC_HELPERS=0` builds as before. The default, `auto`, turns them on when the commit in `AMIGACHROME_GUEST_PINNED_COMMIT` is to hand, from `../amigachrome-guest`, `../guest` or `AMIGACHROME_GUEST`, and says which.

What goes through them:
- **Upstream Pixman's own calls:** `achelpers/ac_string.h` is forced into every compile, so `memcpy`, `memmove` and `memset` calls of 64 bytes or more go to the tagged functions. Smaller ones stay GCC's own code, or `pixman_runtime.c`'s.
- **`pixman.library`'s own copies:** `pixman_blt`'s portable fallback (one `memmove` a row), `calloc` and `realloc` use the helpers too. `libachelpers.a` is linked after `libpixman-1.a`.
- **`pixman_runtime.c`'s small functions:** its `memcpy`, `memset` and the new `memmove` stay plain byte loops. The helpers' front doors hand them everything under 64 bytes, so they are never built on the helpers themselves.

The helpers and their tags pass amigachrome-guest's `tests/m68k` with GCC 16.2 (m68k-linux, qemu-m68k: 68000, 68020 and 68040 code, -O0 and -O2). `achelpers/achelpers.sh` builds them with the m68k toolchain (7 October 2026). `pixman.library` with them has not yet been built with os32-gcc16 or run on AmigaChrome.

## Tests

`out/tests/PixmanTest` is the ABI smoke test. `out/tests/PixBench` is the correctness and performance harness and uses AmigaOS `timer.device` E-clock timing.

The first AC090-native 68040-profile run is preserved in `measurements/20261007-b0.1-ac090-68040.txt`. It exposed an upstream limitation important to m68k: raw `pixman_blt()` has architecture-specific backends but no generic portable-C BLT implementation. `pixman.library` therefore supplies an overlap-safe portable fallback, which also gives AC090 a clean future acceleration hook.

## Browser

See `BROWSER_ACCELERATION_DESIGN.md`. The design rule is simple:

> WebKit describes what must be painted; OpenAmigaPixman decides the fastest correct way to paint it.

The first measurements show that simple fill/copy paths are already cheap relative to generic alpha composition. The highest-value optimisation targets are ARGB OVER, A8-mask OVER and glyph/text composition.

## Licensing

OpenAmigaPixman glue, library ABI and test code are MIT licensed by Dalsin Limited. Upstream Pixman retains its own MIT licence and copyright notices.
