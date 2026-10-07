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

## Tests

`out/tests/PixmanTest` is the ABI smoke test. `out/tests/PixBench` is the correctness and performance harness and uses AmigaOS `timer.device` E-clock timing.

The first AC090-native 68040-profile run is preserved in `measurements/20261007-b0.1-ac090-68040.txt`. It exposed an upstream limitation important to m68k: raw `pixman_blt()` has architecture-specific backends but no generic portable-C BLT implementation. `pixman.library` therefore supplies an overlap-safe portable fallback, which also gives AC090 a clean future acceleration hook.

## Browser

See `BROWSER_ACCELERATION_DESIGN.md`. The design rule is simple:

> WebKit describes what must be painted; OpenAmigaPixman decides the fastest correct way to paint it.

The first measurements show that simple fill/copy paths are already cheap relative to generic alpha composition. The highest-value optimisation targets are ARGB OVER, A8-mask OVER and glyph/text composition.

## Licensing

OpenAmigaPixman glue, library ABI and test code are MIT licensed by Dalsin Limited. Upstream Pixman retains its own MIT licence and copyright notices.
