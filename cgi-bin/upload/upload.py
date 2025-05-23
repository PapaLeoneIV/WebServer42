#!/usr/bin/env python3
import cgi
import os

print("Content-Type: text/plain\n")
# print("<font size=+1>Environment</font><br>")
# for param in os.environ.keys():
#     print(f"<b>{param:20}</b>: {os.environ[param]}<br>")
form = cgi.FieldStorage()


if "file" not in form:
    print("No file part in the request.")
    exit()

file_item = form["file"]

if file_item.filename:
    filename = os.path.basename(file_item.filename)
    with open(f"/home/papaleoneiv/Desktop/42WebServer/{filename}", "wb") as f:
        f.write(file_item.file.read())
    print(f"File '{filename}' uploaded successfully.")
else:
    print("No file was uploaded.")