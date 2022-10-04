// File: memory.c
// Project: Salis v3
// Author: Paul Oliver
// Email: contact@pauloliver.dev

///////////////////////////////////////
// Private functions
///////////////////////////////////////

static void _mem_set_alloc_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(!salis_mem_is_alloc_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_ALLOC_FLAG;
	++core->mem_flagged_alloc;
	assert(salis_mem_is_alloc_at(cidx, addr));
	assert(core->mem_flagged_alloc);
	assert(core->mem_flagged_alloc <= g_size);
}

static void _mem_unset_alloc_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(core->mem_flagged_alloc);
	assert(salis_mem_is_alloc_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_ALLOC_FLAG;
	--core->mem_flagged_alloc;
	assert(!salis_mem_is_alloc_at(cidx, addr));
	assert(core->mem_flagged_alloc <= g_size);
}

static void _mem_set_block_start_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(!salis_mem_is_block_start_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_BLOCK_START_FLAG;
	++core->mem_flagged_block_start;
	assert(salis_mem_is_block_start_at(cidx, addr));
	assert(core->mem_flagged_block_start);
	assert(core->mem_flagged_block_start <= g_size);
}

static void _mem_unset_block_start_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(core->mem_flagged_block_start);
	assert(salis_mem_is_block_start_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_BLOCK_START_FLAG;
	--core->mem_flagged_block_start;
	assert(!salis_mem_is_block_start_at(cidx, addr));
	assert(core->mem_flagged_block_start <= g_size);
}

static void _mem_set_wormhole_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(!salis_mem_is_wormhole_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_WORMHOLE_FLAG;
	++core->mem_flagged_wormhole;
	assert(salis_mem_is_wormhole_at(cidx, addr));
	assert(core->mem_flagged_wormhole);
	assert(core->mem_flagged_wormhole <= g_size);
}

static void _mem_unset_wormhole_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(core->mem_flagged_wormhole);
	assert(salis_mem_is_wormhole_at(cidx, addr));
	addr %= g_size;
	core->mem_data[addr] ^= SLS_MEM_WORMHOLE_FLAG;
	--core->mem_flagged_wormhole;
	assert(!salis_mem_is_wormhole_at(cidx, addr));
	assert(core->mem_flagged_wormhole <= g_size);
}

static void _mem_set_inst_at(size_t cidx, uint32_t addr, uint8_t inst) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	assert(inst <= SLS_INST_COUNT);
	addr %= g_size;
	core->mem_data[addr] &= (SLS_MEM_ALLOC_FLAG | SLS_MEM_BLOCK_START_FLAG | SLS_MEM_WORMHOLE_FLAG);
	core->mem_data[addr] |= inst;
}

///////////////////////////////////////
// Memory getters
///////////////////////////////////////

uint32_t salis_mem_get_flagged_alloc(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->mem_flagged_alloc;
}

uint32_t salis_mem_get_flagged_block_start(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->mem_flagged_block_start;
}

uint32_t salis_mem_get_flagged_wormhole(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->mem_flagged_wormhole;
}

bool salis_mem_is_alloc_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	addr %= g_size;
	return (bool)(core->mem_data[addr] & SLS_MEM_ALLOC_FLAG);
}

bool salis_mem_is_block_start_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	addr %= g_size;
	return (bool)(core->mem_data[addr] & SLS_MEM_BLOCK_START_FLAG);
}

bool salis_mem_is_wormhole_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	addr %= g_size;
	return (bool)(core->mem_data[addr] & SLS_MEM_WORMHOLE_FLAG);
}

uint8_t salis_mem_get_inst_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	addr %= g_size;
	return core->mem_data[addr] & SLS_MEM_INST_MASK;
}

uint8_t salis_mem_get_byte_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	addr %= g_size;
	return core->mem_data[addr];
}
