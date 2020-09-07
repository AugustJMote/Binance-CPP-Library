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
      _APIsecret(std::move(APIsecret))
{
  _endpoints = _resolver.resolve("api.binance.us", "443");
  _ssl_context.set_default_verify_paths();
  Test(30);
}

void Connection::Test(int numTests)
{
  std::vector<double> latencies;
  for (int i = 0; i < numTests; ++i)
  {
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                  _ssl_context);
    boost::asio::connect(stream.lowest_layer(), _endpoints);
    stream.handshake(boost::asio::ssl::stream_base::client);

    boost::asio::streambuf reply;
    std::string request =
        "GET /api/v3/time HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
        "close\r\n\r\n" +
        std::to_string(i); // Need to slightly tweak string, otherwise
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
    latencies.push_back(std::chrono::duration_cast<std::chrono::microseconds>(after -
                                                                              before)
                            .count() /
                        1000.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  std::cout << "Successfully connected to IP: " << (*_endpoints).endpoint() << std::endl
            << "With average latency of "
            << std::accumulate(latencies.begin(), latencies.end(), 0.0) / numTests
            << "ms" << std::endl;
}

void Connection::PlaceOrder()
{
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  boost::asio::connect(stream.lowest_layer(), _endpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::streambuf reply;
  std::string messageBody =
      "symbol=LTCBTC&side=BUY&type=MARKET&quantity=1&timestamp=" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  messageBody += "&signature=" + generateSignature(_APIsecret, messageBody);
  std::string request =
      "POST /api/v3/order/test HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
      "close\r\nContent-Type: text/plain\r\nContent-Length:" +
      std::to_string(messageBody.size()) + "\r\nX-MBX-APIKEY:" + _APIkey + "\r\n\r\n" + messageBody;
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  auto before = std::chrono::steady_clock::now();
  boost::asio::read_until(stream, reply, "}");
  auto after = std::chrono::steady_clock::now();
  std::string s(boost::asio::buffer_cast<const char *>(reply.data()),
                reply.size());
  std::cout << "IP: " << (*_endpoints).endpoint() << std::endl
            << "Real delay: "
            << std::chrono::duration_cast<std::chrono::microseconds>(after -
                                                                     before)
                       .count() /
                   1000.0
            << "ms" << std::endl
            << "Message: " << s << std::endl;
}

std::string Connection::generateSignature(const std::string &key,
                                          const std::string &message)
{
  uint8_t *signature =
      HMAC(EVP_sha256(), key.c_str(), key.size(),
           (unsigned char *)message.c_str(), message.size(), NULL, NULL);
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 32; ++i)
    ss << std::setw(2) << std::setfill('0') << (int)signature[i];
  return ss.str();
}
