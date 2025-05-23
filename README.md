# 42 Web Server  
A lightweight HTTP server built in **C++98** as part of the **42 curriculum**.

---

## 🎯 Project Goals

The main objectives of this project were:

- ✅ Parse a configuration file to handle routing and behavior  
- ✅ Support multiple connections using `select()`  
- ✅ Implement basic HTTP methods: `GET`, `POST`, and `DELETE`  
- ✅ Handle CGI requests (e.g., `.sh`, `.py` scripts)

---

## 🧩 Components Overview

### 🧾 ConfigParser

- Parses a custom configuration file using a **stack** and a **tree** structure.  
- The **stack** helps manage nesting levels of `server` and `location` blocks.  
- The **tree** represents the hierarchical relationship between these blocks for efficient traversal.

---

### 🖧 ServerManager

Responsible for managing active servers and client connections.

Main responsibilities include:

- `RegisterConnection()` — Accept and register new client connections  
- `ProcessRequest()` — Read and parse client requests  
- `SendResponse()` — Construct and send HTTP responses

---

### 📬 RequestParser

A **state machine** that incrementally parses HTTP requests **one character at a time**, following the official RFC 3986 specification.

This design ensures support for partial reads and pipelined requests.

---

### 📦 ResourceParser

Uses the parsed configuration to determine whether a requested resource is valid and accessible.  
If not, it prepares an appropriate HTTP error response (e.g., `404`, `403`, etc.).

---

## 🛠 Features Summary

| Feature            | Status |
|--------------------|--------|
| Config file parsing| ✅      |
| Multiple servers   | ✅      |
| Virtual hosting    | ✅      |
| Select-based I/O   | ✅      |
| Chunked body       | ✅      |
| Basic HTTP methods | ✅      |
| CGI support        | ✅      |
| Error handling     | ✅      |

---