#!/usr/bin/env python3
import sys

# Read the entire POST body from stdin
body = sys.stdin.read()

# Output HTTP headers first
print("Content-Type: text/plain\n")
print("===HELLO WORLD ===")
print(body)
