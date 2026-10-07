/* Copyright (c) 2026 Dalsin Limited.
 * pixman.library 1.0 for AmigaOS 3.x.
 * Upstream Pixman 0.46.4 remains MIT licensed by its authors.
 *
 * ABI rule: callers use request blocks for wide/hot calls. This keeps the
 * library vector ABI stable while the implementation underneath can move
 * from portable 68040+FPU C to hand-tuned 040/FPU, OpenGPU or AC090 magic paths.
 */
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <proto/exec.h>

#include "../include/libraries/pixman.h"

#define REG(r, decl) register decl __asm(#r)
#define LIB_VERSION 1
#define LIB_REVISION 0

struct PixmanLibBase {
    struct Library lib;
    BPTR seglist;
    struct SignalSemaphore lock;
};

struct ExecBase *SysBase;

int start(void) { return -1; }

static const char lib_name[] = PIXMANLIB_NAME;
static const char lib_id[] =
    "pixman.library 1.0 (7.10.2026) Pixman 0.46.4, OpenAmiga port, Dalsin Limited\r\n";

static struct Library *lib_init(REG(d0, struct PixmanLibBase *base),
                                REG(a0, BPTR seglist),
                                REG(a6, struct ExecBase *sys));
static struct Library *lib_open(REG(a6, struct PixmanLibBase *base));
static BPTR lib_close(REG(a6, struct PixmanLibBase *base));
static BPTR lib_expunge(REG(a6, struct PixmanLibBase *base));
static ULONG lib_null(void);

#define LOCK(b) ObtainSemaphore(&(b)->lock)
#define UNLOCK(b) ReleaseSemaphore(&(b)->lock)

