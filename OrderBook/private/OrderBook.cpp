#include "OrderBook.hpp"

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <algorithm>

OrderBook::OrderBook()
    : _resolver(_io_context),
      _ssl_context(boost::asio::ssl::context::tlsv12_client),
      _running(false)
{
  _ssl_context.set_default_verify_paths();
}

OrderBook::~OrderBook() { Stop(); }

bool OrderBook::Init(const std::string &tradingPair)
{
  if (_running)
    return false;
  _running = true;
  PrefetchDepth(tradingPair);
  _main = std::thread([this, &tradingPair]() { MainThread(tradingPair); });
  return true;
}

void OrderBook::MainThread(const std::string tradingPair)
{
  std::cout << "Starting Main Thread" << std::endl;
  try
  {
    _endpoints = _resolver.resolve("stream.binance.us", "9443");
    boost::beast::flat_buffer buffer;
    boost::beast::websocket::stream<
        boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>
        stream(_io_context, _ssl_context);
    auto ep = boost::asio::connect(get_lowest_layer(stream), _endpoints);
    stream.next_layer().handshake(boost::asio::ssl::stream_base::client);
    stream.handshake("stream.binance.us:9443", "/ws/" + tradingPair + "@depth");
    _streamStarted = std::chrono::system_clock::now();
    while (_running && stream.read(buffer))
    {
      std::string json(static_cast<char *>(buffer.data().data()));
      buffer.clear();
      ParseUpdate(json);
      if (std::chrono::system_clock::now() - _streamStarted > std::chrono::hours(23))
      {
        _endpoints = _resolver.resolve("stream.binance.us", "9443");
        auto ep = boost::asio::connect(get_lowest_layer(stream), _endpoints);
        stream.next_layer().handshake(boost::asio::ssl::stream_base::client);
        stream.handshake("stream.binance.us:9443", "/ws/" + tradingPair + "@depth");
        _streamStarted = std::chrono::system_clock::now();
      }
    }
  }
  catch (std::exception &e)
  {
    std::cout << "Connection Failed: " << e.what() << std::endl;
  }
}

bool OrderBook::Stop()
{
  if (!_running)
    return false;
  _running = false;
  _main.join();
  return true;
}

std::map<char, levelmap> OrderBook::Get()
{
  std::map<char, levelmap> orderBook(_orderBook);

  return orderBook;
}

void OrderBook::ParseUpdate(const std::string &json)
{
  // Very messy, but linear search is very fast on modern CPUs
  for (size_t i = 0; i < json.size() - 2; ++i)
  {
    if (json[i] == '"' && json[i + 2] == '"')
    {
      if (json[i + 1] == 'b' || json[i + 1] == 'a')
      {
        char bidOrAsk = json[i + 1];
        i += 2;
        do
        {
          i += 4;
          double price = 0;
          while (isdigit(json[++i]))
          {
            price *= 10;
            price += json[i] - 48;
          }
          double pricedec = 0;
          while (isdigit(json[++i]))
          {
            pricedec *= 10;
            pricedec += json[i] - 48;
          }
          price += pricedec / 100000000;

          i += 2;
          double quantity = 0;
          while (isdigit(json[++i]))
          {
            quantity *= 10;
            quantity += json[i] - 48;
          }
          double quantitydec = 0;
          while (isdigit(json[++i]))
          {
            quantitydec *= 10;
            quantitydec += json[i] - 48;
          }
          quantity += quantitydec / 100000000;

          _orderBook[bidOrAsk][price] = quantity;
        } while (json[i + 2] == ',');
      }
    }
  }
}

void OrderBook::PrefetchDepth(std::string symbol)
{
  std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(_io_context,
                                                                _ssl_context);
  auto httpsEndpoints = _resolver.resolve("api.binance.us", "443");
  boost::asio::connect(stream.lowest_layer(), httpsEndpoints);
  stream.handshake(boost::asio::ssl::stream_base::client);
  boost::asio::streambuf reply;
  std::string request =
      "GET /api/v3/depth?symbol=" + symbol + " HTTP/1.1\r\nHost: api.binance.us\r\nConnection: "
                                             "close\r\n\r\n";
  //   char crequest[request.size() + 2];
  //   strcpy(crequest, request.c_str());
  boost::asio::write(stream,
                     boost::asio::buffer(request.c_str(), request.size()));
  boost::asio::read_until(stream, reply, "}");
  std::string json(boost::asio::buffer_cast<const char *>(reply.data()),
                   reply.size());

  // Very messy, but linear search is very fast on modern CPUs
  for (size_t i = 0; i < json.size() - 2; ++i)
  {
    if (json[i] == '"' && json[i + 5] == '"')
    {
      if (json[i + 1] == 'b' || json[i + 1] == 'a')
      {
        char bidOrAsk = json[i + 1];
        i += 5;
        do
        {
          i += 4;
          double price = 0;
          while (isdigit(json[++i]))
          {
            price *= 10;
            price += json[i] - 48;
          }
          double pricedec = 0;
          while (isdigit(json[++i]))
          {
            pricedec *= 10;
            pricedec += json[i] - 48;
          }
          price += pricedec / 100000000;

          i += 2;
          double quantity = 0;
          while (isdigit(json[++i]))
          {
            quantity *= 10;
            quantity += json[i] - 48;
          }
          double quantitydec = 0;
          while (isdigit(json[++i]))
          {
            quantitydec *= 10;
            quantitydec += json[i] - 48;
          }
          quantity += quantitydec / 100000000;

          _orderBook[bidOrAsk][price] = quantity;
        } while (json[i + 2] == ',');
      }
    }
  }
}
