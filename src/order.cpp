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

#include <iridium/order.hpp>

iridium::TakeProfitDetails::TakeProfitDetails(
    double price,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time) :
    price_(price),
    time_in_force_(time_in_force),
    gtd_time_(gtd_time) {}

double iridium::TakeProfitDetails::price() const noexcept {
  return price_;
}

iridium::TimeInForce
iridium::TakeProfitDetails::time_in_force() const noexcept {
  return time_in_force_;
}

const std::optional<std::time_t>
&iridium::TakeProfitDetails::gtd_time() const noexcept {
  return gtd_time_;
}

iridium::StopLossDetails::StopLossDetails(
    double price,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time) :
    price_(price),
    time_in_force_(time_in_force),
    gtd_time_(gtd_time) {}

double iridium::StopLossDetails::price() const noexcept {
  return price_;
}

iridium::TimeInForce
iridium::StopLossDetails::time_in_force() const noexcept {
  return time_in_force_;
}

const std::optional<std::time_t>
&iridium::StopLossDetails::gtd_time() const noexcept {
  return gtd_time_;
}

iridium::TrailingStopLossDetails::TrailingStopLossDetails(
    double distance,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time) :
    distance_(distance),
    time_in_force_(time_in_force),
    gtd_time_(gtd_time) {}

double
iridium::TrailingStopLossDetails::distance() const noexcept {
  return distance_;
}

iridium::TimeInForce
iridium::TrailingStopLossDetails::time_in_force() const noexcept {
  return time_in_force_;
}

const std::optional<std::time_t>
&iridium::TrailingStopLossDetails::gtd_time() const noexcept {
  return gtd_time_;
}

iridium::Order::Order(std::time_t create_time) :
    create_time_(create_time),
    order_state_(OrderState::kPending),
    order_id_(boost::uuids::to_string(boost::uuids::random_generator()())) {}

const std::string &iridium::Order::order_id() const noexcept {
  return order_id_;
}

iridium::OrderState iridium::Order::order_state() const noexcept {
  return order_state_;
}

std::time_t iridium::Order::create_time() const noexcept {
  return create_time_;
}

void iridium::Order::set_order_state(iridium::OrderState orderState) {
  this->order_state_ = orderState;
}

iridium::MarketOrderRequest::MarketOrderRequest(
    std::time_t create_time,
    const std::string &instrument,
    int units,
    double market_price,
    std::shared_ptr<TakeProfitDetails> take_profit_details,
    std::shared_ptr<StopLossDetails> stop_loss_details,
    std::shared_ptr<TrailingStopLossDetails> trailing_stop_loss_details,
    double price_bound,
    iridium::OrderPositionFill order_position_fill,
    iridium::TimeInForce timeInForce) :
    request_id_(boost::uuids::to_string(boost::uuids::random_generator()())),
    create_time_(create_time),
    instrument_ptr_(std::make_shared<iridium::Instrument>(instrument)),
    units_(units),
    market_price_(market_price),
    take_profit_details_ptr_(std::move(take_profit_details)),
    stop_loss_details_ptr_(std::move(stop_loss_details)),
    trailing_stop_loss_details_ptr_(std::move(trailing_stop_loss_details)),
    price_bound_(price_bound),
    order_position_fill_(order_position_fill),
    time_in_force_(timeInForce) {}

iridium::MarketOrderRequest::MarketOrderRequest(
    std::time_t create_time,
    const std::string &instrument,
    int units,
    double market_price,
    std::optional<double> take_profit_price,
    std::optional<double> stop_loss_price,
    std::optional<double> trailing_stop_loss_distance) :
    request_id_(boost::uuids::to_string(boost::uuids::random_generator()())),
    create_time_(create_time),
    instrument_ptr_(std::make_shared<iridium::Instrument>(instrument)),
    units_(units),
    market_price_(market_price),
    take_profit_details_ptr_(
        take_profit_price.has_value()
        ? std::make_shared<iridium::TakeProfitDetails>(take_profit_price.value())
        : std::shared_ptr<iridium::TakeProfitDetails>(nullptr)),
    stop_loss_details_ptr_(
        stop_loss_price.has_value()
        ? std::make_shared<iridium::StopLossDetails>(stop_loss_price.value())
        : std::shared_ptr<iridium::StopLossDetails>(nullptr)),
    trailing_stop_loss_details_ptr_(
        trailing_stop_loss_distance.has_value()
        ? std::make_shared<iridium::TrailingStopLossDetails>(
            trailing_stop_loss_distance.value())
        : std::shared_ptr<iridium::TrailingStopLossDetails>(nullptr)),
    price_bound_(0.0),
    order_position_fill_(iridium::OrderPositionFill::kReduceFirst),
    time_in_force_(iridium::TimeInForce::kGTC) {}

int iridium::MarketOrderRequest::units() const noexcept {
  return units_;
}

double iridium::MarketOrderRequest::market_price() const noexcept {
  return market_price_;
}

double iridium::MarketOrderRequest::price_bound() const noexcept {
  return price_bound_;
}

