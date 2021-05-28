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

#include <iridium/data.hpp>

// utility methods
std::string
iridium::data::DataFreqToString(const iridium::data::DataFreq &freq) {
  switch (freq) {
    case m1:return "M1";
    case m2:return "M2";
    case m4:return "M4";
    case m5:return "M5";
    case m10:return "M10";
    case m15:return "M15";
    case m30:return "M30";
    case h1:return "H1";
    case h2:return "H2";
    case h4:return "H4";
    case h6:return "H6";
    case h8:return "H8";
    case h12:return "H12";
    case d:return "D";
  }
}

iridium::data::DataFreq
iridium::data::StringToDataFreq(const std::string &freq) {
  if (freq == "M1") return m1;
  if (freq == "M2") return m2;
  if (freq == "M4") return m4;
  if (freq == "M5") return m5;
  if (freq == "M10") return m10;
  if (freq == "M15") return m15;
  if (freq == "M30") return m30;
  if (freq == "H1") return h1;
  if (freq == "H2") return h2;
  if (freq == "H4") return h4;
  if (freq == "H6") return h6;
  if (freq == "H8") return h8;
  if (freq == "H12") return h12;
  if (freq == "D") return d;
  throw std::invalid_argument(
      "No support data frequency only support M1 M2 M4 M5 M10 M15 M30 H1 H2 H4 H6 H8 H12 D");
}

std::shared_ptr<std::vector<iridium::data::DataFreq>>
iridium::data::data_freq_list(
    const std::initializer_list<std::string> &freqs) {
  auto freqs_ptr = std::make_shared<std::vector<iridium::data::DataFreq>>();
  for (const auto &freq : freqs) {
    freqs_ptr->push_back(StringToDataFreq(freq));
  }
  return freqs_ptr;
}

std::shared_ptr<std::vector<double>>
iridium::data::candlestick_closes(const std::vector<Candlestick> &dataList) {
  auto closes = std::make_shared<std::vector<double>>();

  std::for_each(dataList.begin(),
                dataList.end(),
                [&closes](const auto &candle) {
                  closes->push_back(candle.close);
                });
  return closes;
}

std::shared_ptr<std::vector<double>>
iridium::data::candlestick_opens(const std::vector<Candlestick> &dataList) {
  auto opens = std::make_shared<std::vector<double>>();

  std::for_each(dataList.begin(),
                dataList.end(),
                [&opens](const auto &candle) {
                  opens->push_back(candle.open);
                });
  return opens;
}

std::shared_ptr<std::vector<double>>
iridium::data::candlestick_lows(const std::vector<Candlestick> &dataList) {
  auto lows = std::make_shared<std::vector<double>>();

  std::for_each(dataList.begin(),
                dataList.end(),
                [&lows](const auto &candle) {
                  lows->push_back(candle.low);
                });
  return lows;
}

std::shared_ptr<std::vector<double>>
iridium::data::candlestick_highs(const std::vector<Candlestick> &dataList) {
  auto highs = std::make_shared<std::vector<double>>();

  std::for_each(dataList.begin(),
                dataList.end(),
                [&highs](const auto &candle) {
                  highs->push_back(candle.high);
                });
  return highs;
}


std::optional<double>
iridium::data::account_currency_rate(
    const std::string &account,
    const std::string &currency,
    const iridium::data::TickDataMap &tickDataMap) {
  if (account == currency) {
    return 1.0;
  } else {
    auto instruments = std::make_shared<std::vector<std::string>>();
    for (const auto &[name, _] : tickDataMap) {
      instruments->push_back(name);
    }
    auto pair = account + "_" + currency;
    auto reversedPair = currency + "_" + account;
    std::optional<bool> reversed = std::nullopt;
    for (const auto &instrument : *instruments) {
      if (instrument == pair) {
        reversed = false;
        break;
      } else if (instrument == reversedPair) {
        reversed = true;
        break;
      }
    }
    if (reversed) {
      if (reversed.value()) {
        auto data = tickDataMap.at(reversedPair);
        if (data.has_value()) {
          return 1.0 / data->close;
        } else {
          return std::nullopt;
        }
      } else {
        auto data = tickDataMap.at(pair);
        if (data.has_value()) {
          return data->close;
        } else {
          return std::nullopt;
        }
      }
    } else {
      return std::nullopt;
    }
  }
}

