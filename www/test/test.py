#!/usr/bin/env python3
import socket
import time

s = socket.socket()
s.connect(("127.0.0.1", 9300))

# Don't send anything, just keep the connection open
time.sleep(2)  # keep it alive just enough to avoid timeout
# Then close
s.close()
