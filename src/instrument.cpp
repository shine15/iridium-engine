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

#include <iridium/instrument.hpp>

iridium::Instrument::Instrument(const std::string &name) : name_(name) {
  std::regex regex("[a-zA-Z]+_[a-zA-Z]+");
  if (regex_match(name, regex)) {
    std::transform(this->name_.begin(), this->name_.end(), this->name_.begin(), ::toupper);
    std::vector<std::string> results;
    boost::split(results, this->name_, [](char c) { return c == '_'; });
    base_ = results.at(0);
    quote_ = results.at(1);
  } else {
    throw std::invalid_argument("Instrument name format should be like Base_Quote. e.g., EUR_USD");
  }
}

const std::string &iridium::Instrument::name() const noexcept {
  return name_;
}

const std::string &iridium::Instrument::base_name() const noexcept {
  return base_;
}

const std::string &iridium::Instrument::quote_name() const noexcept {
  return quote_;
}

int iridium::pip_point(const iridium::Instrument &instrument) noexcept {
  return instrument.quote_name() == "JPY" ? 2 : 4;
}

std::shared_ptr<iridium::InstrumentList>
iridium::instrument_list(const std::initializer_list<std::string> &names) {
  auto instruments_ptr = std::make_shared<InstrumentList>();
  for (const auto &name : names) {
    auto i = std::make_shared<Instrument>(name);
    instruments_ptr->push_back(i);
  }
  return instruments_ptr;
}
