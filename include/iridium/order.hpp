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

#ifndef INCLUDE_IRIDIUM_ORDER_HPP_
#define INCLUDE_IRIDIUM_ORDER_HPP_

#include <ctime>
#include <optional>
#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <utility>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "instrument.hpp"
#include "util.hpp"

namespace iridium {
/*
 * PENDING	The Order is currently pending execution
 * FILLED	The Order has been filled
 * TRIGGERED	The Order has been triggered
 * CANCELLED	The Order has been cancelled
 */
enum OrderState {
  kPending, kFilled, kTriggered, kCancelled
};

/*
 * GTC	The Order is "Good unTil Cancelled"
 * GTD	The Order is "Good unTil Date" and will be cancelled at the provided time
 * GFD	The Order is "Good For Day" and will be cancelled at 5pm New York time
 * FOK	The Order must be immediately "Filled Or Killed"
 * IOC	The Order must be "Immediately partially filled Or Cancelled"
 */
enum TimeInForce {
  kGTC, kGTD, kGFD, kFOK, kIOC
};

/*
 * OPEN_ONLY	When the Order is filled, only allow Positions to be opened or extended.
 * REDUCE_FIRST	When the Order is filled, always fully reduce an existing Position before opening a new Position.
 * REDUCE_ONLY	When the Order is filled, only reduce an existing Position.
 * DEFAULT	When the Order is filled, use REDUCE_FIRST behaviour for non-client hedging Accounts, and OPEN_ONLY behaviour for client hedging Accounts.
 */
enum OrderPositionFill {
  kOpenOnly, kReduceFirst, kReduceOnly
};

/*
 *  DEFAULT	Trigger an Order the “natural” way: compare its price to the ask for long Orders and bid for short Orders.
 *  INVERSE	Trigger an Order the opposite of the “natural” way: compare its price the bid for long Orders and ask for short Orders.
 *  BID	Trigger an Order by comparing its price to the bid regardless of whether it is long or short.
 *  ASK	Trigger an Order by comparing its price to the ask regardless of whether it is long or short.
 *  MID	Trigger an Order by comparing its price to the midpoint regardless of whether it is long or short.
 */
enum OrderTriggerCondition {
  kDefault, kInverse, kBid, kAsk, kMid
};

class TakeProfitDetails {
 public:
  /*
   * @param price The price that the Take Profit Order will be triggered at.
   * @param timeInForce The time in force for the created Take Profit Order. This may only be GTC, GTD or GFD.
   * @param gtdTime The date when the Take Profit Order will be cancelled on if timeInForce is GTD.
   */
  explicit TakeProfitDetails(
      double price,
      TimeInForce time_in_force = TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time = std::nullopt);

  [[nodiscard]]
  double price() const noexcept;

  [[nodiscard]]
  TimeInForce time_in_force() const noexcept;

  [[nodiscard]]
  const std::optional<std::time_t> &gtd_time() const noexcept;

 private:
  double price_;
  TimeInForce time_in_force_;
  std::optional<std::time_t> gtd_time_;
};

class StopLossDetails {
 public:
  /*
   * @param price The price that the Stop Loss Order will be triggered at.
   * @param timeInForce The time in force for the created Take Profit Order. This may only be GTC, GTD or GFD.
   * @param gtdTime The date when the Take Profit Order will be cancelled on if timeInForce is GTD.
   */
  explicit StopLossDetails(
      double price,
      TimeInForce time_in_force = TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time = std::nullopt);

  [[nodiscard]]
  double price() const noexcept;

  [[nodiscard]]
  TimeInForce time_in_force() const noexcept;

  [[nodiscard]]
  const std::optional<std::time_t> &gtd_time() const noexcept;

 private:
  double price_;
  TimeInForce time_in_force_;
  std::optional<std::time_t> gtd_time_;
};

class TrailingStopLossDetails {
 public:
  /*
   * @param distance The distance (in price units) from the Trade’s fill price that the Trailing Stop Loss Order will be triggered at.
   * @param timeInForce The time in force for the created Take Profit Order. This may only be GTC, GTD or GFD.
   * @param gtdTime The date when the Take Profit Order will be cancelled on if timeInForce is GTD.
   */
  explicit TrailingStopLossDetails(
      double distance,
      TimeInForce time_in_force = TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time = std::nullopt);

  [[nodiscard]]
  double distance() const noexcept;

  [[nodiscard]]
  TimeInForce time_in_force() const noexcept;

  [[nodiscard]]
  const std::optional<std::time_t> &gtd_time() const noexcept;

 private:
  double distance_;
  TimeInForce time_in_force_;
  std::optional<std::time_t> gtd_time_;
};

class Order {
 public:
  explicit Order(std::time_t create_time);

  [[nodiscard]]
  const std::string &order_id() const noexcept;

  [[nodiscard]]
  OrderState order_state() const noexcept;

  [[nodiscard]]
  std::time_t create_time() const noexcept;

  void set_order_state(OrderState orderState);

  virtual ~Order() = default;

