"""
File: salis.py
Project: Salis v3
Author: Paul Oliver
Email: contact@pauloliver.dev
"""

import os
import pathlib
import subprocess

import bidict
import cffi


class Salis:
	""" Direct interface to the Salis library. DLL is loaded and stored as a
	class member. All exported library functions and constants are made available
	as members of the 'lib' property.
	"""
	@staticmethod
	def get_repo_root():
		return str(pathlib.Path(__file__).parent.parent.absolute())

	def __init__(self, debug: bool=False):
		# Select library to load, either the debug (-ggdb) or release (-O3) DLL.
		self.__debug = debug
		self.__lib_type = "deb" if self.__debug else "rel"
		self.__lib_file = os.path.join(self.get_repo_root(), f"core/lib/libsalis-{self.__lib_type}.so")
		self.__ffi = cffi.FFI()
		self.__lib = self.__ffi.dlopen(self.__lib_file)

		# Parse header to extract symbols I am interested in.
		self.__header = os.path.join(self.get_repo_root(), "core/salis.h")
		self.__ffi.cdef(subprocess.check_output(["cpp", self.__header]).decode())

		# Manually parse macro constants and add them as attributes to the lib object.
		self.__consts = {}

		with open(self.__header, "r") as f:
			for line in f.readlines():
				if line.startswith("#define SLS_"):
					const_key = line.split()[1]
					const_val = int(line.split()[2], 0)
					self.__consts[const_key] = const_val

		# Generate symbol map to render instructions to the screen.
		self.__inst_to_symb_map = bidict.bidict({
			# No-ops
			self.lib.SLS_NOP0: ".",
			self.lib.SLS_NOP1: ":",
			self.lib.SLS_NOPA: "a",
			self.lib.SLS_NOPB: "b",
			self.lib.SLS_NOPC: "c",
			self.lib.SLS_NOPD: "d",
			self.lib.SLS_NOPE: "e",
			self.lib.SLS_NOPF: "f",
			self.lib.SLS_NOPG: "g",

			# Control
			self.lib.SLS_ADRB: "[",
			self.lib.SLS_ADRF: "]",
			self.lib.SLS_JMPB: "(",
			self.lib.SLS_JMPF: ")",
			self.lib.SLS_WHLE: "?",
			self.lib.SLS_ENDW: "_",

			# Biology
			self.lib.SLS_MALB: "{",
			self.lib.SLS_MALF: "}",
			self.lib.SLS_SPLT: "$",
			self.lib.SLS_BSWP: "%",
			self.lib.SLS_EATB: "E",
			self.lib.SLS_EATF: "3",

			# Memory
			self.lib.SLS_PUSH: "#",
			self.lib.SLS_PULL: "~",
			self.lib.SLS_COPY: "x",
			self.lib.SLS_WRMH: "w",

			# Math
			self.lib.SLS_ZERO: "z",
			self.lib.SLS_INCR: "^",
			self.lib.SLS_DECR: "v",
			self.lib.SLS_SHFL: "<",
			self.lib.SLS_SHFR: ">",
			self.lib.SLS_NAND: "&",
			self.lib.SLS_NTOR: "|",
		})

		self.__symb_to_inst_map = self.__inst_to_symb_map.inverse

	@property
	def ffi(self):
		return self.__ffi

	@property
	def lib(self):
		return self.__lib

	@property
	def consts(self):
		return self.__consts

	def symb_from_inst(self, inst):
		return self.__inst_to_symb_map[inst]

	def inst_from_symb(self, symb):
		return self.__symb_to_inst_map[symb]
