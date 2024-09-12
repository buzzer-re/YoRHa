from commands import mem_io
from commands import pause, stop, breakpoint, continue_exec, context, kpayload_load, disas
from commands.disassembler import Disassembler
import socket
import argparse
import os

class Registers:
    rax = 0
    rdx = 0


AVAILABLE_COMMANDS = {
    "disas"     : disas.Disassemble,
    "memread"   : mem_io.MemRead,
    "pause"     : pause.PauseDebugger,
    "continue"  : continue_exec.Continue,
    "context"   : context.DebuggerContext
}

def hex2int_from_list(l):
    for i in range(len(l)):
        try:
            if "0x" in l[i]:
                l[i] = str(int(l[i], base=16))
        except: continue

#
# Build argparse objects dynamically and parse the user provided arguments
#
def parse_args(args, expected_arguments) -> argparse.Namespace:
    hex2int_from_list(args)
    parser = argparse.ArgumentParser(add_help=False)
    for argument in expected_arguments:
        argument_action = 'store_true' if argument.type == bool else 'store'
        if not argument.modifiers:
            parser.add_argument(argument.arg_name, action=argument_action)
        else:
            parser.add_argument(*argument.modifiers, action=argument_action)
    parsed = None
    try:
        parsed = parser.parse_args(args)
    except:
        return None
    
    #
    # Convert the types here, because "action" and "type" can't coexist on the argparse
    # Since we want to build a generic argparser builder, we must set the types later
    #
    for argument in expected_arguments:
        attr = getattr(parsed, argument.arg_name)
        if not attr: continue

        setattr(parsed, argument.arg_name, argument.type(attr))

    return parsed


