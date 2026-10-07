/* pixman.library 0.1 inline calls for bebbo m68k-amigaos-gcc. */
#ifndef INLINE_PIXMAN_H
#define INLINE_PIXMAN_H
#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif
#ifndef PIXMAN_BASE_NAME
#define PIXMAN_BASE_NAME PixmanBase
#endif

#define PX_Version() LP0(0x1e, ULONG, PX_Version, , PIXMAN_BASE_NAME)
#define PX_VersionString() LP0(0x24, CONST_STRPTR, PX_VersionString, , PIXMAN_BASE_NAME)
#define PX_ImageCreateBits(req) LP1(0x2a, pixman_image_t *, PX_ImageCreateBits, struct PXCreateBits *, req, a0, , PIXMAN_BASE_NAME)
#define PX_ImageCreateSolid(color) LP1(0x30, pixman_image_t *, PX_ImageCreateSolid, const pixman_color_t *, color, a0, , PIXMAN_BASE_NAME)
#define PX_ImageRef(image) LP1(0x36, pixman_image_t *, PX_ImageRef, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageUnref(image) LP1(0x3c, BOOL, PX_ImageUnref, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageSetTransform(image, transform) LP2(0x42, BOOL, PX_ImageSetTransform, pixman_image_t *, image, a0, const pixman_transform_t *, transform, a1, , PIXMAN_BASE_NAME)
#define PX_ImageSetRepeat(image, repeat) LP2NR(0x48, PX_ImageSetRepeat, pixman_image_t *, image, a0, ULONG, repeat, d0, , PIXMAN_BASE_NAME)
#define PX_ImageSetFilter(req) LP1(0x4e, BOOL, PX_ImageSetFilter, struct PXFilter *, req, a0, , PIXMAN_BASE_NAME)
#define PX_ImageSetComponentAlpha(image, value) LP2NR(0x54, PX_ImageSetComponentAlpha, pixman_image_t *, image, a0, ULONG, value, d0, , PIXMAN_BASE_NAME)
#define PX_ImageGetData(image) LP1(0x5a, uint32_t *, PX_ImageGetData, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageGetWidth(image) LP1(0x60, LONG, PX_ImageGetWidth, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageGetHeight(image) LP1(0x66, LONG, PX_ImageGetHeight, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageGetStride(image) LP1(0x6c, LONG, PX_ImageGetStride, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_ImageGetFormat(image) LP1(0x72, ULONG, PX_ImageGetFormat, pixman_image_t *, image, a0, , PIXMAN_BASE_NAME)
#define PX_Composite32(req) LP1NR(0x78, PX_Composite32, struct PXComposite32 *, req, a0, , PIXMAN_BASE_NAME)
#define PX_Fill(req) LP1(0x7e, BOOL, PX_Fill, struct PXFill *, req, a0, , PIXMAN_BASE_NAME)
#define PX_Blt(req) LP1(0x84, BOOL, PX_Blt, struct PXBlt *, req, a0, , PIXMAN_BASE_NAME)
#define PX_ImageFillRectangles(req) LP1(0x8a, BOOL, PX_ImageFillRectangles, struct PXFillRects *, req, a0, , PIXMAN_BASE_NAME)
#define PX_Region32Init(region) LP1NR(0x90, PX_Region32Init, pixman_region32_t *, region, a0, , PIXMAN_BASE_NAME)
#define PX_Region32InitRect(region, x, y, w, h) LP5NR(0x96, PX_Region32InitRect, pixman_region32_t *, region, a0, LONG, x, d0, LONG, y, d1, ULONG, w, d2, ULONG, h, d3, , PIXMAN_BASE_NAME)
#define PX_Region32Fini(region) LP1NR(0x9c, PX_Region32Fini, pixman_region32_t *, region, a0, , PIXMAN_BASE_NAME)
#define PX_Region32Copy(dest, src) LP2(0xa2, BOOL, PX_Region32Copy, pixman_region32_t *, dest, a0, const pixman_region32_t *, src, a1, , PIXMAN_BASE_NAME)
#define PX_Region32Intersect(dest, a, b) LP3(0xa8, BOOL, PX_Region32Intersect, pixman_region32_t *, dest, a0, const pixman_region32_t *, a, a1, const pixman_region32_t *, b, a2, , PIXMAN_BASE_NAME)
#define PX_Region32Union(dest, a, b) LP3(0xae, BOOL, PX_Region32Union, pixman_region32_t *, dest, a0, const pixman_region32_t *, a, a1, const pixman_region32_t *, b, a2, , PIXMAN_BASE_NAME)

#endif
