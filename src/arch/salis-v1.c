// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * This is based on the original VM architecture in salis-v1:
 * https://git.pauloliver.dev/salis-v1/about/
 */

bool mvec_is_alloc(const Core *core, u64 pix);
void mvec_alloc(Core *core, u64 addr);
void mvec_free(Core *core, u64 addr);
u8 mvec_get_inst(const Core *core, u64 addr);
void mvec_set_inst(Core *core, u64 addr, u8 inst);
bool mvec_is_proc_owner(const Core *core, u64 addr, u64 pix);
void proc_new(Core *core, const Proc *proc);
bool proc_is_live(const Core *core, u64 pix);
const Proc *proc_get(const Core *core, u64 pix);
Proc *proc_fetch(Core *core, u64 pix);

#define INST_LIST    \
    INST(noop, L' ') \
    INST(nop0, L'0') \
    INST(nop1, L'1') \
    INST(nop2, L'2') \
    INST(nop3, L'3') \
                     \
    INST(jmpb, L'(') \
    INST(jmpf, L')') \
    INST(adrb, L'[') \
    INST(adrf, L']') \
    INST(ifnz, L'?') \
                     \
    INST(allb, L'{') \
    INST(allf, L'}') \
    INST(bswp, L'%') \
    INST(bclr, L'|') \
    INST(splt, L'$') \
                     \
    INST(addn, L'+') \
    INST(subn, L'-') \
    INST(muln, L'*') \
    INST(divn, L'/') \
    INST(incn, L'^') \
    INST(decn, L'v') \
    INST(notn, L'!') \
    INST(shfl, L'<') \
    INST(shfr, L'>') \
    INST(zero, L'z') \
    INST(unit, L'u') \
                     \
    INST(pshn, L'#') \
    INST(popn, L'~') \
                     \
    INST(load, L'.') \
    INST(wrte, L':') \
    INST(dupl, L'"') \
    INST(swap, L'x') \
                     \
    INST(keya, L'a') \
    INST(keyb, L'b') \
    INST(keyc, L'c') \
    INST(keyd, L'd') \
    INST(keye, L'e') \
    INST(keyf, L'f') \
    INST(keyg, L'g') \
    INST(keyh, L'h') \
    INST(keyi, L'i') \
    INST(keyj, L'j') \
    INST(keyk, L'k') \
    INST(keyl, L'l') \
    INST(keym, L'm') \
    INST(keyn, L'n') \
    INST(keyo, L'o') \
    INST(keyp, L'p') \
                     \
    INST(loka, L'A') \
    INST(lokb, L'B') \
    INST(lokc, L'C') \
    INST(lokd, L'D') \
    INST(loke, L'E') \
    INST(lokf, L'F') \
    INST(lokg, L'G') \
    INST(lokh, L'H') \
    INST(loki, L'I') \
    INST(lokj, L'J') \
    INST(lokk, L'K') \
    INST(lokl, L'L') \
    INST(lokm, L'M') \
    INST(lokn, L'N') \
    INST(loko, L'O') \
    INST(lokp, L'P')

#define MNEMONIC_BUFF_SIZE (0x5)

enum sinst {
#define INST(name, symb) name,
    INST_LIST
#undef INST
    INST_COUNT
};

#define PROC_FIELDS       \
    PROC_FIELD(u64, ip)   \
    PROC_FIELD(u64, sp)   \
    PROC_FIELD(u64, mb0a) \
    PROC_FIELD(u64, mb0s) \
    PROC_FIELD(u64, mb1a) \
    PROC_FIELD(u64, mb1s) \
    PROC_FIELD(u64, r0x)  \
    PROC_FIELD(u64, r1x)  \
    PROC_FIELD(u64, r2x)  \
    PROC_FIELD(u64, r3x)  \
    PROC_FIELD(u64, s0)   \
    PROC_FIELD(u64, s1)   \
    PROC_FIELD(u64, s2)   \
    PROC_FIELD(u64, s3)   \
    PROC_FIELD(u64, s4)   \
    PROC_FIELD(u64, s5)   \
    PROC_FIELD(u64, s6)   \
    PROC_FIELD(u64, s7)

