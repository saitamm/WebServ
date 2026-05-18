# WebServ

WebServ is a small HTTP/1.1 web server written in C++98. It uses non-blocking sockets with `epoll`, parses an nginx-like configuration file, serves static files, supports uploads and deletion, and can execute CGI scripts for Python, PHP, and Perl.

## Features

- HTTP methods: `GET`, `POST`, and `DELETE`
- Multiple `server` blocks from one configuration file
- Host and port binding with duplicate-listen validation
- Per-location method allow-lists
- Static file serving with basic content-type detection
- Directory index files and optional autoindex listings
- Upload handling with `Content-Length`
- Recursive `DELETE` support inside configured upload stores
- CGI execution for `.py`, `.php`, and `.pl`
- CGI timeout handling with `504 Gateway Timeout`
- Custom and default HTML error pages
- Basic redirects through location `return 301/302 ...`
- Simple session cookie named `user`

## Project Layout

```text
.
├── Includes/              Header files for config, client, request, response, CGI
├── srcs/
│   ├── Client/            Request parsing and client state handling
│   └── Server/            Config parsing, response building, methods, CGI, errors
├── www/                   Example web root, CGI scripts, uploads, static assets
├── file.conf              Example configuration file
├── server.cpp             Main event loop and socket setup
└── Makefile               Build rules
```

## Requirements

- Linux, because the server uses `epoll`
- `c++` with C++98 support
- `make`
- Optional CGI interpreters, depending on your config:
  - `/usr/bin/python3`
  - `/usr/bin/php`
  - `/usr/bin/perl`

## Build

```sh
make
```

This creates the executable:

```text
./WebServ
```

Useful make targets:

```sh
make clean
make fclean
make re
```

## Run

Start the server with a configuration file:

```sh
./WebServ file.conf
```

The included `file.conf` listens on:

```text
127.0.0.1:8800
```

Then open:

```text
http://127.0.0.1:8800/
```

Stop the server with `Ctrl+C`.

## Configuration

The configuration format is line-oriented and inspired by nginx, but it does not use braces or semicolons.

Example:

```conf
server
    listen 127.0.0.1:8800
    server_name localhost
    root www
    index indexServer.html
    client_max_body_size 10000M
    error_page 404 srcs/Server/errors/404.html

    location /
        root www
        index index.html
        methods GET POST DELETE
        autoindex on
        upload_store uploads
        cgi_extensions .py .php .pl
        cgi_pass /usr/bin/python3 /usr/bin/php /usr/bin/perl
```

### Server Directives

| Directive | Description |
| --- | --- |
| `listen HOST:PORT` | Required. IPv4 host and port to bind. |
| `server_name NAME` | Server name used by CGI environment. |
| `root PATH` | Required. Default document root. |
| `index FILE` | Default index file for directories. |
| `error_page CODE PATH` | Custom HTML error page. |
| `client_max_body_size SIZE` | Maximum request body size. Supports `K`, `M`, and `G`. |

### Location Directives

| Directive | Description |
| --- | --- |
| `location PATH` | Starts a location block. |
| `root PATH` | Overrides the server root for this location. |
| `index FILE` | Location-specific index file. |
| `methods GET POST DELETE` | Allowed methods for this location. |
| `autoindex on/off` | Enables or disables directory listings. |
| `upload_store PATH` | Directory where uploaded files are written. |
| `cgi_extensions .py .php .pl` | File extensions that should run as CGI. |
| `cgi_pass PATH...` | Allowed interpreters. Supported paths are `/usr/bin/python3`, `/usr/bin/php`, and `/usr/bin/perl`. |
| `return CODE TARGET` | Redirects requests. `301` and `302` are handled as redirects. |

## Request Examples

Static page:

```sh
curl -i http://127.0.0.1:8800/
```

Upload a file:

```sh
curl -i -X POST \
  -H "Content-Type: application/octet-stream" \
  --data-binary @README.md \
  http://127.0.0.1:8800/
```

Run a Python CGI script, if the matching location enables `.py` and `/usr/bin/python3`:

```sh
curl -i http://127.0.0.1:8800/cgi-bin/script.py
```

Delete an uploaded file:

```sh
curl -i -X DELETE http://127.0.0.1:8800/<uploaded-file-name>
```

## Error Pages

Default error pages live in:

```text
srcs/Server/errors/
```

The server initializes defaults for common statuses such as `400`, `403`, `404`, `405`, `409`, `411`, `413`, `431`, `500`, `501`, `502`, and `504`. A config file can override individual pages with `error_page`.

## Notes

- The server expects exactly one argument: `./WebServ <file.conf>`.
- Request headers larger than roughly 8 KB are rejected.
- Client connections are closed after each response.
- CGI processes are killed after a short timeout.
- The sample `file.conf` is meant for experimentation. Adjust `root`, location `root`, and `upload_store` paths to match where you want files served and uploaded.
