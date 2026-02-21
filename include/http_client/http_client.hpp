#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <chrono>

namespace http_client
{

  /**
   * @brief HTTP method supported by this minimal client.
   */
  enum class Method
  {
    Get,
    Post
  };

  /**
   * @brief Parsed HTTP URL components.
   *
   * Supports:
   * - http://host[:port]/path?query
   *
   * Limitations:
   * - No https/TLS in this minimal version
   * - No userinfo, IPv6 brackets, or full RFC compliance
   */
  struct Url
  {
    std::string scheme;      ///< "http"
    std::string host;        ///< hostname or IPv4
    std::uint16_t port = 80; ///< port (default 80)
    std::string target;      ///< path + optional "?query" (default "/")
  };

  /**
   * @brief HTTP request structure.
   */
  struct Request
  {
    Method method = Method::Get;                          ///< GET or POST
    Url url;                                              ///< Parsed URL
    std::unordered_map<std::string, std::string> headers; ///< Request headers
    std::vector<std::byte> body;                          ///< Request body (for POST)
    std::chrono::milliseconds timeout{5000};              ///< Connect+IO timeout
  };

  /**
   * @brief HTTP response structure.
   */
  struct Response
  {
    int status = 0;                                       ///< Status code (e.g. 200)
    std::string reason;                                   ///< Reason phrase (e.g. "OK")
    std::unordered_map<std::string, std::string> headers; ///< Response headers
    std::vector<std::byte> body;                          ///< Response body bytes
  };

  /**
   * @brief Exception type thrown by this library.
   */
  class Error : public std::runtime_error
  {
  public:
    explicit Error(const std::string &msg) : std::runtime_error(msg) {}
  };

  /**
   * @brief Parse a minimal http URL.
   *
   * Accepted forms:
   * - http://example.com
   * - http://example.com/
   * - http://example.com:8080/path?x=1
   *
   * @param s URL string
   * @return Url Parsed components
   * @throws Error on invalid URL
   */
  inline Url parse_url(std::string_view s)
  {
    Url u;

    const std::string_view prefix = "http://";
    if (s.substr(0, prefix.size()) != prefix)
      throw Error("parse_url: only http:// is supported");

    u.scheme = "http";
    s.remove_prefix(prefix.size());

    if (s.empty())
      throw Error("parse_url: missing host");

    // Split authority vs target
    std::string_view authority = s;
    std::string_view target = "/";
    const std::size_t slash = s.find('/');
    if (slash != std::string_view::npos)
    {
      authority = s.substr(0, slash);
      target = s.substr(slash);
      if (target.empty())
        target = "/";
    }

    if (authority.empty())
      throw Error("parse_url: missing host");

    // host[:port]
    std::string_view host = authority;
    std::uint16_t port = 80;

    const std::size_t colon = authority.find(':');
    if (colon != std::string_view::npos)
    {
      host = authority.substr(0, colon);
      const std::string_view port_sv = authority.substr(colon + 1);
      if (host.empty() || port_sv.empty())
        throw Error("parse_url: invalid host:port");

      std::size_t idx = 0;
      unsigned long p = 0;
      try
      {
        p = std::stoul(std::string(port_sv), &idx, 10);
      }
      catch (...)
      {
        throw Error("parse_url: invalid port");
      }
      if (idx != port_sv.size() || p > 65535)
        throw Error("parse_url: invalid port");
      port = static_cast<std::uint16_t>(p);
    }

    if (host.empty())
      throw Error("parse_url: missing host");

    u.host.assign(host.begin(), host.end());
    u.port = port;
    u.target.assign(target.begin(), target.end());
    if (u.target.empty())
      u.target = "/";

    return u;
  }

  /**
   * @brief Build an HTTP request line.
   */
  inline std::string method_to_string(Method m)
  {
    switch (m)
    {
    case Method::Get:
      return "GET";
    case Method::Post:
      return "POST";
    }
    return "GET";
  }

