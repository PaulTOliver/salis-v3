// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * Implements a TUI for the Salis simulator using the ncurses library.
 */

#include <curses.h>
#include <locale.h>
#include <time.h>

#define CTRL(x)          ((x) & 0x1f)
#define PANE_WIDTH       (27)
#define PROC_FIELD_WIDTH (21)
#define PROC_PAGE_LINES  (12)

enum {
    PAGE_CORE,
    PAGE_PROCESS,
    PAGE_WORLD,
    PAGE_IPC,
    PAGE_COUNT
};

enum {
    PAIR_NOUSE,
    PAIR_NORMAL,
    PAIR_HEADER,
    PAIR_LIVE_PROC,
    PAIR_SELECTED_PROC,
    PAIR_FREE_CELL,
    PAIR_ALLOC_CELL,
    PAIR_MEM_BLOCK_START,
    PAIR_SELECTED_MB1,
    PAIR_SELECTED_MB2,
    PAIR_SELECTED_IP,
    PAIR_SELECTED_SP,
};

bool     g_exit;
bool     g_running;
unsigned g_core;
unsigned g_page;
bool     g_proc_genes;
u64      g_proc_scroll;
u64      g_proc_field_scroll;
u64      g_proc_gene_scroll;
u64      g_proc_selected;
u64      g_wrld_pos;
u64      g_wrld_zoom;
bool     g_wcursor_mode;
int      g_wcursor_x;
int      g_wcursor_y;
u64      g_wcursor_pointed;
u64      g_vlin;
u64      g_vsiz;
u64      g_vlin_rng;
u64      g_vsiz_rng;
u64      g_ivpt_scroll;
char    *g_line_buff;
u64      g_step_block;

const wchar_t *g_zoomed_symbols = (
    L"⠀⠁⠂⠃⠄⠅⠆⠇⡀⡁⡂⡃⡄⡅⡆⡇⠈⠉⠊⠋⠌⠍⠎⠏⡈⡉⡊⡋⡌⡍⡎⡏⠐⠑⠒⠓⠔⠕⠖⠗⡐⡑⡒⡓⡔⡕⡖⡗⠘⠙⠚⠛⠜⠝⠞⠟⡘⡙⡚⡛⡜⡝⡞⡟"
    L"⠠⠡⠢⠣⠤⠥⠦⠧⡠⡡⡢⡣⡤⡥⡦⡧⠨⠩⠪⠫⠬⠭⠮⠯⡨⡩⡪⡫⡬⡭⡮⡯⠰⠱⠲⠳⠴⠵⠶⠷⡰⡱⡲⡳⡴⡵⡶⡷⠸⠹⠺⠻⠼⠽⠾⠿⡸⡹⡺⡻⡼⡽⡾⡿"
    L"⢀⢁⢂⢃⢄⢅⢆⢇⣀⣁⣂⣃⣄⣅⣆⣇⢈⢉⢊⢋⢌⢍⢎⢏⣈⣉⣊⣋⣌⣍⣎⣏⢐⢑⢒⢓⢔⢕⢖⢗⣐⣑⣒⣓⣔⣕⣖⣗⢘⢙⢚⢛⢜⢝⢞⢟⣘⣙⣚⣛⣜⣝⣞⣟"
    L"⢠⢡⢢⢣⢤⢥⢦⢧⣠⣡⣢⣣⣤⣥⣦⣧⢨⢩⢪⢫⢬⢭⢮⢯⣨⣩⣪⣫⣬⣭⣮⣯⢰⢱⢲⢳⢴⢵⢶⢷⣰⣱⣲⣳⣴⣵⣶⣷⢸⢹⢺⢻⢼⢽⢾⢿⣸⣹⣺⣻⣼⣽⣾⣿"
);

#include "graphics.c"

void ui_line_buff_free() {
    if (g_line_buff) {
        free(g_line_buff);
    }

    g_line_buff = NULL;
}

void ui_line_buff_resize() {
    ui_line_buff_free();

    g_line_buff = calloc(COLS + 1, sizeof(char));
}

