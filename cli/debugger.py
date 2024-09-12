from commands import mem_io
from commands import pause, stop, breakpoint, continue_exec, context, kpayload_load, disas
from commands.disassembler import Disassembler
import socket
import argparse
import os
import ast

class Registers:
    rax = 0
    rdx = 0


AVAILABLE_COMMANDS = {
    "disas"     : disas.Disassemble,
    "memread"   : mem_io.MemRead,
    "memwrite"  : mem_io.MemWrite,
    "pause"     : pause.PauseDebugger,
    "continue"  : continue_exec.Continue,
    "context"   : context.DebuggerContext,
    "break"     : breakpoint.BreakpointCommand,
    "breakdel"  : breakpoint.RemoveBreakpoint,
    "unload"    : None,
    "setr"      : context.SetThreadContext
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
    parser._print_message = lambda x,y: None
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

        if argument.type == bytearray and type(attr) == str:
            if "\\x" in attr:
                attr = ast.literal_eval(attr)
            
            attr = attr.encode()

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
            "memwrite"  : self.__memory_write,
            "pause"     : self.__pause_debugger,
            "continue"  : self.__continue,
            "context"   : self.print_context,
            "break"     : self.__breakpoint,
            "breakdel"  : self.__remove_breakpoint,
            "unload"    : self.disconnect,
            "setr"      : self.__set_thread_context
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
            if cmd != "unload":            
                self.dispatcher[cmd](args)
            else:
                self.disconnect(unload_dbg=True)
                
        
    
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
        ctx_cmd = context.DebuggerContext()
        print("Registers: ")

        if not self.__send_cmd(ctx_cmd, True, True):
            print("System is not in a paused state!")
            return
        
        break_list = self.list_breakpoints()        
        rip = ctx_cmd.response.trap_frame.rip
        mem = mem_io.MemRead(rip - 1, 0x10, only_read = True)
        self.__send_cmd(mem, True, trap_fame=self.in_dbg_context)
        print("\n\n")
        print(break_list.breakpoints_lookup)
        try:
            # Filter breakpoints
            if break_list and break_list.num_breakpoints > 0:
                raw_code_bytes = list(mem.data_read)

                for i, code in enumerate(mem.data_read):
                    addr = rip - 1 + i # Minus 1 because RIP points to the next instruction
                    if addr in break_list.breakpoints_lookup:
                        raw_code_bytes[i] = break_list.breakpoints_lookup[addr].old_opcode

                mem.data_read = bytearray(raw_code_bytes)

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

    def __breakpoint(self, args):
        args = parse_args(args, breakpoint.BreakpointCommand.ARGUMENTS)
        
        if not args:
            return False
        
        dbg_cmd = breakpoint.BreakpointCommand(args.address)
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=self.in_dbg_context)
    
    def __remove_breakpoint(self, args):
        args = parse_args(args, breakpoint.RemoveBreakpoint.ARGUMENTS)
        
        if not args:
            return False
        
        break_del_cmd = breakpoint.RemoveBreakpoint(args.address)
        self.__send_cmd(break_del_cmd, wait=False, trap_fame=self.in_dbg_context)

    def __memory_write(self, args):
        args = parse_args(args, mem_io.MemWrite.ARGUMENTS)
        if not args:
            return False
        
        if not args.input and not args.bytes:
            print("You must specify a file with --input/-i or written something with the --bytes/-b argument! ")
            return False
        
        dbg_cmd = mem_io.MemWrite(args.address, args.bytes, args.input)
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=self.in_dbg_context)


    def __set_thread_context(self, args):
        args = parse_args(args, context.SetThreadContext.ARGUMENTS)
        if not args:
            return False
        
        ctx_cmd = context.DebuggerContext(quiet=True)

        if not self.__send_cmd(ctx_cmd, True, True):
            print("System is not in a paused state!")
            return

        # Copy new register values to the trap_frame struct
        for reg in ctx_cmd.response.trap_frame:
            try:
                new_reg_value = getattr(args, reg)
                if new_reg_value !=  None:
                    ctx_cmd.response.trap_frame[reg] = new_reg_value

            except: continue

        dbg_cmd = context.SetThreadContext(ctx_cmd.response.trap_frame)
        self.__send_cmd(dbg_cmd, wait=False, trap_fame=True)
    

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



    