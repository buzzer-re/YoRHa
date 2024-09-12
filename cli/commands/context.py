from .commands import *
from .pause import *
from .disassembler import Disassembler

set_thread_context_t = construct.Struct(
    "header" / dbg_request_header,
    "ctx"   / trap_frame_t
)

class DebuggerContext(Command):
    ARGUMENTS = []
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
        print(f"RAX: 0x{self.response.trap_frame.rax:016x}", end="\t\t")
        print(f"RDX: 0x{self.response.trap_frame.rdx:016x}")
        
        print(f"RCX: 0x{self.response.trap_frame.rcx:016x}", end="\t\t")
        print(f"RBX: 0x{self.response.trap_frame.rbx:016x}")
       
        print(f"RDI: 0x{self.response.trap_frame.rdi:016x}", end="\t\t")
        print(f"RSI: 0x{self.response.trap_frame.rsi:016x}")
       
        print(f"RBP: 0x{self.response.trap_frame.rbp:016x}", end="\t\t")
        print(f"RIP: 0x{self.response.trap_frame.rip:016x}")

        print(f"RSP: 0x{self.response.trap_frame.rsp:016x}", end="\t\t")
        print(f"R8:  0x{self.response.trap_frame.r8:016x}")

        print(f"R9:  0x{self.response.trap_frame.r9:016x}", end="\t\t")
        print(f"R10: 0x{self.response.trap_frame.r10:016x}")

        print(f"R12: 0x{self.response.trap_frame.r12:016x}", end="\t\t")
        print(f"R13: 0x{self.response.trap_frame.r13:016x}")

        print(f"R14: 0x{self.response.trap_frame.r14:016x}", end="\t\t")
        print(f"R15: 0x{self.response.trap_frame.r15:016x}")


class SetThreadContext(Command):
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT)
        self.command_code = DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT
        self.respose_struct = dbg_response_header
        request = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT,
            "argument_size": trap_frame_t.sizeof()
        })
        trap_frame = trap_frame_t.build({
            "rax" : 0x1337,
            "rcx" : 0x1337,
            "rbx" : 0x1337,
            "rdx" : 0x1337,
            "rbp" : 0x1337,
            "rsi" : 0x1337,
            "rdi" : 0x1337,
            "r8" : 0x1337,
            "r9" : 0x1337,
            "r10" : 0x1337,
            "r11" : 0x1337,
            "r12" : 0x1337,
            "r13" : 0x1337,
            "r14" : 0x1337,
            "r15" : 0x1337,
            "rip" : 0x1337,
            "cs" : 0x1337,
            "eflags" : 0x1337,
            "rsp" : 0x1337,
            "ss" : 0x1337, 
        })

        self.command = set_thread_context_t.build({
            "header" : dbg_request_header.parse(request),
            "ctx" : trap_frame_t.parse(trap_frame)
        })

        print(self.command)
