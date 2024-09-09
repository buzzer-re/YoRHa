from .commands import *
from .pause import *

breakpoint_request = construct.Struct(
    "header"     / dbg_request_header,
    "target_address" / construct.Int64ul
)

breakpoint_list_response = construct.Struct(
    "header"   / dbg_response_header,
    "num_breakpoints" / construct.Int64ul
)

breakpoint_entry_t = construct.Struct(
    "address" / construct.Int64ul,
    "old_opcode" / construct.Int8ul,
    "enabled" / construct.Int8ul,
    "unused" / construct.Int64ul
)

class BreakpointCommand(Command):
    def __init__(self, address):
        Command.__init__(self, DebuggerCommandsCode.PLACE_BREAKPOINT)
        self.command_code = DebuggerCommandsCode.PLACE_BREAKPOINT
        self.response_struct = pause_debugger_response
        request  = dbg_request_header.build({
                "cmd_type"  : DebuggerCommandsCode.PLACE_BREAKPOINT,
                "argument_size" : 8
        })
        self.command = breakpoint_request.build({
            "header" : dbg_request_header.parse(request),
            "target_address" : address
        })
    
    #
    # The debugging will receive the same pause structure
    #
    def print_response(self):
        pass


class ListBreakpoints(Command):
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.BREAKPOINT_LIST)
        self.command_code = DebuggerCommandsCode.BREAKPOINT_LIST;
        self.response_struct = breakpoint_list_response
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.BREAKPOINT_LIST,
            "argument_size": 0
        })
        self.num_breakpoints = 0
        self.breakpoints_lookup = {}
    
    def parse_response(self, data):
        #
        # Parse breakpoints into a dictionary
        #
        self.response = self.response_struct.parse(data)
        self.raw_data = data
        self.num_breakpoints = self.response.num_breakpoints

        if self.num_breakpoints == 0:
            return

        breakpoint_list_raw = self.raw_data[self.response_struct.sizeof():]
       
        for i in range(self.num_breakpoints):
            breakpoint_entry = breakpoint_entry_t.parse(breakpoint_list_raw)
            self.breakpoints_lookup[breakpoint_entry.address] = breakpoint_entry
            breakpoint_list_raw = breakpoint_list_raw[breakpoint_entry_t.sizeof():]

    def print_response(self):
        print(self.num_breakpoints)
        if self.num_breakpoints != 0:
            for addr, info in self.breakpoints_lookup.items():
                print(f"{hex(addr)}:")
                print(f"\tOriginal opcode:{info.old_opcode}\n\tEnabled: {info.enabled == 1}")


class RemoveBreakpoint(Command):
        def __init__(self, address):
            Command.__init__(self, DebuggerCommandsCode.BREAKPOINT_REMOVE)
            Command.__init__(self, DebuggerCommandsCode.BREAKPOINT_REMOVE)
            self.command_code = DebuggerCommandsCode.BREAKPOINT_REMOVE
            self.response_struct = pause_debugger_response
            request  = dbg_request_header.build({
                    "cmd_type"  : DebuggerCommandsCode.BREAKPOINT_REMOVE,
                    "argument_size" : 8
            })
            self.command = breakpoint_request.build({
                "header" : dbg_request_header.parse(request),
                "target_address" : address
            })
    