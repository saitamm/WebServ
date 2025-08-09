import socket
import time
import threading

def send_split_header():
    s = socket.socket()
    s.connect(('localhost', 8080))
    s.send(b"DEL")
    time.sleep(10)  # Simulate slow header
    s.send(b"ETE /Body/92823.txt HTTP/1.1\r\nHost: localhost:8080\r\n")
    s.send(b"\r\n\r\n")  # Finish header
    print("Sent split header")
    print(s.recv(1024))
    s.close()

def send_header():
    s = socket.socket()
    s.connect(('localhost', 8080))
    s.send(b"GET / HTTP/1.2\r\nHost: localhost\r\n")
    s.send(b"\r\n\r\n")  # Finish header
    print("Sent split header")
    print(s.recv(1024))
    s.close()
def send_large_header():
    s = socket.socket()
    s.connect(('localhost', 8080))
    header = "GET / HTTP/1.1\r\nHost: localhost\r\n" + "X-A: A\r\n" * 1000 + "\r\n\r\n"
    s.send(header.encode())
    print("Sent large header")
    print(s.recv(1024))
    s.close()

# def no_end_header():
#     s = socket.socket()
#     s.connect(('localhost', 8080))
#     s.send(b"GET / HTTP/1.1\r\nHost: localhost\r\n")  # No \r\n\r\n
#     print("Sent incomplete header")
#     time.sleep(5)
#     s.close()

# Launch threads
threads = [
    threading.Thread(target=send_split_header),
    threading.Thread(target=send_header),
    threading.Thread(target=send_large_header)
]

for t in threads:
    t.start()

for t in threads:
    t.join()
