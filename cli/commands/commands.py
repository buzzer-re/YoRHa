import construct

## Shared debugger packets between commmands ##
dbg_request_header = construct.Struct(
    "cmd_type" / construct.Int32ul,
    "argument_size" / construct.Int64ul
)

dbg_response_header = construct.Struct(
    "cmd_type" / construct.Int32sl,
    "command_status" / construct.Int32sl,
    "response_size" / construct.Int64ul,
)



trap_frame_t = construct.Struct(
    "rax"           / construct.Int64ul,
    "rcx"           / construct.Int64ul,
    "rdx"           / construct.Int64ul,
    "rbx"           / construct.Int64ul,
    "rbp"           / construct.Int64ul,
    "rsi"           / construct.Int64ul,
    "rdi"           / construct.Int64ul,
    "r8"            / construct.Int64ul,
    "r9"            / construct.Int64ul,
    "r10"           / construct.Int64ul,
    "r11"           / construct.Int64ul,
    "r12"           / construct.Int64ul,
    "r13"           / construct.Int64ul,
    "r14"           / construct.Int64ul,
    "r15"           / construct.Int64ul,
    # "error_code"    / construct.Int64ul,
    "rip"           / construct.Int64ul,
    "cs"            / construct.Int64ul,
    "eflags"        / construct.Int64ul,
    "rsp"           / construct.Int64ul,
    "ss"            / construct.Int64ul
)




class CommandArgument:
    def __init__(self, arg_name, modifiers, description, arg_type):
        self.arg_name = arg_name
        self.modifiers = modifiers
        self.description = description
        self.type = arg_type

class DebuggerCommandsCode:
    PAUSE_DEBUGGER      = 0
    STOP_DEBUGGER       = 1
    PLACE_BREAKPOINT    = 2
    DBG_CONTINUE        = 3
    DBG_CONTEXT         = 4
    DBG_MEM_READ        = 5
    DBG_KPAYLOAD_LOAD   = 6
    BREAKPOINT_LIST     = 7
    BREAKPOINT_REMOVE   = 8
    DBG_MEM_WRITE       = 9
    DBG_SET_THREAD_CONTEXT = 10



class Command:
    def __init__(self, command_code):
        self.command_code = command_code
        self.response = None
        self.raw_data = None
        self.max_size = 0x4000
    
    def print_response(self):
        pass

    def serialize(self) -> bytearray:
        return self.command
    
    def parse_response(self, data):
        self.response = self.response_struct.parse(data)
        self.raw_data = data



    
    