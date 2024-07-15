import socket
import sys
import construct

from debugger import *

PS4_IP = "192.168.88.12"
PS4_DBG_PORT = 8881

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1


def main():
    debugger = Debugger(PS4_IP, PS4_DBG_PORT)
    
    if debugger.connect():
        print(f"PS4 KDebugger connected at {PS4_IP}:{PS4_DBG_PORT}.")
        while True:
            cmd_splited = input("> ").split(" ")
            cmd = cmd_splited[0]
            arg = cmd_splited[1]
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
            elif cmd == "break":
                debugger.place_breakpoint(arg)

if __name__ == '__main__':
    main()
