from nyxstone import Nyxstone


class Disassembler:
    def __init__(self):
        self.nyxstone = Nyxstone("x86_64")

    def disas(self, data, base_address = 0):
        return self.nyxstone.disassemble_to_instructions(data, base_address, count=10)
    
    