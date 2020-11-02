#include "api_client.h"

#include <iostream>

int main(void)
{
  net::io_context io_context;

  api_client client(io_context, "www.google.com");

  client.async_resolve_host(
      [&client](const boost::system::error_code &ec, const tcp::resolver::results_type &results) {
        if (!ec)
        {
          client.async_connect_host(
              results, [&client](const boost::system::error_code &                 ec,
                                 const tcp::resolver::results_type::endpoint_type &endpoint) {
                if (!ec)
                {
                  std::cout << "connected at " << endpoint << std::endl;
                  client.async_get("/", [](const boost::system::error_code &    ec,
                                           const api_client::http_response_type response) {
                    if (!ec)
                    {
                      std::cout << "response: " << *response << std::endl;
                    }
                    else
                    {
                      std::clog << "error: " << ec.message() << std::endl;
                    }
                  });
                }
              });
        }
      });
  io_context.run();
  return EXIT_SUCCESS;
}