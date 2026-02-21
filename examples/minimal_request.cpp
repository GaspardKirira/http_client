#include <http_client/http_client.hpp>
#include <iostream>

int main()
{
  http_client::Request req;
  req.method = http_client::Method::Get;
  req.url = http_client::parse_url("http://example.com/");

  try
  {
    auto res = http_client::request(req);

    std::cout << "status: " << res.status << "\n";
  }
  catch (const http_client::Error &e)
  {
    std::cout << "request failed (expected in this version): "
              << e.what() << "\n";
  }

  return 0;
}
