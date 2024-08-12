from .commands import *
from .pause import *

breakpoint_request = construct.Struct(
    "header"     / dbg_request_header,
    "target_address" / construct.Int64ul
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
