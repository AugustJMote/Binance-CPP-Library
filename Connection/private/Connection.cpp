#include <sys/time.h>

#include <Connection.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>

Connection::Connection(std::string APIkey, std::string APIsecret)
    : _resolver(_io_context),
      _ssl_context(boost::asio::ssl::context::sslv23),
      _APIkey(std::move(APIkey)),
      _APIsecret(std::move(APIsecret)) {
  _endpoints = _resolver.resolve(
      "api.binance.us", "443",
      boost::asio::ip::resolver_base::flags::address_configured);
  _ssl_context.set_default_verify_paths();
  Test(30);
}

void Connection::Test(int numTests) {
  std::vector<double> latencies;
  for (int i = 0; i < numTests; ++i) {
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                  _ssl_context);
    boost::asio::connect(stream.lowest_layer(), _endpoints);
    stream.handshake(boost::asio::ssl::stream_base::client);

    boost::asio::streambuf reply;
    std::string request =
        "GET /api/v3/time HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
        "close\r\n\r\n" +
        std::to_string(i);  // Need to slightly tweak string, otherwise
                            // responses are cached and too fast
    char crequest[request.size() + 2];
    strcpy(crequest, request.c_str());
    boost::asio::write(stream,
                       boost::asio::buffer(crequest, std::strlen(crequest)));
    auto before = std::chrono::steady_clock::now();
    boost::asio::read_until(stream, reply, "}");
    auto after = std::chrono::steady_clock::now();
    std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                  reply.size());
    latencies.push_back(
        std::chrono::duration_cast<std::chrono::microseconds>(after - before)
            .count() /
        1000.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  auto endpoint = _endpoints;
  while (endpoint != boost::asio::ip::tcp::resolver::iterator()) {
    std::cout << (*endpoint++).endpoint() << std::endl;
  }
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  std::cout << "Successfully connected to IP: "
            << stream.lowest_layer().remote_endpoint().address().to_string()
            << std::endl
            << "With average round-trip latency of "
            << std::accumulate(latencies.begin(), latencies.end(), 0.0) /
                   numTests
            << "ms" << std::endl;
}

std::string Connection::PlaceOrder(
    const std::string symbol, const std::string side, const std::string type,
    const std::string timeInForce, const double quantity,
    const double quoteOrderQty, const double price,
    const std::string newClientOrderId, const double stopPrice,
    const double icebergQty, const std::string newOrderRespType,
    const int32_t recvWindow) {
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=" + symbol + "&side=" + side + "&type=" + type +
      (timeInForce.size() ? ("&timeInForce=" + timeInForce) : "") +
      (quantity ? "&quantity=" + std::to_string(quantity) : "") +
      (quoteOrderQty ? "&quoteOrderQty=" + std::to_string(quoteOrderQty) : "") +
      (price ? "&price=" + std::to_string(price) : "") +
      (newClientOrderId.size() ? "&newClientOrderId=" + newClientOrderId : "") +
      (stopPrice ? "&stopPrice=" + std::to_string(stopPrice) : "") +
      (icebergQty ? "&icebergQty=" + std::to_string(icebergQty) : "") +
      (newOrderRespType.size() ? "&newOrderRespType=" + newOrderRespType : "") +
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::TestPlaceOrder(
    const std::string symbol, const std::string side, const std::string type,
    const std::string timeInForce, const double quantity,
    const double quoteOrderQty, const double price,
    const std::string newClientOrderId, const double stopPrice,
    const double icebergQty, const std::string newOrderRespType,
    const int32_t recvWindow) {
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=" + symbol + "&side=" + side + "&type=" + type +
      (timeInForce.size() ? ("&timeInForce=" + timeInForce) : "") +
      (quantity ? "&quantity=" + std::to_string(quantity) : "") +
      (quoteOrderQty ? "&quoteOrderQty=" + std::to_string(quoteOrderQty) : "") +
      (price ? "&price=" + std::to_string(price) : "") +
      (newClientOrderId.size() ? "&newClientOrderId=" + newClientOrderId : "") +
      (stopPrice ? "&stopPrice=" + std::to_string(stopPrice) : "") +
      (icebergQty ? "&icebergQty=" + std::to_string(icebergQty) : "") +
      (newOrderRespType.size() ? "&newOrderRespType=" + newOrderRespType : "") +
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order/test HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::QueryOrder(const std::string symbol, const int32_t orderId,
                         const std::string origClientOrderId,
                         const int32_t recvWindow) {
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=" + symbol +
      (orderId ? "&orderId=" + std::to_string(orderId) : "") +
      (origClientOrderId.size() ? "&origClientOrderId=" + origClientOrderId : "") +
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::CancelOrder(const std::string symbol, const int32_t orderId,
                         const std::string origClientOrderId,
                         const std::string newClientOrderId,
                         const int32_t recvWindow) {
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=" + symbol +
      (orderId ? "&orderId=" + std::to_string(orderId) : "") +
      (origClientOrderId.size() ? "&origClientOrderId=" + origClientOrderId : "") +
      (newClientOrderId.size() ? "&newClientOrderId=" + origClientOrderId : "") +
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::AllOrders(const std::string symbol, const int32_t orderId,
                         const int64_t startTime, const int64_t endTime,
                         const int32_t limit,
                         const int32_t recvWindow){
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=" + symbol +
      (orderId ? "&orderId=" + std::to_string(orderId) : "") +
      (startTime ? "&startTime=" + std::to_string(startTime) : "") +
      (endTime ? "&endTime=" + std::to_string(endTime) : "") +
      (limit ? "&limit=" + std::to_string(limit) : "") +
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::AccountInfo(const int32_t recvWindow){
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      (recvWindow ? "&recvWindow=" + std::to_string(recvWindow) : "") +
      "&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey +
      "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  return s;
}

std::string Connection::generateSignature(const std::string &key,
                                          const std::string &message) {
  uint8_t *signature = HMAC(EVP_sha256(), key.c_str(), key.size(),
                            (unsigned char *)message.c_str(), message.size(),
                            NULL, NULL);  // Does HMAC leak?
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 32; ++i)
    ss << std::setw(2) << std::setfill('0') << (int)signature[i];
  return ss.str();
}
