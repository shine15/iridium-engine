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

#include <iridium/trade.hpp>

iridium::Trade::Trade(
    const std::string &instrument,
    double price,
    std::time_t open_time,
    int initial_units,
    double initial_margin,
    double spread,
    double financing,
    double commission,
    std::shared_ptr<TakeProfitOrder> take_profit_order_ptr,
    std::shared_ptr<StopLossOrder> stop_loss_order_ptr,
    std::shared_ptr<TrailingStopLossOrder> trailing_stop_loss_order_ptr) :
    trade_id_(boost::uuids::to_string(boost::uuids::random_generator()())),
    instrument_ptr_(std::make_shared<iridium::Instrument>(instrument)),
    price_(price),
    state_(TradeState::kOpen),
    open_time_(open_time),
    initial_units_(initial_units),
    initial_margin_(initial_margin),
    current_units_(initial_units),
    realized_profit_loss_(0.00),
    close_time_(std::nullopt),
    close_price_(std::nullopt),
    spread_(spread),
    financing_(financing),
    commission_(commission),
    take_profit_order_ptr_(std::move(std::move(std::move(take_profit_order_ptr)))),
    stop_loss_order_ptr_(std::move(stop_loss_order_ptr)),
    trailing_stop_loss_order_ptr_(std::move(trailing_stop_loss_order_ptr)) {}

iridium::Trade::Trade(
    const std::string &instrument,
    double price,
    std::time_t open_time,
    int initial_units,
    double initial_margin,
    double spread,
    double financing,
    double commission,
    std::optional<double> take_profit_price,
    std::optional<double> stop_loss_price,
    std::optional<double> trailing_stop_distance) :
    trade_id_(boost::uuids::to_string(boost::uuids::random_generator()())),
    instrument_ptr_(std::make_shared<iridium::Instrument>(instrument)),
    price_(price),
    state_(TradeState::kOpen),
    open_time_(open_time),
    initial_units_(initial_units),
    initial_margin_(initial_margin),
    current_units_(initial_units),
    realized_profit_loss_(0.00),
    close_time_(std::nullopt),
    close_price_(std::nullopt),
    spread_(spread),
    financing_(financing),
    commission_(commission),
    take_profit_order_ptr_(
        take_profit_price.has_value()
        ? std::make_shared<iridium::TakeProfitOrder>(
            take_profit_price.value(),
            open_time,
            trade_id_)
        : std::shared_ptr<iridium::TakeProfitOrder>(nullptr)),
    stop_loss_order_ptr_(
        stop_loss_price.has_value()
        ? std::make_shared<iridium::StopLossOrder>(
            stop_loss_price.value(),
            open_time,
            trade_id_)
        : std::shared_ptr<iridium::StopLossOrder>(nullptr)),
    trailing_stop_loss_order_ptr_(
        trailing_stop_distance.has_value()
        ? std::make_shared<iridium::TrailingStopLossOrder>(
            trailing_stop_distance.value(),
            open_time,
            trade_id_,
            price,
            initial_units < 0)
        : std::shared_ptr<iridium::TrailingStopLossOrder>(nullptr)) {}

const std::string &iridium::Trade::trade_id() const noexcept {
  return trade_id_;
}

const std::shared_ptr<iridium::Instrument>
&iridium::Trade::instrument() const noexcept {
  return instrument_ptr_;
}

double iridium::Trade::price() const noexcept {
  return price_;
}

std::optional<double> iridium::Trade::close_price() const noexcept {
  return close_price_;
}

iridium::TradeState iridium::Trade::trade_state() const noexcept {
  return state_;
}

std::time_t iridium::Trade::open_time() const noexcept {
  return open_time_;
}

int iridium::Trade::initial_units() const noexcept {
  return initial_units_;
}

double iridium::Trade::initial_margin() const noexcept {
  return initial_margin_;
}

int iridium::Trade::current_units() const noexcept {
  return current_units_;
}

