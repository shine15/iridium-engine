/* Copyright 2020 Iridium. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <memory>
#include <iostream>
#include <boost/filesystem.hpp>
#include <iridium/calendar.hpp>
#include "../strategy/include/simulate.hpp"
#include <iridium/account.hpp>
#include <iridium/csv.hpp>

int main() {
  using iridium::SimulationAccount;
  using iridium::instrument_list;
  using iridium::data::data_freq_list;
  using iridium::data::TradeData;
  using iridium::data::StringToDataFreq;

  // settings
  auto hdf5_file_path = boost::filesystem::path(getenv("HOME"));
  hdf5_file_path += "/.iridium/data/history.h5";
  const auto kBeginYear = 2021;
  const auto kBeginMonth = 1;
  const auto kBeginDay = 1;
  const auto kEndYear = 2021;
  const auto kEndMonth = 3;
  const auto kEndDay = 5;
  const auto kRegion = "Australia/Sydney";
  const auto kHistFreq = "M15";
  const auto kTickFreq = "M1";
  const auto kHistDataCount = 90;

  const auto kAccountCurrency = "USD";
  const auto kCapitalBase = 2000.00;
  const auto kLeverage = 50;

  auto csv_file_path = boost::filesystem::path(getenv("HOME"));
  csv_file_path += "/.iridium/report.csv";

  // account
  auto account_ptr = std::make_shared<SimulationAccount>(kAccountCurrency, kLeverage, kCapitalBase);

  // data
  auto instruments = instrument_list({
    "AUD_CAD", "AUD_JPY", "AUD_NZD", "AUD_SGD", "AUD_USD",
    "CAD_JPY", "CAD_SGD",
    "EUR_AUD", "EUR_CAD", "EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_SGD", "EUR_USD",
    "GBP_AUD", "GBP_CAD", "GBP_JPY", "GBP_NZD", "GBP_SGD", "GBP_USD",
    "NZD_CAD", "NZD_JPY", "NZD_SGD", "NZD_USD",
    "SGD_JPY",
    "USD_CAD", "USD_JPY", "USD_SGD"});
//  auto instruments = instrument_list({"EUR_USD"});
  auto freqs = data_freq_list({kHistFreq, kTickFreq});
  auto hdf5data = std::make_unique<TradeData>(hdf5_file_path.string(), *instruments, *freqs);

  // clock
  iridium::calendar::Clock clock(
      kBeginYear,
      kBeginMonth,
      kBeginDay,
      kEndYear,
      kEndMonth,
      kEndDay,
      kRegion,
      StringToDataFreq(kHistFreq));
  for (auto it = clock.begin(); it != clock.end(); ++it) {
    try {
      auto hist_data_map = hdf5data->history_data(
          *instruments, *it, kHistDataCount, StringToDataFreq(kHistFreq));
      for (int j = 0; j < StringToDataFreq(kHistFreq) / StringToDataFreq(kTickFreq); ++j) {
        auto tick = *it + j * StringToDataFreq(kTickFreq);
        auto data_map = hdf5data->candlestick_data(*instruments, tick, StringToDataFreq(kTickFreq));
        for (auto const &[name, data] : *data_map) {
          if (data) {
            SimulateTrade(
                name,
                tick,
                *hist_data_map->at(name),
                *data_map,
                kHistDataCount,
                account_ptr);
          } else {
            continue;
          }
        }
        ProcessTriggerOrders(account_ptr, tick, *data_map);
        account_ptr->PrintAccountInfo(tick, *data_map);
      }
    } catch (const std::out_of_range& e) {
      std::cout << e.what() << std::endl;
      continue;
    }
  }

  std::cout << *account_ptr << std::endl;

  iridium::data::GenerateTransactionsReport(*account_ptr, csv_file_path.string());

  return 0;
}


