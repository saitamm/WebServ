#!/usr/bin/env python3
import os
import sys

# Print the HTTP header
print("Content-Type: text/plain\n")

# Print environment variables
print("=== ENVIRONMENT VARIABLES ===")
for key, value in os.environ.items():
    print(f"{key} = {value}")
