import socket
import sys
import construct

PS4_IP = "192.168.88.12"
PS4_DBG_PORT = 8887

class CommandsCode:
    PAUSE_DBG = 0
    STOP_DBG = 1


dbg_request_header = construct.Struct(
    "cmd_type" / construct.Int8ul,
    "argument_size" / construct.Int64ul
)


def start_dbg_loop(sock):
    pause_pkt = dbg_request_header.build(dict(cmd_type=CommandsCode.PAUSE_DBG, argument_size=0))
    print("Sending dbg pause cmd -> ", end=" ")
    print(pause_pkt)
    sock.send(pause_pkt)
    res = sock.recv(0x1000)
    print(res)
    sock.close()


def main():
    s = socket.socket()
  #  s.settimeout(5)
    try:
        s.connect((PS4_IP, PS4_DBG_PORT))
    except:
        print(f"Unable to connect into {PS4_IP}:{PS4_DBG_PORT}")
        sys.exit(1)

    start_dbg_loop(s)


if __name__ == '__main__':
    main()