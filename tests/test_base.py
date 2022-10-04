"""
File: test_base.py
Project: Salis v3
Author: Paul Oliver
Email: contact@pauloliver.dev
"""

import unittest

from modules.salis import Salis


class Test_Base(unittest.TestCase):
	SEED = 0
	ORDER = 8
	CORES = 2
	ANC_SIZE = 0
	ANC_DATA = None

	def setUp(self):
		self.salis = Salis(debug=True)
		self.ffi = self.salis.ffi
		self.lib = self.salis.lib
		self.consts = self.salis.consts

		# Transform ancestor data into C array.
		if self.ANC_DATA is None:
			self.ANC_DATA = self.ffi.NULL
		else:
			self.ANC_SIZE = len(self.ANC_DATA)
			self.ANC_DATA = self.ffi.new(
				"uint8_t[]", [self.salis.inst_from_symb(i) for i in self.ANC_DATA]
			)

		self.lib.salis_new(self.SEED, self.ORDER, self.CORES, self.ANC_SIZE, self.ANC_DATA)

	def tearDown(self):
		self.lib.salis_delete()
