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

#include <gtest/gtest.h>
#include <vector>
#include <iridium/calendar.hpp>
#include <iridium/data.hpp>

const auto kFilePath = "/Users/evan/.iridium/data/history.h5";
const auto kInstrumentName = "EUR_USD";
const auto kBeginYear = 2016;
const auto kBeginMonth = 1;
const auto kBeginDay = 1;
const auto kEndYear = 2020;
const auto kEndMonth = 1;
const auto kEndDay = 1;
const auto kRegion = "Australia/Sydney";
const auto kFreqD = "D";
const auto kFreq4H = "H4";

TEST(CalendarTest, TradeStartTime) {
  auto trade_start_times_ptr =
      iridium::calendar::trade_start_times_ptr(
          kBeginYear,
          kBeginMonth,
          kBeginDay,
          kEndYear,
          kEndMonth,
          kEndDay,
          kRegion);
  auto instruments = iridium::instrument_list({kInstrumentName});
  auto freqs = iridium::data::data_freq_list({kFreqD});
  auto hdf5data = std::make_unique<iridium::data::HDF5Data>(
      kFilePath,
      *instruments,
      *freqs);

  auto history_data = hdf5data->history_data_date_range(
      kInstrumentName,
      trade_start_times_ptr->front(),
      trade_start_times_ptr->back(),
      iridium::data::StringToDataFreq(kFreqD));
  auto match = true;
  for (auto it = history_data.get()->begin(); it != history_data.get()->end(); ++it) {
    int index = std::distance(history_data.get()->begin(), it);
    if (it->time != trade_start_times_ptr->at(index)) {
      match = false;
      break;
    }
  }
  EXPECT_EQ(match, true);
}

TEST(CalendarTest, Ticks) {
  auto ticks = iridium::calendar::all_ticks_ptr(
      kBeginYear,
      kBeginMonth,
      kBeginDay,
      kEndYear,
      kEndMonth,
      kEndDay,
      kRegion,
      iridium::data::DataFreq::h4);
  auto instruments = iridium::instrument_list({kInstrumentName});
  auto freqs = iridium::data::data_freq_list({kFreq4H});

  auto hdf5data = std::make_unique<iridium::data::HDF5Data>(
      kFilePath,
      *instruments,
      *freqs);
  auto history_data = hdf5data->history_data_date_range(
      kInstrumentName,
      ticks->front(),
      ticks->back(),
      iridium::data::StringToDataFreq(kFreq4H));
  auto match = true;
  for (auto it = history_data->begin(); it != history_data->end(); ++it) {
    int index = std::distance(history_data->begin(), it);
    if (it->time != ticks->at(index)) {
      match = false;
      break;
    }
  }
  EXPECT_EQ(match, true);
}
