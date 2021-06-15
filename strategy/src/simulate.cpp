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
  if (auto equity = account_ptr->net_asset_value(tick_data_map);
      auto margin_used = account_ptr->margin_used(tick_data_map)) {
    return CalculatePositionSize(
        *equity,
        *margin_used,
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

int CalculateMarketOrderUnits(
    const std::shared_ptr<iridium::Account> &account_ptr,
    const iridium::data::TickDataMap &tick_data_map,
    double market_price,
    double stop_loss_price,
    double acc_quote_rate,
    double risk_pct,
    int pip_num,
    bool is_short,
    int min_size,
    double spread) {
  if (auto equity = account_ptr->net_asset_value(tick_data_map);
      auto margin_used = account_ptr->margin_used(tick_data_map)) {
    auto spread_value = spread * pow(10, -pip_num);
    auto ask = market_price + spread_value / 2.0;
    auto bid = market_price - spread_value / 2.0;
    auto order_price = is_short ? bid : ask;
    return CalculatePositionSize(
        *equity,
        *margin_used,
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

void SimulateTrade(
    const std::string &instrument_name,
    std::time_t tick,
    const iridium::data::DataList &long_term_hist_data,
    const iridium::data::DataList &intermediate_term_hist_data,
    const iridium::data::DataList &short_term_hist_data,
    const iridium::data::TickDataMap &tick_data_map,
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

  // MACD settings
  const auto kMACDFastPeriod = 12;
  const auto kMACDSlowPeriod = 26;
  const auto kMACDSignalPeriod = 9;
  const auto kMACDCrossOverCheck = 4;

  // Peak half size
  const auto kPeakHalfSize = 3;

  // ATR
  const auto kATRPeriod = 14;
  const auto kATRChannel = 3;

  // RSI
  const auto kRSIPeriod = 14;
  const auto kRSIUpperLimit = 70;
  const auto kRSILowerLimit = 30;
  const auto kRSICheck = 3;

  // risk control settings
  const auto kRiskPct = 0.005;
  const auto kProhibitPct = 0.06;

  // profit/loss ratio
  const auto kProfitLossRatio = 3;

  // local time
  auto local_time = TimeToLocalTimeString(tick);

  // check data availability
  auto instrument = std::make_shared<iridium::Instrument>(instrument_name);
  auto base = instrument->base_name();
  auto quote = instrument->quote_name();
  auto pip_num = pip_point(*instrument);
  auto account_quote_rate_opt = iridium::data::account_currency_rate(
      account_ptr->account_currency(),
      quote,
      tick_data_map);
  auto current_price = tick_data_map.at(instrument_name).value().close;

  if (!account_quote_rate_opt.has_value()) return;
  auto acc_quote_rate = account_quote_rate_opt.value();

  // history data
  auto closes = candlestick_closes(short_term_hist_data);
  auto highs = candlestick_highs(short_term_hist_data);
  auto lows = candlestick_lows(short_term_hist_data);

  // rsi
  auto rsi = iridium::indicator::rsi(*closes, kRSIPeriod);

  // atr
  auto atr = iridium::indicator::atr(*highs, *lows, *closes, kATRPeriod);

  // moving average
  auto ema = iridium::indicator::ema(*closes, kFastPeriod);

  // macd
  auto[macd, macd_signal, macd_hist] = iridium::indicator::macd(*closes,
                                                                kMACDFastPeriod,
                                                                kMACDSlowPeriod,
                                                                kMACDSignalPeriod);
  auto count = macd_hist->size();
  auto sliced_macd_hist = std::vector<double>(macd_hist->end() - kMACDCrossOverCheck, macd_hist->end());

  //  take position
  if (!account_ptr->HasPendingOrders(instrument_name) && !account_ptr->HasOpenTrades(instrument_name)) {
    if (std::all_of(
        std::begin(sliced_macd_hist),
        std::end(sliced_macd_hist),
        [](auto v) {
          return v > 0;
        })) {
      if (macd_hist->at(count - 1 - kMACDCrossOverCheck) < 0 &&
          macd->at(count - 1 - kMACDCrossOverCheck) < 0 &&
          macd_signal->at(count - 1 - kMACDCrossOverCheck) < 0 &&
          ema->back() > ema->at(count - 2) &&
          current_price >= ema->back()) {
        for (int i = 0; i < kRSICheck; ++i) {
          if (rsi->at(count - 1 - kMACDCrossOverCheck - i) <= kRSILowerLimit) {
            logger->info("confirmed macd up cross over: {}", local_time);
            auto stop_loss_price = ema->back() - kATRChannel * atr->back();
            auto order_price = current_price;
            auto take_profit_price = order_price + kProfitLossRatio * (order_price - stop_loss_price);
            auto units = CalculateMarketOrderUnits(
                account_ptr,
                tick_data_map,
                order_price,
                stop_loss_price,
                acc_quote_rate,
                kRiskPct,
                pip_num,
                false,
                kMinTradeSize,
                spread);
            if (0 != units && spread <= kMaxSpread) {
              account_ptr->CreateMarketOrder(
                  tick,
                  instrument_name,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
              logger->info(
                  "create market order - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
                  instrument_name,
                  local_time,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
            }

            break;
          }
        }
      }
    }

    if (std::all_of(
        std::begin(sliced_macd_hist),
        std::end(sliced_macd_hist),
        [](auto v) {
          return v < 0;
        })) {
      if (macd_hist->at(count - 1 - kMACDCrossOverCheck) > 0 &&
          macd->at(count - 1 - kMACDCrossOverCheck) > 0 &&
          macd_signal->at(count - 1 - kMACDCrossOverCheck) > 0 &&
          ema->back() < ema->at(count - 2) &&
          current_price <= ema->back()) {
        for (int i = 0; i < kRSICheck; ++i) {
          if (rsi->at(count - 1 - kMACDCrossOverCheck - i) >= kRSIUpperLimit) {
            logger->info("confirmed macd down cross over: {}", local_time);

            auto stop_loss_price = ema->back() + kATRChannel * atr->back();
            auto order_price = current_price;
            auto take_profit_price = order_price - kProfitLossRatio * (stop_loss_price - order_price);
            auto units = CalculateMarketOrderUnits(
                account_ptr,
                tick_data_map,
                order_price,
                stop_loss_price,
                acc_quote_rate,
                kRiskPct,
                pip_num,
                true,
                kMinTradeSize,
                spread);
            if (0 != units && spread <= kMaxSpread) {
              account_ptr->CreateMarketOrder(
                  tick,
                  instrument_name,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
              logger->info(
                  "create market order - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
                  instrument_name,
                  local_time,
                  units,
                  order_price,
                  take_profit_price,
                  stop_loss_price);
            }
            break;
          }
        }
      }
    }
  }

  // close position
  if (account_ptr->HasOpenTrades(instrument_name)) {
    auto macd_hist_0 = macd_hist->at(count - 2);
    auto macd_hist_1 = macd_hist->back();
    if ((macd_hist_0 < 0 && macd_hist_1 > 0 && account_ptr->open_position_size(instrument_name) < 0) ||
        (macd_hist_0 > 0 && macd_hist_1 < 0 && account_ptr->open_position_size(instrument_name) > 0)) {
      ClosePosition(instrument_name, account_ptr, acc_quote_rate, current_price, tick);
      logger->info(
          "close position - instrument: {}, time: {}",
          instrument_name,
          local_time);
    }
  }

}

