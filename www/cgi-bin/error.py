#!/usr/bin/env python3

print("Content-Type: text/plain\n")  # Header

# Intentional error
x = 1 / 0   # Division by zero
print("This will never be printed")
