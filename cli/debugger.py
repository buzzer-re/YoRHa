from commands import pause
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


    def disconnect(self) -> bool:
        try:
            self.socket.close()
            return True
        except Exception as e:
            if not self.quiet:
                print("Unable to disconnect")
                print(e)
        
        return False

    def __send_cmd(self, command) -> str:
        try:
            self.socket.send(command.serialize())
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

    
    def context(self):
        pass



    