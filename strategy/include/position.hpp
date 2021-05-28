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
 * @param margin_used
 * @param leverage
 * @param risk_pct
 * @param stop_loss_pips
 * @param order_price
 * @param acc_quote_rate: Account currency vs quote currency rate
 * @param spread
 * @param pip_num
 * @param is_short
 * @param min_size
 */
int CalculateStopLossPositionSize(
    double equity,
    double margin_used,
    int leverage,
    double risk_pct,
    double stop_loss_pips,
    double order_price,
    double acc_quote_rate,
    int pip_num,
    bool is_short,
    int min_size);

/*
 * @param equity
 * @param margin_available
 * @param leverage
 * @param risk_pct
 * @param stop_loss_price
 * @param order_price
 * @param acc_quote_rate: Account currency vs quote currency rate
 * @param spread
 * @param pip_num
 * @param is_short
 * @param min_size
 */
int CalculatePositionSize(
    double equity,
    double margin_used,
    int leverage,
    double risk_pct,
    double stop_loss_price,
    double order_price,
    double acc_quote_rate,
    int pip_num,
    bool is_short,
    int min_size);

#endif  // APPS_POSITION_HPP_
