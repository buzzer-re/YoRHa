from .commands import *
from .pause import *
from .disassembler import Disassembler

set_thread_context_t = construct.Struct(
    "header" / dbg_request_header,
    "ctx"   / trap_frame_t
)

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
        pass


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