void ui_line(bool clear, int line, int color, int attr, const char *format, ...) {
    assert(line >= 0);
    assert(format);

    if (line >= LINES) {
        return;
    }

    if (clear) {
        move(line, 0);
        clrtoeol();
    }

    va_list args;

    attron(COLOR_PAIR(color) | attr);
    va_start(args, format);

    vsnprintf(g_line_buff, COLS, format, args);
    mvprintw(line, 1, g_line_buff);

    va_end(args);
    attroff(COLOR_PAIR(color) | attr);
}

void ui_clear_line(int l) {
    ui_line(true, l, PAIR_NORMAL, A_NORMAL, "");
}

void ui_field(int line, int col, int color, int attr, const char *format, ...) {
    assert(line >= 0);
    assert(col >= 0);
    assert(format);

    if (line >= LINES || col >= COLS) {
        return;
    }

    va_list args;

    attron(COLOR_PAIR(color) | attr);
    va_start(args, format);

    vsnprintf(g_line_buff, COLS - col, format, args);
    mvprintw(line, col, g_line_buff);

    va_end(args);
    attroff(COLOR_PAIR(color) | attr);
}

void ui_str_field(int l, const char *label, const char *value) {
    assert(label);
    assert(value);
    ui_line(false, l, PAIR_NORMAL, A_NORMAL, "%s : %18s", label, value);
}

void ui_ulx_field(int l, const char *label, u64 value) {
    assert(label);
    ui_line(false, l, PAIR_NORMAL, A_NORMAL, "%-4s : %#18lx", label, value);
}

void ui_print_core(int l) {
    ui_line(false, ++l, PAIR_HEADER, A_BOLD, "CORE [%d]", g_core);
    ui_ulx_field(++l, "mall", g_cores[g_core].mall);
    ui_ulx_field(++l, "mut0", g_cores[g_core].muta[0]);
    ui_ulx_field(++l, "mut1", g_cores[g_core].muta[1]);
    ui_ulx_field(++l, "mut2", g_cores[g_core].muta[2]);
    ui_ulx_field(++l, "mut3", g_cores[g_core].muta[3]);
    ui_ulx_field(++l, "pnum", g_cores[g_core].pnum);
    ui_ulx_field(++l, "pcap", g_cores[g_core].pcap);
    ui_ulx_field(++l, "pfst", g_cores[g_core].pfst);
    ui_ulx_field(++l, "plst", g_cores[g_core].plst);
    ui_ulx_field(++l, "pcur", g_cores[g_core].pcur);
    ui_ulx_field(++l, "psli", g_cores[g_core].psli);
    ui_ulx_field(++l, "ncyc", g_cores[g_core].ncyc);
    ui_ulx_field(++l, "ivpt", g_cores[g_core].ivpt);
}

int ui_proc_pair(u64 pix) {
    if (pix == g_proc_selected) {
        return PAIR_SELECTED_PROC;
    } else if (proc_is_live(&g_cores[g_core], pix)) {
        return PAIR_LIVE_PROC;
    } else {
        return PAIR_NORMAL;
    }
}

const char *ui_proc_state(u64 pix) {
    return proc_is_live(&g_cores[g_core], pix) ? "live" : "dead";
}

void ui_print_process_genome_header(int l) {
    ui_line(false, l++, PAIR_NORMAL, A_NORMAL, "%s : %18s : %s", "stat", "pix", "genome");
}

void ui_print_process_gene(int l, int gcol, u64 gidx, u64 mba, u64 pix, int pair) {
    assert(gcol >= PANE_WIDTH + 2);
    assert(gcol < COLS);
    assert(proc_is_live(&g_cores[g_core], pix));
    assert(pair == PAIR_SELECTED_MB1 || pair == PAIR_SELECTED_MB2);

    const Core *core = &g_cores[g_core];

    u64 addr = mba + gidx;
    u8  byte = mvec_get_byte(core, addr);

    wchar_t gsym[2] = { arch_symbol(byte), L'\0' };
    cchar_t cchar   = { 0 };

    int pair_cell;

    if (arch_proc_ip_addr(core, pix) == addr) {
        pair_cell = PAIR_SELECTED_IP;
    } else if (arch_proc_sp_addr(core, pix) == addr) {
        pair_cell = PAIR_SELECTED_SP;
    } else {
        pair_cell = pair;
    }

    setcchar(&cchar, gsym, 0, pair_cell, NULL);
    mvadd_wch(l, gcol, &cchar);
}