struct Proc {
#define PROC_FIELD(type, name) type name;
    PROC_FIELDS
#undef PROC_FIELD
};

u64 arch_proc_mb0_addr(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->mb0a;
}

u64 arch_proc_mb0_size(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->mb0s;
}

u64 arch_proc_mb1_addr(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->mb1a;
}

u64 arch_proc_mb1_size(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->mb1s;
}

u64 arch_proc_ip_addr(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->ip;
}

u64 arch_proc_sp_addr(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));
    return proc_get(core, pix)->sp;
}

u64 arch_proc_slice(const Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    (void)core;
    (void)pix;

    return 1;
}

void _free_memory_block(Core *core, u64 addr, u64 size) {
    assert(core);
    assert(size);

    for (u64 i = 0; i < size; ++i) {
        mvec_free(core, addr + i);
    }
}

void arch_on_proc_kill(Core *core) {
    assert(core);
    assert(core->pnum > 1);

    Proc *pfst = proc_fetch(core, core->pfst);

    _free_memory_block(core, pfst->mb0a, pfst->mb0s);

    if (pfst->mb1s) {
        _free_memory_block(core, pfst->mb1a, pfst->mb1s);
    }

    memcpy(pfst, &g_dead_proc, sizeof(Proc));
}

#if ACTION == ACT_BENCH || ACTION == ACT_NEW
void arch_anc_init(Core *core, u64 size) {
    assert(core);

#if ANC_HALF == 1
    u64 addr = U64_HALF;
#else
    u64 addr = 0;
#endif

    for (int i = 0; i < ANC_CLONES; ++i) {
        u64 addr_clone = addr + ((MVEC_SIZE / ANC_CLONES) * i);

        Proc *panc = proc_fetch(core, i);

        panc->mb0a = addr_clone;
        panc->mb0s = size;
        panc->ip   = addr_clone;
        panc->sp   = addr_clone;
    }
}
#endif

u8 _get_inst(const Core *core, u64 addr) {
    assert(core);

    return mvec_get_inst(core, addr) % INST_COUNT;
}

void _increment_ip(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

    proc->ip++;
    proc->sp = proc->ip;
}

bool _is_between(u8 inst, u8 lo, u8 hi) {
    assert(inst < INST_COUNT);
    assert(lo < INST_COUNT);
    assert(hi < INST_COUNT);
    assert(lo < hi);

    return (inst >= lo) && (inst <= hi);
}

bool _is_key(u8 inst) {
    assert(inst < INST_COUNT);

    return _is_between(inst, keya, keyp);
}

bool _is_lock(u8 inst) {
    assert(inst < INST_COUNT);

    return _is_between(inst, loka, lokp);
}

bool _is_rmod(u8 inst) {
    assert(inst < INST_COUNT);

    return _is_between(inst, nop0, nop3);
}

bool _key_lock_match(u8 key, u8 lock) {
    assert(key < INST_COUNT);
    assert(lock < INST_COUNT);
    assert(_is_key(key));

    return (key - keya) == (lock - loka);
}

bool _seek(Core *core, u64 pix, bool fwrd) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u8    next = _get_inst(core, proc->ip + 1);

    if (!_is_key(next)) {
        _increment_ip(core, pix);
        return false;
    }

    u8 spin = _get_inst(core, proc->sp);

    if (_key_lock_match(next, spin)) {
        return true;
    }

    if (fwrd) {
        proc->sp++;
    } else {
        proc->sp--;
    }

    return false;
}

