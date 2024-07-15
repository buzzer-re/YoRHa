from .commands import *
from .disassembler import Disassembler
import binascii

pause_debugger_response = construct.Struct(
    "header"     / dbg_response_header,
    "trap_frame" / trap_frame_t,
    "code"       / construct.Array(0x100, construct.Byte) # Breakpoint context
)

class PauseDebugger(Command):
    def __init__(self):
        self.disas = Disassembler()
        Command.__init__(self, DebuggerCommandsCode.PAUSE_DEBUGGER)
        self.cmd_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.PAUSE_DEBUGGER,
            "argument_size": 0
        })
    
    def print_response(self):
        insts = self.disas.disas(self.response.code, self.response.trap_frame.rip)
        for inst in insts:
            print(f"{hex(inst.address)}\t {' '.join([hex(x)[2:] for x in inst.bytes])}\t {inst.assembly}")
    
