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

#ifndef INCLUDE_IRIDIUM_ACCOUNT_HPP_
#define INCLUDE_IRIDIUM_ACCOUNT_HPP_

#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iridium/order.hpp>
#include <iridium/trade.hpp>
#include <iridium/data.hpp>
#include <iridium/forex.hpp>
#include <iridium/logging.hpp>

namespace iridium {
class Account {
 public:
  [[nodiscard]]
  virtual double balance() const = 0;

  [[nodiscard]]
  virtual const std::string &account_currency() const = 0;

  [[nodiscard]]
  virtual int leverage() const = 0;

  [[nodiscard]]
  virtual std::shared_ptr<TradeList>
  open_trades_ptr(const std::string &instrument) const = 0;

  [[nodiscard]]
  virtual std::shared_ptr<LimitOrderList>
  pending_limit_orders_ptr(const std::string &instrument) const = 0;

  [[nodiscard]]
  virtual int open_position_size(const std::string &instrument) const = 0;

  [[nodiscard]]
  virtual std::optional<double>
  net_asset_value(const data::TickDataMap &tick_data_map) const = 0;

  [[nodiscard]]
  virtual std::optional<double>
  margin_used(const data::TickDataMap &tick_data_map) const = 0;

  virtual void CreateLimitOrder(
      std::time_t create_time,
      const std::string &instrument,
      int units,
      double price,
      std::optional<double> take_profit_price = std::nullopt,
      std::optional<double> stop_loss_price = std::nullopt,
      std::optional<double> trailing_stop_loss_distance = std::nullopt) = 0;

  virtual void CloserPosition(
      const std::string &instrument,
      double acc_quote_rate,
      double current_price,
      std::time_t time) = 0;

  virtual void UpdateTradeStopLossPrice(
      const std::shared_ptr<Trade> &trade_ptr,
      double stop_loss_price,
      std::time_t time) = 0;

  virtual void UpdateTradeTakeProfitPrice(
      const std::shared_ptr<Trade> &trade_ptr,
      double take_profit_price,
      std::time_t time) = 0;

  virtual void UpdateTrailingStopDistance(
      const std::shared_ptr<Trade> &trade_ptr,
      double distance,
      std::time_t time) = 0;

  virtual void CancelLimitOrder(const std::shared_ptr<LimitOrder> &order_ptr) = 0;

  virtual bool HasOpenTrades(const std::string &instrument) const = 0;

  virtual bool HasPendingOrders(const std::string &instrument) const = 0;
};

class SimulationAccount: public Account {
 public:
  [[nodiscard]]
  double balance() const override;

  [[nodiscard]]
  const std::string &account_currency() const override;

  [[nodiscard]]
  int leverage() const override;

  [[nodiscard]]
  std::shared_ptr<TradeList>
  open_trades_ptr(const std::string &instrument) const override;

  [[nodiscard]]
  std::shared_ptr<LimitOrderList>
  pending_limit_orders_ptr(const std::string &instrument) const override;

  [[nodiscard]]
  int open_position_size(const std::string &instrument) const override;

  [[nodiscard]]
  std::optional<double>
  net_asset_value(const data::TickDataMap &tick_data_map) const override;

  [[nodiscard]]
  std::optional<double>
  margin_used(const data::TickDataMap &tick_data_map) const override;

  void CreateLimitOrder(
      std::time_t create_time,
      const std::string &instrument,
      int units,
      double price,
      std::optional<double> take_profit_price = std::nullopt,
      std::optional<double> stop_loss_price = std::nullopt,
      std::optional<double> trailing_stop_loss_distance = std::nullopt) override;

  void CloserPosition(
      const std::string &instrument,
      double acc_quote_rate,
      double current_price,
      std::time_t time) override;

  void UpdateTradeStopLossPrice(
      const std::shared_ptr<Trade> &trade_ptr,
      double stop_loss_price,
      std::time_t time
  ) override;

  void UpdateTradeTakeProfitPrice(
      const std::shared_ptr<Trade> &trade_ptr,
      double take_profit_price,
      std::time_t time
  ) override ;

  void UpdateTrailingStopDistance(
      const std::shared_ptr<Trade> &trade_ptr,
      double distance,
      std::time_t time
  ) override;

  void CancelLimitOrder(const std::shared_ptr<LimitOrder> &order_ptr) override;

  bool HasOpenTrades(const std::string &instrument) const override;

  bool HasPendingOrders(const std::string &instrument) const override;

  SimulationAccount(
      const std::string &account_currency,
      int leverage,
      double capital_base,
      double spread);

  [[nodiscard]]
  std::shared_ptr<TradeList> trades_ptr() const;

  std::string string();

  std::string summary(std::time_t tick,
                      const iridium::data::TickDataMap &data_map) const;

  void ProcessOrders(
      std::time_t time,
      const data::TickDataMap &tick_data_map);

  friend std::ostream &operator<<(std::ostream &os, const SimulationAccount &account);

 private:
  std::string account_currency_;
  int leverage_;
  double capital_base_;
  double balance_;
  double spread_;
  std::shared_ptr<TradeList> trades_ptr_;
  std::shared_ptr<OrderList> orders_ptr_;
  std::shared_ptr<spdlog::logger> logger_;

  [[nodiscard]]
  std::shared_ptr<TradeList>
  open_trades_ptr() const;

  [[nodiscard]]
  std::shared_ptr<OrderList>
  pending_orders_ptr() const;

  [[nodiscard]]
  std::shared_ptr<Trade>
  acc_trade_ptr(const TriggerOrder &order) const;

  /*
   * return instrument name, ask low, ask high, bid low, bid high, account vs quote, account vs base, current price
  */
  std::optional<std::tuple<std::string, double, double, double, double, double, double, double>>
  instrument_market_info(const Instrument &instrument, const data::TickDataMap &tick_data_map);

  void PartiallyCloseTrade(
      const std::shared_ptr<Trade> &trade_ptr,
      double acc_quote_rate,
      double current_price,
      int units);

  void CloseTrade(
      const std::shared_ptr<Trade> &trade_ptr,
      double acc_quote_rate,
      double current_price,
      std::time_t time);

  void ProcessLimitOrder(
      const std::shared_ptr<LimitOrder> &order_ptr,
      std::time_t time,
      const data::TickDataMap &tick_data_map);

  void ProcessTriggerOrder(
      const std::shared_ptr<TriggerOrder> &order_ptr,
      std::time_t time,
      const data::TickDataMap &tick_data_map);

  void ProcessPriceTriggerOrder(
      const std::shared_ptr<PriceTriggerOrder> &order_ptr,
      const std::shared_ptr<Trade> &trade_ptr,
      double ask_low,
      double ask_high,
      double bid_low,
      double bid_high,
      double acc_quote_rate,
      std::time_t time);

  void ProcessTrailingStopLossOrder(
      const std::shared_ptr<TrailingStopLossOrder> &order_ptr,
      const std::shared_ptr<Trade> &trade_ptr,
      double ask_low,
      double ask_high,
      double bid_low,
      double bid_high,
      double acc_quote_rate,
      double current_price,
      std::time_t time);
};

}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_ACCOUNT_HPP_
