from .commands import *
from .pause import *
from .disassembler import Disassembler

set_thread_context_t = construct.Struct(
    "header" / dbg_request_header,
    "ctx"   / trap_frame_t
)

class DebuggerContext(Command):
    ARGUMENTS = []
    def __init__(self, quiet = False):
        self.disas = Disassembler()

        Command.__init__(self, DebuggerCommandsCode.DBG_CONTEXT)
        self.command_code = DebuggerCommandsCode.DBG_CONTEXT
        self.response_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : self.command_code,
            "argument_size": 0
        })

        self.quiet = quiet

    def print_response(self):
        if not self.quiet:
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
    ARGUMENTS = [
        CommandArgument("rax", ["--rax"], "rax", arg_type=int),
        CommandArgument("rdx", ["--rdx"], "rdx", arg_type=int),
        CommandArgument("rcx", ["--rcx"], "rcx", arg_type=int),
        CommandArgument("rbx", ["--rbx"], "rbx", arg_type=int),
        CommandArgument("rdi", ["--rdi"], "rdi", arg_type=int),
        CommandArgument("rsi", ["--rsi"], "rsi", arg_type=int),
        CommandArgument("rbp", ["--rbp"], "rbp", arg_type=int),
        CommandArgument("rip", ["--rip"], "rip", arg_type=int),
        CommandArgument("rsp", ["--rsp"], "rsp", arg_type=int),
        CommandArgument("r8", ["--r8"], "r8", arg_type=int),
        CommandArgument("r9", ["--r9"], "r9", arg_type=int),
        CommandArgument("r10", ["--r10"], "r10", arg_type=int),
        CommandArgument("r11", ["--r11"], "r11", arg_type=int),
        CommandArgument("r12", ["--r12"], "r12", arg_type=int),
        CommandArgument("r13", ["--r13"], "r13", arg_type=int),
        CommandArgument("r14", ["--r14"], "r14", arg_type=int),
        CommandArgument("r15", ["--r15"], "r15", arg_type=int),
        CommandArgument("cs", ["--cs"], "cs", arg_type=int),
        CommandArgument("eflags", ["--eflags"], "eflags", arg_type=int),
        CommandArgument("ss", ["--ss"], "ss", arg_type=int),
    ]
    def __init__(self, trap_frame):
        Command.__init__(self, DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT)
        self.command_code = DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT
        self.respose_struct = dbg_response_header

        request = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.DBG_SET_THREAD_CONTEXT,
            "argument_size": trap_frame_t.sizeof()
        })

        self.command = set_thread_context_t.build({
            "header" : dbg_request_header.parse(request),
            "ctx" : trap_frame
        })
