from .commands import *
from .pause import *
from .disassembler import Disassembler


class DebuggerContext(Command):
    def __init__(self):
        self.disas = Disassembler()

        Command.__init__(self, DebuggerCommandsCode.DBG_CONTEXT)
        self.command_code = DebuggerCommandsCode.DBG_CONTEXT
        self.response_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : self.command_code,
            "argument_size": 0
        })

    def print_response(self):
        pass