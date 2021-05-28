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

#include <iridium/forex.hpp>

double iridium::CalculatePipValue(int units, double acc_quote_rate, int decimals) {
    return units * pow(10, -decimals) * (1 / acc_quote_rate);
}

double iridium::CalculateGainsLosses(
    double change,
    int units,
    double acc_quote_rate,
    int decimals) {
    return change * CalculatePipValue(units, acc_quote_rate, decimals);
}

double iridium::CalculateMarginUsed(int units, double acc_base_rate, int leverage) {
    return abs(units) * (1 / acc_base_rate) * (1 / static_cast<double >(leverage));
}

double iridium::CalculateMarginAvailable(double nav, double margin_used) {
    auto available = nav - margin_used;
    return available > 0 ? available : 0.00;
}

bool iridium::CheckMarginCall(double nav, double margin_used) {
    return nav <= margin_used / 2;
}

int iridium::CalculatePositionSize(
    double equity,
    double risk_pct,
    int stop_loss_pips,
    double acc_quote_rate,
    int pip_num) {
    auto loss = equity * risk_pct;
    auto quote_currency_loss = loss * acc_quote_rate;
    auto pip_value = quote_currency_loss / stop_loss_pips;
    return static_cast<int>(floor(pip_value * pow(10, pip_num)));
}

double iridium::CalculatePositionValue(int size, double current_price, double acc_quote_rate) {
    return abs(size) * current_price * (1.0 / acc_quote_rate);
}

