from .commands import *

class MemRead(Command):
    def __init__(self, size):
        Command.__init__(self, DebuggerCommandsCode.DBG_MEM_READ)
        self.command_code = DebuggerCommandsCode.DBG_MEM_READ
        self.response_struct = dbg_response_header
        self.command = dbg_request_header.build({
            "cmd_type" : self.command_code,
            "argument_size": size
        })
    
    def print_response(self):
        print(self.response)