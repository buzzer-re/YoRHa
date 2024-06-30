import socket

s = socket.socket()
s.connect(("192.168.88.12", 8888))
s.send("Hello from python my dude!".encode());
s.close()