void ui_print_process_genes(int l, u64 pix) {
    ui_line(true, l, ui_proc_pair(pix), A_NORMAL, "%s : %#18lx :", ui_proc_state(pix), pix);

    if (!proc_is_live(&g_cores[g_core], pix)) {
        return;
    }

    const Core *core = &g_cores[g_core];

    int scol = PANE_WIDTH + 2;
    int gcol = scol - g_proc_gene_scroll;
    u64 mb0a = arch_proc_mb0_addr(core, pix);
    u64 mb0s = arch_proc_mb0_size(core, pix);
    u64 mb1a = arch_proc_mb1_addr(core, pix);
    u64 mb1s = arch_proc_mb1_size(core, pix);

    for (u64 gidx = 0; gidx < mb0s && gcol < COLS; ++gidx, ++gcol) {
        if (gcol >= scol) {
            ui_print_process_gene(l, gcol, gidx, mb0a, pix, PAIR_SELECTED_MB1);
        }
    }

    for (u64 gidx = 0; gidx < mb1s && gcol < COLS; ++gidx, ++gcol) {
        if (gcol >= scol) {
            ui_print_process_gene(l, gcol, gidx, mb1a, pix, PAIR_SELECTED_MB2);
        }
    }

    clrtoeol();
}

void ui_print_process_field_header_element(int l, int fidx, const char *name) {
    assert(fidx >= 0);
    assert(name);

    if (fidx < (int)g_proc_field_scroll) {
        return;
    }

    int foff = fidx - g_proc_field_scroll;
    int fcol = foff * PROC_FIELD_WIDTH + PANE_WIDTH - 1;

    ui_field(l, fcol, PAIR_NORMAL, A_NORMAL, " : %18s", name);
}

void ui_print_process_field_header(int l) {
    ui_line(true, l, PAIR_NORMAL, A_NORMAL, "%s : %18s", "stat", "pix");

    int fidx = 0;

#define PROC_FIELD(type, name) ui_print_process_field_header_element(l, fidx++, #name);
    PROC_FIELDS
#undef PROC_FIELD
}

void ui_print_process_field_element(int l, int fidx, int fclr, u64 field) {
    assert(fidx >= 0);

    if (fidx < (int)g_proc_field_scroll) {
        return;
    }

    int foff = fidx - g_proc_field_scroll;
    int fcol = foff * PROC_FIELD_WIDTH + PANE_WIDTH - 1;

    ui_field(l, fcol, fclr, A_NORMAL, " : %#18lx", field);
}


void ui_print_process_fields(int l, u64 pix) {
    ui_line(true, l, ui_proc_pair(pix), A_NORMAL, "%s : %#18lx", ui_proc_state(pix), pix);

    const Proc *proc = proc_get(&g_cores[g_core], pix);
    int         fidx = 0;
    int         fclr = ui_proc_pair(pix);

#define PROC_FIELD(type, name) ui_print_process_field_element(l, fidx++, fclr, proc->name);
    PROC_FIELDS
#undef PROC_FIELD
}

void ui_print_process(int l) {
    l++;

    ui_line(true, l++, PAIR_HEADER, A_BOLD,
        "PROCESS [vs:%#lx | ps:%#lx | pf:%#lx | pl:%#lx | fs:%#lx | gs:%#lx]",
        g_proc_scroll,
        g_proc_selected,
        g_cores[g_core].pfst,
        g_cores[g_core].plst,
        g_proc_field_scroll,
        g_proc_gene_scroll
    );

    u64 pix = g_proc_scroll;

    if (g_proc_genes) {
        ui_print_process_genome_header(l++);

        while (l < LINES) {
            ui_print_process_genes(l++, pix++);
        }
    } else {
        ui_print_process_field_header(l++);

        while (l < LINES) {
            ui_print_process_fields(l++, pix++);
        }
    }
}

void ui_world_resize() {
    assert(g_wrld_zoom);

    g_vlin     = 0;
    g_vsiz     = 0;
    g_vlin_rng = 0;
    g_vsiz_rng = 0;

    if (COLS > PANE_WIDTH) {
        g_vlin     = COLS - PANE_WIDTH;
        g_vsiz     = LINES * g_vlin;
        g_vlin_rng = g_vlin * g_wrld_zoom;
        g_vsiz_rng = g_vsiz * g_wrld_zoom;

        gfx_resize(g_vsiz);
    }
}

