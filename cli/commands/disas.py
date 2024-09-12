from .mem_io import *
import argparse
from .disassembler import Disassembler

MAX_X64_INST_SIZE = 16

class Disassemble(MemRead):
    ARGUMENTS = [
        CommandArgument("count", ["--count", "-c"], "Number of instructions to disassemble", arg_type=int),
        CommandArgument("address", [], "Address to be read", arg_type=int)
    ]

    def __init__(self, address, count, print_disas = True):
        MemRead.__init__(self, address, count * MAX_X64_INST_SIZE)
        self.disas = Disassembler()
        self.address = address
        self.count = count
        self.print_disas = print_disas
        self.code = None

    def print_response(self):
        self.code = self.disas.disas(self.data_read, self.address)
        if self.raw_data and self.print_disas:

            for i, code in enumerate(self.code):
                if i > self.count: break
                print(f"{hex(code.address)}:\t{code.mnemonic}\t{code.op_str}")
                # print("0x%x:\t%s\t%s" %(code.address, code.mnemonic, code.op_str))