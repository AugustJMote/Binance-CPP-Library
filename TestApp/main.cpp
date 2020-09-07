#include "Connection.hpp"
#include "OrderBook.hpp"

int main()
{
  //________________________________________
  // Creating an orderbook for btc to usdt
  //________________________________________
  OrderBook btcusdt;
  btcusdt.Init("btcusdt");
  std::this_thread::sleep_for(
      std::chrono::seconds(5)); // Waiting for some updates to happen
  std::map<char, levelmap> temp = btcusdt.Get();
  std::cout << "Bids:" << std::endl;
  for (auto const &[level, quantity] : temp['b'])
  {
    if (!quantity)
      continue;
    std::cout << "level: " << std::setw(10) << level
              << " quantity: " << std::setw(10) << quantity << std::endl;
  }
  std::cout << "Asks:" << std::endl;
  for (auto const &[level, quantity] : temp['a'])
  {
    if (!quantity)
      continue;
    std::cout << "level: " << std::setw(10) << level
              << " quantity: " << std::setw(10) << quantity << std::endl;
  }
  //________________________________________
  // Connecting to the HTTPS API
  //________________________________________
  Connection test("Your key", "Your Secret"); // Key and Secret not needed for this example

  return 0;
}
