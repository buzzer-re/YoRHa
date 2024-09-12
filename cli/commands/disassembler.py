from capstone import *

class Disassembler:
    def __init__(self):
        self.cs = Cs(CS_ARCH_X86, CS_MODE_64)

    def disas(self, data, base_address = 0):
        return self.cs.disasm(data, base_address)
    
    