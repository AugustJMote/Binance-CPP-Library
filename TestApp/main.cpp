#include "Connection.hpp"
#include "OrderBook.hpp"

int main()
{
  OrderBook test;
  test.Init("btcusdt");
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::map<double, levelmap> temp = test.Get();
  for (auto const &[book, levels] : temp)
    for (auto const &[level, quantity] : levels)
    {
      std::cout << "level: " << level << std::endl;
      std::cout << "quantity: " << quantity << std::endl;
    }
  return 0;
}