  /**
   * @brief Lowercase ASCII helper for header parsing.
   */
  inline char ascii_lower(char c)
  {
    if (c >= 'A' && c <= 'Z')
      return static_cast<char>(c - 'A' + 'a');
    return c;
  }

  /**
   * @brief Trim ASCII whitespace from both ends.
   */
  inline std::string_view trim(std::string_view v)
  {
    auto is_ws = [](char c)
    { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };

    while (!v.empty() && is_ws(v.front()))
      v.remove_prefix(1);
    while (!v.empty() && is_ws(v.back()))
      v.remove_suffix(1);
    return v;
  }

  /**
   * @brief Parse a raw HTTP response (status line + headers + body).
   *
   * This helper is useful for unit tests and for integrating custom transports.
   *
   * @param raw Complete response bytes
   * @return Response Parsed response
   * @throws Error on malformed response
   */
  inline Response parse_http_response(const std::vector<std::byte> &raw)
  {
    Response res;

    const char *data = reinterpret_cast<const char *>(raw.data());
    const std::size_t n = raw.size();
    std::string_view sv(data, n);

    const std::size_t header_end = sv.find("\r\n\r\n");
    if (header_end == std::string_view::npos)
      throw Error("parse_http_response: missing header terminator");

    std::string_view head = sv.substr(0, header_end);
    std::string_view body = sv.substr(header_end + 4);

    // Status line
    const std::size_t first_crlf = head.find("\r\n");
    std::string_view status_line = head;
    std::string_view headers_block;
    if (first_crlf != std::string_view::npos)
    {
      status_line = head.substr(0, first_crlf);
      headers_block = head.substr(first_crlf + 2);
    }

    // HTTP/1.1 200 OK
    const std::size_t sp1 = status_line.find(' ');
    if (sp1 == std::string_view::npos)
      throw Error("parse_http_response: invalid status line");

    const std::size_t sp2 = status_line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos)
      throw Error("parse_http_response: invalid status line");

    const std::string_view code_sv = status_line.substr(sp1 + 1, sp2 - (sp1 + 1));
    try
    {
      res.status = std::stoi(std::string(code_sv));
    }
    catch (...)
    {
      throw Error("parse_http_response: invalid status code");
    }

    res.reason = std::string(trim(status_line.substr(sp2 + 1)));

    // Headers
    while (!headers_block.empty())
    {
      const std::size_t crlf = headers_block.find("\r\n");
      std::string_view line = headers_block;
      if (crlf != std::string_view::npos)
        line = headers_block.substr(0, crlf);

      headers_block = (crlf == std::string_view::npos)
                          ? std::string_view{}
                          : headers_block.substr(crlf + 2);

      if (line.empty())
        continue;

      const std::size_t colon = line.find(':');
      if (colon == std::string_view::npos)
        continue;

      std::string key(line.substr(0, colon));
      std::string_view val_sv = trim(line.substr(colon + 1));

      // Normalize key to lowercase to simplify lookups
      for (char &c : key)
        c = ascii_lower(c);

      res.headers[key] = std::string(val_sv);
    }

    // Body
    res.body.assign(reinterpret_cast<const std::byte *>(body.data()),
                    reinterpret_cast<const std::byte *>(body.data() + body.size()));

    return res;
  }

  /**
   * @brief Perform a minimal HTTP request (synchronous, plain TCP).
   *
   * This header-only version exposes the API but intentionally does not ship
   * with a platform socket implementation here. It is meant to be completed
   * in the .cpp version or extended with a small internal socket layer.
   *
   * Current status:
   * - API is stable
   * - Parsing helpers are ready
   *
   * @param req Request
   * @return Response Response
   * @throws Error on network/protocol errors
   */
  inline Response request(const Request & /*req*/)
  {
    throw Error("http_client::request: network transport not implemented in this header-only version yet");
  }

} // namespace http_client
