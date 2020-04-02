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

#include <ta-lib/ta_func.h>
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <iridium/data.hpp>
#include <iridium/instrument.hpp>
#include <iridium/account.hpp>
#include <iridium/util.hpp>
#include <iridium/forex.hpp>
#include "position.hpp"

void SimulateTrade(
    const std::string &instrument_name,
    std::time_t tick,
    const iridium::data::DataList &trade_data,
    const iridium::data::TickDataMap &tick_data_map,
    int hist_data_count,
    const std::shared_ptr<iridium::Account> &account_ptr);

void TakePosition(
    const std::time_t time,
    const std::string &instrument,
    const std::shared_ptr<iridium::Account> &account_ptr,
    const iridium::data::TickDataMap &tick_data_map,
    double max_position_value_pct_per_trade,
    double risk_pct,
    double stop_loss_price_pct,
    double current_price,
    double account_quote_rate,
    double account_base_rate,
    int pip_num,
    bool is_short,
    int min_size,
    double spread);

void CloserPosition(
    const std::string &instrument,
    const std::shared_ptr<iridium::Account> &account_ptr,
    double rate,
    double current_price,
    std::time_t time);

void PrintAccountInfo(
    const iridium::Account &account,
    std::time_t tick,
    const iridium::data::TickDataMap &data_map);

#endif  // APPS_SIMULATE_HPP_