double iridium::Trade::realized_profit_loss() const noexcept {
  return realized_profit_loss_;
}

const std::optional<std::time_t>
&iridium::Trade::close_time() const noexcept {
  return close_time_;
}

double iridium::Trade::spread() const noexcept {
  return spread_;
}

double iridium::Trade::financing() const noexcept {
  return financing_;
}

double iridium::Trade::commission() const noexcept {
  return commission_;
}

const std::shared_ptr<iridium::TakeProfitOrder>
&iridium::Trade::take_profit_order_ptr() const noexcept {
  return take_profit_order_ptr_;
}

const std::shared_ptr<iridium::StopLossOrder>
&iridium::Trade::stop_loss_order_ptr() const noexcept {
  return stop_loss_order_ptr_;
}

const std::shared_ptr<iridium::TrailingStopLossOrder>
&iridium::Trade::trailing_stop_loss_order_ptr() const noexcept {
  return trailing_stop_loss_order_ptr_;
}

std::optional<double>
iridium::Trade::take_profit_price() const noexcept {
  if (this->take_profit_order_ptr_) {
    return this->take_profit_order_ptr_->price();
  } else {
    return std::nullopt;
  }
}

std::optional<double>
iridium::Trade::stop_loss_price() const noexcept {
  if (this->stop_loss_order_ptr_) {
    return this->stop_loss_order_ptr_->price();
  } else {
    return std::nullopt;
  }
}

std::optional<double>
iridium::Trade::trailing_stop_distance() const noexcept {
  if (this->trailing_stop_loss_order_ptr_) {
    return this->trailing_stop_loss_order_ptr_->distance();
  } else {
    return std::nullopt;
  }
}

std::optional<double> iridium::Trade::trailing_stop_price() const noexcept {
  if (this->trailing_stop_loss_order_ptr_) {
    return this->trailing_stop_loss_order_ptr_->trailing_stop_price();
  } else {
    return std::nullopt;
  }
}

void iridium::Trade::set_take_profit_price(double price) {
  this->take_profit_order_ptr_->set_price(price);
}

void iridium::Trade::set_stop_loss_price(double price) {
  this->stop_loss_order_ptr_->set_price(price);
}

void iridium::Trade::set_trailing_stop_distance(double distance) {
  this->trailing_stop_loss_order_ptr_->set_distance(distance);
}

void iridium::Trade::UpdateTakeProfitOrder(double price, std::time_t open_time) {
  if (this->take_profit_order_ptr_) {
    this->set_take_profit_price(price);
  } else {
    auto order_ptr = std::make_shared<TakeProfitOrder>(price, open_time, trade_id_);
    this->set_take_profit_order_ptr(order_ptr);
  }
}

void iridium::Trade::UpdateStopLossOrder(double price, std::time_t open_time) {
  if (this->stop_loss_order_ptr_) {
    this->set_stop_loss_price(price);
  } else {
    auto order_ptr = std::make_shared<StopLossOrder>(price, open_time, trade_id_);
    this->set_stop_loss_order_ptr(order_ptr);
  }
}
void iridium::Trade::UpdateTrailingStopLossOrder(double distance, std::time_t open_time) {
  if (this->trailing_stop_loss_order_ptr_) {
    this->set_trailing_stop_distance(distance);
  } else {
    auto order_ptr = std::make_shared<TrailingStopLossOrder>(
        distance,
        open_time,
        trade_id_,
        price_,
        current_units_ < 0);
    this->set_trailing_stop_loss_order_ptr(order_ptr);
  }
}

double iridium::Trade::PartiallyCloseTrade(
    double rate,
    double current_price,
    int units) {
  auto trading_cost = iridium::CalculateGainsLosses(
      this->spread_,
      abs(units),
      rate,
      iridium::pip_point(*(this->instrument_ptr_))) + this->commission();
  auto profit_loss = (current_price - this->price()) * (1 / rate) * units - trading_cost;
  this->current_units_ -= units;
  this->realized_profit_loss_ += profit_loss;
  return profit_loss;
}

