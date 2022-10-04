"""
File: test_evolver.py
Project: Salis v3
Author: Paul Oliver
Email: contact@pauloliver.dev
"""

from tests.test_base import Test_Base


class Test_Evolver_Disabled(Test_Base):
	CORES = 3
	CYCLES = 1000
	ORDER = 4
	SEED = 0

	def test_disabled(self):
		for cycle in range(self.CYCLES):
			self.lib.salis_cycle()
			self.assertEqual(self.lib.salis_get_cycle(), cycle + 1)

			for cidx in range(self.CORES):
				self.assertEqual(self.lib.salis_evo_get_calls(cidx), (cycle + 1) * 2)
				self.assertEqual(self.lib.salis_evo_get_last_write(cidx), cycle)
				self.assertEqual(self.lib.salis_evo_get_last_address(cidx), 0)
				self.assertEqual(self.lib.salis_evo_get_last_inst(cidx), 0)
				self.assertTrue(self.lib.salis_evo_wrote_last_cycle(cidx))

				for addr in range(self.lib.salis_get_size()):
					# TODO: Eventually I want to make the evolver spawn new
					# organisms when an existing instruction is written to
					# memory. I will need to adapt this test then.
					self.assertFalse(self.lib.salis_mem_is_alloc_at(cidx, addr))
					self.assertFalse(self.lib.salis_mem_is_block_start_at(cidx, addr))
					self.assertFalse(self.lib.salis_mem_is_wormhole_at(cidx, addr))
					self.assertEqual(self.lib.salis_mem_get_inst_at(cidx, addr), self.lib.SLS_NOP0)
					self.assertEqual(self.lib.salis_mem_get_byte_at(cidx, addr), 0)

				for sidx in range(self.consts["SLS_EVO_STATE_WORDS"]):
					self.assertEqual(self.lib.salis_evo_get_state(cidx, sidx), 0)


class Test_Evolver_Enabled(Test_Base):
	CORES = 1
	ORDER = 16
	SEED = 123456

	def test_enabled(self):
		while not self.lib.salis_evo_wrote_last_cycle(0):
			self.lib.salis_cycle()

		cycle = self.lib.salis_get_cycle()
		last_address = self.lib.salis_evo_get_state(0, 1)
		last_inst = self.lib.salis_evo_get_state(0, 0) % self.consts["SLS_INST_COUNT"]

		self.assertTrue(last_address < self.lib.salis_get_size())
		self.assertEqual(self.lib.salis_evo_get_calls(0), cycle + 1)
		self.assertEqual(self.lib.salis_evo_get_last_write(0), cycle - 1)
		self.assertEqual(self.lib.salis_evo_get_last_address(0), last_address)
		self.assertEqual(self.lib.salis_evo_get_last_inst(0), last_inst)

		for addr in range(self.lib.salis_get_size()):
			# TODO: Eventually I want to make the evolver spawn new
			# organisms when an existing instruction is written to
			# memory. I will need to adapt this test then.
			self.assertFalse(self.lib.salis_mem_is_alloc_at(0, addr))
			self.assertFalse(self.lib.salis_mem_is_block_start_at(0, addr))
			self.assertFalse(self.lib.salis_mem_is_wormhole_at(0, addr))

			exp_inst = last_inst if addr == last_address else self.lib.SLS_NOP0
			exp_byte = exp_inst

			self.assertEqual(self.lib.salis_mem_get_inst_at(0, addr), exp_inst)
			self.assertEqual(self.lib.salis_mem_get_byte_at(0, addr), exp_byte)
