// Project: Salis
// Author:  Paul Oliver
// Email:   contact@pauloliver.dev

/*
 * Simple benchmark test helps measure simulation speed by stepping the
 * simulator N times and printing results.
 */

#if ACTION != ACT_BENCH
#error Using bench UI with unsupported action
#endif

int main() {
    printf("Salis Benchmark Test\n\n");

    salis_init("", SEED);
    salis_step(BENCH_STEPS);

    printf("seed        => %#lx\n", SEED);
    printf("g_steps     => %#lx\n", g_steps);
    printf("g_syncs     => %#lx\n", g_syncs);

    for (int i = 0; i < CORE_COUNT; ++i) {
        putchar('\n');
        printf("core %d mall => %#lx\n", i, g_cores[i].mall);
        printf("core %d mut0 => %#lx\n", i, g_cores[i].muta[0]);
        printf("core %d mut1 => %#lx\n", i, g_cores[i].muta[1]);
        printf("core %d mut2 => %#lx\n", i, g_cores[i].muta[2]);
        printf("core %d mut3 => %#lx\n", i, g_cores[i].muta[3]);
        printf("core %d pnum => %#lx\n", i, g_cores[i].pnum);
        printf("core %d pcap => %#lx\n", i, g_cores[i].pcap);
        printf("core %d pfst => %#lx\n", i, g_cores[i].pfst);
        printf("core %d plst => %#lx\n", i, g_cores[i].plst);
        printf("core %d pcur => %#lx\n", i, g_cores[i].pcur);
        printf("core %d psli => %#lx\n", i, g_cores[i].psli);
        printf("core %d ncyc => %#lx\n", i, g_cores[i].ncyc);
        printf("core %d ivpt => %#lx\n", i, g_cores[i].ivpt);
        putchar('\n');

        for (int j = 0; j < 32; ++j) {
            printf("%02x ", g_cores[i].mvec[j]);
        }

        putchar('\n');
    }

    salis_free();
}
