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

#ifndef INCLUDE_IRIDIUM_TRADE_HPP_
#define INCLUDE_IRIDIUM_TRADE_HPP_

#include <string>
#include <ctime>
#include <memory>
#include <iostream>
#include <vector>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "instrument.hpp"
#include "order.hpp"
#include "util.hpp"
#include "forex.hpp"

namespace iridium {

enum TradeState {
  kOpen,
  kClosed
};

class Trade {
 public:
  Trade(
      const std::string &instrument,
      double price,
      std::time_t open_time,
      int initial_units,
      double initial_margin,
      double spread,
      double financing = 0.0,
      double commission = 0.0,
      std::optional<double> take_profit_price = std::nullopt,
      std::optional<double> stop_loss_price = std::nullopt,
      std::optional<double> trailing_stop_distance = std::nullopt);

  [[nodiscard]]
  const std::string &trade_id() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<Instrument> &instrument() const noexcept;

  [[nodiscard]]
  double price() const noexcept;

  [[nodiscard]]
  std::optional<double> close_price() const noexcept;

  [[nodiscard]]
  TradeState trade_state() const noexcept;

  [[nodiscard]]
  std::time_t open_time() const noexcept;

  [[nodiscard]]
  int initial_units() const noexcept;

  [[nodiscard]]
  double initial_margin() const noexcept;

  [[nodiscard]]
  int current_units() const noexcept;

  [[nodiscard]]
  double realized_profit_loss() const noexcept;

  [[nodiscard]]
  const std::optional<std::time_t> &close_time() const noexcept;

  [[nodiscard]]
  double spread() const noexcept;

  [[nodiscard]]
  double financing() const noexcept;

  [[nodiscard]]
  double commission() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<TakeProfitOrder> &take_profit_order_ptr() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<StopLossOrder> &stop_loss_order_ptr() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<TrailingStopLossOrder> &trailing_stop_loss_order_ptr() const noexcept;

  [[nodiscard]]
  std::optional<double> take_profit_price() const noexcept;

  [[nodiscard]]
  std::optional<double> stop_loss_price() const noexcept;

  [[nodiscard]]
  std::optional<double> trailing_stop_distance() const noexcept;

  [[nodiscard]]
  std::optional<double> trailing_stop_price() const noexcept;

  void UpdateTakeProfitOrder(double price, std::time_t open_time);

  void UpdateStopLossOrder(double price, std::time_t open_time);

  void UpdateTrailingStopLossOrder(double distance, std::time_t open_time);

  /*
   * @param rate: account currency vs quote currency rate
   * @param current_price
   * @param units
   */
  [[nodiscard]]
  double PartiallyCloseTrade(double rate, double current_price, int units);

  /*
   *  @param rate: account currency vs quote currency rate
   *  @param current_price
   *  @param time: trade close time
   */
  [[nodiscard]]
  double CloseTrade(double rate, double current_price, std::time_t time);

  friend std::ostream &operator<<(std::ostream &os, const Trade &trade);

 private:
  std::string trade_id_;
  std::shared_ptr<Instrument> instrument_ptr_;
  double price_;
  TradeState state_;
  std::time_t open_time_;
  int initial_units_;
  double initial_margin_;
  int current_units_;
  double realized_profit_loss_;
  std::optional<std::time_t> close_time_;
  std::optional<double> close_price_;
  double spread_;
  double financing_;
  double commission_;
  std::shared_ptr<TakeProfitOrder> take_profit_order_ptr_;
  std::shared_ptr<StopLossOrder> stop_loss_order_ptr_;
  std::shared_ptr<TrailingStopLossOrder> trailing_stop_loss_order_ptr_;

  Trade(
      const std::string &instrument,
      double price,
      std::time_t open_time,
      int initial_units,
      double initial_margin,
      double spread,
      double financing = 0.0,
      double commission = 0.0,
      std::shared_ptr<TakeProfitOrder> take_profit_order_ptr =
          std::shared_ptr<TakeProfitOrder>(nullptr),
      std::shared_ptr<StopLossOrder> stop_loss_order_ptr =
          std::shared_ptr<StopLossOrder>(nullptr),
      std::shared_ptr<TrailingStopLossOrder> trailing_stop_loss_order_ptr =
          std::shared_ptr<TrailingStopLossOrder>(nullptr));

  void set_take_profit_price(double price);

  void set_stop_loss_price(double price);

  void set_trailing_stop_distance(double distance);

  void set_take_profit_order_ptr(const std::shared_ptr<TakeProfitOrder> &take_profit_order_ptr);

  void set_stop_loss_order_ptr(const std::shared_ptr<StopLossOrder> &stop_loss_order_ptr);

  void set_trailing_stop_loss_order_ptr(
      const std::shared_ptr<TrailingStopLossOrder> &trailing_stop_loss_order_ptr);
};

/*
 * @param trade
 * @param rate: account currency vs quote currency rate
 * @param price: current instrument price
 */
double CalculateUnrealizedProfitLoss(const Trade &trade, double rate, double current_price);

/*
 * @param trade
 * @param rate: account currency vs base currency rate
 * @param leverage
 */
double CalculateMarginUsed(const Trade &trade, double rate, int leverage);

using TradeList = std::vector<std::shared_ptr<Trade>>;

std::string TradeStateToString(TradeState state);
}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_TRADE_HPP_
