from .mem_io import *
import argparse
from .disassembler import Disassembler

MAX_X64_INST_SIZE = 16

class Disassemble(MemRead):
    ARGUMENTS = [
        CommandArgument("count", ["-c", "--count"], "Number of instructions to disassemble"),
    ]

    def __init__(self, address, count):
        MemRead.__init__(self, address, count * MAX_X64_INST_SIZE)
        self.disas = Disassembler()
        self.address = address
        self.count = count

    def print_response(self):
        if self.raw_data:
            data = self.raw_data[self.response_struct.sizeof():]
            
            for i, code in enumerate(self.disas.disas(data, self.address)):
                if i > self.count: break
                print(f"{hex(code.address)}:\t{code.mnemonic}\t{code.op_str}")
                # print("0x%x:\t%s\t%s" %(code.address, code.mnemonic, code.op_str))