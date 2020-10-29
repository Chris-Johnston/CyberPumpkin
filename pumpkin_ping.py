"""
Pumpkin ping

Instructs the pumpkin to ping somewhere
"""

host = "bing.com"

# mode, frame time, num frames
message_bytes = [2, 0, 0]

message_bytes += [ord(x) for x in host]
message_bytes.append(0) # null terminate

print(message_bytes)

import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('10.1.1.190', 8001))
s.send(bytearray(message_bytes))
s.close()
