import socket
import time

HOST = "127.0.0.1"   # your server IP
PORT = 8080          # your server port

# Build a very long header value
long_header_value = "A" * 20000  

# Build a very long body
long_body = "B" * 100000  # 100 KB of body

# Construct headers
headers = (
    "POST / HTTP/1.1\r\n"
    f"Host: {HOST}\r\n"
    f"X-Test-Header: {long_header_value}\r\n"
    f"Content-Length: {len(long_body)}\r\n"
    "Connection: close\r\n\r\n"
)

# Open socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    # Send headers slowly
    for line in headers.split("\r\n"):
        if line:
            s.sendall((line + "\r\n").encode())
            time.sleep(0.5)  # delay between header lines
    s.sendall(b"\r\n")  # end of headers

    # Send body slowly, chunk by chunk
    chunk_size = 1024
    for i in range(0, len(long_body), chunk_size):
        chunk = long_body[i:i+chunk_size]
        s.sendall(chunk.encode())
        time.sleep(0.2)  # delay between body chunks

    # Receive response
    response = b""
    while True:
        part = s.recv(4096)
        if not part:
            break
        response += part

print(response.decode(errors="ignore"))
