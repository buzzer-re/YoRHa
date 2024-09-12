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


class DbgCommandCompleter(Completer):
    def get_completions(self, document, complete_event):
        
        command_tokens = document.text.split(" ")
        if len(command_tokens) > 1:
            command = command_tokens[0]
            if command in AVAILABLE_COMMANDS and AVAILABLE_COMMANDS[command] != None:
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
