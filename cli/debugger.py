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
    "disas" : disas.Disassemble
}

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

        self.online = self.dbg_controller_socket != False or self.dbg_trap_socket != False
        self.in_dbg_context = self.dbg_trap_socket != None
        self.regs = Registers()
        self.disas = Disassembler()
        self.dispatcher = {
            "disas" : self.__disassemble
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

            return False
        
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


    def __send_cmd(self, command, wait=True, trap_fame=False) -> bool:
        sock = self.dbg_controller_socket
        # Create the trap frame connection
        if trap_fame:
            if not self.in_dbg_context:
                self.dbg_trap_socket = self.connect(self.dbg_trap_port)
                if not self.dbg_trap_socket:
                    print(f"Unable to connect to the PS4, is the debugger running ?")
                    return None
                self.in_dbg_context = True

            sock = self.dbg_trap_socket
        
        try:
            sock.send(command.serialize())

            if wait:    
                response = sock.recv(command.max_size)
                command.parse_response(response)
                command.print_response()
            return True
        except Exception as e:
            print("Unable to send command!")
            print(e)

        return False


    def launch_cmd(self, cmd, args):
        if cmd in self.dispatcher:
            self.dispatcher[cmd](cmd, args)
        
    

    def __disassemble(self, cmd: str, args: list) -> bool:
        for i in range(len(args)):
            try:
                if "0x" in args[i]:
                    args[i] = str(int(args[i], base=16))
            except: continue
        
        parser = argparse.ArgumentParser(add_help=False)
        parser.add_argument("address", type=int)
        parser.add_argument("-c", "--count", type=int)
        parser._print_message = lambda x, y: None
        parsed = None
        try:
            parsed = parser.parse_args(args)
        except:
            print(f"Wrong usage of command: {cmd}")
            return False
        
        disas_cmd = disas.Disassemble(parsed.address, parsed.count)
        self.__send_cmd(disas_cmd, wait=True, trap_fame=self.in_dbg_context)
        

    def continue_execution(self):
        continue_cmd = continue_exec.Continue()
        self.__send_cmd(continue_cmd, False, True)
        self.in_dbg_context = False


    def memory_read(self, addr, size):
        memory_read_req = mem_io.MemRead(addr, size)
        self.__send_cmd(memory_read_req, True, trap_fame=self.in_dbg_context)


    def pause_debugger(self) -> bool:
        pause_cmd = pause.PauseDebugger()
        self.__send_cmd(pause_cmd, False, False)


    def print_context(self) -> bool:
        ctx_cmd = context.DebuggerContext()
        self.__send_cmd(ctx_cmd, True, True)
        break_list = self.list_breakpoints()
        if not ctx_cmd:
            print("System is not in a paused state!")
            return
        
        print(f"RAX: {hex(ctx_cmd.response.trap_frame.rax)}")
        print(f"RDX: {hex(ctx_cmd.response.trap_frame.rdx)}")
        print(f"RCX: {hex(ctx_cmd.response.trap_frame.rcx)}")
        print(f"RBX: {hex(ctx_cmd.response.trap_frame.rbx)}")
        print(f"RDI: {hex(ctx_cmd.response.trap_frame.rdi)}")
        print(f"RSI: {hex(ctx_cmd.response.trap_frame.rsi)}")
        print(f"RBP: {hex(ctx_cmd.response.trap_frame.rbp)}")
        print(f"RSP: {hex(ctx_cmd.response.trap_frame.rsp)}")
        print(f"R8: {hex(ctx_cmd.response.trap_frame.r8)}")
        print(f"R9: {hex(ctx_cmd.response.trap_frame.r9)}")
        print(f"R10: {hex(ctx_cmd.response.trap_frame.r11)}")
        print(f"R12: {hex(ctx_cmd.response.trap_frame.r12)}")
        print(f"R13: {hex(ctx_cmd.response.trap_frame.r13)}")
        print(f"R14: {hex(ctx_cmd.response.trap_frame.r14)}")
        print(f"R15: {hex(ctx_cmd.response.trap_frame.r15)}")
        print(f"RIP: {hex(ctx_cmd.response.trap_frame.rip)}")
                
        try:
            # Filter breakpoints
            if break_list and break_list.num_breakpoints > 0:
                for i, code in enumerate(ctx_cmd.response.code):
                    addr = ctx_cmd.response.trap_frame.rip + i - 1 # Minus 1 because RIP points to the next instruction
                    if addr in break_list.breakpoints_lookup:
                        ctx_cmd.response.code[i] = break_list.breakpoints_lookup[addr].old_opcode

            insts = self.disas.disas(ctx_cmd.response.code, ctx_cmd.response.trap_frame.rip - 1)

            for inst in insts:
                print(f"{hex(inst.address)}\t {' '.join([hex(x)[2:] for x in inst.bytes])}\t {inst.assembly}",  end="")

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



    