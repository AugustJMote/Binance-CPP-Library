#pragma once

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <unordered_map>
#include <iostream>
#include <string>
#include <thread>
#include <iomanip>
#include <atomic>

typedef std::map<double, double> levelmap;

class OrderBook
{
private:
  boost::asio::ip::tcp::resolver::iterator _endpoints;
  boost::asio::io_context _io_context;
  boost::asio::ip::tcp::resolver _resolver;
  boost::asio::ssl::context _ssl_context;
  std::map<double, levelmap> _orderBook;
  std::atomic<bool> _running;
  std::chrono::system_clock::time_point _streamStarted;
  std::thread _main;

public:
  OrderBook();
  ~OrderBook();
  bool Init(const std::string &tradingPair);
  bool Stop();
  std::map<double, levelmap> Get();

private:
  void MainThread(const std::string tradingPair);
  void ParseJson(const std::string &json);
};
