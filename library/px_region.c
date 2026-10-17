/* Copyright (c) 2026 AmigaChrome Team. MIT licence (LICENSE).
 *
 * Pixman's region32 calls (px_core.h) in Pixman's public data layout: the
 * extents, then data, which is 0 for one rectangle (the extents) or points
 * at a size and count with that many boxes after them; an empty region has
 * a count of 0. The boxes are y-x banded: sorted by y, then x; the boxes of
 * a band share y1 and y2 and never touch; two bands next to each other
 * never have the same x spans (they would be one band).
 *
 * Union and intersection both work band by band: the region is cut at
 * every y where a box of either side starts or ends, each band's spans are
 * merged (union) or overlapped (intersection), and equal neighbouring bands
 * are joined. This is the Team's own method, written from that layout. */
#include "px_core.h"

static pixman_region32_data_t px_empty = { 0, 0 };

static const pixman_box32_t *boxes(const pixman_region32_t *r, long *n) {
    if (!r->data) { *n = r->extents.x2 > r->extents.x1 && r->extents.y2 > r->extents.y1; return &r->extents; }
    *n = r->data->numRects;
    return (const pixman_box32_t *)(r->data + 1);
}

static void set_empty(pixman_region32_t *r) {
    r->extents.x1 = r->extents.y1 = r->extents.x2 = r->extents.y2 = 0;
    r->data = &px_empty;
}

void px_region32_init(pixman_region32_t *r) { set_empty(r); }

void px_region32_init_rect(pixman_region32_t *r, int x, int y, unsigned w, unsigned h) {
    if (!w || !h) { set_empty(r); return; }
    r->extents.x1 = x; r->extents.y1 = y;
    r->extents.x2 = x + (int)w; r->extents.y2 = y + (int)h;
    r->data = 0;
}

static void drop(const struct px_env *e, pixman_region32_t *r) {
    if (r->data && r->data != &px_empty && r->data->size) e->free(e->ctx, r->data);
}

void px_region32_fini(const struct px_env *e, pixman_region32_t *r) { drop(e, r); }

/* boxes into a region: 0 when memory runs out (dst is then empty) */
static int set_boxes(const struct px_env *e, pixman_region32_t *dst, const pixman_box32_t *b, long n) {
    long i;
    pixman_region32_data_t *d;
    pixman_box32_t *o;
    drop(e, dst);
    if (n == 0) { set_empty(dst); return 1; }
    dst->extents = b[0];
    for (i = 1; i < n; i++) {
        if (b[i].x1 < dst->extents.x1) dst->extents.x1 = b[i].x1;
        if (b[i].x2 > dst->extents.x2) dst->extents.x2 = b[i].x2;
        if (b[i].y2 > dst->extents.y2) dst->extents.y2 = b[i].y2;
    }
    if (n == 1) { dst->data = 0; return 1; }
    if (!(d = e->alloc(e->ctx, sizeof *d + (unsigned long)n * sizeof *b))) { set_empty(dst); return 0; }
    d->size = n; d->numRects = n;
    o = (pixman_box32_t *)(d + 1);
    for (i = 0; i < n; i++) o[i] = b[i];
    dst->data = d;
    return 1;
}

int px_region32_copy(const struct px_env *e, pixman_region32_t *dst, const pixman_region32_t *src) {
    long n;
    const pixman_box32_t *b;
    if (dst == src) return 1;
    b = boxes(src, &n);
    if (!src->data) { drop(e, dst); *dst = *src; return 1; }
    return set_boxes(e, dst, b, n);
}

/* The x spans of region r's band that covers y0..y1 (r's boxes either cover
 * it whole or not at all, since the band edges include all of r's). */
static long spans_at(const pixman_box32_t *b, long n, int y0, int *xs, long cap) {
    long i, k = 0;
    for (i = 0; i < n; i++)
        if (b[i].y1 <= y0 && b[i].y2 > y0 && k + 2 <= cap) { xs[k++] = b[i].x1; xs[k++] = b[i].x2; }
    return k;
}

/* the sorted, distinct y edges of both regions */
static long edges(const pixman_box32_t *a, long na, const pixman_box32_t *b, long nb, int *ys) {
    long i, j, k = 0;
    for (i = 0; i < na; i++) { ys[k++] = a[i].y1; ys[k++] = a[i].y2; }
    for (i = 0; i < nb; i++) { ys[k++] = b[i].y1; ys[k++] = b[i].y2; }
    for (i = 1; i < k; i++) {                       /* insertion sort: regions here are small */
        int v = ys[i];
        for (j = i - 1; j >= 0 && ys[j] > v; j--) ys[j + 1] = ys[j];
        ys[j + 1] = v;
    }
    for (i = 0, j = 0; i < k; i++) if (!j || ys[j - 1] != ys[i]) ys[j++] = ys[i];
    return j;
}

