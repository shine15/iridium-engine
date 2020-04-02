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

#include "simulate.hpp"

void SimulateTrade(
    const std::string &instrument_name,
    std::time_t tick,
    const iridium::data::DataList &trade_data,
    const iridium::data::TickDataMap &tick_data_map,
    int hist_data_count,
    const std::shared_ptr<iridium::Account> &account_ptr) {
  // Trade simulation settings
  const auto kSpread = 3.0;
  const auto kMinTradeSize = 100;

  // MA strategy settings
  const auto kSlowPeriod = 26;
  const auto kFastPeriod = 12;
  /*
  const auto KRSIPeriod = 14;
  const auto kMinRSIValue = 50;
  */

  // risk control settings
  const auto kMaxPositionValuePctPerTrade = 0.2;
  const auto kRiskPct = 0.02;
  const auto kStopLossPricePct = 0.005;

  // check data availability
  auto instrument = std::make_shared<iridium::Instrument>(instrument_name);
  auto base = instrument->base_name();
  auto quote = instrument->quote_name();
  auto pip_num = pip_point(*instrument);
  auto account_base_rate = iridium::data::account_currency_rate(
      account_ptr->account_currency(),
      base,
      tick_data_map);
  auto account_quote_rate = iridium::data::account_currency_rate(
      account_ptr->account_currency(),
      quote,
      tick_data_map);
  auto current_price = trade_data.back().close;

  if (account_base_rate.has_value() && account_quote_rate.has_value()) {
    auto account_base_rate_value = account_base_rate.value();
    auto account_quote_rate_value = account_quote_rate.value();

    auto open_trades_ptr = account_ptr->open_trades_ptr(instrument_name);

    auto closes = candlestick_closes(trade_data);

    /*
    auto rsi_out = std::make_shared<std::vector<TA_Real>>(hist_data_count);
    TA_Integer rsi_out_beg;
    TA_Integer rsi_out_num;
    TA_RSI(
        0,
        hist_data_count - 1,
        closes->data(),
        KRSIPeriod,
        &rsi_out_beg,
        &rsi_out_num,
        rsi_out->data());
    */
    
    auto slow_out = std::make_shared<std::vector<TA_Real>>(hist_data_count);
    TA_Integer slow_out_beg;
    TA_Integer slow_out_num;
    TA_MA(
        0,
        hist_data_count - 1,
        closes->data(),
        kSlowPeriod,
        TA_MAType_EMA,
        &slow_out_beg,
        &slow_out_num,
        slow_out->data());

    auto fast_out = std::make_shared<std::vector<TA_Real>>(hist_data_count);
    TA_Integer fast_out_beg;
    TA_Integer fast_out_num;
    TA_MA(
        0,
        hist_data_count - 1,
        closes->data(),
        kFastPeriod,
        TA_MAType_EMA,
        &fast_out_beg,
        &fast_out_num,
        fast_out->data());

    std::optional<bool> is_short_take_position;
    auto is_close_position = false;

    if (slow_out->at(slow_out_num - 2) > fast_out->at(fast_out_num - 2) &&
        slow_out->at(slow_out_num - 1) < fast_out->at(fast_out_num - 1)) {
      std::cout << instrument_name << " - up cross time: "
                << TimeToLocalTimeString(tick) << std::endl;
      if (open_trades_ptr->empty()) {
        is_short_take_position = false;
      } else {
        if (account_ptr->position_size(instrument_name) < 0) {
          is_close_position = true;
        }
      }
    }

    if (slow_out->at(slow_out_num - 2) < fast_out->at(fast_out_num - 2) &&
        slow_out->at(slow_out_num - 1) > fast_out->at(fast_out_num - 1)) {
      std::cout << instrument_name << " - down cross time: "
                << TimeToLocalTimeString(tick) << std::endl;
      if (open_trades_ptr->empty()) {
        is_short_take_position = true;
      } else {
        if (account_ptr->position_size(instrument_name) > 0) {
          is_close_position = true;
        }
      }
    }

    if (is_short_take_position.has_value()) {
      TakePosition(
          tick,
          instrument_name,
          account_ptr,
          tick_data_map,
          kMaxPositionValuePctPerTrade,
          kRiskPct,
          kStopLossPricePct,
          current_price,
          account_quote_rate_value,
          account_base_rate_value,
          pip_num,
          is_short_take_position.value(),
          kMinTradeSize,
          kSpread);
    }

    if (is_close_position) {
      CloserPosition(
          instrument_name,
          account_ptr,
          account_quote_rate_value,
          current_price,
          tick);
    }
  }
}

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
    double spread) {
  auto equity = account_net_asset_value(*account_ptr, tick_data_map);
  auto margin_used = account_margin_used(*account_ptr, tick_data_map);
  if (equity.has_value() && margin_used.has_value()) {
    auto margin_available = iridium::CalculateMarginAvailable(equity.value(), margin_used.value());
    auto[size, stop_loss_price] = CalculateStopLossPositionSize(
        equity.value(),
        max_position_value_pct_per_trade,
        margin_available,
        account_ptr->leverage(),
        risk_pct,
        stop_loss_price_pct,
        current_price,
        account_quote_rate,
        pip_num,
        is_short,
        min_size,
        spread);
    if (0 != size) {
      auto distance = abs(stop_loss_price - current_price);
      account_ptr->CreateMarketOrder(
          time,
          margin_available,
          instrument,
          size,
          current_price,
          account_quote_rate,
          account_base_rate,
          spread,
          std::nullopt,
          std::nullopt,
          distance);
    }
  }
}

void CloserPosition(const std::string &instrument,
                    const std::shared_ptr<iridium::Account> &account_ptr,
                    double rate,
                    double current_price,
                    std::time_t time) {
  account_ptr->CloserPosition(instrument, rate, current_price, time);
}

void PrintAccountInfo(
    const iridium::Account &account,
    std::time_t tick,
    const iridium::data::TickDataMap &data_map) {
  auto net_asset_value = iridium::account_net_asset_value(account, data_map);
  auto margin_used = iridium::account_margin_used(account, data_map);
  if (net_asset_value.has_value() && margin_used.has_value()) {
    auto margin_available = iridium::CalculateMarginAvailable(net_asset_value.value(), margin_used.value());
    std::cout << "time: " << TimeToLocalTimeString(tick)
              << " NAV: " << net_asset_value.value()
              << " Balance: " << account.balance()
              << " Margin Used: " << margin_used.value()
              << " Margin Available: " << margin_available
              << std::endl;
  }
}