static ULONG PX_Version(REG(a6, struct PixmanLibBase *base));
static CONST_STRPTR PX_VersionString(REG(a6, struct PixmanLibBase *base));
static pixman_image_t *PX_ImageCreateBits(REG(a0, struct PXCreateBits *r), REG(a6, struct PixmanLibBase *base));
static pixman_image_t *PX_ImageCreateSolid(REG(a0, const pixman_color_t *c), REG(a6, struct PixmanLibBase *base));
static pixman_image_t *PX_ImageRef(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static BOOL PX_ImageUnref(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static BOOL PX_ImageSetTransform(REG(a0, pixman_image_t *image), REG(a1, const pixman_transform_t *t), REG(a6, struct PixmanLibBase *base));
static void PX_ImageSetRepeat(REG(a0, pixman_image_t *image), REG(d0, ULONG repeat), REG(a6, struct PixmanLibBase *base));
static BOOL PX_ImageSetFilter(REG(a0, struct PXFilter *r), REG(a6, struct PixmanLibBase *base));
static void PX_ImageSetComponentAlpha(REG(a0, pixman_image_t *image), REG(d0, ULONG value), REG(a6, struct PixmanLibBase *base));
static uint32_t *PX_ImageGetData(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static LONG PX_ImageGetWidth(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static LONG PX_ImageGetHeight(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static LONG PX_ImageGetStride(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static ULONG PX_ImageGetFormat(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base));
static void PX_Composite32(REG(a0, struct PXComposite32 *r), REG(a6, struct PixmanLibBase *base));
static BOOL PX_Fill(REG(a0, struct PXFill *r), REG(a6, struct PixmanLibBase *base));
static BOOL PX_Blt(REG(a0, struct PXBlt *r), REG(a6, struct PixmanLibBase *base));
static BOOL PX_ImageFillRectangles(REG(a0, struct PXFillRects *r), REG(a6, struct PixmanLibBase *base));
static void PX_Region32Init(REG(a0, pixman_region32_t *r), REG(a6, struct PixmanLibBase *base));
static void PX_Region32InitRect(REG(a0, pixman_region32_t *r), REG(d0, LONG x), REG(d1, LONG y),
                                REG(d2, ULONG w), REG(d3, ULONG h), REG(a6, struct PixmanLibBase *base));
static void PX_Region32Fini(REG(a0, pixman_region32_t *r), REG(a6, struct PixmanLibBase *base));
static BOOL PX_Region32Copy(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *src), REG(a6, struct PixmanLibBase *base));
static BOOL PX_Region32Intersect(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *a),
                                 REG(a2, const pixman_region32_t *b), REG(a6, struct PixmanLibBase *base));
static BOOL PX_Region32Union(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *a),
                             REG(a2, const pixman_region32_t *b), REG(a6, struct PixmanLibBase *base));

static const APTR lib_vectors[] = {
    (APTR)lib_open, (APTR)lib_close, (APTR)lib_expunge, (APTR)lib_null,
    (APTR)PX_Version, (APTR)PX_VersionString,
    (APTR)PX_ImageCreateBits, (APTR)PX_ImageCreateSolid,
    (APTR)PX_ImageRef, (APTR)PX_ImageUnref,
    (APTR)PX_ImageSetTransform, (APTR)PX_ImageSetRepeat, (APTR)PX_ImageSetFilter,
    (APTR)PX_ImageSetComponentAlpha,
    (APTR)PX_ImageGetData, (APTR)PX_ImageGetWidth, (APTR)PX_ImageGetHeight,
    (APTR)PX_ImageGetStride, (APTR)PX_ImageGetFormat,
    (APTR)PX_Composite32, (APTR)PX_Fill, (APTR)PX_Blt,
    (APTR)PX_ImageFillRectangles,
    (APTR)PX_Region32Init, (APTR)PX_Region32InitRect, (APTR)PX_Region32Fini,
    (APTR)PX_Region32Copy, (APTR)PX_Region32Intersect, (APTR)PX_Region32Union,
    (APTR)-1
};

static const struct {
    ULONG size;
    const APTR *vectors;
    APTR data;
    APTR init;
} lib_inittable = {
    sizeof(struct PixmanLibBase), lib_vectors, NULL, (APTR)lib_init
};

const struct Resident lib_romtag = {
    RTC_MATCHWORD, (struct Resident *)&lib_romtag, (APTR)(&lib_romtag + 1),
    RTF_AUTOINIT, LIB_VERSION, NT_LIBRARY, 0,
    (char *)lib_name, (char *)lib_id, (APTR)&lib_inittable
};

static struct Library *lib_init(REG(d0, struct PixmanLibBase *base),
                                REG(a0, BPTR seglist),
                                REG(a6, struct ExecBase *sys))
{
    SysBase = sys;
    base->seglist = seglist;
    base->lib.lib_Revision = LIB_REVISION;
    InitSemaphore(&base->lock);
    return &base->lib;
}

static struct Library *lib_open(REG(a6, struct PixmanLibBase *base))
{
    base->lib.lib_OpenCnt++;
    base->lib.lib_Flags &= ~LIBF_DELEXP;
    return &base->lib;
}

static BPTR lib_close(REG(a6, struct PixmanLibBase *base))
{
    base->lib.lib_OpenCnt--;
    if (base->lib.lib_OpenCnt == 0 && (base->lib.lib_Flags & LIBF_DELEXP))
        return lib_expunge(base);
    return 0;
}

static BPTR lib_expunge(REG(a6, struct PixmanLibBase *base))
{
    BPTR seglist;
    if (base->lib.lib_OpenCnt) {
        base->lib.lib_Flags |= LIBF_DELEXP;
        return 0;
    }
    seglist = base->seglist;
    Remove(&base->lib.lib_Node);
    FreeMem((UBYTE *)base - base->lib.lib_NegSize,
            base->lib.lib_NegSize + base->lib.lib_PosSize);
    return seglist;
}

static ULONG lib_null(void) { return 0; }

static ULONG PX_Version(REG(a6, struct PixmanLibBase *base))
{
    (void)base;
    return (ULONG)pixman_version();
}

static CONST_STRPTR PX_VersionString(REG(a6, struct PixmanLibBase *base))
{
    (void)base;
    return (CONST_STRPTR)pixman_version_string();
}

static pixman_image_t *PX_ImageCreateBits(REG(a0, struct PXCreateBits *r), REG(a6, struct PixmanLibBase *base))
{
    pixman_image_t *out = NULL;
    if (!r) return NULL;
    LOCK(base);
    out = pixman_image_create_bits(r->format, r->width, r->height, r->bits, r->rowstride_bytes);
    UNLOCK(base);
    return out;
}

static pixman_image_t *PX_ImageCreateSolid(REG(a0, const pixman_color_t *c), REG(a6, struct PixmanLibBase *base))
{
    pixman_image_t *out = NULL;
    if (!c) return NULL;
    LOCK(base);
    out = pixman_image_create_solid_fill(c);
    UNLOCK(base);
    return out;
}

static pixman_image_t *PX_ImageRef(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    pixman_image_t *out;
    if (!image) return NULL;
    LOCK(base); out = pixman_image_ref(image); UNLOCK(base);
    return out;
}

static BOOL PX_ImageUnref(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!image) return FALSE;
    LOCK(base); out = pixman_image_unref(image); UNLOCK(base);
    return out ? TRUE : FALSE;
}

static BOOL PX_ImageSetTransform(REG(a0, pixman_image_t *image), REG(a1, const pixman_transform_t *t), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!image) return FALSE;
    LOCK(base); out = pixman_image_set_transform(image, t); UNLOCK(base);
    return out ? TRUE : FALSE;
}

