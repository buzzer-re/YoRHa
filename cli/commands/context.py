from .commands import *
from .pause import *
from .disassembler import Disassembler


class DebuggerContext(Command):
    def __init__(self):
        self.disas = Disassembler()

        Command.__init__(self, DebuggerCommandsCode.DBG_CONTEXT)
        self.command_code = DebuggerCommandsCode.DBG_CONTEXT
        self.response_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : self.command_code,
            "argument_size": 0
        })

    def print_response(self):

        print(f"RAX: {hex(self.response.trap_frame.rax)}")
        print(f"RDX: {hex(self.response.trap_frame.rdx)}")
        print(f"RCX: {hex(self.response.trap_frame.rcx)}")
        print(f"RBX: {hex(self.response.trap_frame.rbx)}")
        print(f"RDI: {hex(self.response.trap_frame.rdi)}")
        print(f"RSI: {hex(self.response.trap_frame.rsi)}")
        print(f"RBP: {hex(self.response.trap_frame.rbp)}")
        print(f"RSP: {hex(self.response.trap_frame.rsp)}")
        print(f"R8: {hex(self.response.trap_frame.r8)}")
        print(f"R9: {hex(self.response.trap_frame.r9)}")
        print(f"R10: {hex(self.response.trap_frame.r11)}")
        print(f"R12: {hex(self.response.trap_frame.r12)}")
        print(f"R13: {hex(self.response.trap_frame.r13)}")
        print(f"R14: {hex(self.response.trap_frame.r14)}")
        print(f"R15: {hex(self.response.trap_frame.r15)}")

        try:
            insts = self.disas.disas(self.response.code, self.response.trap_frame.rip - 1)
            for inst in insts:
                print(f"{hex(inst.address)}\t {' '.join([hex(x)[2:] for x in inst.bytes])}\t {inst.assembly}")
        except Exception as e:
            print(e)