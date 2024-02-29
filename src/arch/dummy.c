// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * Defines a minimal viable architecture for the Salis VM. Useful for
 * debugging and benchmarking. Also, this file can be used as a template when
 * implementing a real architecture.
 */

void mvec_alloc(Core *core, u64 addr);

#define PROC_FIELDS \
    PROC_FIELD(u64, dmmy)

struct Proc {
#define PROC_FIELD(type, name) type name;
    PROC_FIELDS
#undef PROC_FIELD
};

#define MNEMONIC_BUFF_SIZE (0x10)

const wchar_t *g_arch_byte_symbols = (
    L"⠀⠁⠂⠃⠄⠅⠆⠇⡀⡁⡂⡃⡄⡅⡆⡇⠈⠉⠊⠋⠌⠍⠎⠏⡈⡉⡊⡋⡌⡍⡎⡏⠐⠑⠒⠓⠔⠕⠖⠗⡐⡑⡒⡓⡔⡕⡖⡗⠘⠙⠚⠛⠜⠝⠞⠟⡘⡙⡚⡛⡜⡝⡞⡟"
    L"⠠⠡⠢⠣⠤⠥⠦⠧⡠⡡⡢⡣⡤⡥⡦⡧⠨⠩⠪⠫⠬⠭⠮⠯⡨⡩⡪⡫⡬⡭⡮⡯⠰⠱⠲⠳⠴⠵⠶⠷⡰⡱⡲⡳⡴⡵⡶⡷⠸⠹⠺⠻⠼⠽⠾⠿⡸⡹⡺⡻⡼⡽⡾⡿"
    L"⢀⢁⢂⢃⢄⢅⢆⢇⣀⣁⣂⣃⣄⣅⣆⣇⢈⢉⢊⢋⢌⢍⢎⢏⣈⣉⣊⣋⣌⣍⣎⣏⢐⢑⢒⢓⢔⢕⢖⢗⣐⣑⣒⣓⣔⣕⣖⣗⢘⢙⢚⢛⢜⢝⢞⢟⣘⣙⣚⣛⣜⣝⣞⣟"
    L"⢠⢡⢢⢣⢤⢥⢦⢧⣠⣡⣢⣣⣤⣥⣦⣧⢨⢩⢪⢫⢬⢭⢮⢯⣨⣩⣪⣫⣬⣭⣮⣯⢰⢱⢲⢳⢴⢵⢶⢷⣰⣱⣲⣳⣴⣵⣶⣷⢸⢹⢺⢻⢼⢽⢾⢿⣸⣹⣺⣻⣼⣽⣾⣿"
);

u64 arch_proc_mb0_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
}

u64 arch_proc_mb0_size(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
}

u64 arch_proc_mb1_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
}

u64 arch_proc_mb1_size(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
}

u64 arch_proc_ip_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
}

u64 arch_proc_sp_addr(const Core *core, u64 pix) {
    assert(core);
    assert(pix >= core->pfst && pix <= core->plst);

    (void)core;
    (void)pix;

    return 0;
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
    return g_arch_byte_symbols[inst];
}

void arch_gfx_render_mnemonic(u8 inst, char *buff) {
    sprintf(buff, "dummy %#x", inst);
}
