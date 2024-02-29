// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * Core of the Salis simulator. Can be built against different architectures
 * and UI modules.
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define ACT_BENCH (1)
#define ACT_LOAD  (2)
#define ACT_NEW   (3)

#define ASM_LINE_LEN (0x100)

#define MALL_FLAG (0x80)
#define IPCM_FLAG (0x80)
#define INST_CAPS (0x80)
#define INST_MASK (0x7f)

typedef struct Core Core;
typedef struct Ipcm Ipcm;
typedef struct Proc Proc;
typedef thrd_t      Thread;
typedef uint64_t    u64;
typedef uint8_t     u8;

struct Core {
    u64    mall;
    u64    muta[4];
    u64    pnum;
    u64    pcap;
    u64    pfst;
    u64    plst;
    u64    pcur;
    u64    psli;
    u64    ncyc;

    Thread thread;
    u64    tix;

    u64    ivpt;
    u8    *iviv;
    u64   *ivav;

    Proc  *pvec;
    u8     mvec[MVEC_SIZE];
    u8     tgap[TGAP_SIZE];
};

Core       g_cores[CORE_COUNT];
u64        g_steps;
u64        g_syncs;
#if ACTION == ACT_LOAD || ACTION == ACT_NEW
char       g_asav_pbuf[AUTO_SAVE_NAME_LEN];
#endif
const Proc g_dead_proc;

#include ARCH_SOURCE

#if ACTION == ACT_BENCH || ACTION == ACT_NEW
char g_mnemo_table[0x100][MNEMONIC_BUFF_SIZE];
#endif

u64 mvec_loop(u64 addr) {
    return addr % MVEC_SIZE;
}

bool mvec_is_alloc(const Core *core, u64 addr) {
    assert(core);
    return core->mvec[mvec_loop(addr)] & MALL_FLAG ? true : false;
}

void mvec_alloc(Core *core, u64 addr) {
    assert(core);
    assert(!mvec_is_alloc(core, addr));
    core->mvec[mvec_loop(addr)] |= MALL_FLAG;
    core->mall++;
}

void mvec_free(Core *core, u64 addr) {
    assert(core);
    assert(mvec_is_alloc(core, addr));
    core->mvec[mvec_loop(addr)] ^= MALL_FLAG;
    core->mall--;
}

u8 mvec_get_byte(const Core *core, u64 addr) {
    assert(core);
    return core->mvec[mvec_loop(addr)];
}

u8 mvec_get_inst(const Core *core, u64 addr) {
    assert(core);
    return core->mvec[mvec_loop(addr)] & INST_MASK;
}

void mvec_set_inst(Core *core, u64 addr, u8 inst) {
    assert(core);
    assert(inst < INST_CAPS);
    core->mvec[mvec_loop(addr)] &= MALL_FLAG;
    core->mvec[mvec_loop(addr)] |= inst;
}

#if MUTA_FLIP_BIT == 1
void mvec_flip_bit(Core *core, u64 addr, int bit) {
    assert(core);
    assert(bit < 8);
    core->mvec[mvec_loop(addr)] ^= (1 << bit) & INST_MASK;
}
#endif

bool mvec_is_proc_owner(const Core *core, u64 addr, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    u64 mb0a = arch_proc_mb0_addr(core, pix);
    u64 mb0s = arch_proc_mb0_size(core, pix);

    if (((addr - mb0a) % MVEC_SIZE) < mb0s) {
        return true;
    }

    u64 mb1a = arch_proc_mb1_addr(core, pix);
    u64 mb1s = arch_proc_mb1_size(core, pix);

    if (((addr - mb1a) % MVEC_SIZE) < mb1s) {
        return true;
    }

    return false;
}

u64 mvec_get_owner(const Core *core, u64 addr) {
    assert(core);
    assert(mvec_is_alloc(core, addr));

    for (u64 pix = core->pfst; pix <= core->plst; ++pix) {
        if (mvec_is_proc_owner(core, addr, pix)) {
            return pix;
        }
    }

    assert(false);
    return -1;
}

#if ACTION == ACT_BENCH || ACTION == ACT_NEW
u64 muta_smix(u64 *seed) {
    assert(seed);

    u64 next = (*seed += 0x9e3779b97f4a7c15);
    next     = (next ^ (next >> 30)) * 0xbf58476d1ce4e5b9;
    next     = (next ^ (next >> 27)) * 0x94d049bb133111eb;

    return next ^ (next >> 31);
}
#endif

u64 muta_ro64(u64 x, int k) {
    return (x << k) | (x >> (64 - k));
}

