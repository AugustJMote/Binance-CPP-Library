<<<<<<< HEAD
# *Work In Progress*

# Binance CPP Library

A set of libraries for trading on Binance through a C++ application.

* [Connection](https://github.com/Beavergus/Binance-Trading-Platform/tree/master/Connection) provides an interface for utililizing the HTTPS API.
* [OrderBook](https://github.com/Beavergus/Binance-Trading-Platform/tree/master/OrderBook) provides a live updating orderbook which runs in the background.

### Prerequisites

See [Cmake](https://github.com/Beavergus/Binance-Trading-Platform/blob/master/CMakeLists.txt) for all dependancies.

## Getting Started

Simply include library headers where needed and make sure to satisfy all dependancies.
See [the test app](https://github.com/Beavergus/Binance-Trading-Platform/blob/master/TestApp/main.cpp) for examples.

## Built With

* [Boost](https://www.boost.org/) - Boost.Asio, Boost.Beast, and others

## Authors

* **August Mote** - [LinkedIn](https://www.linkedin.com/in/augustjmote/)

## License

This project is licensed under the MIT License
=======
# Binance CPP Library

A set of libraries for trading on Binance.us through a C++ application.

* [Connection](https://github.com/Beavergus/Binance-Trading-Platform/tree/master/Connection) provides an interface for utililizing the HTTPS API.
* [OrderBook](https://github.com/Beavergus/Binance-Trading-Platform/tree/master/OrderBook) provides a live updating orderbook which runs a WebSocket feed in the background.

### Prerequisites

Posix OS - relies on PThread.

See [Cmake](https://github.com/Beavergus/Binance-Trading-Platform/blob/master/CMakeLists.txt) for all dependancies.

### To Do
* Incorporate logging instead of printing
* Provide convient ways to export data
* Provide a general WebSocket stream library instead of or in addition to the orderbook
* Support more endpoints
* Better distinguish between routines and functions including using constexpr
* encorporate enums or structs for common requests and replies
* Facilities for monitoring API limits

## Getting Started

Simply include library headers where needed and make sure to satisfy all dependancies.
See [the test app](https://github.com/Beavergus/Binance-Trading-Platform/blob/master/TestApp/main.cpp) for examples.

## Built With

* [Boost](https://www.boost.org/) - Boost.Asio, Boost.Beast, and others

## Credits

* [Binance.us API Documentation](https://github.com/binance-us/binance-official-api-docs)

## License

This project is licensed under the MIT License
>>>>>>> 739abc064b22c46a5ffb39022c2f764f1b59d2dc
