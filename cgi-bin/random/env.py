#!/usr/bin/python3


import os

print("Content-type: text/html\n")
print("<font size=+1>Environment</font><br>")
for param in os.environ.keys():
    print(f"<b>{param:20}</b>: {os.environ[param]}<br>")