void ui_print_cell(u64 i, u64 r, u64 x, u64 y) {
    wchar_t inst_nstr[2] = { L'\0', L'\0' };
    cchar_t cchar        = { 0 };
    u64     inst_avrg    = g_gfx_inst[i] / g_wrld_zoom;

    if (g_wrld_zoom == 1) {
        inst_nstr[0] = arch_symbol((u8)inst_avrg);
    } else {
        inst_nstr[0] = g_zoomed_symbols[(u8)inst_avrg];
    }

    int pair_cell;

    if (g_wcursor_mode && r == (u64)g_wcursor_x && y == (u64)g_wcursor_y) {
        pair_cell = PAIR_NORMAL;
    } else if (g_gfx_ipas[i] != 0) {
        pair_cell = PAIR_SELECTED_IP;
    } else if (g_gfx_spas[i] != 0) {
        pair_cell = PAIR_SELECTED_SP;
    } else if (g_gfx_mb0s[i] != 0) {
        pair_cell = PAIR_SELECTED_MB1;
    } else if (g_gfx_mb1s[i] != 0) {
        pair_cell = PAIR_SELECTED_MB2;
    } else if (g_gfx_mbst[i] != 0) {
        pair_cell = PAIR_MEM_BLOCK_START;
    } else if (g_gfx_mall[i] != 0) {
        pair_cell = PAIR_ALLOC_CELL;
    } else {
        pair_cell = PAIR_FREE_CELL;
    }

    setcchar(&cchar, inst_nstr, 0, pair_cell, NULL);
    mvadd_wch(y, x, &cchar);
}

void ui_print_wcursor_bar() {
    ui_clear_line(LINES - 1);

    const Core *core = &g_cores[g_core];

    char cmnem[MNEMONIC_BUFF_SIZE];
    char cownr[PROC_FIELD_WIDTH];

    u64 cpos  = g_vlin * g_wcursor_y + g_wcursor_x;
    u64 caddr = cpos * g_wrld_zoom + g_wrld_pos;
    u8  cbyte = mvec_get_byte(core, caddr);

    if (mvec_is_alloc(core, caddr)) {
        g_wcursor_pointed = mvec_get_owner(core, caddr);
        snprintf(cownr, PROC_FIELD_WIDTH, "%#lx", g_wcursor_pointed);
    } else {
        g_wcursor_pointed = (u64)(-1);
        snprintf(cownr, PROC_FIELD_WIDTH, "-");
    }

    arch_mnemonic(cbyte, cmnem);

    mvprintw(
        LINES - 1,
        1,
        (
            "cursor"
            " | x:%#x"
            " | y:%#x"
            " | addr:%#lx"
            " | isum:%#lx"
            " | iavr:%#lx"
            " | mall:%#lx"
            " | mbst:%#lx"
            " | mnem:<%s>"
            " | ownr:%s"
        ),
        g_wcursor_x,
        g_wcursor_y,
        caddr,
        g_gfx_inst[cpos],
        g_gfx_inst[cpos] / g_wrld_zoom,
        g_gfx_mall[cpos],
        g_gfx_mbst[cpos],
        cmnem,
        cownr
    );
}

void ui_print_world(int l) {
    l++;

    ui_line(false, l++, PAIR_HEADER, A_BOLD, "WORLD");
    ui_ulx_field(l++, "wrlp", g_wrld_pos);
    ui_ulx_field(l++, "wrlz", g_wrld_zoom);
    ui_ulx_field(l++, "psel", g_proc_selected);
    ui_ulx_field(l++, "pabs", g_proc_selected % g_cores[g_core].pcap);
    ui_ulx_field(l++, "vrng", g_vsiz_rng);
    ui_str_field(l++, "curs", g_wcursor_mode ? "on" : "off");

    l++;

    ui_line(false, l++, PAIR_HEADER, A_BOLD, "SELECTED");

    const Proc *psel = proc_get(&g_cores[g_core], g_proc_selected);

#define PROC_FIELD(type, name) ui_ulx_field(l++, #name, psel->name);
    PROC_FIELDS
#undef PROC_FIELD

    if (!g_vlin) {
        return;
    }

    gfx_render(&g_cores[g_core], g_wrld_pos, g_wrld_zoom, g_proc_selected);

    if (g_wcursor_mode) {
        int xmax = g_vlin - 1;
        int ymax = LINES - 2;

        g_wcursor_x = (g_wcursor_x < xmax) ? g_wcursor_x : xmax;
        g_wcursor_y = (g_wcursor_y < ymax) ? g_wcursor_y : ymax;
    }

    for (u64 i = 0; i < g_vsiz; ++i) {
        u64 r = i % g_vlin;
        u64 x = r + PANE_WIDTH;
        u64 y = i / g_vlin;

        ui_print_cell(i, r, x, y);
    }

    if (g_wcursor_mode) {
        ui_print_wcursor_bar();
    }
}