static int combine(const struct px_env *e, pixman_region32_t *dst, const pixman_region32_t *ra,
                   const pixman_region32_t *rb, int unite) {
    long na, nb, ne, i, nout = 0, cap, prev_start = -1, prev_n = 0;
    const pixman_box32_t *a = boxes(ra, &na), *b = boxes(rb, &nb);
    int *ys, *sa, *sb, *out_x;
    pixman_box32_t *out;
    int ok;
    if (!unite && (!na || !nb)) { drop(e, dst); set_empty(dst); return 1; }
    cap = 2 * (na + nb) + 2;
    ys = e->alloc(e->ctx, (unsigned long)cap * sizeof *ys);
    sa = e->alloc(e->ctx, (unsigned long)(2 * na + 2) * sizeof *sa);
    sb = e->alloc(e->ctx, (unsigned long)(2 * nb + 2) * sizeof *sb);
    out_x = e->alloc(e->ctx, (unsigned long)(2 * (na + nb) + 2) * sizeof *out_x);
    /* at most (bands) x (spans a band) boxes */
    out = e->alloc(e->ctx, (unsigned long)(cap * (na + nb + 1)) * sizeof *out);
    if (!ys || !sa || !sb || !out_x || !out) {
        if (ys) e->free(e->ctx, ys);
        if (sa) e->free(e->ctx, sa);
        if (sb) e->free(e->ctx, sb);
        if (out_x) e->free(e->ctx, out_x);
        if (out) e->free(e->ctx, out);
        return 0;
    }
    ne = edges(a, na, b, nb, ys);
    for (i = 0; i + 1 < ne; i++) {
        int y0 = ys[i], y1 = ys[i + 1];
        long ka = spans_at(a, na, y0, sa, 2 * na + 2), kb = spans_at(b, nb, y0, sb, 2 * nb + 2), ia = 0, ib = 0, k = 0, j;
        /* the band's spans: both sorted already (a band's boxes are in x order) */
        if (unite) {
            while (ia < ka || ib < kb) {
                int x0, x1;
                if (ib >= kb || (ia < ka && sa[ia] <= sb[ib])) { x0 = sa[ia]; x1 = sa[ia + 1]; ia += 2; }
                else { x0 = sb[ib]; x1 = sb[ib + 1]; ib += 2; }
                if (k && out_x[k - 1] >= x0) { if (x1 > out_x[k - 1]) out_x[k - 1] = x1; }
                else { out_x[k++] = x0; out_x[k++] = x1; }
            }
        } else {
            while (ia < ka && ib < kb) {
                int x0 = sa[ia] > sb[ib] ? sa[ia] : sb[ib], x1 = sa[ia + 1] < sb[ib + 1] ? sa[ia + 1] : sb[ib + 1];
                if (x0 < x1) { out_x[k++] = x0; out_x[k++] = x1; }
                if (sa[ia + 1] < sb[ib + 1]) ia += 2; else ib += 2;
            }
        }
        if (!k) { prev_start = -1; continue; }
        /* the same spans as the band just above, touching it: one band */
        if (prev_start >= 0 && prev_n == k / 2 && out[prev_start].y2 == y0) {
            for (j = 0; j < k / 2; j++)
                if (out[prev_start + j].x1 != out_x[2 * j] || out[prev_start + j].x2 != out_x[2 * j + 1]) break;
            if (j == k / 2) {
                for (j = 0; j < k / 2; j++) out[prev_start + j].y2 = y1;
                continue;
            }
        }
        prev_start = nout; prev_n = k / 2;
        for (j = 0; j < k / 2; j++) {
            out[nout].x1 = out_x[2 * j]; out[nout].x2 = out_x[2 * j + 1];
            out[nout].y1 = y0; out[nout].y2 = y1;
            nout++;
        }
    }
    ok = set_boxes(e, dst, out, nout);
    e->free(e->ctx, ys); e->free(e->ctx, sa); e->free(e->ctx, sb); e->free(e->ctx, out_x); e->free(e->ctx, out);
    return ok;
}

int px_region32_intersect(const struct px_env *e, pixman_region32_t *dst, const pixman_region32_t *a,
                          const pixman_region32_t *b) {
    return combine(e, dst, a, b, 0);
}

int px_region32_union(const struct px_env *e, pixman_region32_t *dst, const pixman_region32_t *a,
                      const pixman_region32_t *b) {
    return combine(e, dst, a, b, 1);
}
