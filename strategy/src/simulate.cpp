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
    const std::shared_ptr<iridium::Account> &account_ptr,
    double spread) {
  // logging
  auto logger = iridium::logger();

  // Trade simulation settings
  const auto kMinTradeSize = 1000;
  const auto kMaxSpread = 3.0;

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
  const auto kRiskPct = 0.005;
  const auto kProhibitPct = 0.06;

  // profit/loss ratio
  const auto kProfitLossRatio = 2;

  // local time
  auto local_time = TimeToLocalTimeString(tick);

  // check data availability
  auto instrument = std::make_shared<iridium::Instrument>(instrument_name);
  auto base = instrument->base_name();
  auto quote = instrument->quote_name();
  auto pip_num = pip_point(*instrument);
  auto acc_quote_rate = iridium::data::account_currency_rate(
      account_ptr->account_currency(),
      quote,
      tick_data_map);
  auto current_price = tick_data_map.at(instrument_name).value().close;

  if (acc_quote_rate.has_value()) {
    auto acc_quote_rate_value = acc_quote_rate.value();
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
    std::vector<int> slow_out_left_padding(slow_out_beg, 0);
    slow_out->insert(slow_out->begin(), slow_out_left_padding.begin(), slow_out_left_padding.end());
    slow_out->erase(slow_out->begin() + hist_data_count, slow_out->end());

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
    std::vector<int> fast_out_left_padding(fast_out_beg, 0);
    fast_out->insert(fast_out->begin(), fast_out_left_padding.begin(), fast_out_left_padding.end());
    fast_out->erase(fast_out->begin() + hist_data_count, fast_out->end());

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
    std::vector<int> stop_loss_out_left_padding(stop_loss_out_beg, 0);
    stop_loss_out->insert(stop_loss_out->begin(), stop_loss_out_left_padding.begin(), stop_loss_out_left_padding.end());
    stop_loss_out->erase(stop_loss_out->begin() + hist_data_count, stop_loss_out->end());

    std::optional<bool> ma_cross_is_long;
    std::optional<int> ma_cross_distance_to_last;

    auto diffs = std::make_unique<std::vector<TA_Real>>();
    for (int i = 0; i < hist_data_count; ++i) {
      if (fast_out->at(i) == 0 || slow_out->at(i) == 0) {
        diffs->push_back(0);
      } else {
        diffs->push_back(fast_out->at(i) - slow_out->at(i));
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
        hist_data_count - 1,
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
        hist_data_count - 1,
        highs->data(),
        lows->data(),
        closes->data(),
        kATRPeriod,
        &atr_out_beg,
        &atr_out_num,
        atr_out->data());
    auto atr = atr_out->at(atr_out_num - 1);

    if (!account_ptr->HasPendingOrders(instrument_name) && !account_ptr->HasOpenTrades(instrument_name)) {
      if (
          ma_cross_is_long.has_value()
              && ma_cross_distance_to_last.has_value()
              && (ma_cross_distance_to_last.value() > 1)
              && (adx > 20)
          ) {
        // Nearest peak high & low
        auto sliced_highs_before_cross =
            iridium::algorithm::SliceFirstN(*highs, highs->size() - ma_cross_distance_to_last.value());
        auto nearest_peak_high_before_cross =
            iridium::algorithm::FindNearestPeakHigh(sliced_highs_before_cross, kPeakHalfSize);
        auto sliced_lows_before_cross =
            iridium::algorithm::SliceFirstN(*lows, lows->size() - ma_cross_distance_to_last.value());
        auto nearest_peak_low_before_cross =
            iridium::algorithm::FindNearestPeakLow(sliced_lows_before_cross, kPeakHalfSize);
        // Cross up pullback
        if (ma_cross_is_long.value() && nearest_peak_high_before_cross.has_value()) {
          auto sliced_highs_after_cross = iridium::algorithm::SliceLastN(*highs, ma_cross_distance_to_last.value());
          auto highest_after_cross = std::max_element(sliced_highs_after_cross.begin(), sliced_highs_after_cross.end());
          if (
              (*highest_after_cross > nearest_peak_high_before_cross.value())
                  && (current_price > slow_out->back())
                  && (current_price < fast_out->back())
                  && (fast_out->back() > slow_out->back())
                  && (slow_out->back() > stop_loss_out->back())
                  && (current_price > nearest_peak_high_before_cross.value())
              ) {
            auto stop_loss_price = stop_loss_out->back() - atr;
            auto order_price = highs->back();
            auto take_profit_price = order_price + kProfitLossRatio * (order_price - stop_loss_price);
            auto units = CalculateLimitOrderUnits(
                account_ptr,
                tick_data_map,
                order_price,
                stop_loss_price,
                acc_quote_rate.value(),
                kRiskPct,
                pip_num,
                false,
                kMinTradeSize);
            if (0 != units && spread <= kMaxSpread) {
              account_ptr->CreateLimitOrder(
                  tick,
                  instrument_name,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
              logger->info(
                  "create limit order - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
                  instrument_name,
                  local_time,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
            }
          }
        }
        // Cross down pullback
        if (!ma_cross_is_long.value() && nearest_peak_low_before_cross.has_value()) {
          auto sliced_lows_after_cross = iridium::algorithm::SliceLastN(*lows, ma_cross_distance_to_last.value());
          auto lowest_after_cross = std::min_element(sliced_lows_after_cross.begin(), sliced_lows_after_cross.end());
          if (
              (*lowest_after_cross < nearest_peak_low_before_cross.value())
                  && (current_price < slow_out->back())
                  && (current_price > fast_out->back())
                  && (fast_out->back() < slow_out->back())
                  && (slow_out->back() < stop_loss_out->back())
                  && (current_price < nearest_peak_low_before_cross.value())
              ) {
            auto stop_loss_price = stop_loss_out->back() + atr;
            auto order_price = lows->back();
            auto take_profit_price = order_price - kProfitLossRatio * (stop_loss_price - order_price);
            auto units = CalculateLimitOrderUnits(
                account_ptr,
                tick_data_map,
                order_price,
                stop_loss_price,
                acc_quote_rate.value(),
                kRiskPct,
                pip_num,
                true,
                kMinTradeSize);
            if (0 != units && spread <= kMaxSpread) {
              account_ptr->CreateLimitOrder(
                  tick,
                  instrument_name,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
              logger->info(
                  "create limit order - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
                  instrument_name,
                  local_time,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
            }
          }
        }
      }
    }

    if (account_ptr->HasPendingOrders(instrument_name)) {
      auto pending_limit_orders_ptr = account_ptr->pending_limit_orders_ptr(instrument_name);
      for (const auto &order_ptr : *pending_limit_orders_ptr) {
        if ((order_ptr->units() > 0 && current_price < stop_loss_out->back()) ||
            (order_ptr->units() < 0 && current_price > stop_loss_out->back())) {
          account_ptr->CancelLimitOrder(order_ptr);
          logger->info(
              "cancel pending limit order - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
              order_ptr->instrument_ptr()->name(),
              local_time,
              order_ptr->units(),
              order_ptr->price(),
              order_ptr->take_profit_price().value(),
              order_ptr->stop_loss_price().value());
        }
      }
    }

    if (account_ptr->HasOpenTrades(instrument_name)) {
      // close position
      if (
          ma_cross_is_long.has_value()
              && ma_cross_distance_to_last.has_value()
              && (ma_cross_distance_to_last.value() >= 0)
          ) {
        if ((ma_cross_is_long.value() && account_ptr->open_position_size(instrument_name) < 0)
            || (!ma_cross_is_long.value() && account_ptr->open_position_size(instrument_name) > 0)) {
          ClosePosition(instrument_name, account_ptr, acc_quote_rate_value, current_price, tick);
          logger->info(
              "close position - instrument: {}, time: {}",
              instrument_name,
              local_time);
        }
      }
    }
    // update stop loss price
    auto open_trades_ptr = account_ptr->open_trades_ptr(instrument_name);
    for (const auto &trade_ptr : *open_trades_ptr) {
      auto units = trade_ptr->current_units();
      auto price = trade_ptr->price();
      auto stop_loss_price = trade_ptr->stop_loss_price();
      if (stop_loss_price.has_value()) {
        auto stop_loss_change = abs(price - stop_loss_price.value());
        // break even
        if (units > 0 &&
            (current_price - price) >= stop_loss_change &&
            stop_loss_price < price) {
          account_ptr->UpdateTradeStopLossPrice(trade_ptr, price, tick);
          logger->info(
              "break even - instrument: {}, time: {}",
              instrument_name,
              local_time);
        }
        if (units < 0 &&
            (price - current_price) >= stop_loss_change &&
            stop_loss_price > price) {
          account_ptr->UpdateTradeStopLossPrice(trade_ptr, price, tick);
          logger->info(
              "break even - instrument: {}, time: {}",
              instrument_name,
              local_time);
        }
      }

    }
  }
}

void ClosePosition(const std::string &instrument,
                   const std::shared_ptr<iridium::Account> &account_ptr,
                   double acc_quote_rate,
                   double current_price,
                   std::time_t time) {
  account_ptr->CloserPosition(instrument, acc_quote_rate, current_price, time);
}

int CalculateLimitOrderUnits(
    const std::shared_ptr<iridium::Account> &account_ptr,
    const iridium::data::TickDataMap &tick_data_map,
    double order_price,
    double stop_loss_price,
    double acc_quote_rate,
    double risk_pct,
    int pip_num,
    bool is_short,
    int min_size) {
  auto equity = account_ptr->net_asset_value(tick_data_map);
  auto margin_used = account_ptr->margin_used(tick_data_map);
  if (equity.has_value() && margin_used.has_value()) {
    return CalculatePositionSize(
        equity.value(),
        margin_used.value(),
        account_ptr->leverage(),
        risk_pct,
        stop_loss_price,
        order_price,
        acc_quote_rate,
        pip_num,
        is_short,
        min_size);
  }
  return 0;
}
