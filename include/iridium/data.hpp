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

#ifndef INCLUDE_IRIDIUM_DATA_HPP_
#define INCLUDE_IRIDIUM_DATA_HPP_

#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include "H5Cpp.h"
#include "instrument.hpp"
#include "algorithm.hpp"
#include "util.hpp"

namespace iridium {
namespace data {
enum DataFreq {
  m1 = 60,
  m2 = 2 * m1,
  m4 = 4 * m1,
  m5 = 5 * m1,
  m10 = 10 * m1,
  m15 = 15 * m1,
  m30 = 30 * m1,
  h1 = 60 * m1,
  h2 = 2 * h1,
  h4 = 4 * h1,
  h6 = 6 * h1,
  h8 = 8 * h1,
  h12 = 12 * h1,
  d = 24 * h1
};

std::string DataFreqToString(const DataFreq &freq);

DataFreq StringToDataFreq(const std::string &freq);

std::shared_ptr<std::vector<DataFreq>>
data_freq_list(const std::initializer_list<std::string> &freqs);

struct Candlestick {
  std::time_t time;
  float open;
  float close;
  float high;
  float low;
  int volume;
};

using DataListMap = std::map<std::string, std::shared_ptr<std::vector<Candlestick>>>;
using TickDataMap = std::map<std::string, std::optional<iridium::data::Candlestick>>;
using DataList = std::vector<Candlestick>;

std::shared_ptr<std::vector<double>> candlestick_closes(const std::vector<Candlestick> &dataList);

class HDF5Data {
 public:
  HDF5Data(
      const std::string &file_name,
      const InstrumentList &instruments,
      const std::vector<DataFreq> &freqs);

  [[nodiscard]]
  std::optional<Candlestick>
  candlestickData(const std::string &instrument_name, std::time_t time, DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<TickDataMap>
  candlestickData(const InstrumentList &instruments, std::time_t time, DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<DataList>
  history_data_date_range(
      const std::string &instrument_name,
      std::time_t begin,
      std::time_t end,
      DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<DataList>
  history_data(
      const std::string &instrument_name,
      std::time_t end,
      int count,
      DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<DataListMap>
  history_data(
      const InstrumentList &instruments,
      std::time_t end,
      int count,
      DataFreq freq) const;

 private:
  std::unique_ptr<H5::H5File> file_;

  std::vector<std::shared_ptr<Instrument>> instruments_;

  std::vector<DataFreq> freqs_;

  std::map<std::string, std::shared_ptr<std::vector<int>>> time_indices_;

  std::map<std::string, std::shared_ptr<H5::DataSet>> datasets_;

  H5::CompType candlestick_type_;

  [[nodiscard]]
  int time_index(
      const std::string &instrument_name,
      std::time_t time,
      DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<H5::DataSet>
  instrument_dataset(const std::string &instrument_name, DataFreq freq) const;

  [[nodiscard]]
  std::string
  dataset_name(const std::string &instrument_name, DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<std::vector<int>>
  time_indices(const std::string &instrument_name, DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<DataList> history_data_(
      const std::string &instrument_name,
      std::time_t begin,
      int count,
      DataFreq freq,
      bool reversed) const;
};

std::optional<float>
account_currency_rate(const std::string &account, const std::string &currency,
                      const TickDataMap &tickDataMap);
}  // namespace data

}  // namespace iridium

std::ostream &operator<<(std::ostream &os, const iridium::data::Candlestick &candlestick);

#endif  // INCLUDE_IRIDIUM_DATA_HPP_
