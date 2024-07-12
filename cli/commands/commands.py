import construct

dbg_request_header = construct.Struct(
    "cmd_type" / construct.Int8ul,
    "argument_size" / construct.Int64ul
)

dbg_response_header = construct.Struct(
    "cmd_type" / construct.Int8ul,
    "command_status" / construct.Int8ul,
    "response_size" / construct.Int8ul,
)


trap_frame_t = construct.Struct(
    "rax"           / construct.Int64ul,
    "rcx"           / construct.Int64ul,
    "rdx"           / construct.Int64ul,
    "rbp"           / construct.Int64ul,
    "rsi"           / construct.Int64ul,
    "rdi"           / construct.Int64ul,
    "r8"            / construct.Int64ul,
    "r9"            / construct.Int64ul,
    "r11"           / construct.Int64ul,
    "r12"           / construct.Int64ul,
    "r13"           / construct.Int64ul,
    "r14"           / construct.Int64ul,
    "r15"           / construct.Int64ul,
    "error_code"    / construct.Int64ul,
    "rip"           / construct.Int64ul,
    "cs"            / construct.Int64ul,
    "eflags"        / construct.Int64ul,
    "rsp"           / construct.Int64ul,
    "ss"            / construct.Int64ul
)

pause_debugger_response = construct.Struct(
    "header"     / dbg_response_header,
    "trap_frame" / trap_frame_t
)

class DebuggerCommandsCode:
    PAUSE_DEBUGGER = 0

class Command:
    MAX_SIZE = 0x1000
    def __init__(self, command_code):
        self.command_code = command_code
    
    def print_response(self):
        print(self.response)
        pass

    def serialize(self) -> bytearray:
        return self.command
    
    def parse_response(self, data):
        self.response = self.cmd_struct.parse(data)



    
    