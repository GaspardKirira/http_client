#include <http_client/http_client.hpp>
#include <iostream>
#include <vector>

int main()
{
  const std::string raw_s =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 13\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "Hello, world!";

  std::vector<std::byte> raw;
  raw.reserve(raw_s.size());
  for (unsigned char c : raw_s)
    raw.push_back(static_cast<std::byte>(c));

  auto res = http_client::parse_http_response(raw);

  std::cout << "status: " << res.status << "\n";
  std::cout << "reason: " << res.reason << "\n";

  for (const auto &[k, v] : res.headers)
    std::cout << k << ": " << v << "\n";

  std::string body(reinterpret_cast<const char *>(res.body.data()), res.body.size());
  std::cout << "body: " << body << "\n";

  return 0;
}
