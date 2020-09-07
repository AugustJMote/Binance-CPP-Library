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
  void PlaceOrder();

 private:
  std::string generateSignature(const std::string& key,
                                const std::string& message);
};
