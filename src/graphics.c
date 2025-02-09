// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * This module renders the contents of the VM memory buffer into a 7 channel
 * image. It supports zooming in and out, condensing the state of several
 * bytes of memory into single pixels, when zoomed out. When zoomed in, each
 * pixel represents a single byte in memory.
 */

u64 g_gfx_vsiz;     // zoom level

u64 *g_gfx_inst;    // instruction channel
u64 *g_gfx_mall;    // allocated state channel
u64 *g_gfx_mbst;    // memory block start channel
u64 *g_gfx_mb0s;    // selected organism's memory block #1 channel
u64 *g_gfx_mb1s;    // selected organism's memory block #2 channel
u64 *g_gfx_ipas;    // selected organism's IP channel
u64 *g_gfx_spas;    // selected organism's SP channel

void gfx_init(u64 vsiz) {
    assert(vsiz);

    g_gfx_vsiz = vsiz;

    g_gfx_inst = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_mall = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_mbst = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_mb0s = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_mb1s = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_ipas = calloc(g_gfx_vsiz, sizeof(u64));
    g_gfx_spas = calloc(g_gfx_vsiz, sizeof(u64));

    assert(g_gfx_inst);
    assert(g_gfx_mall);
    assert(g_gfx_mbst);
    assert(g_gfx_mb0s);
    assert(g_gfx_mb1s);
    assert(g_gfx_ipas);
    assert(g_gfx_spas);
}

void gfx_free() {
    if (g_gfx_vsiz == 0) {
        return;
    }

    assert(g_gfx_inst);
    assert(g_gfx_mall);
    assert(g_gfx_mbst);
    assert(g_gfx_mb0s);
    assert(g_gfx_mb1s);
    assert(g_gfx_ipas);
    assert(g_gfx_spas);

    g_gfx_vsiz = 0;

    free(g_gfx_inst);
    free(g_gfx_mall);
    free(g_gfx_mbst);
    free(g_gfx_mb0s);
    free(g_gfx_mb1s);
    free(g_gfx_ipas);
    free(g_gfx_spas);

    g_gfx_inst = NULL;
    g_gfx_mall = NULL;
    g_gfx_mbst = NULL;
    g_gfx_mb0s = NULL;
    g_gfx_mb1s = NULL;
    g_gfx_ipas = NULL;
    g_gfx_spas = NULL;
}

void gfx_resize(u64 vsiz) {
    assert(vsiz);

    gfx_free();
    gfx_init(vsiz);
}

void gfx_render_inst(const Core *core, u64 pos, u64 zoom) {
    assert(core);

    for (u64 i = 0; i < g_gfx_vsiz; ++i) {
        g_gfx_inst[i] = 0;
        g_gfx_mall[i] = 0;

        for (u64 j = 0; j < zoom; ++j) {
            u64 addr = pos + (i * zoom) + j;

            g_gfx_inst[i] += mvec_get_byte(core, addr);
            g_gfx_mall[i] += mvec_is_alloc(core, addr) ? 1 : 0;
        }
    }
}

void gfx_clear_array(u64 *arry) {
    assert(arry);
    memset(arry, 0, g_gfx_vsiz * sizeof(u64));
}

#ifdef MVEC_LOOP
void gfx_accumulate_pixel(u64 pos, u64 zoom, u64 pixa, u64 *arry) {
    assert(arry);

    u64 beg_mod = pos % MVEC_SIZE;
    u64 end_mod = beg_mod + (g_gfx_vsiz * zoom);
    u64 pix_mod = pixa % MVEC_SIZE;

#ifndef NDEBUG
    u64 inc_cnt = 0;
#endif

    while (pix_mod < end_mod) {
        if (pix_mod >= beg_mod && pix_mod < end_mod) {
            u64 pixi = (pix_mod - beg_mod) / zoom;
            assert(pixi < g_gfx_vsiz);
            arry[pixi]++;

#ifndef NDEBUG
            inc_cnt++;
#endif
        }

        pix_mod += MVEC_SIZE;
    }


#ifndef NDEBUG
    if (zoom != 1) {
        assert(inc_cnt <= 2);
    }
#endif
}
#else
void gfx_accumulate_pixel(u64 pos, u64 zoom, u64 pixa, u64 *arry) {
    assert(arry);

    u64 end = pos + (g_gfx_vsiz * zoom);

    if (pixa < pos || pixa >= end) {
        return;
    }

    u64 pixi = (pixa - pos) / zoom;
    assert(pixi < g_gfx_vsiz);
    arry[pixi]++;
}
#endif

void gfx_render_mbst(const Core *core, u64 pos, u64 zoom) {
    assert(core);

    gfx_clear_array(g_gfx_mbst);

    for (u64 pix = core->pfst; pix <= core->plst; ++pix) {
        u64 mb0a = arch_proc_mb0_addr(core, pix);
        u64 mb1a = arch_proc_mb1_addr(core, pix);

        gfx_accumulate_pixel(pos, zoom, mb0a, g_gfx_mbst);
        gfx_accumulate_pixel(pos, zoom, mb1a, g_gfx_mbst);
    }
}

void gfx_render_mb0s(const Core *core, u64 pos, u64 zoom, u64 psel) {
    assert(core);

    gfx_clear_array(g_gfx_mb0s);

    if (psel < core->pfst || psel > core->plst) {
        return;
    }

    u64 mb0a = arch_proc_mb0_addr(core, psel);
    u64 mb0s = arch_proc_mb0_size(core, psel);

    for (u64 i = 0; i < mb0s; ++i) {
        gfx_accumulate_pixel(pos, zoom, mb0a + i, g_gfx_mb0s);
    }
}

void gfx_render_mb1s(const Core *core, u64 pos, u64 zoom, u64 psel) {
    assert(core);

    gfx_clear_array(g_gfx_mb1s);

    if (psel < core->pfst || psel > core->plst) {
        return;
    }

    u64 mb1a = arch_proc_mb1_addr(core, psel);
    u64 mb1s = arch_proc_mb1_size(core, psel);

    for (u64 i = 0; i < mb1s; ++i) {
        gfx_accumulate_pixel(pos, zoom, mb1a + i, g_gfx_mb1s);
    }
}

void gfx_render_ipas(const Core *core, u64 pos, u64 zoom, u64 psel) {
    assert(core);

    gfx_clear_array(g_gfx_ipas);

    if (psel < core->pfst || psel > core->plst) {
        return;
    }

    u64 ipa = arch_proc_ip_addr(core, psel);

    gfx_accumulate_pixel(pos, zoom, ipa, g_gfx_ipas);
}

void gfx_render_spas(const Core *core, u64 pos, u64 zoom, u64 psel) {
    assert(core);

    gfx_clear_array(g_gfx_spas);

    if (psel < core->pfst || psel > core->plst) {
        return;
    }

    u64 spa = arch_proc_sp_addr(core, psel);

    gfx_accumulate_pixel(pos, zoom, spa, g_gfx_spas);
}

void gfx_render(const Core *core, u64 pos, u64 zoom, u64 psel) {
    assert(core);

    gfx_render_inst(core, pos, zoom);
    gfx_render_mbst(core, pos, zoom);
    gfx_render_mb0s(core, pos, zoom, psel);
    gfx_render_mb1s(core, pos, zoom, psel);
    gfx_render_ipas(core, pos, zoom, psel);
    gfx_render_spas(core, pos, zoom, psel);
}
