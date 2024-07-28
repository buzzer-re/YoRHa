import socket
import sys
import construct

from debugger import *

PS4_IP = "192.168.88.12"
PS4_CTRL_DBG_PORT = 8882
PS4_DBG_PORT = 8880

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1

def main():
    debugger = Debugger(PS4_IP, PS4_CTRL_DBG_PORT, PS4_DBG_PORT)
    
    if debugger.connect():
        print(f"PS4 KDebugger connected at {PS4_IP}:{PS4_CTRL_DBG_PORT}.")
        while True:
            # Dummy and dumb cmd handler loop
            cmd_splited = input("> ").split(" ")
            cmd = cmd_splited[0]
            arg = cmd_splited[1] if len(cmd_splited) > 1 else ""
            if cmd in ("pause", "context"):
                if debugger.pause_debugger():
                    debugger.print_context()

            elif cmd == "unload":
                if debugger.disconnect(unload_dbg=True):
                    print("Unloaded Debugger!")
                    break
            elif cmd == "continue":
                # implement in both sides
                pass
            elif cmd == "quit":
                if debugger.disconnect():
                    print("Closed connection!")
                    break
            elif cmd == "break":
                debugger.place_breakpoint(arg)

if __name__ == '__main__':
    main()
