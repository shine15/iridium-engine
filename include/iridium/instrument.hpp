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

#ifndef INCLUDE_IRIDIUM_INSTRUMENT_HPP_
#define INCLUDE_IRIDIUM_INSTRUMENT_HPP_

#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

namespace iridium {
class Instrument {
 public:
  explicit Instrument(const std::string &);

  [[nodiscard]]
  const std::string &base_name() const noexcept;

  [[nodiscard]]
  const std::string &quote_name() const noexcept;

  [[nodiscard]]
  const std::string &name() const noexcept;

 private:
  std::string name_;
  std::string base_;
  std::string quote_;
};

int pip_point(const Instrument &instrument) noexcept;

using InstrumentList = std::vector<std::shared_ptr<Instrument>>;

std::shared_ptr<InstrumentList>
instrument_list(const std::initializer_list<std::string> &names);

std::string InstrumentListToString(const InstrumentList &list);

}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_INSTRUMENT_HPP_
