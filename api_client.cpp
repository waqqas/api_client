
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
  std::unique_ptr<beast::tcp_stream> stream_;
  std::unique_ptr<tcp::resolver>     resolver_;
  const std::string &                host_;
  const std::string &                port_;

  enum
  {
    starting,
    resolving,
    connecting
  } state_;

  template <typename Self>
  void operator()(Self &self, const boost::system::error_code &error,
                  const tcp::resolver::results_type::endpoint_type &endpoint)
  {
    self.complete(error);
  }

  template <typename Self>
  void operator()(Self &self, const boost::system::error_code &error,
                  const tcp::resolver::results_type &results)

  {
    if (!error)
    {
      stream_->async_connect(results, std::move(self));
    }
  }

  template <typename Self>
  void operator()(Self &self)
  {
    resolver_->async_resolve(host_, port_, std::move(self));
  }
};

template <typename CompletionToken>
auto async_resolve_and_connect(boost::asio::io_context &io, const std::string &host,
                               const std::string &port, CompletionToken &&token) ->
    typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                       void(const boost::system::error_code &)>::return_type
{

  std::unique_ptr<beast::tcp_stream> stream   = std::make_unique<beast::tcp_stream>(io);
  std::unique_ptr<tcp::resolver>     resolver = std::make_unique<tcp::resolver>(io);

  return boost::asio::async_compose<CompletionToken, void(const boost::system::error_code &)>(
      async_connect_initiation2{std::move(stream), std::move(resolver), host, port,
                                async_connect_initiation2::starting},
      token);
}

//------------------------------------------------------------------------------

void test_callback()
{
  boost::asio::io_context io_context;

  async_resolve_and_connect(io_context, "www.google.com", "80",
                            [&io_context](const boost::system::error_code &error) {
                              if (!error)
                              {
                                std::cout << "connected " << std::endl;
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

  std::future<void> c =
      async_resolve_and_connect(io_context, "www.google.com", "80", boost::asio::use_future);

  io_context.run();

  try
  {
    c.get();

    std::cout << "connected" << std::endl;
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