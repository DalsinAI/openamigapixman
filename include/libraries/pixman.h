/* Copyright (c) 2026 Dalsin Limited.
 * Amiga pixman.library ABI. Upstream Pixman keeps its own MIT licence. */
#ifndef LIBRARIES_PIXMAN_H
#define LIBRARIES_PIXMAN_H

#include <exec/types.h>
#include <pixman-1/pixman.h>

#define PIXMANLIB_NAME "pixman.library"
#define PIXMANLIB_VERSION 0
#define PIXMANLIB_REVISION 1

struct PXCreateBits {
    pixman_format_code_t format;
    LONG width;
    LONG height;
    uint32_t *bits;
    LONG rowstride_bytes;
};

struct PXComposite32 {
    pixman_op_t op;
    pixman_image_t *src;
    pixman_image_t *mask;
    pixman_image_t *dest;
    LONG src_x, src_y;
    LONG mask_x, mask_y;
    LONG dest_x, dest_y;
    LONG width, height;
};

struct PXFill {
    uint32_t *bits;
    LONG stride;
    LONG bpp;
    LONG x, y, width, height;
    uint32_t xor_value;
};

struct PXBlt {
    uint32_t *src_bits;
    uint32_t *dst_bits;
    LONG src_stride;
    LONG dst_stride;
    LONG src_bpp;
    LONG dst_bpp;
    LONG src_x, src_y;
    LONG dest_x, dest_y;
    LONG width, height;
};

struct PXFilter {
    pixman_image_t *image;
    pixman_filter_t filter;
    const pixman_fixed_t *params;
    LONG n_params;
};

struct PXFillRects {
    pixman_op_t op;
    pixman_image_t *image;
    const pixman_color_t *color;
    LONG n_rects;
    const pixman_rectangle16_t *rects;
};

#endif