void _jump(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

#ifndef NDEBUG
    u8 next = _get_inst(core, proc->ip + 1);
    u8 spin = _get_inst(core, proc->sp);
    assert(_is_key(next));
    assert(_is_lock(spin));
    assert(_key_lock_match(next, spin));
#endif

    proc->ip = proc->sp;
}

void _get_reg_addr_list(Core *core, u64 pix, u64 **rlist, int rcount, bool offset) {
    assert(core);
    assert(proc_is_live(core, pix));

    assert(rlist);
    assert(rcount);
    assert(rcount < 4);

    Proc *proc = proc_fetch(core, pix);
    u64   madr = proc->ip + (offset ? 2 : 1);

    for (int i = 0; i < rcount; ++i) {
        rlist[i] = &proc->r0x;
    }

    for (int i = 0; i < rcount; ++i) {
        u64 mnxt = madr + i;
        u8  mins = _get_inst(core, mnxt);

        if (!_is_rmod(mins)) {
            break;
        }

        switch (mins) {
        case nop0:
            rlist[i] = &proc->r0x;
            break;
        case nop1:
            rlist[i] = &proc->r1x;
            break;
        case nop2:
            rlist[i] = &proc->r2x;
            break;
        case nop3:
            rlist[i] = &proc->r3x;
            break;
        }
    }
}

void _addr(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *reg;

#ifndef NDEBUG
    u8 next = _get_inst(core, proc->ip + 1);
    u8 spin = _get_inst(core, proc->sp);
    assert(_is_key(next));
    assert(_is_lock(spin));
    assert(_key_lock_match(next, spin));
#endif

    _get_reg_addr_list(core, pix, &reg, 1, true);
    *reg = proc->sp;

    _increment_ip(core, pix);
}

void _ifnz(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *reg;

    _get_reg_addr_list(core, pix, &reg, 1, false);

    u64 jmod = _is_rmod(_get_inst(core, proc->ip + 1)) ? 1 : 0;
    u64 rmod = *reg ? 1 : 2;

    proc->ip += jmod + rmod;
    proc->sp = proc->ip;
}

void _free_child_memory_of(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

    assert(proc->mb1s);

    _free_memory_block(core, proc->mb1a, proc->mb1s);

    proc->mb1a = 0;
    proc->mb1s = 0;
}

void _alloc(Core *core, u64 pix, bool fwrd) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *regs[2];

    _get_reg_addr_list(core, pix, regs, 2, false);

    u64 bsize = *regs[0];

    // do nothing if block-size is zero
    if (!bsize) {
        _increment_ip(core, pix);
        return;
    }

    // do nothing if sp is not adjacent to allocated mem. block
    if (proc->mb1s) {
        u64 exp_addr = proc->mb1a;

        if (fwrd) {
            exp_addr += proc->mb1s;
        } else {
            exp_addr--;
        }

        if (proc->sp != exp_addr) {
            _increment_ip(core, pix);
            return;
        }
    }

    // allocation was successful, store block address on register
    if (proc->mb1s == bsize) {
        _increment_ip(core, pix);
        *regs[1] = proc->mb1a;
        return;
    }

    // sp collided with another allocated block, clear and try again
    if (mvec_is_alloc(core, proc->sp)) {
        if (proc->mb1s) {
            _free_child_memory_of(core, pix);
        }

        if (fwrd) {
            proc->sp++;
        } else {
            proc->sp--;
        }

        return;
    }

    // otherwise enlarge block
    mvec_alloc(core, proc->sp);

    // adjust child block address and size
    if (!proc->mb1s || !fwrd) {
        proc->mb1a = proc->sp;
    }

    proc->mb1s++;

    // move sp to new location
    if (fwrd) {
        proc->sp++;
    } else {
        proc->sp--;
    }
}

void _bswap(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

    if (proc->mb1s) {
        u64 tmpa = proc->mb0a;
        u64 tmps = proc->mb0s;

        proc->mb0a = proc->mb1a;
        proc->mb0s = proc->mb1s;
        proc->mb1a = tmpa;
        proc->mb1s = tmps;
    }

    _increment_ip(core, pix);
}