std::ostream &operator<<(std::ostream &os, const iridium::data::Candlestick &candlestick) {
  os << "time: " << TimeToLocalTimeString(candlestick.time)
     << " open: " << candlestick.open
     << " close: " << candlestick.close
     << " high: " << candlestick.high
     << " low: " << candlestick.low
     << " volume: " << candlestick.volume;
  return os;
}

// TradeData Pimpl
class iridium::data::TradeData::DataImpl {
 public:
  DataImpl(
      const std::string &file_name,
      const InstrumentList &instruments,
      const std::vector<DataFreq> &freqs);

  [[nodiscard]]
  int time_index(
      const std::string &instrument_name,
      std::time_t time,
      DataFreq freq) const;

  [[nodiscard]]
  std::shared_ptr<H5::DataSet>
  instrument_dataset(const std::string &instrument_name, DataFreq freq) const;

  [[nodiscard]]
  static std::string
  dataset_name(const std::string &instrument_name, DataFreq freq);

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

 private:
  std::unique_ptr<H5::H5File> file_;

  std::vector<std::shared_ptr<Instrument>> instruments_;

  std::vector<DataFreq> freqs_;

  std::map<std::string, std::shared_ptr<std::vector<int>>> time_indices_;

  std::map<std::string, std::shared_ptr<H5::DataSet>> datasets_;

  H5::CompType candlestick_type_;
};

iridium::data::TradeData::DataImpl::DataImpl(
    const std::string &file_name,
    const iridium::InstrumentList &instruments,
    const std::vector<DataFreq> &freqs) :
    instruments_(instruments),
    freqs_(freqs) {
  using H5::H5File;
  using H5::CompType;
  using H5::PredType;
  using H5::DataSet;
  using H5::FileIException;
  using H5::DataSetIException;

  try {
    file_ = std::make_unique<H5File>(file_name, H5F_ACC_RDONLY);
    // Dataset & time indices in memory
    CompType time_index_type(sizeof(int));
    time_index_type.insertMember("time", 0, PredType::NATIVE_INT);
    for (auto &instrument : instruments) {
      for (auto &freq : freqs) {
        auto name = instrument->name() + "_" + DataFreqToString(freq);
        auto dataset_name = "/instruments/" + name;
        auto dataset = std::make_shared<DataSet>(file_->openDataSet(dataset_name));
        datasets_[name] = dataset;
        auto size = dataset->getSpace().getSimpleExtentNpoints();
        auto times = std::make_shared<std::vector<int>>(size);
        dataset->read(times->data(), time_index_type);
        time_indices_[name] = times;
      }
    }
    // CompType candlestick history data
    candlestick_type_ = CompType(sizeof(Candlestick));
    candlestick_type_.insertMember("time", HOFFSET(Candlestick, time), PredType::NATIVE_INT);
    candlestick_type_.insertMember("open", HOFFSET(Candlestick, open), PredType::NATIVE_DOUBLE);
    candlestick_type_.insertMember("close", HOFFSET(Candlestick, close), PredType::NATIVE_DOUBLE);
    candlestick_type_.insertMember("high", HOFFSET(Candlestick, high), PredType::NATIVE_DOUBLE);
    candlestick_type_.insertMember("low", HOFFSET(Candlestick, low), PredType::NATIVE_DOUBLE);
    candlestick_type_.insertMember("volume", HOFFSET(Candlestick, volume), PredType::NATIVE_INT);
  } catch (FileIException const &err) {
    throw err;
  } catch (DataSetIException const &err) {
    throw err;
  }
}

std::string
iridium::data::TradeData::DataImpl::dataset_name(
    const std::string &instrument_name,
    iridium::data::DataFreq freq) {
  return instrument_name + "_" + DataFreqToString(freq);
}

std::shared_ptr<H5::DataSet>
iridium::data::TradeData::DataImpl::instrument_dataset(
    const std::string &instrument_name,
    iridium::data::DataFreq freq) const {
  return datasets_.at(dataset_name(instrument_name, freq));
}

std::shared_ptr<std::vector<int>>
iridium::data::TradeData::DataImpl::time_indices(
    const std::string &instrument_name,
    iridium::data::DataFreq freq) const {
  return time_indices_.at(dataset_name(instrument_name, freq));
}

