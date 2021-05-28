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

#include <math.h>
#include <gtest/gtest.h>
#include <memory>
#include <iridium/forex.hpp>
#include <iridium/instrument.hpp>

TEST(ForexTest, CalculatePipValue) {
    auto i = std::make_unique<iridium::Instrument>("EUR_USD");
    auto quote = i->quote_name();
    // base account AUD, AUD_USD
    auto acc_quote_rate = 0.6323;
    auto units = 10000;
    auto decimals = iridium::pip_point(*(i));
    auto pip_value = round(iridium::CalculatePipValue(units, acc_quote_rate, decimals)
        * pow(10, 5)) / pow(10, 5);
    EXPECT_EQ(pip_value, 1.58153);
}

TEST(ForexTest, CalculateGainLoss) {
    auto i = std::make_unique<iridium::Instrument>("EUR_USD");
    auto quote = i->quote_name();
    // base account AUD, AUD_USD
    auto acc_quote_rate = 0.63168;
    auto units = 100000;
    auto decimals = iridium::pip_point(*(i));
    auto open_price = 1.08197;
    auto close_price = 1.08198;
    auto stop_loss = round(
        iridium::CalculateGainsLosses(
            (close_price - open_price) * pow(10, decimals),
            units,
            acc_quote_rate, decimals)
            * pow(10, 5)) / pow(10, 5);
    EXPECT_EQ(stop_loss, 1.58308);
}

TEST(ForexTest, CalculateMarginUsed) {
    auto units = 500000;
    auto leverage = 100;
    // account USD, pair EUR_USD, base EUR, quote USD
    auto rate = 1.365;
    auto acc_base_rate = 1 / rate;
    auto margin_used = iridium::CalculateMarginUsed(units, acc_base_rate, leverage);
    EXPECT_EQ(margin_used, 6825.00);
}

TEST(forexTest, checkMarginCall) {
    auto margin_used = 10000.00;
    auto margin_call_nav = 4000.00;
    EXPECT_EQ(iridium::CheckMarginCall(margin_call_nav, margin_used), true);
}
