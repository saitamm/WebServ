<<<<<<< HEAD
print ("hello, World!")
gfhgfhgf;
# import socket
# import time
# import threading
    s = socket.socket()
#     s.connect(('localhost', 8080))
#     s.send(b"DELETE /Body/92823.txt HTTP/1.1\r\nHost: localhost:8080\r\n")
#     s.send(b"\r\n\r\n")  # Finish header
#     print("Sent split header")
#     print(s.recv(1024))
#     s.close()
=======
#!/usr/bin/env python3
import sys

# Read the entire POST body from stdin
body = sys.stdin.read()
>>>>>>> origin/keltoum

# Output HTTP headers first
print("Content-Type: text/plain\n")
print("===HELLO WORLD ===")
print(body)
