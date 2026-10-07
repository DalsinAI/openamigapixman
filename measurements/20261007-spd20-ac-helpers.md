# SPD-20: AC090's native helpers in pixman.library, off and on

7 October 2026. The same PixBench (harness sha256 8d4676ac...) against pixman.library built with AC_HELPERS=0 and AC_HELPERS=1 (os32-gcc16 stove), on an AmigaChrome scratch instance: AmigaOS 3.2.3, AC090 as a 68040 with FPU, pace hyper. Four boots, off, on, off, on, two runs each; the library's folder assigned into LIBS: for each boot. Every check passed and every run drew the same hashes, off and on.

```
test               helpers off    helpers on   change   range off / on
fill32                    91.2          91.8    +0.7%   89-96 / 89-92
blt32                    231.5         218.5    -5.6%   214-251 / 214-223
src32                    102.4          80.8   -21.1%   99-105 / 80-91
over32               1047857.4     1048279.6    +0.0%   1047105-1049057 / 1045808-1063048
mask_a8_over32        549475.7      581811.7    +5.9%   518793-641384 / 519340-656842
fillrects24              126.2         124.8    -1.1%   125-156 / 121-151
paint640x480          603960.6      607216.3    +0.5%   599977-689223 / 598860-615027
```

- Copies of whole rows gain: src32 by a fifth, blt32 a little. Fills are unchanged (Pixman's fills do not go through memset).
- The browser's real cost is untouched, as expected: over32 about 1.05 s and mask_a8_over32 about 0.55 s a 640x480 frame, paint640x480 about 0.6 s. That is the next thing to profile (AC_PROFILE=1 during over32).
- Raw logs: 20261007-spd20-ac-helpers-off-ac090-68040.txt and -on-, with their .meta.
