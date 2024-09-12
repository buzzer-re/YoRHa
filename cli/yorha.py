from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter
from prompt_toolkit import print_formatted_text as print
from prompt_toolkit.history import FileHistory
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.completion import Completer, Completion

from configparser import ConfigParser
import os
import re
from time import sleep
import sys


from debugger import *


PS4_IP = "192.168.88.12"
PS4_CTRL_DBG_PORT = 8887
PS4_DBG_PORT = 8888
homepath = os.path.expanduser("~")
HISTFILE = f"{homepath}/.yorhadbg_history"
CONFIG = f"{homepath}/.yorhadbg.ini"
IP_REGEX = r"^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$"
VERSION = "0.1"
DEV_BUILD = True

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1

# def main2():
#     debugger = Debugger(PS4_IP, PS4_CTRL_DBG_PORT, PS4_DBG_PORT)
    
#     if debugger.online:
#         print(f"PS4 KDebugger connected at {PS4_IP}:{PS4_CTRL_DBG_PORT}.")
#         while True:
#             # Dummy and dumb cmd handler loop
#             dbg_type = "PS4 KDBG" if debugger.in_dbg_context else "PS4 Ctrl" 
#             cmd_splited = input(f"{dbg_type}> ").split(" ")
#             cmd = cmd_splited[0]
#             arg = cmd_splited[1] if len(cmd_splited) > 1 else ""
#             arg2 = cmd_splited[2] if len(cmd_splited) > 1 else ""

#             if cmd == "pause":
#                 if debugger.pause_debugger():
#                     debugger.print_context()
#             elif cmd == "context":
#                 debugger.print_context()

#             elif cmd == "unload":
#                 if debugger.disconnect(unload_dbg=True):
#                     print("Unloaded Debugger!")
#                     break
                    
#             elif cmd == "continue":
#                 debugger.continue_execution();

#             elif cmd == "quit":
#                 if debugger.disconnect():
#                     print("Closed connection!")
#                     break
#             elif cmd == "break":
#                 debugger.place_breakpoint(arg)
            
#             elif cmd == "memread":
#                 print(arg2)
#                 print(arg)
#                 # Get data over network
#                 debugger.memory_read(int(arg, base=16), int(arg2, base=16))
#                 pass
#             elif cmd == "break_list":
#                 debugger.list_breakpoints()

#             elif cmd == "remove_breakpoint":
#                 debugger.remove_breakpoint(arg)

#             elif cmd == "load_payload":
#                 print(arg2)
#                 print(arg)
#                 debugger.load_payload(arg)

#             elif cmd == "memwrite":
#                 addr = int(arg, base=16)
#                 data = open(arg2, "rb").read()
#                 debugger.write_memory(addr, data)
#                 pass
            
#             elif cmd == "set_thread_context_test":
#                 debugger.set_thread_ctx()
                
#             elif cmd == "assemble":
#                 pass
#     else:
#         print("Unable to connect to the remote PS4!")


class DbgCommandCompleter(Completer):
    def get_completions(self, document, complete_event):
        command_tokens = document.text.split(" ")
        if len(command_tokens) > 1:
            command = command_tokens[0]
            if command in AVAILABLE_COMMANDS:
                for argument in AVAILABLE_COMMANDS[command].ARGUMENTS:
                    for token in argument.modifiers:
                        yield Completion(token, start_position=0)
        else:    
            for command in AVAILABLE_COMMANDS.keys():
                yield Completion(command, start_position=0)

#
# Create a new config
#
def create_config() -> bool:

    if os.path.exists(CONFIG):
        return True
    
    print("Configuration file does not exist, creating...")

    cfg = PromptSession()
    while True:
        ip = cfg.prompt("\nPlayStation 4 IP Address: ")
        if re.match(IP_REGEX, ip): break
    
        print(f"Invalid IP address: {ip}")
    
    new_cfg = ConfigParser()
    new_cfg["CONNECTION"] = {"HOST" : ip}
    
    with open(CONFIG, "w") as cfg_fd:
        new_cfg.write(cfg_fd)

    print(f"Configuration saved at {CONFIG} !")



def parse_config() -> ConfigParser:
    if create_config():
        cfg = ConfigParser()
        
        cfg.read(CONFIG)
        return cfg

    return None



def print_banner():
    if DEV_BUILD: return
    
    msg = "- Glory... to mankind! -"
    print(f"Welcome to YoRHa DBG CLI Version {VERSION}!\n\t", end="")
    
    for x in msg:
        print(x, end='')
        sys.stdout.flush()
        sleep(0.05)
    
    sleep(1)
    print("\n\n")


def main():
    print_banner()
    if not os.path.exists(HISTFILE):
        open(HISTFILE, "w").close()
    

    dbg_cfg = parse_config()
    if not dbg_cfg:
        print("Unable to load configuration!")
        exit(1)

    try:
        ip = dbg_cfg["CONNECTION"]["HOST"] # this is not a python dict btw
    except:
        print("Invalid configuration!")
        exit(1)

    print(f"PlayStation 4 Host: {ip}")

    session = PromptSession(completer=DbgCommandCompleter(), history=FileHistory(HISTFILE), auto_suggest=AutoSuggestFromHistory())
    print(f"[+] Starting connection with {ip}...", end=" ")

    debugger = Debugger(ip, PS4_CTRL_DBG_PORT, PS4_DBG_PORT, quiet=True)

    if debugger.online:
        print("Connected! [+]")
    else:
        print("Unable to connect, make sure that the PS4 is online and running the YorhaDBG kpayload [-]")
        exit(1)
    
    while debugger.online:
        try:
            cmd = session.prompt('> ')
            splited = cmd.split(" ")
            cmd_name = splited[0]

            if cmd_name not in AVAILABLE_COMMANDS:
                print(f"Invalid command: {cmd_name}")
                continue
            
            # remove white spaces
            args = list(filter(lambda x: x != '',splited[1:]))
            debugger.launch_cmd(cmd_name, args)
            

        except KeyboardInterrupt:
            continue
        except EOFError:
            break
        else:
            pass
    pass

if __name__ == '__main__':
    main()