 private:
  std::string order_id_;
  OrderState order_state_;
  std::time_t create_time_;
};

class MarketOrderRequest {
 public:
  MarketOrderRequest(
      std::time_t create_time,
      const std::string &instrument,
      int units,
      double market_price,
      std::shared_ptr<TakeProfitDetails> take_profit_details =
      std::shared_ptr<TakeProfitDetails>(nullptr),
      std::shared_ptr<StopLossDetails> stop_loss_details =
      std::shared_ptr<StopLossDetails>(nullptr),
      std::shared_ptr<TrailingStopLossDetails> trailing_stop_loss_details =
      std::shared_ptr<TrailingStopLossDetails>(
          nullptr),
      double price_bound = 0.00,
      OrderPositionFill order_position_fill = OrderPositionFill::kReduceFirst,
      TimeInForce timeInForce = TimeInForce::kGTC);

  MarketOrderRequest(
      std::time_t create_time,
      const std::string &instrument,
      int units,
      double market_price,
      std::optional<double> take_profit_price =
      std::nullopt,
      std::optional<double> stop_loss_price =
      std::nullopt,
      std::optional<double> trailing_stop_loss_distance =
      std::nullopt);

  [[nodiscard]]
  int units() const noexcept;

  [[nodiscard]]
  double market_price() const noexcept;

  [[nodiscard]]
  double price_bound() const noexcept;

  [[nodiscard]]
  OrderPositionFill order_position_fill() const noexcept;

  [[nodiscard]]
  TimeInForce time_in_force() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<Instrument> &instrument_ptr() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<TakeProfitDetails> &take_profit_details_ptr() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<StopLossDetails> &stop_loss_details_ptr() const noexcept;

  [[nodiscard]]
  const std::shared_ptr<TrailingStopLossDetails> &trailing_stop_loss_details_ptr() const noexcept;

  friend std::ostream &operator<<(std::ostream &os, const MarketOrderRequest &request);

 private:
  std::string request_id_;
  std::time_t create_time_;
  std::shared_ptr<Instrument> instrument_ptr_;
  int units_;
  double market_price_;
  std::shared_ptr<TakeProfitDetails> take_profit_details_ptr_;
  std::shared_ptr<StopLossDetails> stop_loss_details_ptr_;
  std::shared_ptr<TrailingStopLossDetails> trailing_stop_loss_details_ptr_;
  double price_bound_;
  OrderPositionFill order_position_fill_;
  TimeInForce time_in_force_;
};

class TriggerOrder : public Order {
 public:
  TriggerOrder(
      std::time_t create_time,
      std::string trade_id,
      TimeInForce time_in_force =
      TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time =
      std::nullopt,
      OrderTriggerCondition order_trigger_condition =
      OrderTriggerCondition::kDefault);

  [[nodiscard]]
  const std::string &trade_id() const noexcept;

  [[nodiscard]]
  TimeInForce time_in_force() const noexcept;

  [[nodiscard]]
  const std::optional<std::time_t> &gtd_time() const noexcept;

  [[nodiscard]]
  OrderTriggerCondition order_trigger_condition() const noexcept;

 private:
  std::string trade_id_;
  TimeInForce time_in_force_;
  std::optional<std::time_t> gtd_time_;
  OrderTriggerCondition order_trigger_condition_;
};

class PriceTriggerOrder : public TriggerOrder {
 public:
  PriceTriggerOrder(
      double price,
      std::time_t create_time,
      const std::string &trade_id,
      TimeInForce time_in_force =
      TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time =
      std::nullopt,
      OrderTriggerCondition order_trigger_condition =
      OrderTriggerCondition::kDefault);

  [[nodiscard]]
  double price() const;

  void set_price(double d);

  friend std::ostream &operator<<(std::ostream &os, const PriceTriggerOrder &order);

 private:
  double price_;
};

using StopLossOrder = PriceTriggerOrder;

using TakeProfitOrder = PriceTriggerOrder;

class DistanceTriggerOrder : public TriggerOrder {
 public:
  DistanceTriggerOrder(
      double distance, time_t create_time,
      const std::string &trade_id,
      double trade_price,
      bool is_short,
      TimeInForce time_in_force =
          TimeInForce::kGTC,
      const std::optional<std::time_t> &gtd_time =
          std::nullopt,
      OrderTriggerCondition order_trigger_condition =
          OrderTriggerCondition::kDefault);

  [[nodiscard]]
  double distance() const;

  void set_distance(double distance);

  void UpdateTrailingStopPrice(double currentPrice);

  [[nodiscard]]
  double trailing_stop_price() const;

  friend std::ostream &operator<<(std::ostream &os, const DistanceTriggerOrder &order);

 private:
  double distance_;
  double trailing_stop_price_;
  bool is_short_;
};

using TrailingStopLossOrder = DistanceTriggerOrder;

using OrderList = std::vector<std::shared_ptr<Order>>;

std::string OrderStateToString(OrderState state);
}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_ORDER_HPP_
