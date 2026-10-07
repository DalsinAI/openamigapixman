/* pixman.library 1.0 smoke test. Public domain test fixture. */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <stdio.h>
#include <string.h>
#include <proto/pixman.h>

struct Library *PixmanBase;

static uint32_t src_bits[16 * 16];
static uint32_t dst_bits[16 * 16];

static ULONG checksum(const uint32_t *p, ULONG n)
{
    ULONG h = 2166136261UL;
    while (n--) {
        ULONG v = *p++;
        h ^= v; h *= 16777619UL;
    }
    return h;
}

int main(void)
{
    struct PXFill fill;
    struct PXCreateBits src_req, dst_req;
    struct PXComposite32 comp;
    pixman_image_t *src, *dst;
    pixman_region32_t a, b, u;
    ULONG sum;
    BOOL ok;

    PixmanBase = OpenLibrary(PIXMANLIB_NAME, 1);
    if (!PixmanBase) {
        puts("FAIL open pixman.library");
        return 20;
    }

    printf("PIXMANLIB %lu upstream=%s\n",
           (unsigned long)PX_Version(), PX_VersionString());

    memset(src_bits, 0, sizeof(src_bits));
    memset(dst_bits, 0, sizeof(dst_bits));

    fill.bits = src_bits;
    fill.stride = 16;
    fill.bpp = 32;
    fill.x = 0; fill.y = 0; fill.width = 16; fill.height = 16;
    fill.xor_value = 0xff336699UL;
    ok = PX_Fill(&fill);
    if (!ok) {
        puts("FAIL fill");
        CloseLibrary(PixmanBase);
        return 20;
    }

    src_req.format = PIXMAN_a8r8g8b8;
    src_req.width = 16; src_req.height = 16;
    src_req.bits = src_bits; src_req.rowstride_bytes = 16 * 4;
    dst_req = src_req;
    dst_req.bits = dst_bits;

    src = PX_ImageCreateBits(&src_req);
    dst = PX_ImageCreateBits(&dst_req);
    if (!src || !dst) {
        puts("FAIL image create");
        if (src) PX_ImageUnref(src);
        if (dst) PX_ImageUnref(dst);
        CloseLibrary(PixmanBase);
        return 20;
    }

    comp.op = PIXMAN_OP_SRC;
    comp.src = src; comp.mask = NULL; comp.dest = dst;
    comp.src_x = comp.src_y = 0;
    comp.mask_x = comp.mask_y = 0;
    comp.dest_x = comp.dest_y = 0;
    comp.width = comp.height = 16;
    PX_Composite32(&comp);

    sum = checksum(dst_bits, 16 * 16);
    printf("COMPOSITE sum=%08lx first=%08lx\n",
           (unsigned long)sum, (unsigned long)dst_bits[0]);

    PX_Region32InitRect(&a, 0, 0, 10, 10);
    PX_Region32InitRect(&b, 5, 5, 10, 10);
    PX_Region32Init(&u);
    ok = PX_Region32Union(&u, &a, &b);
    printf("REGION union=%ld\n",
           (long)ok);

    PX_Region32Fini(&u);
    PX_Region32Fini(&b);
    PX_Region32Fini(&a);
    PX_ImageUnref(dst);
    PX_ImageUnref(src);
    CloseLibrary(PixmanBase);

    if (dst_bits[0] != 0xff336699UL) {
        puts("FAIL composite pixel");
        return 20;
    }

    puts("PASS pixman.library");
    return 0;
}
