#include <http_client/http_client.hpp>
#include <iostream>

int main()
{
  try
  {
    auto u = http_client::parse_url("http://example.com:8080/users?id=42");

    std::cout << "scheme: " << u.scheme << "\n";
    std::cout << "host:   " << u.host << "\n";
    std::cout << "port:   " << u.port << "\n";
    std::cout << "target: " << u.target << "\n";
  }
  catch (const http_client::Error &e)
  {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