iridium::OrderPositionFill
iridium::MarketOrderRequest::order_position_fill() const noexcept {
  return order_position_fill_;
}

iridium::TimeInForce
iridium::MarketOrderRequest::time_in_force() const noexcept {
  return time_in_force_;
}

const std::shared_ptr<iridium::Instrument>
&iridium::MarketOrderRequest::instrument_ptr() const noexcept {
  return instrument_ptr_;
}

const std::shared_ptr<iridium::TakeProfitDetails>
&iridium::MarketOrderRequest::take_profit_details_ptr() const noexcept {
  return take_profit_details_ptr_;
}

const std::shared_ptr<iridium::StopLossDetails>
&iridium::MarketOrderRequest::stop_loss_details_ptr() const noexcept {
  return stop_loss_details_ptr_;
}

const std::shared_ptr<iridium::TrailingStopLossDetails> &
iridium::MarketOrderRequest::trailing_stop_loss_details_ptr() const noexcept {
  return trailing_stop_loss_details_ptr_;
}

iridium::TriggerOrder::TriggerOrder(
    time_t create_time,
    std::string trade_id,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time,
    iridium::OrderTriggerCondition order_trigger_condition) :
    Order(create_time),
    trade_id_(std::move(trade_id)),
    time_in_force_(time_in_force),
    gtd_time_(gtd_time),
    order_trigger_condition_(order_trigger_condition) {}

const std::string
&iridium::TriggerOrder::trade_id() const noexcept {
  return trade_id_;
}

iridium::TimeInForce
iridium::TriggerOrder::time_in_force() const noexcept {
  return time_in_force_;
}

const std::optional<std::time_t>
&iridium::TriggerOrder::gtd_time() const noexcept {
  return gtd_time_;
}

iridium::OrderTriggerCondition
iridium::TriggerOrder::order_trigger_condition() const noexcept {
  return order_trigger_condition_;
}

iridium::PriceTriggerOrder::PriceTriggerOrder(
    double price,
    std::time_t create_time,
    const std::string &trade_id,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time,
    iridium::OrderTriggerCondition order_trigger_condition)
    : TriggerOrder(
    create_time,
    trade_id,
    time_in_force,
    gtd_time,
    order_trigger_condition),
      price_(price) {}

double iridium::PriceTriggerOrder::price() const {
  return price_;
}

void iridium::PriceTriggerOrder::set_price(double d) {
  this->price_ = d;
}

iridium::DistanceTriggerOrder::DistanceTriggerOrder(
    double distance,
    time_t create_time,
    const std::string &trade_id,
    double trade_price,
    bool is_short,
    iridium::TimeInForce time_in_force,
    const std::optional<std::time_t> &gtd_time,
    iridium::OrderTriggerCondition order_trigger_condition)
    : TriggerOrder(
    create_time,
    trade_id,
    time_in_force,
    gtd_time,
    order_trigger_condition),
      distance_(distance),
      is_short_(is_short),
      trailing_stop_price_(trade_price - (is_short ? -distance : distance)) {}

double iridium::DistanceTriggerOrder::distance() const {
  return distance_;
}

void iridium::DistanceTriggerOrder::set_distance(double distance) {
  this->distance_ = distance;
}

void iridium::DistanceTriggerOrder::UpdateTrailingStopPrice(double currentPrice) {
  trailing_stop_price_ = currentPrice - (is_short_ ? -distance_ : distance_);
}

double iridium::DistanceTriggerOrder::trailing_stop_price() const {
  return trailing_stop_price_;
}

std::string iridium::OrderStateToString(iridium::OrderState state) {
  switch (state) {
    case kPending:return "PENDING";
    case kFilled:return "FILLED";
    case kTriggered:return "TRIGGERED";
    case kCancelled:return "CANCELLED";
  }
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::MarketOrderRequest &request) {
  os << "id: " << request.request_id_
     << " create time: " << TimeToLocalTimeString(request.create_time_)
     << " instrument: " << request.instrument_ptr_->name()
     << " units: " << request.units_
     << " price: " << request.market_price_
     << " stop loss: " << request.stop_loss_details_ptr_->price()
     << " take profit: " << request.take_profit_details_ptr_->price()
     << " trailing stop: " << request.trailing_stop_loss_details_ptr_->distance();
  return os;
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::PriceTriggerOrder &order) {
  os << "id: " << order.order_id()
     << " state: " << OrderStateToString(order.order_state())
     << " create time: " << TimeToLocalTimeString(order.create_time())
     << " trade id: " << order.trade_id()
     << " trigger price: " << order.price();
  return os;
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::DistanceTriggerOrder &order) {
  os << "id: " << order.order_id()
     << " state: " << OrderStateToString(order.order_state())
     << " create time: " << TimeToLocalTimeString(order.create_time())
     << " trade id: " << order.trade_id()
     << " distance: " << order.distance_
     << " trailing stop loss price: " << order.trailing_stop_price_;
  return os;
}
