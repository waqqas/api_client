
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

using boost::asio::ip::tcp;

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http  = beast::http;           // from <boost/beast/http.hpp>
namespace net   = boost::asio;           // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// This is the simplest example of a composed asynchronous operation, where we
// simply repackage an existing operation. The asynchronous operation
// requirements are met by delegating responsibility to the underlying
// operation.

struct async_resolve_initiation
{
  template <typename CompletionHandler>
  void operator()(CompletionHandler &&completion_handler, tcp::resolver &resolver,
                  const std::string &host, const std::string &port) const
  {
    auto executor =
        boost::asio::get_associated_executor(completion_handler, resolver.get_executor());

    resolver.async_resolve(
        host, port,
        boost::asio::bind_executor(executor, std::forward<CompletionHandler>(completion_handler)));
  }
};

struct async_connect_initiation
{
  template <typename CompletionHandler>
  void operator()(CompletionHandler &&completion_handler, beast::tcp_stream &stream,
                  const tcp::resolver::results_type &results) const
  {

    stream.expires_after(std::chrono::seconds(10));

    auto executor = boost::asio::get_associated_executor(completion_handler, stream.get_executor());

    stream.async_connect(results, std::forward<CompletionHandler>(completion_handler));
  }
};

template <typename CompletionToken>
auto async_resolve_host(tcp::resolver &resolver, const std::string &host, const std::string &port,
                        CompletionToken &&token) ->
    typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                       void(const boost::system::error_code &,
                                            const tcp::resolver::results_type &)>::return_type
{
  return boost::asio::async_initiate<CompletionToken, void(const boost::system::error_code &,
                                                           const tcp::resolver::results_type &)>(
      async_resolve_initiation(), token, std::ref(resolver), host, port);
}

template <typename CompletionToken>
auto async_connect_host(beast::tcp_stream &stream, const tcp::resolver::results_type &results,
                        CompletionToken &&token) ->
    typename boost::asio::async_result<
        typename std::decay<CompletionToken>::type,
        void(const boost::system::error_code &,
             const tcp::resolver::results_type::endpoint_type &)>::return_type
{
  return boost::asio::async_initiate<CompletionToken,
                                     void(const boost::system::error_code &,
                                          const tcp::resolver::results_type::endpoint_type &)>(
      async_connect_initiation(), token, std::ref(stream), results);
}
//------------------------------------------------------------------------------

void test_callback()
{
  boost::asio::io_context io_context;

  tcp::resolver resolver(io_context);

  // Test our asynchronous operation using a lambda as a callback.
  async_resolve_host(resolver, "www.google.com", "80",
                     [&io_context](const boost::system::error_code &  error,
                                   const tcp::resolver::results_type &results) {
                       if (!error)
                       {
                         tcp::resolver::results_type::const_iterator it;
                         for (it = results.begin(); it != results.end(); ++it)
                         {
                           std::cout << "result1: " << it->host_name() << std::endl;
                         }

                         beast::tcp_stream stream(io_context);
                         async_connect_host(
                             stream, results,
                             [](const boost::system::error_code &                 error,
                                const tcp::resolver::results_type::endpoint_type &endpoint) {
                               if (!error)
                               {
                                 std::cout << "connected at " << endpoint << std::endl;
                               }
                             });
                       }
                       else
                       {
                         std::cout << "Error: " << error.message() << "\n";
                       }
                     });

  io_context.run();
}

//------------------------------------------------------------------------------

void test_future()
{
  boost::asio::io_context io_context;

  tcp::resolver resolver(io_context);

  // Test our asynchronous operation using the use_future completion token.
  // This token causes the operation's initiating function to return a future,
  // which may be used to synchronously wait for the result of the operation.
  std::future<tcp::resolver::results_type> r =
      async_resolve_host(resolver, "www.google.com", "80", boost::asio::use_future);

  io_context.run();

  try
  {
    // Get the result of the operation.
    const tcp::resolver::results_type &results = r.get();

    std::cout << results.size() << " number of results\n";
    tcp::resolver::results_type::const_iterator it;
    for (it = results.begin(); it != results.end(); ++it)
    {
      std::cout << "result2: " << it->host_name() << std::endl;
    }

    beast::tcp_stream stream(io_context);

    std::future<tcp::resolver::results_type::endpoint_type> c =
        async_connect_host(stream, results, boost::asio::use_future);

    io_context.run();

    try
    {
      const tcp::resolver::results_type::endpoint_type &endpoint = c.get();

      std::cout << endpoint << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cout << "Error: " << e.what() << "\n";
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

int main(void)
{
  test_callback();
  test_future();
  return 0;
}