void ui_print_ipc_field(int l, u64 i, int color) {
    u8  iinst = g_cores[g_core].iviv[i];
    u64 iaddr = g_cores[g_core].ivav[i];

    ui_field(l, PANE_WIDTH, color, A_NORMAL, "%#18x : %#18x : %#18x", i, iinst, iaddr);
}

void ui_print_ipc_data() {
    ui_field(0, PANE_WIDTH, PAIR_NORMAL, A_NORMAL, "%18s : %18s : %18s", "ipci", "inst", "addr");

    int l = 1 - g_ivpt_scroll;

    for (u64 i = 0; i < SYNC_INTERVAL; ++i) {
        if (i == g_cores[g_core].ivpt) {
            if (l >= 1) {
                ui_print_ipc_field(l++, i, PAIR_SELECTED_PROC);
            }

            continue;
        }

        u8 iinst = g_cores[g_core].iviv[i];

        if ((iinst & IPCM_FLAG) != 0) {
            if (l >= 1) {
                ui_print_ipc_field(l++, i, PAIR_LIVE_PROC);
            }

            continue;
        }
    }

    for (; l < LINES; ++l) {
        if (l >= 1) {
            move(l, PANE_WIDTH);
            clrtoeol();
        }
    }
}

void ui_print_ipc(int l) {
    l++;

    const Core *core = &g_cores[g_core];

    ui_line(true, l++, PAIR_HEADER, A_BOLD, "IPC [%#lx]", g_ivpt_scroll);
    ui_ulx_field(l++, "ivpt", core->ivpt);
    ui_ulx_field(l++, "ivpi", core->iviv[core->ivpt]);
    ui_ulx_field(l++, "ivpa", core->ivav[core->ivpt]);

    ui_print_ipc_data();
}

void ui_print() {
    int l = 1;

    ui_line(false, l++, PAIR_HEADER, A_BOLD, "SALIS [%d:%d]", g_core, CORE_COUNT);
    ui_str_field(l++, "name", SIM_NAME);
    ui_ulx_field(l++, "seed", SEED);
    ui_str_field(l++, "fbit", MUTA_FLIP_BIT ? "yes" : "no");
    ui_ulx_field(l++, "asav", AUTO_SAVE_INTERVAL);
    ui_str_field(l++, "arch", ARCHITECTURE);
    ui_ulx_field(l++, "size", MVEC_SIZE);
    ui_ulx_field(l++, "syni", SYNC_INTERVAL);
    ui_ulx_field(l++, "step", g_steps);
    ui_ulx_field(l++, "sync", g_syncs);
    ui_ulx_field(l++, "step", g_step_block);

    switch (g_page) {
    case PAGE_CORE:
        ui_print_core(l);
        break;
    case PAGE_PROCESS:
        ui_print_process(l);
        break;
    case PAGE_WORLD:
        ui_print_world(l);
        break;
    case PAGE_IPC:
        ui_print_ipc(l);
        break;
    default:
        break;
    }
}

