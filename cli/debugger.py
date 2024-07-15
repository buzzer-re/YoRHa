from commands import pause, stop, breakpoint
import socket

class Registers:
    rax = 0
    rdx = 0


class Debugger:
    def __init__(self, host, port, quiet = False):
        self.host = host
        self.port = port
        self.socket = None
        self.quiet = quiet
        self.regs = Registers()
    

    def connect(self) -> bool:
        self.socket = socket.socket()
        self.socket.settimeout(5)
        try:
            self.socket.connect((self.host, self.port))
            return True
        except Exception as e:
            if not self.quiet:
                print("Exception connecting to the PS4  ")
                print(e)
            
        return False


    def disconnect(self, unload_dbg=False) -> bool:
        try:
            if unload_dbg:
                self.__send_cmd(stop.QuitDebugger(), False)
            self.socket.close()
            return True
        except Exception as e:
            if not self.quiet:
                print("Unable to disconnect")
                print(e)
        
        return False

    def __send_cmd(self, command, wait=True) -> str:
        try:
            self.socket.send(command.serialize())
            if wait:
                response = self.socket.recv(command.MAX_SIZE)
                command.parse_response(response)
                command.print_response()
        except Exception as e:
            print("Unable to send command!")
            print(e)


    def pause_debugger(self) -> bool:
        pause_cmd = pause.PauseDebugger()
        self.__send_cmd(pause_cmd)
        pass

    def place_breakpoint(self, addr):
        print(f"Placing breakpoint at {addr}")
        addr = int(addr, base=16)
        dbg_cmd = breakpoint.BreakpointCommand(addr)
        self.__send_cmd(dbg_cmd)
    
    def context(self):
        pass



    