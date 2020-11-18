#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

class Connection {
 private:
  boost::asio::io_context _io_context;
  boost::asio::ip::tcp::resolver _resolver;
  boost::asio::ssl::context _ssl_context;
  boost::asio::ip::tcp::resolver::iterator _endpoints;
  std::string _APIkey;
  std::string _APIsecret;

 public:
  Connection(std::string APIkey, std::string APIsecret);
  void Test(int numTests);
  std::string PlaceOrder(const std::string symbol, const std::string side,
                         const std::string type, const std::string timeInForce,
                         const double quantity, const double quoteOrderQty,
                         const double price, const std::string newClientOrderId,
                         const double stopPrice, const double icebergQty,
                         const std::string newOrderRespType,
                         const int32_t recvWindow);

 private:
  std::string generateSignature(const std::string &key,
                                const std::string &message);
};