void ev_vscroll(int ev) {
    switch (g_page) {
    case PAGE_PROCESS:
        switch (ev) {
        case 'W':
            g_proc_scroll += (LINES > PROC_PAGE_LINES) ? LINES - PROC_PAGE_LINES : 0;
            break;
        case 'S':
            g_proc_scroll -= (LINES > PROC_PAGE_LINES) ? LINES - PROC_PAGE_LINES : 0;
            break;
        case 'w':
            g_proc_scroll += 1;
            break;
        case 's':
            g_proc_scroll -= 1;
            break;
        case 'q':
            g_proc_scroll = 0;
            break;
        default:
            break;
        }

        break;
    case PAGE_WORLD: {
        switch (ev) {
        case 'W':
            g_wrld_pos += g_vsiz_rng;
            break;
        case 'S':
            g_wrld_pos -= g_vsiz_rng;
            break;
        case 'w':
            g_wrld_pos += g_vlin_rng;
            break;
        case 's':
            g_wrld_pos -= g_vlin_rng;
            break;
        case 'q':
            g_wrld_pos = 0;
            break;
        default:
            break;
        }

        break;
    }
    case PAGE_IPC:
        switch (ev) {
        case 'W':
            g_ivpt_scroll += LINES;
            break;
        case 'S':
            g_ivpt_scroll -= g_ivpt_scroll < (u64)LINES ? g_ivpt_scroll : (u64)LINES;
            break;
        case 'w':
            g_ivpt_scroll += 1;
            break;
        case 's':
            g_ivpt_scroll -= g_ivpt_scroll ? 1 : 0;
            break;
        case 'q':
            g_ivpt_scroll = 0;
            break;
        }

        break;
    default:
        break;
    }
}

