from .commands import *

class PauseDebugger(Command):
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.PAUSE_DEBUGGER)
        self.cmd_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.PAUSE_DEBUGGER,
            "argument_size": 0
        })
    
    def print_response(self):
        print(self.response)
    
