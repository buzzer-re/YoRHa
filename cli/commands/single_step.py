from .commands import *


class SingleStep(Command):
    ARGUMENTS = []

    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.DBG_SNGLE_STEP)
        self.response_struct = dbg_response_header
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.DBG_SNGLE_STEP,
            "argument_size": 0
        })