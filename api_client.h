#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http  = beast::http;           // from <boost/beast/http.hpp>
namespace net   = boost::asio;           // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

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

struct async_request
{
  using http_response_type = std::unique_ptr<http::response<http::dynamic_body>>;

  beast::tcp_stream &                               stream_;
  std::unique_ptr<http::request<http::string_body>> req_;
  enum
  {
    writing,
    reading,
    waiting_for_response
  } state_;
  std::unique_ptr<beast::flat_buffer> buffer_;
  http_response_type                  res_;

  async_request(beast::tcp_stream &stream, std::unique_ptr<http::request<http::string_body>> req)
    : stream_(stream)
    , req_(std::move(req))
    , state_(writing)
  {
    res_    = std::make_unique<http::response<http::dynamic_body>>();
    buffer_ = std::make_unique<boost::beast::flat_buffer>();
  }

  template <typename Self>
  void operator()(Self &self, const boost::system::error_code &error = boost::system::error_code(),
                  const std::size_t bytes_transferred = 0)
  {
    if (!error)
    {
      switch (state_)
      {
      case writing:
        state_ = reading;
        stream_.expires_after(std::chrono::seconds(30));
        http::async_write(stream_, *req_, std::move(self));
        return;
      case reading:
        state_ = waiting_for_response;
        stream_.expires_after(std::chrono::seconds(30));
        http::async_read(stream_, *buffer_, *res_, std::move(self));
        return;
      case waiting_for_response:
        break;
      }
    }
    req_.reset();
    self.complete(error, std::move(res_));
  }
};

struct api_client
{
  net::io_context & io_;
  beast::tcp_stream stream_;
  tcp::resolver     resolver_;
  const std::string base_url_;
  const std::string port_;
  std::string       bearer_token;

  using http_response_type = async_request::http_response_type;

  api_client(net::io_context &io, const std::string &base_url, const std::string &port = "80")
    : io_(io)
    , stream_(io_)
    , resolver_(io_)
    , base_url_(base_url)
    , port_(port)
  {}

  void set_auth_token(const std::string &token)
  {
    bearer_token = token;
  }

  template <typename CompletionToken>
  auto async_get(const std::string &path, CompletionToken &&token) ->
      typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                         void(const boost::system::error_code &,
                                              const http_response_type)>::return_type
  {
    std::unique_ptr<http::request<http::string_body>> req =
        std::make_unique<http::request<http::string_body>>();

    req->version(11);
    req->method(http::verb::get);
    req->target(path);
    req->set(http::field::host, base_url_);
    req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    if (!bearer_token.empty())
    {
      req->set(http::field::authorization, "Bearer " + bearer_token);
    }

    return boost::asio::async_compose<CompletionToken, void(const boost::system::error_code &,
                                                            const http_response_type)>(
        async_request(std::ref(stream_), std::move(req)), token, stream_, io_);
  }

  template <typename CompletionToken>
  auto async_post(const std::string &path, const std::string &body, CompletionToken &&token) ->
      typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                         void(const boost::system::error_code &,
                                              const http_response_type)>::return_type
  {
    std::unique_ptr<http::request<http::string_body>> req =
        std::make_unique<http::request<http::string_body>>();

    req->version(11);
    req->method(http::verb::post);
    req->target(path);
    req->set(http::field::host, base_url_);
    req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    if (!bearer_token.empty())
    {
      req->set(http::field::authorization, "Bearer " + bearer_token);
    }

    if (body.length() > 0)
    {
      req->set(http::field::content_type, "application/json");
      req->body() = body;
      req->prepare_payload();
    }

    return boost::asio::async_compose<CompletionToken, void(const boost::system::error_code &,
                                                            const http_response_type)>(
        async_request(std::ref(stream_), std::move(req)), token, stream_, io_);
  }

  template <typename CompletionToken>
  auto async_resolve_host(CompletionToken &&token) ->
      typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                                         void(const boost::system::error_code &,
                                              const tcp::resolver::results_type &)>::return_type
  {
    return boost::asio::async_initiate<CompletionToken, void(const boost::system::error_code &,
                                                             const tcp::resolver::results_type &)>(
        async_resolve_initiation(), token, std::ref(resolver_), base_url_, port_);
  }

  template <typename CompletionToken>
  auto async_connect_host(const tcp::resolver::results_type &results, CompletionToken &&token) ->
      typename boost::asio::async_result<
          typename std::decay<CompletionToken>::type,
          void(const boost::system::error_code &,
               const tcp::resolver::results_type::endpoint_type &)>::return_type
  {
    return boost::asio::async_initiate<CompletionToken,
                                       void(const boost::system::error_code &,
                                            const tcp::resolver::results_type::endpoint_type &)>(
        async_connect_initiation(), token, std::ref(stream_), results);
  }
};

#endif
