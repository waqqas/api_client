
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

template <typename CompletionToken>
auto async_resolve_host(tcp::resolver &resolver, const std::string &host, const std::string &port,
                        CompletionToken &&token)
    // The return type of the initiating function is deduced from the combination
    // of CompletionToken type and the completion handler's signature. When the
    // completion token is a simple callback, the return type is void. However,
    // when the completion token is boost::asio::yield_context (used for stackful
    // coroutines) the return type would be std::size_t, and when the completion
    // token is boost::asio::use_future it would be std::future<std::size_t>.
    -> typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                          void(boost::system::error_code,
                                               tcp::resolver::results_type)>::return_type
{
  // When delegating to the underlying operation we must take care to perfectly
  // forward the completion token. This ensures that our operation works
  // correctly with move-only function objects as callbacks, as well as other
  // completion token types.
  return resolver.async_resolve(host, port, std::forward<CompletionToken>(token));
}

//------------------------------------------------------------------------------

void test_callback()
{
  boost::asio::io_context io_context;

  tcp::resolver resolver(io_context);

  // Test our asynchronous operation using a lambda as a callback.
  async_resolve_host(
      resolver, "www.google.com", "80",
      [](const boost::system::error_code &error, tcp::resolver::results_type results) {
        if (!error)
        {
          std::cout << results.size() << " number of results\n";
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
  std::future<tcp::resolver::results_type> f =
      async_resolve_host(resolver, "www.google.com", "80", boost::asio::use_future);

  io_context.run();

  try
  {
    // Get the result of the operation.
    tcp::resolver::results_type n = f.get();
    std::cout << n.size() << " number of results\n";
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