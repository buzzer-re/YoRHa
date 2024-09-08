import socket
import sys
import construct

from debugger import *

PS4_IP = "192.168.88.12"
PS4_CTRL_DBG_PORT = 8887
PS4_DBG_PORT = 8888

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1

def main():
    debugger = Debugger(PS4_IP, PS4_CTRL_DBG_PORT, PS4_DBG_PORT)
    
    if debugger.online:
        print(f"PS4 KDebugger connected at {PS4_IP}:{PS4_CTRL_DBG_PORT}.")
        while True:
            # Dummy and dumb cmd handler loop
            dbg_type = "PS4 KDBG" if debugger.in_dbg_context else "PS4 Ctrl" 
            cmd_splited = input(f"{dbg_type}> ").split(" ")
            cmd = cmd_splited[0]
            arg = cmd_splited[1] if len(cmd_splited) > 1 else ""
            arg2 = int(cmd_splited[2], base=16) if len(cmd_splited) > 1 else ""

            if cmd == "pause":
                if debugger.pause_debugger():
                    debugger.print_context()
            elif cmd == "context":
                debugger.print_context()

            elif cmd == "unload":
                if debugger.disconnect(unload_dbg=True):
                    print("Unloaded Debugger!")
                    break
                    
            elif cmd == "continue":
                debugger.continue_execution();

            elif cmd == "quit":
                if debugger.disconnect():
                    print("Closed connection!")
                    break
            elif cmd == "break":
                debugger.place_breakpoint(arg)
            
            elif cmd == "memread":
                print(arg2)
                print(arg)
                # Get data over network
                debugger.memory_read(int(arg, base=16), arg2)
                pass
            elif cmd == "break_list":
                debugger.list_breakpoints()

            elif cmd == "load_payload":
                print(arg2)
                print(arg)
                debugger.load_payload(arg)

            elif cmd == "memwrite":
                pass

            elif cmd == "assemble":
                pass
    else:
        print("Unable to connect to the remote PS4!")

if __name__ == '__main__':
    main()
