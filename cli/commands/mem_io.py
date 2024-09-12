from .commands import *
from .utils import hexdump

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
    ARGUMENTS = [
        CommandArgument("count", ["-c", "--count"], "Number of bytes to read", arg_type=int),
        # CommandArgument("hex",   ["--hex"], "Display as a HexDump", arg_type=bool),
        CommandArgument("output",  ["-o", "--output"], "Save data into a file", arg_type=str),
        CommandArgument("address", [], "Address to read", arg_type=int)
    ]

    def __init__(self, addr, size = 16, output_file = None, only_read = False):
        Command.__init__(self, DebuggerCommandsCode.DBG_MEM_READ)
        self.command_code = DebuggerCommandsCode.DBG_MEM_READ
        self.response_struct = dbg_response_header
        request  = dbg_request_header.build({
                "cmd_type"  : DebuggerCommandsCode.DBG_MEM_READ,
                "argument_size" : memory_read_request_t.sizeof() - dbg_request_header.sizeof()
        })
        
        self.command = memory_read_request_t.build({
            "header" : dbg_request_header.parse(request),
            "target_address": addr,
            "read_size" : size
        })

        self.print_hex = output_file == None
        self.output_file = output_file
        self.max_size = self.response_struct.sizeof() + size
        self.only_read = only_read
    
    
    def parse_response(self, data):
        super().parse_response(data)
        self.data_read = self.raw_data[self.response_struct.sizeof():]
    #
    # Print response overload from the class Command
    #
    def print_response(self):
        self.data_read = self.raw_data[self.response_struct.sizeof():]
        if self.raw_data and not self.only_read:
            if self.print_hex:
                print(hexdump.hexdump(self.data_read))
            else:
                with open(self.output_file, "wb") as out_fd:
                    out_fd.write(self.data_read)
                    print(f"[+] Saved {len(self.data_read)} bytes at {self.output_file}! [+]")



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