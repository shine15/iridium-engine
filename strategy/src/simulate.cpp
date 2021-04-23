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

#include "../include/simulate.hpp"

void SimulateTrade(
    const std::string &instrument_name,
    std::time_t tick,
    const iridium::data::DataList &hist_data,
    const iridium::data::TickDataMap &tick_data_map,
    int hist_data_count,
    const std::shared_ptr<iridium::Account> &account_ptr) {
  // Trade simulation settings
  const auto kSpread = 3.0;
  const auto kMinTradeSize = 100;

  // MA strategy settings
  const auto kFastPeriod = 12;
  const auto kSlowPeriod = 26;
  const auto kStopLossPeriod = 52;


  // Peak half size
  const auto kPeakHalfSize = 8;

  // ADX
  const auto kADXPeriod = 14;

  // ATR
  const auto kATRPeriod = 14;

  // risk control settings
  const auto kMaxPositionValuePctPerTrade = 0.1;
  const auto kRiskPct = 0.005;

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
  auto current_price = tick_data_map.at(instrument_name).value().close;

  if (account_base_rate.has_value() && account_quote_rate.has_value()) {
    auto account_base_rate_value = account_base_rate.value();
    auto account_quote_rate_value = account_quote_rate.value();

    auto closes = candlestick_closes(hist_data);
    auto highs = candlestick_highs(hist_data);
    auto lows = candlestick_lows(hist_data);

    // Moving Average
    auto slow_out = std::make_unique<std::vector<TA_Real>>(hist_data_count);
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
    std::vector<int> slow_out_left_padding (slow_out_beg,0);
    slow_out->insert(slow_out->begin(), slow_out_left_padding.begin(), slow_out_left_padding.end());
    slow_out->erase(slow_out->begin() + hist_data_count , slow_out->end());

    auto fast_out = std::make_unique<std::vector<TA_Real>>(hist_data_count);
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
    std::vector<int> fast_out_left_padding (fast_out_beg,0);
    fast_out->insert(fast_out->begin(), fast_out_left_padding.begin(), fast_out_left_padding.end());
    fast_out->erase(fast_out->begin() + hist_data_count , fast_out->end());

    auto stop_loss_out = std::make_unique<std::vector<TA_Real>>(hist_data_count);
    TA_Integer stop_loss_out_beg;
    TA_Integer stop_loss_out_num;
    TA_MA(
        0,
        hist_data_count - 1,
        closes->data(),
        kStopLossPeriod,
        TA_MAType_EMA,
        &stop_loss_out_beg,
        &stop_loss_out_num,
        stop_loss_out->data());
    std::vector<int> stop_loss_out_left_padding (stop_loss_out_beg,0);
    stop_loss_out->insert(stop_loss_out->begin(), stop_loss_out_left_padding.begin(), stop_loss_out_left_padding.end());
    stop_loss_out->erase(stop_loss_out->begin() + hist_data_count , stop_loss_out->end());

    std::optional<bool> ma_cross_is_long;
    std::optional<int> ma_cross_distance_to_last;

    auto diffs = std::make_unique<std::vector<TA_Real>>();
    for (int i = 0; i < hist_data_count; ++i) {
      if (fast_out->at(i) == 0 || slow_out->at(i) == 0) {
        diffs->push_back(0);
      } else {
        diffs->push_back(fast_out->at(i)-slow_out->at(i));
      }
    }
    for (int i = hist_data_count - 1; i >= 1; --i) {
      if (diffs->at(i - 1) < 0 && diffs->at(i) > 0) {
        ma_cross_is_long = true;
        ma_cross_distance_to_last = hist_data_count - 1 - i;
        break;
      } else if (diffs->at(i - 1) > 0 && diffs->at(i) < 0) {
        ma_cross_is_long = false;
        ma_cross_distance_to_last = hist_data_count - 1 - i;
        break;
      }
    }

    // ADX
    auto adx_out = std::make_shared<std::vector<TA_Real>>(hist_data_count);
    TA_Integer adx_out_beg;
    TA_Integer adx_out_num;
    TA_ADX(
        0,
        hist_data_count -1,
        highs->data(),
        lows->data(),
        closes->data(),
        kADXPeriod,
        &adx_out_beg,
        &adx_out_num,
        adx_out->data());
    auto adx = adx_out->at(adx_out_num - 1);

    // ATR
    auto atr_out = std::make_shared<std::vector<TA_Real>>(hist_data_count);
    TA_Integer atr_out_beg;
    TA_Integer atr_out_num;
    TA_ATR(
        0,
        hist_data_count -1,
        highs->data(),
        lows->data(),
        closes->data(),
        kATRPeriod,
        &atr_out_beg,
        &atr_out_num,
        atr_out->data());
    auto atr = atr_out->at(atr_out_num - 1);

    if (!account_ptr->HasOpenTrades(instrument_name)) {
      if (
          ma_cross_is_long.has_value()
          && ma_cross_distance_to_last.has_value()
          && (ma_cross_distance_to_last.value() > 1)
          && (adx > 20)
          ) {
        // Nearest peak high & low
        auto sliced_highs_before_cross = iridium::algorithm::SliceFirstN(*highs, highs->size() - ma_cross_distance_to_last.value());
        auto nearest_peak_high_before_cross = iridium::algorithm::FindNearestPeakHigh(sliced_highs_before_cross, kPeakHalfSize);
        auto sliced_lows_before_cross = iridium::algorithm::SliceFirstN(*lows, lows->size() - ma_cross_distance_to_last.value());
        auto nearest_peak_low_before_cross = iridium::algorithm::FindNearestPeakLow(sliced_lows_before_cross, kPeakHalfSize);
        // Cross up pullback
        if (ma_cross_is_long.value() && nearest_peak_high_before_cross.has_value()) {
          auto sliced_highs_after_cross = iridium::algorithm::SliceLastN(*highs, ma_cross_distance_to_last.value());
          auto highest_after_cross = std::max_element(sliced_highs_after_cross.begin(), sliced_highs_after_cross.end());
          if (
              (*highest_after_cross > nearest_peak_high_before_cross.value())
              && (current_price >= slow_out->back())
              && (current_price <= fast_out->back())
              && (fast_out->back() > slow_out->back())
              && (slow_out->back() > stop_loss_out->back())
              ) {
            auto stop_loss_price = stop_loss_out->back() - atr;
            TakePosition(
                tick,
                instrument_name,
                account_ptr,
                tick_data_map,
                kMaxPositionValuePctPerTrade,
                kRiskPct,
                stop_loss_price,
                current_price,
                account_quote_rate_value,
                account_base_rate_value,
                pip_num,
                false,
                kMinTradeSize,
                kSpread);
          }
        }
        // Cross down pullback
        if (!ma_cross_is_long.value() && nearest_peak_low_before_cross.has_value()) {
          auto sliced_lows_after_cross = iridium::algorithm::SliceLastN(*lows, ma_cross_distance_to_last.value());
          auto lowest_after_cross = std::min_element(sliced_lows_after_cross.begin(), sliced_lows_after_cross.end());
          if (
              (*lowest_after_cross < nearest_peak_low_before_cross.value())
              && (current_price <= slow_out->back())
              && (current_price >= fast_out->back())
              && (fast_out->back() < slow_out->back())
              && (slow_out->back() < stop_loss_out->back())
              ) {
            auto stop_loss_price = stop_loss_out->back() + atr;
            TakePosition(
                tick,
                instrument_name,
                account_ptr,
                tick_data_map,
                kMaxPositionValuePctPerTrade,
                kRiskPct,
                stop_loss_price,
                current_price,
                account_quote_rate_value,
                account_base_rate_value,
                pip_num,
                true,
                kMinTradeSize,
                kSpread);
          }
        }
      }
    } else {
      if (
          ma_cross_is_long.has_value()
          && ma_cross_distance_to_last.has_value()
          && (ma_cross_distance_to_last.value() == 0)
          ) {
        if ((ma_cross_is_long.value() && account_ptr->position_size(instrument_name) < 0)
        || (!ma_cross_is_long.value() && account_ptr->position_size(instrument_name) > 0))
          ClosePosition(instrument_name, account_ptr, account_quote_rate_value, current_price, tick);
      }
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
    double stop_loss_price,
    double current_price,
    double account_quote_rate,
    double account_base_rate,
    int pip_num,
    bool is_short,
    int min_size,
    double spread) {
  auto equity = account_ptr->net_asset_value(tick_data_map);
  auto margin_used =  account_ptr->margin_used(tick_data_map);
  if (equity.has_value() && margin_used.has_value()) {
    auto margin_available = iridium::CalculateMarginAvailable(equity.value(), margin_used.value());
    auto[size, calculated_stop_loss_price] = CalculateStopLossPositionSize(
        equity.value(),
        max_position_value_pct_per_trade,
        margin_available,
        account_ptr->leverage(),
        risk_pct,
        stop_loss_price,
        current_price,
        account_quote_rate,
        pip_num,
        is_short,
        min_size,
        spread);
    if (0 != size) {
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
          calculated_stop_loss_price,
          std::nullopt);
    }
  }
}

void ClosePosition(const std::string &instrument,
                   const std::shared_ptr<iridium::Account> &account_ptr,
                   double account_quote_rate,
                   double current_price,
                   std::time_t time) {
  account_ptr->CloserPosition(instrument, account_quote_rate, current_price, time);
}
