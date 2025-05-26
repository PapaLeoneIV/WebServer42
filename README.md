# 🚀 Lightweight C++98 HTTP Web Server | 42 Project

A high-performance, lightweight HTTP server built in **C++98**, developed as part of the **42 School Curriculum**. This project showcases essential web server fundamentals including **non-blocking I/O**, **select() for multiplexing**, **CGI integration**, and **virtual hosting**—all written without modern C++ features, ensuring full C++98 compatibility.

---

## 🎯 Project Objectives

This project was designed to provide deep insights into network programming and server architecture. The main goals include:

- ✅ Parsing and applying custom server configurations  
- ✅ Supporting multiple simultaneous client connections using `select()`  
- ✅ Handling requests with **non-blocking file descriptors**  
- ✅ Implementing core HTTP methods: `GET`, `POST`, and `DELETE`  
- ✅ Executing **CGI scripts** (`.py`, `.sh`, etc.)  
- ✅ Managing error handling and response status codes

---

## 🔧 How This C++ HTTP Server Works

This web server listens on multiple ports, handles multiple clients using `select()`, and supports serving static files, executing CGI programs, and returning appropriate HTTP responses.

It is fully compliant with basic HTTP/1.1 behaviors and can handle chunked bodies, timeouts, and persistent connections.

---

## 🧩 Components Overview

### 🧾 ConfigParser

- Parses a custom configuration file using a **stack-based parser** and builds a **tree structure** to represent nested `server` and `location` blocks.  
- Enables flexible and hierarchical configuration similar to NGINX.

### 🖧 ServerManager

Manages all active servers and client sockets:

- `registerConnection()` — Accept and register new client connections  
- `processRequest()` — Read and parse incoming HTTP requests  
- `sendResponse()` — Build and send HTTP responses (static or CGI)

### 📬 RequestParser

- Implements a **state machine** to parse HTTP requests one character at a time.  
- Supports **partial reads** and **HTTP pipelining**.  
- Follows [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986) to ensure valid URI parsing.

### 📦 ResourceValidator

- Validates requested resources based on the parsed configuration.  
- Handles errors like `404 Not Found`, `403 Forbidden`, and `405 Method Not Allowed`.

---

## ⚙️ Supported Features

| Feature                     | Status |
|-----------------------------|--------|
| Custom configuration parser | ✅      |
| Multiple server instances   | ✅      |
| Virtual hosting             | ✅      |
| Non-blocking `select()` I/O | ✅      |
| Chunked transfer encoding   | ✅      |
| HTTP methods: GET, POST, DELETE | ✅  |
| CGI script execution        | ✅      |
| Detailed error handling     | ✅      |
| C++98 compatibility         | ✅      |

---

## 🧪 Example Use Cases

- 🔄 Serve static content  
- 🐍 Run Python CGI scripts  
- ⚙️ Build a lightweight microservice  
- 📚 Use as a learning project for network programming in C++

---

## 🖥️ Running the Server

```bash
make
./webserv config/server.conf
```

> Ensure your configuration file is properly structured. Example provided in `config/valid`.

---

## 📁 Directory Structure

```
includes/       # Header files  
src/            # Source code for server components  
config/         # Example configuration files
servers/        # Includes some (html, css, js) file referenced in config files
static/         # Includes some file to use for testing
cgi-bin/        # Includes some .py and .sh scripts
```

---

## 📚 Configuration File Overview

The config file supports:

```
server {
    listen 8080;
    host localhost;
    server_name localhost;

    location / {
        root www/html;
        index index.html;
    }

    location /cgi-bin/ {
      root www/html;   
      cgi_path /usr/bin/python3 /bin/bash;                     
      cgi_ext .py .sh;                                         
  }
}
```


## 🤝 Contributions

This project was completed as part of the **42 curriculum** all alone.

---

## 🌐 Related Links

- [RFC 3986 – URI Syntax](https://datatracker.ietf.org/doc/html/rfc3986)  
- [HTTP/1.1 Spec](https://www.rfc-editor.org/rfc/rfc2616)  
- [CGI Spec](https://tools.ietf.org/html/rfc3875)

---