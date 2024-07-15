from .commands import *


class QuitDebugger(Command):
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.STOP_DEBUGGER)
        self.cmd_struct = dbg_request_header
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.STOP_DEBUGGER,
            "argument_size": 0
        })
    
    def print_response(self):
        pass