u64 muta_next(Core *core) {
    assert(core);

    u64 r = muta_ro64(core->muta[1] * 5, 7) * 9;
    u64 t = core->muta[1] << 17;

    core->muta[2] ^= core->muta[0];
    core->muta[3] ^= core->muta[1];
    core->muta[1] ^= core->muta[2];
    core->muta[0] ^= core->muta[3];

    core->muta[2] ^= t;
    core->muta[3]  = muta_ro64(core->muta[3], 45);

    return r;
}

void muta_cosmic_ray(Core *core) {
    assert(core);

    u64 a = muta_next(core) % MUTA_RANGE;
    u64 b = muta_next(core);

    if (a < MVEC_SIZE) {
#if MUTA_FLIP_BIT == 1
        mvec_flip_bit(core, a, (int)(b % 8));
#else
        mvec_set_inst(core, a, b & INST_MASK);
#endif
    }
}

void proc_new(Core *core, const Proc *proc) {
    assert(core);
    assert(proc);

    if (core->pnum == core->pcap) {
        u64   new_pcap = core->pcap * 2;
        Proc *new_pvec = calloc(new_pcap, sizeof(Proc));

        for (u64 pix = core->pfst; pix <= core->plst; ++pix) {
            u64 iold = pix % core->pcap;
            u64 inew = pix % new_pcap;
            memcpy(&new_pvec[inew], &core->pvec[iold], sizeof(Proc));
        }

        free(core->pvec);
        core->pcap = new_pcap;
        core->pvec = new_pvec;
    }

    core->pnum++;
    core->plst++;
    memcpy(&core->pvec[core->plst % core->pcap], proc, sizeof(Proc));
}

void proc_kill(Core *core) {
    assert(core);
    assert(core->pnum > 1);

    arch_on_proc_kill(core);

    core->pcur++;
    core->pfst++;
    core->pnum--;
}

bool proc_is_live(const Core *core, u64 pix) {
    assert(core);

    return pix >= core->pfst && pix <= core->plst;
}

const Proc *proc_get(const Core *core, u64 pix) {
    assert(core);

    if (proc_is_live(core, pix)) {
        return &core->pvec[pix % core->pcap];
    } else {
        return &g_dead_proc;
    }
}

Proc *proc_fetch(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    return &core->pvec[pix % core->pcap];
}

#if ACTION == ACT_LOAD || ACTION == ACT_NEW
void core_save(FILE *f, const Core *core) {
    assert(f);
    assert(core);

    fwrite(&core->mall, sizeof(u64), 1, f);
    fwrite( core->muta, sizeof(u64), 4, f);
    fwrite(&core->pnum, sizeof(u64), 1, f);
    fwrite(&core->pcap, sizeof(u64), 1, f);
    fwrite(&core->pfst, sizeof(u64), 1, f);
    fwrite(&core->plst, sizeof(u64), 1, f);
    fwrite(&core->pcur, sizeof(u64), 1, f);
    fwrite(&core->psli, sizeof(u64), 1, f);
    fwrite(&core->ncyc, sizeof(u64), 1, f);
    fwrite(&core->ivpt, sizeof(u64), 1, f);

    fwrite(core->iviv, sizeof(u8),   SYNC_INTERVAL, f);
    fwrite(core->ivav, sizeof(u64),  SYNC_INTERVAL, f);
    fwrite(core->pvec, sizeof(Proc), core->pcap,    f);
    fwrite(core->mvec, sizeof(u8),   MVEC_SIZE,     f);
}
#endif

#if ACTION == ACT_BENCH || ACTION == ACT_NEW
u64 core_assemble_ancestor(int cix, const char *anc) {
    assert(cix >= 0 && cix < CORE_COUNT);
    assert(anc);

    if (anc[0] == '_') {
        return 0;
    }

    FILE *f = fopen(anc, "r");

    assert(f);

    u64   addr               = 0;
    char  line[ASM_LINE_LEN] = {0};
    Core *core               = &g_cores[cix];

    for (; fgets(line, ASM_LINE_LEN, f); ++addr) {
#ifndef NDEBUG
        bool line_ok = false;
#endif

        line[strcspn(line, "\r\n")] = '\0';

        for (int i = 0; i < 0x100; ++i) {
            if (strcmp(line, g_mnemo_table[i]) == 0) {
                mvec_alloc(core, addr);
                mvec_set_inst(core, addr, i);
#ifndef NDEBUG
                line_ok = true;
#endif
                break;
            }
        }

        assert(line_ok);
    }

    fclose(f);

    return addr;
}

