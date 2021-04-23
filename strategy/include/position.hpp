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

#ifndef APPS_POSITION_HPP_
#define APPS_POSITION_HPP_

#include <math.h>
#include <tuple>
#include <cmath>
#include <cfenv>
#include <climits>
#include <iridium/forex.hpp>

/*
 * @param equity
 * @param max_position_value_pct_per_trade
 * @param margin_available
 * @param leverage
 * @param risk_pct
 * @param stop_loss_pips
 * @param current_price
 * @param rate: Account currency vs quote currency rate
 * @param spread
 * @param pip_num
 * @param is_short
 * @param min_size
 */
int CalculateStopLossPositionSize(
    double equity,
    double max_position_value_pct_per_trade,
    double margin_available,
    int leverage,
    double risk_pct,
    double stop_loss_pips,
    double current_price,
    double rate,
    int pip_num,
    bool is_short,
    int min_size);

/*
 * @param equity
 * @param max_position_value_pct_per_trade
 * @param margin_available
 * @param leverage
 * @param risk_pct
 * @param stop_loss_price
 * @param current_price
 * @param rate: Account currency vs quote currency rate
 * @param spread
 * @param pip_num
 * @param is_short
 * @param min_size
 * @param spread
 */
std::tuple<int, double>
CalculateStopLossPositionSize(
    double equity,
    double max_position_value_pct_per_trade,
    double margin_available,
    int leverage,
    double risk_pct,
    double stop_loss_price,
    double current_price,
    double rate,
    int pip_num,
    bool is_short,
    int min_size,
    double spread);

#endif  // APPS_POSITION_HPP_
