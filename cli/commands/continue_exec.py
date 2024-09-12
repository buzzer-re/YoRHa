from .commands import *

class Continue(Command):
    ARGUMENTS = []
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.DBG_CONTINUE)
        self.command_code = DebuggerCommandsCode.DBG_CONTINUE
        self.response_struct = None
        self.command = dbg_request_header.build({
            "cmd_type" : self.command_code,
            "argument_size": 0
        })