from .commands import *


kpayload_loader_request_t = construct.Struct(
    "stop_at_entry" / construct.Int8ul,
    "kpayload_size" / construct.Int64ul
)

class KPayloadLoader(Command):
    def __init__(self, payload_data):
        self.data = payload_data
        Command.__init__(self, DebuggerCommandsCode.DBG_KPAYLOAD_LOAD)
        self.command_code = DebuggerCommandsCode.DBG_KPAYLOAD_LOAD
        self.response_struct = None
        self.dbg_request = dbg_request_header.build({
            "cmd_type": self.command_code,
            "argument_size": kpayload_loader_request_t.sizeof()
        })

        self.command = self.dbg_request + kpayload_loader_request_t.build({
            "stop_at_entry": 1,
            "kpayload_size" : len(payload_data)
        })
        
        print( kpayload_loader_request_t.sizeof())

        print(len(payload_data))
        self.command += payload_data