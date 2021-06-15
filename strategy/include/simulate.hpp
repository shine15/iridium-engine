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

#ifndef APPS_SIMULATE_HPP_
#define APPS_SIMULATE_HPP_

#include "ta_func.h"
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <iridium/data.hpp>
#include <iridium/instrument.hpp>
#include <iridium/account.hpp>
#include <iridium/util.hpp>
#include <iridium/forex.hpp>
#include <iridium/algorithm.hpp>
#include <iridium/logging.hpp>
#include <algorithm>
#include "position.hpp"
#include "indicator.hpp"

void SimulateTrade(
    const std::string &instrument_name,
    std::time_t tick,
    const iridium::data::DataList &long_term_hist_data,
    const iridium::data::DataList &intermediate_term_hist_data,
    const iridium::data::DataList &short_term_hist_data,
    const iridium::data::TickDataMap &tick_data_map,
    const std::shared_ptr<iridium::Account> &account_ptr,
    double spread);

#endif  // APPS_SIMULATE_HPP_
