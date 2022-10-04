// File: evolver.c
// Project: Salis v3
// Author: Paul Oliver
// Email: contact@pauloliver.dev

///////////////////////////////////////
// Private functions
///////////////////////////////////////

static uint32_t _evo_generate_random_number(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	uint32_t tmp_1;
	uint32_t tmp_2;
	tmp_2 = core->evo_state[3];
	tmp_2 ^= tmp_2 << 11;
	tmp_2 ^= tmp_2 >> 8;
	core->evo_state[3] = core->evo_state[2];
	core->evo_state[2] = core->evo_state[1];
	core->evo_state[1] = core->evo_state[0];
	tmp_1 = core->evo_state[0];
	tmp_2 ^= tmp_1;
	tmp_2 ^= tmp_1 >> 19;
	core->evo_state[0] = tmp_2;
	++core->evo_calls;
	return tmp_2;
}

static uint8_t _evo_randomize_at(size_t cidx, uint32_t addr) {
	assert(g_is_init);
	assert(cidx < g_cores);
	size_t nidx = (cidx + 1) % g_cores;
	uint8_t inst = (uint8_t)(_evo_generate_random_number(cidx) % SLS_INST_COUNT);

	if (!salis_mem_is_wormhole_at(cidx, addr)) {
		// TODO: Create new organism if written instruction matches the
		// existing instruction.
		_mem_set_inst_at(cidx, addr, inst);
	} else if (salis_mem_is_wormhole_at(nidx, addr)) {
		_mem_set_inst_at(nidx, addr, inst);
	}

	return inst;
}

static void _evo_cycle(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	uint32_t rand_num = _evo_generate_random_number(cidx);

	if (rand_num < g_size) {
		core->evo_last_write = g_cycle;
		core->evo_last_address = rand_num;
		core->evo_last_inst = _evo_randomize_at(cidx, core->evo_last_address);
		core->evo_wrote_last_cycle = true;
	} else {
		core->evo_wrote_last_cycle = false;
	}
}

///////////////////////////////////////
// Evolver getters
///////////////////////////////////////

uint32_t salis_evo_get_state(size_t cidx, size_t sidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	assert(sidx < SLS_EVO_STATE_WORDS);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_state[sidx];
}

uint64_t salis_evo_get_calls(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_calls;
}

uint64_t salis_evo_get_last_write(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_last_write;
}

uint32_t salis_evo_get_last_address(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_last_address;
}

uint8_t salis_evo_get_last_inst(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_last_inst;
}

bool salis_evo_wrote_last_cycle(size_t cidx) {
	assert(g_is_init);
	assert(cidx < g_cores);
	struct Salis_Core *core = &g_salis_cores[cidx];
	return core->evo_wrote_last_cycle;
}
