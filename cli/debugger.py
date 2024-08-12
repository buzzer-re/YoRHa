from commands import pause, stop, breakpoint, continue_exec, context
import socket

class Registers:
    rax = 0
    rdx = 0


class Debugger:
    def __init__(self, host, port, dbg_port, quiet = False):
        self.host = host
        self.ctrl_port = port
        self.socket = None
        self.dbg_trap_socket = None
        self.quiet = quiet
        self.dbg_trap_port = dbg_port
        self.dbg_controller_socket = self.connect(self.ctrl_port)
        self.online = self.dbg_controller_socket != False
        self.in_dbg_context = False
        self.regs = Registers()


    def connect(self, port) -> int:
        sock = socket.socket()
        sock.settimeout(5)
        try:
            sock.connect((self.host, port))
        except Exception as e:
            if not self.quiet:
                print("Exception connecting to the PS4  ")
                print(e)

            return False
        
        return sock


    def disconnect(self, unload_dbg=False) -> bool:
        try:
            if unload_dbg:
                self.__send_cmd(stop.QuitDebugger(), False, False)
            
            if self.in_dbg_context:
                self.dbg_trap_socket.close()
            
            self.dbg_controller_socket.close()
            
            return True
        except Exception as e:
            if not self.quiet:
                print("Unable to disconnect")
                print(e)
        
        return False



    def __send_cmd(self, command, wait=True, trap_fame=False) -> bool:
        sock = self.dbg_controller_socket

        # Create the trap frame connection
        if trap_fame:
            if not self.in_dbg_context:
                self.dbg_trap_socket = self.connect(self.dbg_trap_port)
                if not self.dbg_trap_socket:
                    print(f"Unable to connect to the PS4, is the debugger running ?")
                    return None
                self.in_dbg_context = True

            sock = self.dbg_trap_socket
        
        try:
            sock.send(command.serialize())

            if wait:    
                response = sock.recv(command.MAX_SIZE)
                command.parse_response(response)
                command.print_response()

        except Exception as e:
            print("Unable to send command!")
            print(e)


    def continue_execution(self):
        continue_cmd = continue_exec.Continue()
        self.__send_cmd(continue_cmd, False, True)
        self.in_dbg_context = False


    def pause_debugger(self) -> bool:
        pause_cmd = pause.PauseDebugger()
        self.__send_cmd(pause_cmd, False, False)


    def print_context(self) -> bool:
        ctx_cmd = context.DebuggerContext()
        self.__send_cmd(ctx_cmd, True, True)

    def place_breakpoint(self, addr):
        print(f"Placing breakpoint at {addr}")
        addr = int(addr, base=16)
        dbg_cmd = breakpoint.BreakpointCommand(addr)
        self.__send_cmd(dbg_cmd)
    
    def context(self):
        pass



    