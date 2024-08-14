from .commands import *
import binascii

pause_debugger_response = construct.Struct(
    "header"     / dbg_response_header,
    "trap_frame" / trap_frame_t,
   # "code"       / construct.Array(0x100, construct.Byte) # Breakpoint context
)

class PauseDebugger(Command):
    def __init__(self):
        Command.__init__(self, DebuggerCommandsCode.PAUSE_DEBUGGER)
        self.response_struct = pause_debugger_response
        self.command = dbg_request_header.build({
            "cmd_type" : DebuggerCommandsCode.PAUSE_DEBUGGER,
            "argument_size": 0
        })
    