static void PX_ImageSetRepeat(REG(a0, pixman_image_t *image), REG(d0, ULONG repeat), REG(a6, struct PixmanLibBase *base))
{
    if (!image) return;
    LOCK(base); pixman_image_set_repeat(image, (pixman_repeat_t)repeat); UNLOCK(base);
}

static BOOL PX_ImageSetFilter(REG(a0, struct PXFilter *r), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!r || !r->image) return FALSE;
    LOCK(base);
    out = pixman_image_set_filter(r->image, r->filter, r->params, r->n_params);
    UNLOCK(base);
    return out ? TRUE : FALSE;
}

static void PX_ImageSetComponentAlpha(REG(a0, pixman_image_t *image), REG(d0, ULONG value), REG(a6, struct PixmanLibBase *base))
{
    if (!image) return;
    LOCK(base); pixman_image_set_component_alpha(image, value ? 1 : 0); UNLOCK(base);
}

static uint32_t *PX_ImageGetData(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    uint32_t *out;
    if (!image) return NULL;
    LOCK(base); out = pixman_image_get_data(image); UNLOCK(base);
    return out;
}

static LONG PX_ImageGetWidth(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    LONG out;
    if (!image) return 0;
    LOCK(base); out = pixman_image_get_width(image); UNLOCK(base);
    return out;
}

static LONG PX_ImageGetHeight(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    LONG out;
    if (!image) return 0;
    LOCK(base); out = pixman_image_get_height(image); UNLOCK(base);
    return out;
}

static LONG PX_ImageGetStride(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    LONG out;
    if (!image) return 0;
    LOCK(base); out = pixman_image_get_stride(image); UNLOCK(base);
    return out;
}

static ULONG PX_ImageGetFormat(REG(a0, pixman_image_t *image), REG(a6, struct PixmanLibBase *base))
{
    ULONG out;
    if (!image) return 0;
    LOCK(base); out = (ULONG)pixman_image_get_format(image); UNLOCK(base);
    return out;
}

static void PX_Composite32(REG(a0, struct PXComposite32 *r), REG(a6, struct PixmanLibBase *base))
{
    if (!r || !r->src || !r->dest || r->width <= 0 || r->height <= 0) return;
    LOCK(base);
    pixman_image_composite32(r->op, r->src, r->mask, r->dest,
        r->src_x, r->src_y, r->mask_x, r->mask_y,
        r->dest_x, r->dest_y, r->width, r->height);
    UNLOCK(base);
}

static BOOL PX_Fill(REG(a0, struct PXFill *r), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!r || !r->bits) return FALSE;
    LOCK(base);
    out = pixman_fill(r->bits, r->stride, r->bpp, r->x, r->y,
                      r->width, r->height, r->xor_value);
    UNLOCK(base);
    return out ? TRUE : FALSE;
}

static void px_copy_overlap(UBYTE *dst, const UBYTE *src, ULONG n)
{
    ULONG i;
    if (dst > src && dst < src + n) {
        for (i = n; i != 0; --i) dst[i - 1] = src[i - 1];
    } else {
        for (i = 0; i < n; ++i) dst[i] = src[i];
    }
}

