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

#include "../include/position.hpp"

using iridium::CalculatePositionSize;
using iridium::CalculatePositionValue;

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
    int min_size) {
  auto size = CalculatePositionSize(equity, risk_pct, stop_loss_pips, rate, pip_num);
  auto trade_value = CalculatePositionValue(size, current_price, rate);
  if (margin_available == 0.0 || size < min_size) {
    size = 0;
  }
  if (trade_value >= margin_available * leverage) {
    size = static_cast<int>(margin_available * leverage * rate / current_price);
  }
  if (trade_value >= equity * max_position_value_pct_per_trade * leverage) {
    size = static_cast<int>(
        equity * max_position_value_pct_per_trade * leverage * rate / current_price);
  }
  return size * (is_short ? -1 : 1);
}

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
    double spread) {
  auto stop_loss_pips = std::round(abs(current_price - stop_loss_price)  * pow(10, pip_num)) + spread;
  auto calculated_stop_loss_price = current_price - (is_short ? -1 : 1) * (stop_loss_pips - spread) * pow(10, -pip_num);
  auto size = CalculateStopLossPositionSize(
      equity,
      max_position_value_pct_per_trade,
      margin_available,
      leverage,
      risk_pct,
      stop_loss_pips,
      current_price,
      rate,
      pip_num,
      is_short,
      min_size);
  return  std::make_tuple(size, calculated_stop_loss_price);
}