void core_init(int cix, u64 *seed, const char *anc) {
    assert(cix >= 0 && cix < CORE_COUNT);
    assert(seed);
    assert(anc);

    Core *core = &g_cores[cix];

    if (*seed) {
        core->muta[0] = muta_smix(seed);
        core->muta[1] = muta_smix(seed);
        core->muta[2] = muta_smix(seed);
        core->muta[3] = muta_smix(seed);
    }

    core->pnum = 1;
    core->pcap = 1;
    core->iviv = calloc(SYNC_INTERVAL, sizeof(u8));
    core->ivav = calloc(SYNC_INTERVAL, sizeof(u64));
    core->pvec = calloc(core->pcap, sizeof(Proc));

    assert(core->iviv);
    assert(core->ivav);
    assert(core->pvec);

    u64 anc_size = core_assemble_ancestor(cix, anc);

    arch_anc_init(core, anc_size);
}
#endif

#if ACTION == ACT_LOAD
void core_load(FILE *f, Core *core) {
    assert(f);
    assert(core);

    fread(&core->mall, sizeof(u64), 1, f);
    fread( core->muta, sizeof(u64), 4, f);
    fread(&core->pnum, sizeof(u64), 1, f);
    fread(&core->pcap, sizeof(u64), 1, f);
    fread(&core->pfst, sizeof(u64), 1, f);
    fread(&core->plst, sizeof(u64), 1, f);
    fread(&core->pcur, sizeof(u64), 1, f);
    fread(&core->psli, sizeof(u64), 1, f);
    fread(&core->ncyc, sizeof(u64), 1, f);
    fread(&core->ivpt, sizeof(u64), 1, f);

    core->iviv = calloc(SYNC_INTERVAL, sizeof(u8));
    core->ivav = calloc(SYNC_INTERVAL, sizeof(u64));
    core->pvec = calloc(core->pcap, sizeof(Proc));

    assert(core->iviv);
    assert(core->ivav);
    assert(core->pvec);

    fread(core->iviv, sizeof(u8),   SYNC_INTERVAL, f);
    fread(core->ivav, sizeof(u64),  SYNC_INTERVAL, f);
    fread(core->pvec, sizeof(Proc), core->pcap,    f);
    fread(core->mvec, sizeof(u8),   MVEC_SIZE,     f);
}
#endif

void core_pull_ipcm(Core *core) {
    assert(core);
    assert(core->ivpt < SYNC_INTERVAL);

    u8  *iinst = &core->iviv[core->ivpt];
    u64 *iaddr = &core->ivav[core->ivpt];

    if ((*iinst & IPCM_FLAG) != 0) {
        mvec_set_inst(core, *iaddr, *iinst & INST_MASK);

        *iinst = 0;
        *iaddr = 0;
    }

    assert(*iinst == 0);
    assert(*iaddr == 0);
}

void core_push_ipcm(Core *core, u8 inst, u64 addr) {
    assert(core);
    assert(core->ivpt < SYNC_INTERVAL);
    assert((inst & IPCM_FLAG) == 0);

    u8  *iinst = &core->iviv[core->ivpt];
    u64 *iaddr = &core->ivav[core->ivpt];

    assert(*iinst == 0);
    assert(*iaddr == 0);

    *iinst = inst | IPCM_FLAG;
    *iaddr = addr;
}

void core_step(Core *core) {
    assert(core);

    if (core->psli != 0) {
        core_pull_ipcm(core);
        arch_proc_step(core, core->pcur);

        core->psli--;
        core->ivpt++;

        return;
    }

    if (core->pcur != core->plst) {
        core->psli = arch_proc_slice(core, ++core->pcur);
        core_step(core);
        return;
    }

    core->pcur = core->pfst;
    core->psli = arch_proc_slice(core, core->pcur);
    core->ncyc++;

    while (core->mall > MVEC_SIZE / 2 && core->pnum > 1) {
        proc_kill(core);
    }

    muta_cosmic_ray(core);
    core_step(core);
}

#if ACTION == ACT_LOAD || ACTION == ACT_NEW
void salis_save(const char *path) {
    FILE *f = fopen(path, "wb");

    assert(f);

    for (int i = 0; i < CORE_COUNT; ++i) {
        core_save(f, &g_cores[i]);
    }

    fwrite(&g_steps, sizeof(u64), 1, f);
    fwrite(&g_syncs, sizeof(u64), 1, f);
    fclose(f);
}

