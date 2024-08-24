from .commands import *


memory_read_request_t = construct.Struct(
    "header"     / dbg_request_header,
    "target_address" / construct.Int64ul,
    "read_size"     / construct.Int64ul
)
class MemRead(Command):
    def __init__(self, addr, size):
        Command.__init__(self, DebuggerCommandsCode.DBG_MEM_READ)
        self.command_code = DebuggerCommandsCode.DBG_MEM_READ
        self.response_struct = dbg_response_header
        request  = dbg_request_header.build({
                "cmd_type"  : DebuggerCommandsCode.DBG_MEM_READ,
                "argument_size" : 16
        })
        
        self.command = memory_read_request_t.build({
            "header" : dbg_request_header.parse(request),
            "target_address": addr,
            "read_size" : size
        })
    
    def print_response(self):
        if self.raw_data:
            print(self.raw_data)
            print(f"Read: {self.response.response_size} bytes")
            data_read = self.raw_data[self.response_struct.sizeof():]
            print(len(data_read))
            print(data_read)