int iridium::data::TradeData::DataImpl::time_index(
    const std::string &instrument_name,
    std::time_t time,
    iridium::data::DataFreq freq) const {
  using iridium::algorithm::BinarySearch;
  auto index = BinarySearch(
      *(time_indices(instrument_name, freq).get()),
      static_cast<int>(time),
      true);
  if (index == -1) {
    throw std::out_of_range("Candlestick Data Not Found");
  } else {
    return index;
  }
}

std::shared_ptr<iridium::data::DataList>
iridium::data::TradeData::DataImpl::history_data_(
    const std::string &instrument_name,
    std::time_t begin,
    int count,
    iridium::data::DataFreq freq,
    bool reversed = false) const {
  using H5::DataSpace;
  auto begin_index = time_index(instrument_name, begin, freq);
  hsize_t data_start[] =
      {static_cast<hsize_t>(reversed ? begin_index : begin_index - count + 1)};
  hsize_t data_count[] = {static_cast<hsize_t>(count)};
  hsize_t data_stride[] = {1};
  hsize_t data_block[] = {1};
  auto dataset = instrument_dataset(instrument_name, freq);
  auto fspace = dataset->getSpace();
  fspace.selectHyperslab(
      H5S_SELECT_SET,
      data_count,
      data_start,
      data_stride,
      data_block);
  auto candles = std::make_shared<std::vector<data::Candlestick>>(count);
  hsize_t m_dim[1] = {static_cast<hsize_t>(count)};
  DataSpace mspace(1, m_dim);
  dataset->read(
      candles->data(),
      candlestick_type_,
      mspace,
      fspace);
  std::sort(std::begin(*candles),
            std::end(*candles),
            [](auto c1, auto c2) {
              return c1.time < c2.time;
            });
  return candles;
}

// TradeData public methods
iridium::data::TradeData::TradeData(
    const std::string &file_name,
    const InstrumentList &instruments,
    const std::vector<DataFreq> &freqs) :
    pimpl_(std::move(std::make_unique<DataImpl>(file_name, instruments, freqs))) {}

iridium::data::TradeData::~TradeData() = default;

std::optional<iridium::data::Candlestick>
iridium::data::TradeData::candlestick_data(
    const std::string &instrument_name,
    std::time_t time,
    iridium::data::DataFreq freq) const {
  try {
    return history_data(instrument_name, time, 1, freq)->front();
  } catch (...) {
    return std::nullopt;
  }
}

std::shared_ptr<iridium::data::TickDataMap>
iridium::data::TradeData::candlestick_data(
    const iridium::InstrumentList &instruments,
    std::time_t time,
    iridium::data::DataFreq freq) const {
  auto data_map =
      std::make_shared<std::map<std::string, std::optional<iridium::data::Candlestick>>>();
  for (const auto &instrument : instruments) {
    data_map->insert(
        {instrument->name(),
         candlestick_data(instrument->name(), time, freq)});
  }
  return data_map;
}

std::shared_ptr<iridium::data::DataList>
iridium::data::TradeData::history_data_date_range(
    const std::string &instrument_name,
    std::time_t begin,
    std::time_t end,
    iridium::data::DataFreq freq) const {
  auto begin_index = pimpl_->time_index(instrument_name, begin, freq);
  auto end_index = pimpl_->time_index(instrument_name, end, freq);
  return pimpl_->history_data_(
      instrument_name,
      begin,
      begin_index - end_index + 1,
      freq);
}

std::shared_ptr<iridium::data::DataList>
iridium::data::TradeData::history_data(
    const std::string &instrument_name,
    std::time_t end,
    int count,
    iridium::data::DataFreq freq) const {
  return pimpl_->history_data_(
      instrument_name,
      end,
      count,
      freq,
      true);
}

std::shared_ptr<iridium::data::DataListMap>
iridium::data::TradeData::history_data(
    const InstrumentList &instruments,
    std::time_t end,
    int count,
    iridium::data::DataFreq freq) const {
  auto hist_data_map = std::make_shared<DataListMap>();
  for (const auto &instrument : instruments) {
    auto hist_data = history_data(instrument->name(), end, count + 1, freq);
    hist_data->pop_back();
    hist_data_map->insert({instrument->name(), hist_data});
  }
  return hist_data_map;
}
