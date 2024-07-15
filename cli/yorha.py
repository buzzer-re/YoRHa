import socket
import sys
import construct

from debugger import *

PS4_IP = "192.168.88.12"
PS4_DBG_PORT = 8889

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1


# def start_dbg_loop(sock):
#     pause_pkt = dbg_request_header.build(dict(cmd_type=CommandsCode.PAUSE_DBG, argument_size=0))
#     sock.send(pause_pkt)
#     res = pause_debugger_response.parse(sock.recv(0x1000))
#     sock.close()
#     print(res)



def main():
    debugger = Debugger(PS4_IP, PS4_DBG_PORT)
    
    if debugger.connect():
        print(f"PS4 KDebugger connected at {PS4_IP}:{PS4_DBG_PORT}.")
        while True:
            cmd = input("> ")
            if cmd == "pause":
                if debugger.pause_debugger():
                    debugger.print_context()

            elif cmd == "unload":
                if debugger.disconnect(unload_dbg=True):
                    print("Unloaded Debugger!")
                    break
            elif cmd == "quit":
                if debugger.disconnect():
                    print("Closed connection!")
                    break

if __name__ == '__main__':
    main()
