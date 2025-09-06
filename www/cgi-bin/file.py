
#!/usr/bin/env python3
import os

body = sys.stdin.read()

print("Content-Type: text/plain\n")
print("About to crash...")

# Force segfault by killing self with SIGSEGV
os.kill(os.getpid(), 11)  
