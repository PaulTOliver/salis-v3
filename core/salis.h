// File: salis.h
// Project: Salis v3
// Author: Paul Oliver
// Email: contact@pauloliver.dev

///////////////////////////////////////
// Constants
///////////////////////////////////////

#define SLS_INST_COUNT 0x20
#define SLS_MEM_ALLOC_FLAG 0x20
#define SLS_MEM_BLOCK_START_FLAG 0x40
#define SLS_MEM_WORMHOLE_FLAG 0x80
#define SLS_MEM_INST_MASK 0x1f
#define SLS_EVO_STATE_WORDS 4

///////////////////////////////////////
// Instruction set
///////////////////////////////////////

enum {
	// No-ops
	SLS_NOP0,
	SLS_NOP1,
	SLS_NOPA,
	SLS_NOPB,
	SLS_NOPC,
	SLS_NOPD,
	SLS_NOPE,
	SLS_NOPF,
	SLS_NOPG,

	// Control
	SLS_ADRB,
	SLS_ADRF,
	SLS_JMPB,
	SLS_JMPF,
	SLS_WHLE,
	SLS_ENDW,

	// Biology
	SLS_MALB,
	SLS_MALF,
	SLS_SPLT,
	SLS_BSWP,
	SLS_EATB,
	SLS_EATF,

	// Memory
	SLS_PUSH,
	SLS_PULL,
	SLS_COPY,
	SLS_WRMH,

	// Math
	SLS_ZERO,
	SLS_INCR,
	SLS_DECR,
	SLS_SHFL,
	SLS_SHFR,
	SLS_NAND,
	SLS_NTOR,
};

///////////////////////////////////////
// Main simulation control
///////////////////////////////////////

void salis_new(unsigned seed, uint32_t order, size_t cores, uint32_t anc_size, uint8_t *anc_data);
void salis_delete(void);
void salis_cycle(void);

///////////////////////////////////////
// Main getters
///////////////////////////////////////

uint64_t salis_get_cycle(void);
unsigned salis_get_seed(void);
uint32_t salis_get_order(void);
uint32_t salis_get_size(void);
uint32_t salis_get_cap(void);
size_t salis_get_cores(void);

///////////////////////////////////////
// Memory getters
///////////////////////////////////////

uint32_t salis_mem_get_flagged_alloc(size_t cidx);
uint32_t salis_mem_get_flagged_block_start(size_t cidx);
uint32_t salis_mem_get_flagged_wormhole(size_t cidx);
bool salis_mem_is_alloc_at(size_t cidx, uint32_t addr);
bool salis_mem_is_block_start_at(size_t cidx, uint32_t addr);
bool salis_mem_is_wormhole_at(size_t cidx, uint32_t addr);
uint8_t salis_mem_get_inst_at(size_t cidx, uint32_t addr);
uint8_t salis_mem_get_byte_at(size_t cidx, uint32_t addr);

///////////////////////////////////////
// Evolver getters
///////////////////////////////////////

uint32_t salis_evo_get_state(size_t cidx, size_t sidx);
uint64_t salis_evo_get_calls(size_t cidx);
uint64_t salis_evo_get_last_write(size_t cidx);
uint32_t salis_evo_get_last_address(size_t cidx);
uint8_t salis_evo_get_last_inst(size_t cidx);
bool salis_evo_wrote_last_cycle(size_t cidx);

///////////////////////////////////////
// Process getters
///////////////////////////////////////

// ...
