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
#include <iridium/calendar.hpp>
#include "simulate.hpp"
#include <iridium/account.hpp>

int main() {
  using iridium::Account;
  using iridium::instrument_list;
  using iridium::data::data_freq_list;
  using iridium::data::HDF5Data;
  using iridium::data::StringToDataFreq;

  // settings
  const auto kFilePath = "/Users/evan/.iridium/data/history.h5";
  const auto kInstrument1 = "EUR_USD";
  const auto kInstrument2 = "EUR_JPY";
  const auto kInstrument3 = "USD_JPY";
  const auto kBeginYear = 2019;
  const auto kBeginMonth = 1;
  const auto kBeginDay = 1;
  const auto kEndYear = 2019;
  const auto kEndMonth = 3;
  const auto kEndDay = 1;
  const auto kRegion = "Australia/Sydney";
  const auto kHistFreq = "D";
  const auto kTickFreq = "M1";
  const auto kHistDataCount = 60;

  const auto kAccountCurrency = "USD";
  const auto kCapitalBase = 10000.00;
  const auto kLeverage = 50;

  // account
  auto account_ptr = std::make_shared<Account>(kAccountCurrency, kLeverage, kCapitalBase);

  // data
  auto instruments = instrument_list({kInstrument1, kInstrument2, kInstrument3});
//  auto instruments = instrument_list({kInstrument1});
  auto freqs = data_freq_list({kHistFreq, kTickFreq});
  auto hdf5data = std::make_unique<HDF5Data>(kFilePath, *instruments, *freqs.get());

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
    auto hist_data_map = hdf5data->history_data(
        *instruments, *it, kHistDataCount, StringToDataFreq(kHistFreq));
    for (int j = 0; j < StringToDataFreq(kHistFreq) / StringToDataFreq(kTickFreq); ++j) {
      auto tick = *it + j * StringToDataFreq(kTickFreq);
      auto data_map = hdf5data->candlestickData(*instruments, tick, StringToDataFreq(kTickFreq));
      for (auto const &[name, data] : *data_map) {
        if (data) {
          hist_data_map->at(name)->back() = data.value();
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
      PrintAccountInfo(*account_ptr, tick, *data_map);
    }
  }
  std::cout << *account_ptr << std::endl;

  return 0;
}


