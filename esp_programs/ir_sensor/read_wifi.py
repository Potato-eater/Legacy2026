# small test program. not used.
import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(("0.0.0.0", 1234))

while True:

    data, addr = sock.recvfrom(1024)

    if len(data) == 64:

        values = struct.unpack("16f", data)

        print(values)