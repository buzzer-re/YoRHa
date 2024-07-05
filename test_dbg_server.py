import socket

s = socket.socket()
s.connect(("192.168.88.12", 8888))
s.send("\x01\x00\x00\x00".encode());
s.close()
