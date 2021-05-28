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
    double margin_used,
    int leverage,
    double risk_pct,
    double stop_loss_pips,
    double order_price,
    double acc_quote_rate,
    int pip_num,
    bool is_short,
    int min_size) {
  auto size = CalculatePositionSize(equity, risk_pct, stop_loss_pips, acc_quote_rate, pip_num);
  auto trade_value = CalculatePositionValue(size, order_price, acc_quote_rate);
  auto margin_available = equity - margin_used;
  if (margin_available == 0.0) {
    size = 0;
  }
  if (trade_value >= margin_available * leverage) {
    size = static_cast<int>(margin_available * leverage * acc_quote_rate / order_price);
  }
  if (size < min_size) {
    size = 0;
  }
  return size * (is_short ? -1 : 1);
}

int CalculatePositionSize(
    double equity,
    double margin_available,
    int leverage,
    double risk_pct,
    double stop_loss_price,
    double order_price,
    double acc_quote_rate,
    int pip_num,
    bool is_short,
    int min_size) {
  auto stop_loss_pips = std::round(abs(order_price - stop_loss_price)  * pow(10, pip_num));
  auto size = CalculateStopLossPositionSize(
      equity,
      margin_available,
      leverage,
      risk_pct,
      stop_loss_pips,
      order_price,
      acc_quote_rate,
      pip_num,
      is_short,
      min_size);
  return size;
}