void ev_hscroll(int ev) {
    switch (g_page) {
    case PAGE_PROCESS: {
        u64 *hs_var = g_proc_genes ? &g_proc_gene_scroll : &g_proc_field_scroll;

        switch (ev) {
        case 'A':
            *hs_var = 0;
            break;
        case 'a':
            *hs_var -= *hs_var ? 1 : 0;
            break;
        case 'd':
            (*hs_var)++;
            break;
        default:
            break;
        }

        break;
    }

    case PAGE_WORLD:
        switch (ev) {
        case 'a':
            g_wrld_pos -= g_wrld_zoom;
            break;
        case 'd':
            g_wrld_pos += g_wrld_zoom;
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
}

void ev_zoom(int ev) {
    switch (g_page) {
    case PAGE_WORLD:
        switch (ev) {
        case 'x':
            g_wrld_zoom *= (g_vlin != 0 && g_vsiz_rng < MVEC_SIZE) ? 2 : 1;
            ui_world_resize();
            break;
        case 'z':
            g_wrld_zoom /= (g_wrld_zoom != 1) ? 2 : 1;
            ui_world_resize();
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
}

void ev_move_wcursor(int ev) {
    switch (ev) {
    case KEY_UP:
        g_wcursor_y -= (g_wcursor_y != 0) ? 1 : 0;
        break;
    case KEY_DOWN:
        g_wcursor_y += (g_wcursor_y < LINES - 2) ? 1 : 0;
        break;
    case KEY_LEFT:
        g_wcursor_x -= (g_wcursor_x != 0) ? 1 : 0;
        break;
    case KEY_RIGHT:
        g_wcursor_x += ((u64)g_wcursor_x < g_vlin - 1) ? 1 : 0;
        break;
    default:
        break;
    }
}

void ev_sel_proc(int ev) {
    if (g_page != PAGE_PROCESS && g_page != PAGE_WORLD) {
        return;
    }

    switch (ev) {
    case 'o':
        g_proc_selected -= 1;
        break;
    case 'p':
        g_proc_selected += 1;
        break;
    case 'f':
        g_proc_selected = g_cores[g_core].pfst;
        break;
    case 'l':
        g_proc_selected = g_cores[g_core].plst;
        break;
    default:
        break;
    }
}

void ev_goto_sel_proc() {
    switch (g_page) {
    case PAGE_PROCESS:
        g_proc_scroll = g_proc_selected;
        break;
    case PAGE_WORLD:
        g_wrld_pos = g_cores[g_core].pvec[g_proc_selected % g_cores[g_core].pcap].mb0a;
        break;
    default:
        break;
    }
}

void ev_handle() {
    int ev = getch();

    if (g_page == PAGE_WORLD && g_wcursor_mode) {
        switch (ev) {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            ev_move_wcursor(ev);
            return;
        case '\n':
            if (g_wcursor_pointed != (u64)(-1)) {
                g_proc_selected = g_wcursor_pointed;
            }

            break;
        default:
            break;
        }
    }

    switch (ev) {
    case CTRL('c'):
        g_exit = true;
        break;
    case KEY_SLEFT:
        clear();
        g_core = (g_core - 1) % CORE_COUNT;
        break;
    case KEY_SRIGHT:
        clear();
        g_core = (g_core + 1) % CORE_COUNT;
        break;
    case KEY_LEFT:
        clear();
        g_page = (g_page - 1) % PAGE_COUNT;
        break;
    case KEY_RIGHT:
        clear();
        g_page = (g_page + 1) % PAGE_COUNT;
        break;
    case KEY_RESIZE:
        clear();
        ui_line_buff_resize();
        ui_world_resize();

        if (g_vlin) {
            while (g_vsiz_rng >= MVEC_SIZE * 2 && g_wrld_zoom != 1) {
                g_wrld_zoom /= 2;
                ui_world_resize();
            }
        }

        g_wcursor_mode = false;
        break;
    case 'W':
    case 'S':
    case 'w':
    case 's':
    case 'q':
        ev_vscroll(ev);
        break;
    case 'A':
    case 'a':
    case 'd':
        ev_hscroll(ev);
        break;
    case 'z':
    case 'x':
        ev_zoom(ev);
        break;
    case 'o':
    case 'p':
    case 'f':
    case 'l':
        ev_sel_proc(ev);
        break;
    case 'k':
        ev_goto_sel_proc();
        break;
    case 'g':
        if (g_page == PAGE_PROCESS) {
            clear();
            g_proc_genes = !g_proc_genes;
        }

        break;
    case 'c':
        if (g_page == PAGE_WORLD) {
            clear();

            if (g_vlin == 0) {
                g_wcursor_mode = false;
            } else {
                g_wcursor_mode = !g_wcursor_mode;
            }
        }

        break;
    case ' ':
        g_running = !g_running;
        nodelay(stdscr, g_running);
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
        if (!g_running) {
            u64 cycles = 1 << (((ev - '0') ? (ev - '0') : 10) - 1);
            salis_step(cycles);
        }

        break;
    default:
        break;
    }
}

void init() {
    setlocale(LC_ALL, "");

    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    start_color();
    init_color(COLOR_BLACK, 0, 0, 0);

    init_pair(PAIR_NORMAL,          COLOR_WHITE,  COLOR_BLACK  );
    init_pair(PAIR_HEADER,          COLOR_BLUE,   COLOR_BLACK  );
    init_pair(PAIR_LIVE_PROC,       COLOR_BLUE,   COLOR_BLACK  );
    init_pair(PAIR_SELECTED_PROC,   COLOR_YELLOW, COLOR_BLACK  );
    init_pair(PAIR_FREE_CELL,       COLOR_BLACK,  COLOR_BLUE   );
    init_pair(PAIR_ALLOC_CELL,      COLOR_BLACK,  COLOR_CYAN   );
    init_pair(PAIR_MEM_BLOCK_START, COLOR_BLACK,  COLOR_WHITE  );
    init_pair(PAIR_SELECTED_MB1,    COLOR_BLACK,  COLOR_YELLOW );
    init_pair(PAIR_SELECTED_MB2,    COLOR_BLACK,  COLOR_GREEN  );
    init_pair(PAIR_SELECTED_IP,     COLOR_BLACK,  COLOR_RED    );
    init_pair(PAIR_SELECTED_SP,     COLOR_BLACK,  COLOR_MAGENTA);

#if ACTION == ACT_NEW
    salis_init();
#elif ACTION == ACT_LOAD
    salis_load();
#endif

    g_wrld_zoom  = 1;
    g_step_block = 1;

    ui_line_buff_resize();
    ui_world_resize();
}

void exec() {
    while (!g_exit) {
        if (g_running) {
            clock_t beg = clock();
            salis_step(g_step_block - (g_steps % g_step_block));
            clock_t end = clock();

            if ((end - beg) < (CLOCKS_PER_SEC / 30)) {
                g_step_block <<= 1;
            }

            if ((end - beg) >= (CLOCKS_PER_SEC / 60) && g_step_block != 1) {
                g_step_block >>= 1;
            }
        }

        ui_print();
        ev_handle();
    }
}

void quit() {
    gfx_free();
    ui_line_buff_free();
    salis_save(SIM_PATH);
    salis_free();
    endwin();
}

int main() {
    init();
    exec();
    quit();

    return 0;
}
