
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
#include <optional>
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

struct async_connect_initiation2
{
  beast::tcp_stream &stream_;
  tcp::resolver &    resolver_;
  const std::string &host_;
  const std::string &port_;

  template <typename Self>
  void operator()(Self &self)
  {
    resolver_.async_resolve(host_, port_, std::move(self));
  }

  template <typename Self>
  void operator()(Self &self, const boost::system::error_code &error,
                  const tcp::resolver::results_type &results)

  {
    if (!error)
    {
      stream_.async_connect(results, std::move(self));
    }
    else
    {
      self.complete(error, std::nullopt);
    }
  }

  template <typename Self>
  void operator()(Self &self, const boost::system::error_code &error,
                  const tcp::resolver::results_type::endpoint_type &endpoint)
  {
    self.complete(error, std::optional<tcp::resolver::results_type::endpoint_type>(endpoint));
  }
};

struct api_client
{
  boost::asio::io_context &io_;
  beast::tcp_stream        stream_;
  tcp::resolver            resolver_;
  const std::string        base_url_;
  const std::string        port_;

  api_client(boost::asio::io_context &io, const std::string &base_url)
    : io_(io)
    , stream_(io)
    , resolver_(io)
    , base_url_(base_url)
    , port_("80")
  {}
  template <typename CompletionToken>
  auto async_resolve_and_connect(CompletionToken &&token) -> typename boost::asio::async_result<
      typename std::decay<CompletionToken>::type,
      void(const boost::system::error_code &,
           std::optional<tcp::resolver::results_type::endpoint_type>)>::return_type
  {

    return boost::asio::async_compose<
        CompletionToken, void(const boost::system::error_code &,
                              std::optional<tcp::resolver::results_type::endpoint_type>)>(
        async_connect_initiation2{
            stream_,
            resolver_,
            base_url_,
            port_,
        },
        token);
  }
};
//------------------------------------------------------------------------------

void test_callback()
{
  boost::asio::io_context io_context;

  api_client client(io_context, "www.google.com");

  client.async_resolve_and_connect(
      [&io_context](const boost::system::error_code &error, auto endpoint) {
        if (!error)
        {
          std::cout << "connected at " << *endpoint << std::endl;
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
  api_client              client(io_context, "www.google.com");
  std::future             c = client.async_resolve_and_connect(boost::asio::use_future);

  io_context.run();

  try
  {
    auto endpoint = c.get();

    std::cout << "connected at " << *endpoint << std::endl;
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