class Debugger:
    def __init__(self, host, port, dbg_port, quiet = False):
        self.host = host
        self.ctrl_port = port
        self.socket = None
        self.dbg_trap_socket = None
        self.quiet = quiet
        self.dbg_trap_port = dbg_port
        self.dbg_controller_socket = self.connect(self.ctrl_port)
        
        if not self.dbg_controller_socket:
            self.dbg_trap_socket = self.connect(self.dbg_trap_port)

        self.online = self.dbg_controller_socket != None or self.dbg_trap_socket != None
        self.in_dbg_context = self.dbg_trap_socket != None
        self.regs = Registers()
        self.disas = Disassembler()
        self.dispatcher = {
            "disas"     : self.__disassemble,
            "memread"   : self.__memory_read,
            "pause"     : self.__pause_debugger,
            "continue"  : self.__continue,
            "context"   : self.print_context
        }

    def connect(self, port) -> int:
        sock = socket.socket()
        sock.settimeout(5)
        try:
            sock.connect((self.host, port))
        except Exception as e:
            if not self.quiet:
                print("Exception connecting to the PS4  ")
                print(e)

            return None
        
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


    def __send_cmd(self, command, wait=True, trap_fame=False, retry=True) -> bool:
        sock = self.dbg_controller_socket
        # Create the trap frame connection
        success = False
        if trap_fame:
            if self.dbg_trap_socket == None or retry:
                self.dbg_trap_socket = self.connect(self.dbg_trap_port)
                if not self.dbg_trap_socket:
                    print(f"PS4 not trapped into the debugger!")
                    return None
                self.in_dbg_context = True

            sock = self.dbg_trap_socket
        
        try:
            sock.send(command.serialize())

            if wait:
                response = sock.recv(command.max_size)
                success = True
                command.parse_response(response)
                command.print_response()
            return True
        except Exception as e:
            if retry and not success:
                if self.__send_cmd(command, wait, trap_fame=trap_fame, retry=False):
                    return True
                
            print("Unable to send command!")
            print(e)

        return False


    def launch_cmd(self, cmd, args):
        if cmd in self.dispatcher:
            self.dispatcher[cmd](args)
        
    
    def __disassemble(self, args: list) -> bool:
        args = parse_args(args, disas.Disassemble.ARGUMENTS)        
        disas_cmd = disas.Disassemble(args.address, args.count)
        self.__send_cmd(disas_cmd, wait=True, trap_fame=self.in_dbg_context)
        

    def __continue(self, args):
        continue_cmd = continue_exec.Continue()
        if self.__send_cmd(continue_cmd, False, True):
            self.in_dbg_context = False


    def __memory_read(self, args) -> False:
        args = parse_args(args, mem_io.MemRead.ARGUMENTS)
        if not args:
            return False
        
        if not args.count:
            args.count = 16

        memory_read_req = mem_io.MemRead(args.address, args.count, args.output)
        self.__send_cmd(memory_read_req, True, trap_fame=self.in_dbg_context)


    def __pause_debugger(self, args) -> bool:
        pause_cmd = pause.PauseDebugger()
        if self.__send_cmd(pause_cmd, False, False):
            self.in_dbg_context = True
            self.print_context()


    def print_context(self, args = []) -> bool:
        print("Registers: ")
        ctx_cmd = context.DebuggerContext()
        self.__send_cmd(ctx_cmd, True, True)
        
        if not ctx_cmd:
            print("System is not in a paused state!")
            return
        
        break_list = self.list_breakpoints()        
        rip = ctx_cmd.response.trap_frame.rip
        mem = mem_io.MemRead(rip - 1, 0x10, only_read = True)
        self.__send_cmd(mem, True, trap_fame=self.in_dbg_context)
        print("\n\n")
        try:
            # Filter breakpoints
            if break_list and break_list.num_breakpoints > 0:
                for i, code in enumerate(mem.data_read):
                    addr = rip - 1 + i # Minus 1 because RIP points to the next instruction
                    if addr in break_list.breakpoints_lookup:
                        mem.data_read[i] = break_list.breakpoints_lookup[addr].old_opcode

            insts = self.disas.disas(mem.data_read, ctx_cmd.response.trap_frame.rip - 1)
            print("Disassembly: ")
            for inst in insts:
                print(f"{hex(inst.address)}:\t{inst.mnemonic}\t{inst.op_str}", end=" ")
                if ctx_cmd.response.trap_frame.rip == inst.address:
                    print("; <=== RIP", end="")

                if break_list and inst.address in break_list.breakpoints_lookup:
                    print("; Software Breakpoint", end="")

                print()

        except Exception as e:
            print(bytearray(ctx_cmd.response.code))
            print(e)

    def place_breakpoint(self, addr):
        addr = int(addr, base=16)
        dbg_cmd = breakpoint.BreakpointCommand(addr)
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=self.in_dbg_context)
    

    def write_memory(self, addr, data):
        dbg_cmd = mem_io.MemWrite(addr, data)
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=self.in_dbg_context)

    def set_thread_ctx(self):
        dbg_cmd = context.SetThreadContext()
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=True)

    def remove_breakpoint(self, addr):
        addr = int(addr, base=16)
        break_del_cmd = breakpoint.RemoveBreakpoint(addr)
        self.__send_cmd(break_del_cmd, wait=False, trap_fame=self.in_dbg_context)

    def disas(self, addr):
        memory_read_req = mem_io.MemRead(addr, 100)
        self.__send_cmd(memory_read_req, wait=True, trap_fame=True)
    

    def load_payload(self, file_path):
        if not os.path.exists(file_path):
            print(f"{file_path} does not exist!")
            return False

        with open(file_path, "rb") as fd:
            payload_data = fd.read()
            kpayload_command_req = kpayload_load.KPayloadLoader(payload_data)
            self.__send_cmd(kpayload_command_req, wait=False, trap_fame=False)


    def list_breakpoints(self):
        list_bp = breakpoint.ListBreakpoints()
        if self.__send_cmd(list_bp, wait=True, trap_fame=self.in_dbg_context):
            return list_bp
        else:
            print("Error listing breakpoints")
        return None
    
    def context(self):
        pass



    