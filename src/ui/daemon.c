// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * Implements a minimal UI for the Salis simulator with minimal output and
 * interruptible via OS signals. Ideal for running Salis in the background.
 */

#include <signal.h>
#include <unistd.h>

volatile bool g_running;
u64           g_step_block;

void sig_handler(int signo) {
    switch (signo) {
    case SIGINT:
    case SIGTERM:
        printf("signal received, stopping simulator...\n");
        g_running = false;
        break;
    }
}

void step_block() {
    clock_t beg = clock();
    salis_step(g_step_block - (g_steps % g_step_block));
    clock_t end = clock();

    if ((end - beg) < (CLOCKS_PER_SEC * 4)) {
        g_step_block <<= 1;
    }

    if ((end - beg) >= (CLOCKS_PER_SEC * 2) && g_step_block != 1) {
        g_step_block >>= 1;
    }

    printf("simulator running on step '%#lx'\n", g_steps);
}

int main() {
#if ACTION == ACT_NEW
    salis_init();
#elif ACTION == ACT_LOAD
    salis_load();
#endif

    g_running    = true;
    g_step_block = 1;

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    while (g_running) {
        step_block();
    }

    salis_save(SIM_PATH);
    salis_free();

    return 0;
}
