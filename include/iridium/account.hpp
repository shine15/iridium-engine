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

namespace iridium {
class Account {
 public:
  Account(
      const std::string &account_currency,
      int leverage,
      double capital_base);

  void CreateMarketOrder(
      std::time_t create_time,
      double margin_available,
      const std::string &instrument,
      int units,
      double market_price,
      double account_quote_rate,
      double account_base_rate,
      double spread,
      std::optional<double> take_profit_price = std::nullopt,
      std::optional<double> stop_loss_price = std::nullopt,
      std::optional<double> trailing_stop_loss_distance = std::nullopt,
      double financing = 0.0,
      double commission = 0.0);

  /*
   * @param trade_ptr
   * @param rate: account currency vs quote currency rate
   * @param current_price
   * @param units
  */
  void PartiallyCloseTrade(
      const std::shared_ptr<Trade> &trade_ptr,
      double rate,
      double current_price,
      int units);

  /*
   * @param trade_ptr
   * @param rate: account currency vs quote currency rate
   * @param current_price
   * @param time
  */
  void CloseTrade(
      const std::shared_ptr<Trade> &trade_ptr,
      double rate,
      double current_price,
      std::time_t time);

  /*
   * @param instrument
   * @param rate: account currency vs quote currency rate
   * @param current_price
   * @param time
  */
  void CloserPosition(
      const std::string &instrument,
      double rate,
      double current_price,
      std::time_t time);

  [[nodiscard]]
  double balance() const;

  [[nodiscard]]
  const std::string &account_currency() const;

  [[nodiscard]]
  int leverage() const;

  [[nodiscard]]
  std::shared_ptr<TradeList>
  open_trades_ptr(const std::string &instrument_name) const;

  [[nodiscard]]
  std::shared_ptr<TradeList>
  open_trades_ptr() const;

  [[nodiscard]]
  const std::shared_ptr<TradeList> &all_trades_ptr() const;

  void UpdateBalance(double profit_loss);

  int position_size(const std::string &instrument);

  friend std::ostream &operator<<(std::ostream &os, const Account &account);

 private:
  std::string account_currency_;
  int leverage_;
  double capital_base_;
  double balance_;
  std::shared_ptr<TradeList> trades_ptr_;
};

std::optional<float>
account_net_asset_value(const Account &account, const data::TickDataMap &tick_data_map);

std::optional<float>
account_margin_used(const Account &account, const data::TickDataMap &tick_data_map);

void ProcessTriggerOrders(
    const std::shared_ptr<Account> &account_ptr,
    std::time_t time,
    const data::TickDataMap &tick_data_map);

void
ProcessStopLossOrder(
    const std::shared_ptr<StopLossOrder> &order_ptr,
    const std::shared_ptr<Account> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    double current_price);

void ProcessTakeProfitOrder(
    const std::shared_ptr<TakeProfitOrder> &order_ptr,
    const std::shared_ptr<Account> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    double current_price);

void ProcessTrailingStopLossOrder(
    const std::shared_ptr<TrailingStopLossOrder> &order_ptr,
    const std::shared_ptr<Account> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    double current_price);
}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_ACCOUNT_HPP_
