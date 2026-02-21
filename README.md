# http_client

Lightweight header-only C++ HTTP client utilities.

Designed for modern C++ applications that need minimal HTTP request and
response handling without external dependencies.

This library currently provides:

-   URL parsing
-   HTTP response parsing
-   Structured request/response types
-   A minimal, extensible API surface

It is intentionally small and transport-agnostic.

## Installation

### Using Vix Registry

``` bash
vix add gaspardkirira/http_client
```

### Manual download

Download the latest release:

👉 https://github.com/GaspardKirira/http_client/releases

Or clone:

``` bash
git clone https://github.com/GaspardKirira/http_client.git
```

Add the `include/` directory to your project.

## Quick Example

``` cpp
#include <http_client/http_client.hpp>
#include <iostream>

int main()
{
  auto url = http_client::parse_url("http://example.com:8080/api");

  std::cout << "host: " << url.host << "\n";
  std::cout << "port: " << url.port << "\n";
  std::cout << "target: " << url.target << "\n";
}
```

## Features

-   Header-only
-   Zero external dependencies
-   URL parsing (`parse_url`)
-   HTTP response parsing (`parse_http_response`)
-   Structured `Request` and `Response` types
-   C++17 compatible

## What this library provides

### URL parsing

``` cpp
auto u = http_client::parse_url("http://example.com:8080/path?x=1");
```

Extracts:

-   `scheme`
-   `host`
-   `port`
-   `target` (path + query)

Only `http://` is supported in this minimal version.

### HTTP response parsing

``` cpp
std::vector<std::byte> raw = ...;
auto res = http_client::parse_http_response(raw);

std::cout << res.status << "\n";
```

Parses:

-   status code
-   reason phrase
-   headers
-   body

Headers are normalized to lowercase for simpler lookup.

### Request / Response types

``` cpp
http_client::Request req;
req.method = http_client::Method::Get;
req.url = http_client::parse_url("http://example.com/");
```

The transport layer is intentionally left minimal in this version.

## Complexity

All parsing operations are linear in input size.

  Operation             Complexity
  --------------------- ------------
  parse_url             O(n)
  parse_http_response   O(n)

## Design Philosophy

This is a minimal, pragmatic HTTP client foundation.

It is not:

-   A full RFC-compliant HTTP implementation
-   A TLS client
-   An async runtime
-   A full networking stack

The goal is:

-   Small size
-   Clear API
-   Easy embedding
-   Predictable behavior
-   No hidden runtime coupling

Higher-level transport layers can be built on top of this API.

## Tests

Run tests with:

``` bash
vix build
vix tests
```

## Examples

See the `examples/` directory:

-   parse_url.cpp
-   parse_response.cpp
-   minimal_request.cpp

Run an example:

``` bash
vix run examples/parse_url.cpp
```

## Why use this library?

-   No heavy dependencies
-   Ideal for tooling and internal systems
-   Good foundation for custom HTTP transports
-   Easy to embed in CLI tools or micro frameworks

## Roadmap

Future improvements may include:

-   Basic synchronous TCP transport
-   HTTPS support
-   Header builder utilities
-   Query string helpers

## License

MIT License.