double iridium::Trade::CloseTrade(
    double rate,
    double current_price,
    std::time_t time) {
  auto profit_loss = PartiallyCloseTrade(rate, current_price, this->current_units_);
  this->close_price_ = current_price;
  this->close_time_ = time;
  if (auto order_ptr = this->take_profit_order_ptr_) {
    if (OrderState::kTriggered != order_ptr->order_state())
      this->take_profit_order_ptr_->set_order_state(OrderState::kCancelled);
  }

  if (auto order_ptr = this->stop_loss_order_ptr_) {
    if (OrderState::kTriggered != order_ptr->order_state())
      this->stop_loss_order_ptr_->set_order_state(OrderState::kCancelled);
  }
  if (auto order_ptr = this->trailing_stop_loss_order_ptr_) {
    if (OrderState::kTriggered != order_ptr->order_state())
      this->trailing_stop_loss_order_ptr_->set_order_state(OrderState::kCancelled);
  }

  this->state_ = TradeState::kClosed;
  return profit_loss;
}

void iridium::Trade::set_take_profit_order_ptr(
    const std::shared_ptr<TakeProfitOrder> &take_profit_order_ptr) {
  this->take_profit_order_ptr_ = take_profit_order_ptr;
}

void iridium::Trade::set_stop_loss_order_ptr(
    const std::shared_ptr<StopLossOrder> &stop_loss_order_ptr) {
  this->stop_loss_order_ptr_ = stop_loss_order_ptr;
}

void iridium::Trade::set_trailing_stop_loss_order_ptr(
    const std::shared_ptr<TrailingStopLossOrder> &trailing_stop_loss_order_ptr) {
  this->trailing_stop_loss_order_ptr_ = trailing_stop_loss_order_ptr;
}

double iridium::CalculateUnrealizedProfitLoss(
    const iridium::Trade &trade,
    double rate,
    double current_price) {
  auto cost = iridium::CalculateGainsLosses(
      trade.spread() / 2,
      abs(trade.current_units()),
      rate,
      iridium::pip_point(*(trade.instrument()))) + trade.commission();
  return (current_price - trade.price()) * (1 / rate) * trade.current_units() - cost;
}

double iridium::CalculateMarginUsed(
    const iridium::Trade &trade,
    double rate,
    int leverage) {
  return CalculateMarginUsed(trade.current_units(), rate, leverage);
}

std::string iridium::TradeStateToString(iridium::TradeState state) {
  switch (state) {
    case kOpen:return "OPEN";
    case kClosed:return "CLOSED";
  }
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::Trade &trade) {
  os << "id: " << trade.trade_id_
     << " instrument: " << trade.instrument()->name()
     << " price: " << trade.price_
     << " state: " << TradeStateToString(trade.state_)
     << " open time: " << TimeToLocalTimeString(trade.open_time_)
     << " initial units: " << trade.initial_units_
     << " initial margin: " << trade.initial_margin_
     << " current units: " << trade.current_units_
     << " realized profit loss: " << trade.realized_profit_loss_
     << " spread: " << trade.spread_
     << " close time: "
     << (trade.close_time_.has_value()
         ? TimeToLocalTimeString(trade.close_time_.value()) : std::string())
     << " close price: "
     << (trade.close_price_.has_value()
         ? std::to_string(trade.close_price_.value()) : std::string())
     << std::endl;
  if (trade.take_profit_order_ptr_) {
    os << "take profit order:" << std::endl
       << *trade.take_profit_order_ptr_;
  }
  if (trade.stop_loss_order_ptr_) {
    os << "stop loss order:" << std::endl
       << *trade.stop_loss_order_ptr_;
  }
  if (trade.trailing_stop_loss_order_ptr_) {
    os << "trailing stop loss order:" << std::endl
       << *trade.trailing_stop_loss_order_ptr_;
  }

  return os;
}
