#!/usr/bin/env python3

import os
import cgitb
from urllib.parse import parse_qs

cgitb.enable()  # Enables detailed error reporting in browser

# Output HTTP header
print("Content-Type: text/plain\n")

# Get QUERY_STRING
query_string = os.environ.get("QUERY_STRING", "")
params = parse_qs(query_string)  # <-- use urllib.parse.parse_qs

# Get 'name' parameter
name = params.get("name", [""])[0]

print("Hello,", name)