void _bclear(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

    if (proc->mb1s) {
        _free_child_memory_of(core, pix);
    }

    _increment_ip(core, pix);
}

void _split(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);

    if (proc->mb1s) {
        Proc child = {0};

        child.ip   = proc->mb1a;
        child.sp   = proc->mb1a;
        child.mb0a = proc->mb1a;
        child.mb0s = proc->mb1s;

        proc->mb1a = 0;
        proc->mb1s = 0;

        proc_new(core, &child);
    } else {
        assert(!proc->mb1a);
    }

    _increment_ip(core, pix);
}

void _3rop(Core *core, u64 pix, u8 inst) {
    assert(core);
    assert(proc_is_live(core, pix));

    u64 *regs[3];

    _get_reg_addr_list(core, pix, regs, 3, false);

    switch (inst) {
    case addn:
        *regs[0] = *regs[1] + *regs[2];
        break;
    case subn:
        *regs[0] = *regs[1] - *regs[2];
        break;
    case muln:
        *regs[0] = *regs[1] * *regs[2];
        break;
    case divn:
        // do nothing on div. by zero
        if (*regs[2]) {
            *regs[0] = *regs[1] / *regs[2];
        }

        break;
    default:
        assert(false);
    }

    _increment_ip(core, pix);
}

void _1rop(Core *core, u64 pix, u8 inst) {
    assert(core);
    assert(proc_is_live(core, pix));

    u64 *reg;

    _get_reg_addr_list(core, pix, &reg, 1, false);

    switch (inst) {
    case incn:
        (*reg)++;
        break;
    case decn:
        (*reg)--;
        break;
    case notn:
        *reg = !(*reg);
        break;
    case shfl:
        *reg <<= 1;
        break;
    case shfr:
        *reg >>= 1;
        break;
    case zero:
        *reg = 0;
        break;
    case unit:
        *reg = 1;
        break;
    default:
        assert(false);
    }

    _increment_ip(core, pix);
}

void _push(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *reg;

    _get_reg_addr_list(core, pix, &reg, 1, false);

    proc->s7 = proc->s6;
    proc->s6 = proc->s5;
    proc->s5 = proc->s4;
    proc->s4 = proc->s3;
    proc->s3 = proc->s2;
    proc->s2 = proc->s1;
    proc->s1 = proc->s0;
    proc->s0 = *reg;

    _increment_ip(core, pix);
}

void _pop(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *reg;

    _get_reg_addr_list(core, pix, &reg, 1, false);

    *reg     = proc->s0;
    proc->s0 = proc->s1;
    proc->s1 = proc->s2;
    proc->s2 = proc->s3;
    proc->s3 = proc->s4;
    proc->s4 = proc->s5;
    proc->s5 = proc->s6;
    proc->s6 = proc->s7;
    proc->s7 = 0;

    _increment_ip(core, pix);
}

int _sp_dir(u64 src, u64 dst) {
    if (src == dst) {
        return 0;
    } else if (src - dst <= dst - src) {
        return -1;
    } else {
        return 1;
    }
}

void _load(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *regs[2];

    _get_reg_addr_list(core, pix, regs, 2, false);

    int sp_dir = _sp_dir(proc->sp, *regs[0]);

    if (sp_dir == 1) {
        proc->sp++;
    } else if (sp_dir == -1) {
        proc->sp--;
    } else {
        *regs[1] = mvec_get_inst(core, *regs[0]);
        _increment_ip(core, pix);
    }
}

bool _is_writeable_by(const Core *core, u64 addr, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    return !mvec_is_alloc(core, addr) || mvec_is_proc_owner(core, addr, pix);
}

