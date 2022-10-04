// File: salis.c
// Project: Salis v3
// Author: Paul Oliver
// Email: contact@pauloliver.dev

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>

#include "salis.h"

///////////////////////////////////////
// Globals
///////////////////////////////////////

struct Salis_Core
{
	size_t core_id;
	thrd_t core_thread;

	// Memory state
	uint32_t mem_flagged_alloc;
	uint32_t mem_flagged_block_start;
	uint32_t mem_flagged_wormhole;
	uint8_t *mem_data;

	// Evolver state
	uint32_t evo_state[SLS_EVO_STATE_WORDS];
	uint64_t evo_calls;
	uint64_t evo_last_write;
	uint32_t evo_last_address;
	uint8_t evo_last_inst;
	bool evo_wrote_last_cycle;

	// Process state
	// ...
};

static bool g_is_init;
static uint64_t g_cycle;
static unsigned g_seed;
static uint32_t g_order;
static uint32_t g_size;
static uint32_t g_cap;
static size_t g_cores;
static struct Salis_Core *g_salis_cores;

///////////////////////////////////////
// Load modules
///////////////////////////////////////

#include "memory.c"
#include "evolver.c"

///////////////////////////////////////
// Private functions
///////////////////////////////////////

static int _cycle_core(void *arg) {
	size_t cidx = *(size_t *)arg;
	_evo_cycle(cidx);
	return 0;
}

///////////////////////////////////////
// Main simulation control
///////////////////////////////////////

void salis_new(unsigned seed, uint32_t order, size_t cores, uint32_t anc_size, uint8_t *anc_data) {
	assert(!g_is_init);

	g_is_init = true;
	g_seed = seed;
	g_order = order;
	g_size = 1 << g_order;
	g_cap = g_size / 2;
	g_cores = cores;
	g_salis_cores = calloc(g_cores, sizeof(struct Salis_Core));
	assert(g_salis_cores);

	if (seed) {
		srand(seed);
	}

	for (size_t cidx = 0; cidx < g_cores; ++cidx) {
		struct Salis_Core *core = &g_salis_cores[cidx];
		core->core_id = cidx;
		core->mem_data = calloc(g_size, sizeof(uint8_t));
		assert(core->mem_data);

		if (seed) {
			for (size_t sidx = 0; sidx < SLS_EVO_STATE_WORDS; ++sidx) {
				core->evo_state[sidx] = rand();
			}
		}

		if (anc_size) {
			assert(anc_data);
			_mem_set_block_start_at(cidx, 0);
		}

		for (uint32_t addr = 0; addr < anc_size; ++addr) {
			_mem_set_alloc_at(cidx, addr);
			_mem_set_inst_at(cidx, addr, anc_data[addr]);
		}
	}
}

void salis_delete(void) {
	assert(g_is_init);

	for (size_t cidx = 0; cidx < g_cores; ++cidx) {
		struct Salis_Core *core = &g_salis_cores[cidx];
		free(core->mem_data);
	}

	free(g_salis_cores);
}

void salis_cycle(void) {
	assert(g_is_init);

	for (size_t cidx = 0; cidx < g_cores; ++cidx) {
		struct Salis_Core *core = &g_salis_cores[cidx];
		thrd_create(&core->core_thread, _cycle_core, &core->core_id);
	}

	for (size_t cidx = 0; cidx < g_cores; ++cidx) {
		struct Salis_Core *core = &g_salis_cores[cidx];
		thrd_join(core->core_thread, NULL);
	}

	++g_cycle;
}

///////////////////////////////////////
// Main getters
///////////////////////////////////////

uint64_t salis_get_cycle(void) {
	assert(g_is_init);
	return g_cycle;
}

unsigned salis_get_seed(void) {
	assert(g_is_init);
	return g_seed;
}

uint32_t salis_get_order(void) {
	assert(g_is_init);
	return g_order;
}

uint32_t salis_get_size(void) {
	assert(g_is_init);
	return g_size;
}

uint32_t salis_get_cap(void) {
	assert(g_is_init);
	return g_cap;
}

size_t salis_get_cores(void) {
	assert(g_is_init);
	return g_cores;
}
