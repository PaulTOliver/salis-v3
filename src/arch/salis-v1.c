// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * This is based on the original VM architecture in salis-v1:
 * https://git.pauloliver.dev/salis-v1/about/
 */

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
    PROC_FIELD(u64, rax)  \
    PROC_FIELD(u64, rbx)  \
    PROC_FIELD(u64, rcx)  \
    PROC_FIELD(u64, rdx)  \
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
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].mb0a;
}

u64 arch_proc_mb0_size(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].mb0s;
}

u64 arch_proc_mb1_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].mb1a;
}

u64 arch_proc_mb1_size(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].mb1s;
}

u64 arch_proc_ip_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].ip;
}

u64 arch_proc_sp_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);
    return core->pvec[pix % core->pcap].sp;
}

u64 arch_proc_slice(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 1;
}

void arch_proc_step(Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return;
}

#ifndef NDEBUG
void arch_validate_proc(const Core *core, u64 pix) {
    (void)core;
    (void)pix;

    assert(true);
}
#endif

wchar_t arch_gfx_symbol(u8 inst) {
    switch (inst % INST_COUNT) {
#define INST(name, symb) case name: return symb;
    INST_LIST
#undef INST
    }

    assert(false);
    return L'\0';
}

void arch_gfx_render_mnemonic(u8 inst, char *buff) {
    switch (inst % INST_COUNT) {
#define INST(name, symb) case name: sprintf(buff, #name); break;
    INST_LIST
#undef INST
    }

    assert(false);
}
