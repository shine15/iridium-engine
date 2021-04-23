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

#include <gtest/gtest.h>
#include <memory>
#include <iridium/instrument.hpp>

TEST(InstrumentTest, Success1) {
    auto i = std::make_unique<iridium::Instrument>("EUR_USD");
    EXPECT_EQ(i->name(), "EUR_USD");
    EXPECT_EQ(i->base_name(), "EUR");
    EXPECT_EQ(i->quote_name(), "USD");
    EXPECT_EQ(iridium::pip_point(*(i)), 4);
}

TEST(InstrumentTest, Success2) {
    auto i = std::make_unique<iridium::Instrument>("USD_JPY");
    EXPECT_EQ(iridium::pip_point(*(i)), 2);
}

TEST(InstrumentTest, InvalidArgument) {
    try {
        auto i = std::make_unique<iridium::Instrument>("123456");
        FAIL() << "Expected std::invalid_argument";
    } catch (std::invalid_argument const &err) {
        EXPECT_EQ(
            err.what(),
            std::string("Instrument name format should be like Base_Quote. e.g., EUR_USD"));
    } catch (...) {
        FAIL() << "Expected std::invalid_argument";
    }
}