void salis_auto_save() {
    if (g_steps % AUTO_SAVE_INTERVAL != 0) {
        return;
    }

#ifndef NDEBUG
    int rem = snprintf(
#else
    snprintf(
#endif
        g_asav_pbuf,
        AUTO_SAVE_NAME_LEN,
        "%s-%#018lx",
        SIM_PATH,
        g_steps
    );

    assert(rem >= 0);
    assert(rem < AUTO_SAVE_NAME_LEN);

    salis_save(g_asav_pbuf);
}
#endif

#if ACTION == ACT_BENCH || ACTION == ACT_NEW
void salis_init() {
    for (int i = 0; i < 0x100; ++i) {
        arch_mnemonic(i, g_mnemo_table[i]);
    }

    u64   seed       = SEED;
    char  anc_list[] = ANC_LIST;

    assert(anc_list);

    for (int i = 0; i < CORE_COUNT; ++i) {
        core_init(i, &seed, strtok(i ? NULL : anc_list, ","));
    }

#if ACTION == ACT_NEW
    salis_auto_save();
#endif
}
#endif

#if ACTION == ACT_LOAD
void salis_load() {
    FILE *f = fopen(SIM_PATH, "rb");

    assert(f);

    for (int i = 0; i < CORE_COUNT; ++i) {
        core_load(f, &g_cores[i]);
    }

    fread(&g_steps, sizeof(u64), 1, f);
    fread(&g_syncs, sizeof(u64), 1, f);
    fclose(f);
}
#endif

int salis_thread(Core *core) {
    assert(core);

    for (u64 i = 0; i < core->tix; ++i) {
        core_step(core);
    }

    return 0;
}

void salis_run_thread(u64 ns) {
    for (int i = 0; i < CORE_COUNT; ++i) {
        g_cores[i].tix = ns;

        thrd_create(
            &g_cores[i].thread,
            (thrd_start_t)salis_thread,
            &g_cores[i]
        );
    }

    for (int i = 0; i < CORE_COUNT; ++i) {
        thrd_join(g_cores[i].thread, NULL);
    }

    g_steps += ns;
}

void salis_sync() {
    u8  *iviv0 = g_cores[0].iviv;
    u64 *ivav0 = g_cores[0].ivav;

    for (int i = 1; i < CORE_COUNT; ++i) {
        g_cores[i - 1].iviv = g_cores[i].iviv;
        g_cores[i - 1].ivav = g_cores[i].ivav;
    }

    g_cores[CORE_COUNT - 1].iviv = iviv0;
    g_cores[CORE_COUNT - 1].ivav = ivav0;

    for (int i = 0; i < CORE_COUNT; ++i) {
        g_cores[i].ivpt = 0;
    }

    g_syncs++;
}

void salis_loop(u64 ns, u64 dt) {
    assert(dt);

    if (ns < dt) {
        salis_run_thread(ns);
        return;
    }

    salis_run_thread(dt);
    salis_sync();
#if ACTION == ACT_LOAD || ACTION == ACT_NEW
    salis_auto_save();
#endif
    salis_loop(ns - dt, SYNC_INTERVAL);
}

#ifndef NDEBUG
void salis_validate_core(const Core *core) {
    assert(core->plst >= core->pfst);
    assert(core->pnum == core->plst + 1 - core->pfst);
    assert(core->pnum <= core->pcap);
    assert(core->pcur >= core->pfst && core->pcur <= core->plst);
    assert(core->ncyc <= g_steps);

    u64 mall = 0;

    for (u64 i = 0; i < MVEC_SIZE; ++i) {
        mall += mvec_is_alloc(core, i) ? 1 : 0;
    }

    assert(core->mall == mall);

    for (u64 i = core->pfst; i <= core->plst; ++i) {
        arch_validate_proc(core, i);
    }

    for (u64 i = 0; i < SYNC_INTERVAL; ++i) {
        u8 iinst = core->iviv[i];

        if ((iinst & IPCM_FLAG) == 0) {
            u64 iaddr = core->ivav[i];

            assert(iinst == 0);
            assert(iaddr == 0);
        }
    }

    assert(core->ivpt == g_steps % SYNC_INTERVAL);
}

void salis_validate() {
    assert(g_steps / SYNC_INTERVAL == g_syncs);

    for (int i = 0; i < CORE_COUNT; ++i) {
        salis_validate_core(&g_cores[i]);
    }
}
#endif

void salis_step(u64 ns) {
    assert(ns);
    salis_loop(ns, SYNC_INTERVAL - (g_steps % SYNC_INTERVAL));

#ifndef NDEBUG
    salis_validate();
#endif
}

void salis_free() {
    for (int i = 0; i < CORE_COUNT; ++i) {
        assert(g_cores[i].pvec);
        assert(g_cores[i].iviv);
        assert(g_cores[i].ivav);

        free(g_cores[i].pvec);
        free(g_cores[i].iviv);
        free(g_cores[i].ivav);

        g_cores[i].pvec = NULL;
        g_cores[i].iviv = NULL;
        g_cores[i].ivav = NULL;
    }
}

#include UI