static BOOL px_blt_portable(struct PXBlt *r)
{
    LONG row, first, last, step;
    ULONG bytespp, rowbytes;
    UBYTE *src_base, *dst_base, *src, *dst;

    if (r->src_bpp != r->dst_bpp || r->width <= 0 || r->height <= 0 ||
        r->src_stride <= 0 || r->dst_stride <= 0 ||
        r->src_x < 0 || r->src_y < 0 || r->dest_x < 0 || r->dest_y < 0)
        return FALSE;

    if (r->src_bpp != 8 && r->src_bpp != 16 &&
        r->src_bpp != 24 && r->src_bpp != 32)
        return FALSE;

    bytespp = (ULONG)r->src_bpp >> 3;
    rowbytes = (ULONG)r->width * bytespp;
    src_base = (UBYTE *)r->src_bits;
    dst_base = (UBYTE *)r->dst_bits;

    if (r->src_bits == r->dst_bits && r->dest_y > r->src_y) {
        first = r->height - 1; last = -1; step = -1;
    } else {
        first = 0; last = r->height; step = 1;
    }

    for (row = first; row != last; row += step) {
        src = src_base + ((ULONG)(r->src_y + row) * (ULONG)r->src_stride * 4UL)
                       + (ULONG)r->src_x * bytespp;
        dst = dst_base + ((ULONG)(r->dest_y + row) * (ULONG)r->dst_stride * 4UL)
                       + (ULONG)r->dest_x * bytespp;
        px_copy_overlap(dst, src, rowbytes);
    }
    return TRUE;
}

static BOOL PX_Blt(REG(a0, struct PXBlt *r), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!r || !r->src_bits || !r->dst_bits) return FALSE;
    LOCK(base);
    out = pixman_blt(r->src_bits, r->dst_bits,
        r->src_stride, r->dst_stride, r->src_bpp, r->dst_bpp,
        r->src_x, r->src_y, r->dest_x, r->dest_y, r->width, r->height);
    if (!out)
        out = px_blt_portable(r);
    UNLOCK(base);
    return out ? TRUE : FALSE;
}

static BOOL PX_ImageFillRectangles(REG(a0, struct PXFillRects *r), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!r || !r->image || !r->color || r->n_rects <= 0 || !r->rects) return FALSE;
    LOCK(base);
    out = pixman_image_fill_rectangles(r->op, r->image, r->color, r->n_rects, r->rects);
    UNLOCK(base);
    return out ? TRUE : FALSE;
}

static void PX_Region32Init(REG(a0, pixman_region32_t *r), REG(a6, struct PixmanLibBase *base))
{
    if (!r) return;
    LOCK(base); pixman_region32_init(r); UNLOCK(base);
}

static void PX_Region32InitRect(REG(a0, pixman_region32_t *r), REG(d0, LONG x), REG(d1, LONG y),
                                REG(d2, ULONG w), REG(d3, ULONG h), REG(a6, struct PixmanLibBase *base))
{
    if (!r) return;
    LOCK(base); pixman_region32_init_rect(r, x, y, w, h); UNLOCK(base);
}

static void PX_Region32Fini(REG(a0, pixman_region32_t *r), REG(a6, struct PixmanLibBase *base))
{
    if (!r) return;
    LOCK(base); pixman_region32_fini(r); UNLOCK(base);
}

static BOOL PX_Region32Copy(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *src), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!dst || !src) return FALSE;
    LOCK(base); out = pixman_region32_copy(dst, src); UNLOCK(base);
    return out ? TRUE : FALSE;
}

static BOOL PX_Region32Intersect(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *a),
                                 REG(a2, const pixman_region32_t *b), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!dst || !a || !b) return FALSE;
    LOCK(base); out = pixman_region32_intersect(dst, a, b); UNLOCK(base);
    return out ? TRUE : FALSE;
}

static BOOL PX_Region32Union(REG(a0, pixman_region32_t *dst), REG(a1, const pixman_region32_t *a),
                             REG(a2, const pixman_region32_t *b), REG(a6, struct PixmanLibBase *base))
{
    pixman_bool_t out;
    if (!dst || !a || !b) return FALSE;
    LOCK(base); out = pixman_region32_union(dst, a, b); UNLOCK(base);
    return out ? TRUE : FALSE;
}
