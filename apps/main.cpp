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
  const auto kBeginMonth = 5;
  const auto kBeginDay = 1;
  const auto kEndYear = 2021;
  const auto kEndMonth = 6;
  const auto kEndDay = 7;
  const auto kRegion = "Australia/Sydney";

  // triple screen trading system settings
  const auto kShortTermTimeFrame = "M15";
  const auto kIntermediateTermTimeFrame = "H1";
  const auto kLongTermTimeFrame = "H4";
  const auto kSimulateTickTimeFrame = "M1";
  const auto kHistDataCount = 90;

  const auto kAccountCurrency = "USD";
  const auto kCapitalBase = 2000.00;
  const auto kLeverage = 50;
  const auto kSpread = 3.0;

  auto csv_file_path = boost::filesystem::path(getenv("HOME"));
  csv_file_path += "/.iridium/report.csv";

  // account
  auto account_ptr = std::make_shared<SimulationAccount>(kAccountCurrency, kLeverage, kCapitalBase, kSpread);

  // data
//  auto instruments = instrument_list({
//    "AUD_CAD", "AUD_JPY", "AUD_NZD", "AUD_SGD", "AUD_USD",
//    "CAD_JPY", "CAD_SGD",
//    "EUR_AUD", "EUR_CAD", "EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_SGD", "EUR_USD",
//    "GBP_AUD", "GBP_CAD", "GBP_JPY", "GBP_NZD", "GBP_SGD", "GBP_USD",
//    "NZD_CAD", "NZD_JPY", "NZD_SGD", "NZD_USD",
//    "SGD_JPY",
//    "USD_CAD", "USD_JPY", "USD_SGD"});
  auto instruments = instrument_list({"EUR_USD"});
  auto freqs = data_freq_list({kShortTermTimeFrame, kIntermediateTermTimeFrame, kLongTermTimeFrame, kSimulateTickTimeFrame});
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
      StringToDataFreq(kLongTermTimeFrame));
  for (auto it = clock.begin(); it != clock.end(); ++it) {
    try {
      // long term history data
      auto long_hist_data_map = hdf5data->history_data(
          *instruments, *it, kHistDataCount, StringToDataFreq(kLongTermTimeFrame));
      for (int i = 0; i < StringToDataFreq(kLongTermTimeFrame) / StringToDataFreq(kIntermediateTermTimeFrame); ++i) {
        // intermediate term history data
        auto intermediate_tick = *it + i * StringToDataFreq(kIntermediateTermTimeFrame);
        auto intermediate_hist_data_map = hdf5data->history_data(
            *instruments, intermediate_tick, kHistDataCount, StringToDataFreq(kIntermediateTermTimeFrame));
        for (int j = 0; j < StringToDataFreq(kIntermediateTermTimeFrame) / StringToDataFreq(kShortTermTimeFrame); ++j) {
          // short term history data
          auto short_tick = intermediate_tick + j * StringToDataFreq(kShortTermTimeFrame);
          auto short_hist_data_map = hdf5data->history_data(
              *instruments, short_tick, kHistDataCount, StringToDataFreq(kShortTermTimeFrame));
          for (int k = 0; k < StringToDataFreq(kShortTermTimeFrame) / StringToDataFreq(kSimulateTickTimeFrame); ++k) {
            // simulate term data
            auto simulate_tick = short_tick + k * StringToDataFreq(kSimulateTickTimeFrame);
            auto simulate_data_map = hdf5data->candlestick_data(*instruments, simulate_tick, StringToDataFreq(kSimulateTickTimeFrame));
            for (auto const &[name, data] : *simulate_data_map) {
              if (data) {
                // instrument history data
                auto long_term_hist_data_ptr = long_hist_data_map->at(name);
                auto intermediate_term_hist_data_ptr = intermediate_hist_data_map->at(name);
                auto short_term_hist_data_ptr = short_hist_data_map->at(name);
                SimulateTrade(
                    name,
                    simulate_tick,
                    *long_term_hist_data_ptr,
                    *intermediate_term_hist_data_ptr,
                    *short_term_hist_data_ptr,
                    *simulate_data_map,
                    account_ptr,
                    kSpread);
              } else {
                continue;
              }
            }
            account_ptr->ProcessOrders(simulate_tick, *simulate_data_map);
            iridium::logger()->info(account_ptr->summary(simulate_tick, *simulate_data_map));
          }
        }
      }
    } catch (const std::out_of_range& e) {
      iridium::logger()->error(e.what());
      continue;
    }
  }

  iridium::logger()->info(account_ptr->string());

  iridium::data::GenerateTransactionsReport(*account_ptr, csv_file_path.string());

  return 0;
}

