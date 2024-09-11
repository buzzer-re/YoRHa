from .commands import *


memory_read_request_t = construct.Struct(
    "header"     / dbg_request_header,
    "target_address" / construct.Int64ul,
    "read_size"     / construct.Int64ul
)

memory_write_request_t = construct.Struct(
    "header"    / dbg_request_header,
    "target_address" / construct.Int64ul,
    "write_size"    / construct.Int64ul
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

        self.max_size = self.response_struct.sizeof() + size
    
    def print_response(self):
        if self.raw_data:
            print(self.raw_data)
            print(f"Read: {self.response.response_size} bytes")
            data_read = self.raw_data[self.response_struct.sizeof():]
            open("dump.bin", "wb").write(data_read)


class MemWrite(Command):
    def __init__(self, addr, data):
        Command.__init__(self, DebuggerCommandsCode.DBG_MEM_WRITE)
        self.command_code = DebuggerCommandsCode.DBG_MEM_WRITE
        self.response_struct = dbg_response_header
        request = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.DBG_MEM_WRITE,
            "argument_size": 16
        })

        self.command = memory_write_request_t.build({
            "header" : dbg_request_header.parse(request),
            "target_address": addr,
            "write_size": len(data)
        })

        self.command = self.command + data