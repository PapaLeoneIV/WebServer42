#!/usr/bin/env python3

import cgi
import cgitb
import sys
import os

cgitb.enable()  # Show detailed error traceback in browser

# Use correct env & stdin for field parsing
form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)

print("Content-Type: text/html\n")

if "file" not in form:
    print("<h1>No file part in the request.</h1>")
else:
    file_item = form["file"]

    if file_item.filename:
        # Save the file to disk
        filename = os.path.basename(file_item.filename)
        with open(f"/home/papaleoneiv/Desktop/WebServer42/{filename}", "wb") as f:
            f.write(file_item.file.read())

        print(f"<h1>Uploaded file: {filename}</h1>")
    else:
        print("<h1>No file was uploaded.</h1>")



""" #!/usr/bin/env python3

import cgi
import os
import sys
data = sys.stdin.buffer.read()
with open("/tmp/cgi-upload-dump.txt", "wb") as dump:
    dump.write(data.encode() if isinstance(data, str) else data)
print("Content-Type: text/plain\n")
print("<font size=+1>Environment</font><br>")
for param in os.environ.keys():
    print(f"<b>{param:20}</b>: {os.environ[param]}<br>")
form = cgi.FieldStorage()

print("<font size=+1>Form</font><br>")
print(form)
if "file" not in form:
    print("No file part in the request.")
    exit()

file_item = form["file"]

if file_item.filename:
    filename = os.path.basename(file_item.filename)
    with open(f"/home/papaleoneiv/Desktop/WebServer42/{filename}", "wb") as f:
        f.write(file_item.file.read())
    print(f"File '{filename}' uploaded successfully.")
else:
    print("No file was uploaded.") """