void _write(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u64  *regs[2];

    _get_reg_addr_list(core, pix, regs, 2, false);

    int sp_dir = _sp_dir(proc->sp, *regs[0]);

    if (sp_dir == 1) {
        proc->sp++;
    } else if (sp_dir == -1) {
        proc->sp--;
    } else {
        if (_is_writeable_by(core, *regs[0], pix)) {
            mvec_set_inst(core, *regs[0], *regs[1] % INST_CAPS);
        }

        _increment_ip(core, pix);
    }
}

void _2rop(Core *core, u64 pix, u8 inst) {
    assert(core);
    assert(proc_is_live(core, pix));

    u64 *regs[2];

    _get_reg_addr_list(core, pix, regs, 2, false);

    switch (inst) {
    case dupl:
        *regs[1] = *regs[0];
        break;
    case swap:
        {
            u64 tmp  = *regs[0];
            *regs[0] = *regs[1];
            *regs[1] = tmp;
        }

        break;
    default:
        assert(false);
    }

    _increment_ip(core, pix);
}

void arch_proc_step(Core *core, u64 pix) {
    assert(core);
    assert(proc_is_live(core, pix));

    Proc *proc = proc_fetch(core, pix);
    u8    inst = _get_inst(core, proc->ip);

    switch (inst) {
    case jmpb:
        if (_seek(core, pix, false)) {
            _jump(core, pix);
        }

        break;
    case jmpf:
        if (_seek(core, pix, true)) {
            _jump(core, pix);
        }

        break;
    case adrb:
        if (_seek(core, pix, false)) {
            _addr(core, pix);
        }

        break;
    case adrf:
        if (_seek(core, pix, true)) {
            _addr(core, pix);
        }

        break;
    case ifnz:
        _ifnz(core, pix);
        break;
    case allb:
        _alloc(core, pix, false);
        break;
    case allf:
        _alloc(core, pix, true);
        break;
    case bswp:
        _bswap(core, pix);
        break;
    case bclr:
        _bclear(core, pix);
        break;
    case splt:
        _split(core, pix);
        break;
    case addn:
    case subn:
    case muln:
    case divn:
        _3rop(core, pix, inst);
        break;
    case incn:
    case decn:
    case notn:
    case shfl:
    case shfr:
    case zero:
    case unit:
        _1rop(core, pix, inst);
        break;
    case pshn:
        _push(core, pix);
        break;
    case popn:
        _pop(core, pix);
        break;
    case load:
        _load(core, pix);
        break;
    case wrte:
        _write(core, pix);
        break;
    case dupl:
    case swap:
        _2rop(core, pix, inst);
        break;
    default:
        _increment_ip(core, pix);
        break;
    }

    return;
}

#ifndef NDEBUG
void arch_validate_proc(const Core *core, u64 pix) {
    assert(core);

    const Proc *proc = proc_get(core, pix);

    assert(proc->mb0s);

    if (proc->mb1a) {
        assert(proc->mb1s);
    }

    for (u64 i = 0; i < proc->mb0s; ++i) {
        u64 addr = proc->mb0a + i;
        assert(mvec_is_alloc(core, addr));
        assert(mvec_is_proc_owner(core, addr, pix));
    }

    for (u64 i = 0; i < proc->mb1s; ++i) {
        u64 addr = proc->mb1a + i;
        assert(mvec_is_alloc(core, addr));
        assert(mvec_is_proc_owner(core, addr, pix));
    }
}
#endif

wchar_t arch_symbol(u8 inst) {
    switch (inst % INST_COUNT) {
#define INST(name, symb) case name: return symb;
    INST_LIST
#undef INST
    }

    assert(false);
    return L'\0';
}

void arch_mnemonic(u8 inst, char *buff) {
    assert(buff);

    switch (inst % INST_COUNT) {
#define INST(name, symb) case name: snprintf(buff, MNEMONIC_BUFF_SIZE, #name); return;
    INST_LIST
#undef INST
    }

    assert(false);
}
