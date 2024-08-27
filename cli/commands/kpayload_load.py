from .commands import *


class KPayloadLoader(Command):
    def __init__(self, payload_data):
        self.data = payload_data
        Command.__init__(self, DebuggerCommandsCode.DBG_KPAYLOAD_LOAD)
        self.command_code = DebuggerCommandsCode.DBG_KPAYLOAD_LOAD
        self.response_struct = None
        self.command = dbg_request_header.build({
            "cmd_type": self.command_code,
            "argument_size": len(self.data)
        })

        self.command += payload_data
        print(self.command)
