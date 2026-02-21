#include <http_client/http_client.hpp>

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

static void test_parse_url_basic()
{
  auto u = http_client::parse_url("http://example.com");

  assert(u.scheme == "http");
  assert(u.host == "example.com");
  assert(u.port == 80);
  assert(u.target == "/");
}

static void test_parse_url_with_path_query()
{
  auto u = http_client::parse_url("http://example.com:8080/users?id=42");

  assert(u.scheme == "http");
  assert(u.host == "example.com");
  assert(u.port == 8080);
  assert(u.target == "/users?id=42");
}

static void test_parse_url_errors()
{
  bool threw = false;

  try
  {
    (void)http_client::parse_url("https://example.com");
  }
  catch (const http_client::Error &)
  {
    threw = true;
  }
  assert(threw);

  threw = false;
  try
  {
    (void)http_client::parse_url("http://");
  }
  catch (const http_client::Error &)
  {
    threw = true;
  }
  assert(threw);

  threw = false;
  try
  {
    (void)http_client::parse_url("http://example.com:");
  }
  catch (const http_client::Error &)
  {
    threw = true;
  }
  assert(threw);

  threw = false;
  try
  {
    (void)http_client::parse_url("http://example.com:99999/");
  }
  catch (const http_client::Error &)
  {
    threw = true;
  }
  assert(threw);
}

static void test_parse_http_response_basic()
{
  const std::string raw_s =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 5\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "hello";

  std::vector<std::byte> raw;
  raw.reserve(raw_s.size());
  for (unsigned char c : raw_s)
    raw.push_back(static_cast<std::byte>(c));

  auto res = http_client::parse_http_response(raw);

  assert(res.status == 200);
  assert(res.reason == "OK");

  // keys are normalized to lowercase in parser
  assert(res.headers.at("content-length") == "5");
  assert(res.headers.at("content-type") == "text/plain");

  const std::string body(reinterpret_cast<const char *>(res.body.data()), res.body.size());
  assert(body == "hello");
}

static void test_request_not_implemented()
{
  http_client::Request req;
  req.method = http_client::Method::Get;
  req.url = http_client::parse_url("http://example.com/");

  bool threw = false;
  try
  {
    (void)http_client::request(req);
  }
  catch (const http_client::Error &)
  {
    threw = true;
  }
  assert(threw);
}

int main()
{
  test_parse_url_basic();
  test_parse_url_with_path_query();
  test_parse_url_errors();
  test_parse_http_response_basic();
  test_request_not_implemented();

  std::cout << "All tests passed\n";
  return 0;
}
