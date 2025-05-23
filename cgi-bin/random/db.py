#!/usr/bin/env python3

import cgi

print("Content-Type: text/html\n")

form = cgi.FieldStorage()

print("<html><body>")
print("<h2>Received Parameters:</h2>")
if not form:
    print("<p>No parameters received.</p>")
else:
    print("<ul>")
    for key in form.keys():
        print(f"<li>{key} = {form.getvalue(key)}</li>")
    print("</ul>")
print("</body></html>")