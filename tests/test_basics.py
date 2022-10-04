"""
File: test_basics.py
Project: Salis v3
Author: Paul Oliver
Email: contact@pauloliver.dev
"""

from tests.test_base import Test_Base


class Test_Basic(Test_Base):
	def test_flags(self):
		self.assertEqual(self.consts["SLS_INST_COUNT"], 32)
		self.assertEqual(self.consts["SLS_MEM_ALLOC_FLAG"], 1 << 5)
		self.assertEqual(self.consts["SLS_MEM_BLOCK_START_FLAG"], 1 << 6)
		self.assertEqual(self.consts["SLS_MEM_WORMHOLE_FLAG"], 1 << 7)
		self.assertEqual(self.consts["SLS_MEM_INST_MASK"], 0b11111)
		self.assertEqual(self.consts["SLS_EVO_STATE_WORDS"], 4)


class Test_Initial_State_Empty(Test_Base):
	def test_initial_state(self):
		self.assertEqual(self.lib.salis_get_cycle(), 0)
		self.assertEqual(self.lib.salis_get_seed(), self.SEED)
		self.assertEqual(self.lib.salis_get_order(), self.ORDER)
		self.assertEqual(self.lib.salis_get_size(), 1 << self.ORDER)
		self.assertEqual(self.lib.salis_get_cap(), (1 << self.ORDER) // 2)
		self.assertEqual(self.lib.salis_get_cores(), self.CORES)

		for cidx in range(self.CORES):
			self.assertEqual(self.lib.salis_mem_get_flagged_alloc(cidx), 0)
			self.assertEqual(self.lib.salis_mem_get_flagged_block_start(cidx), 0)
			self.assertEqual(self.lib.salis_mem_get_flagged_wormhole(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_calls(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_write(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_address(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_inst(cidx), 0)
			self.assertFalse(self.lib.salis_evo_wrote_last_cycle(cidx))

			for addr in range(self.lib.salis_get_size()):
				self.assertFalse(self.lib.salis_mem_is_alloc_at(cidx, addr))
				self.assertFalse(self.lib.salis_mem_is_block_start_at(cidx, addr))
				self.assertFalse(self.lib.salis_mem_is_wormhole_at(cidx, addr))
				self.assertEqual(self.lib.salis_mem_get_inst_at(cidx, addr), self.lib.SLS_NOP0)
				self.assertEqual(self.lib.salis_mem_get_byte_at(cidx, addr), 0)

			for sidx in range(self.consts["SLS_EVO_STATE_WORDS"]):
				self.assertEqual(self.lib.salis_evo_get_state(cidx, sidx), 0)


class Test_Initial_State_Compiled(Test_Base):
	SEED = 123456
	ANC_DATA = "abcdefg.:.:"

	def test_initial_state(self):
		self.assertEqual(self.lib.salis_get_cycle(), 0)
		self.assertEqual(self.lib.salis_get_seed(), self.SEED)
		self.assertEqual(self.lib.salis_get_order(), self.ORDER)
		self.assertEqual(self.lib.salis_get_size(), 1 << self.ORDER)
		self.assertEqual(self.lib.salis_get_cap(), (1 << self.ORDER) // 2)
		self.assertEqual(self.lib.salis_get_cores(), self.CORES)

		for cidx in range(self.CORES):
			self.assertEqual(self.lib.salis_mem_get_flagged_alloc(cidx), len(self.ANC_DATA))
			self.assertEqual(self.lib.salis_mem_get_flagged_block_start(cidx), 1)
			self.assertEqual(self.lib.salis_mem_get_flagged_wormhole(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_calls(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_write(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_address(cidx), 0)
			self.assertEqual(self.lib.salis_evo_get_last_inst(cidx), 0)
			self.assertFalse(self.lib.salis_evo_wrote_last_cycle(cidx))

			for addr in range(self.lib.salis_get_size()):
				self.assertEqual(self.lib.salis_mem_is_alloc_at(cidx, addr), addr < len(self.ANC_DATA))
				self.assertEqual(self.lib.salis_mem_is_block_start_at(cidx, addr), addr == 0)
				self.assertFalse(self.lib.salis_mem_is_wormhole_at(cidx, addr))

				self.assertEqual(
					self.lib.salis_mem_get_inst_at(cidx, addr),
					self.ANC_DATA[addr] if addr < len(self.ANC_DATA) else self.lib.SLS_NOP0,
				)

			for sidx in range(self.consts["SLS_EVO_STATE_WORDS"]):
				self.assertNotEqual(self.lib.salis_evo_get_state(cidx, sidx), 0)

	def test_looping(self):
		for cidx in range(self.CORES):
			for addr in range(self.lib.salis_get_size() * -5, self.lib.salis_get_size() * 5):
				self.assertEqual(
					self.lib.salis_mem_get_byte_at(cidx, self.ffi.cast("uint32_t", addr)),
					self.lib.salis_mem_get_byte_at(cidx, addr % self.lib.salis_get_size()),
				)
