import socket
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

host = socket.gethostname()

port = 9981

s.connect((host, port))

s.send(b'hello')

# do not close it will cause busy loop
