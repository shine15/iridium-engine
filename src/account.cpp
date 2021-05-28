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

#include <iridium/account.hpp>

double iridium::SimulationAccount::balance() const {
  return balance_;
}

const std::string &iridium::SimulationAccount::account_currency() const {
  return account_currency_;
}

int iridium::SimulationAccount::leverage() const {
  return leverage_;
}

std::shared_ptr<iridium::TradeList>
iridium::SimulationAccount::open_trades_ptr(const std::string &instrument) const {
  auto open_trades_ptr = std::make_shared<TradeList>();
  std::copy_if(
      trades_ptr_->begin(),
      trades_ptr_->end(),
      std::back_inserter(*open_trades_ptr),
      [&instrument](const auto &trade) {
        return trade->instrument_ptr()->name() == instrument &&
            trade->trade_state() == TradeState::kOpen;
      });
  return open_trades_ptr;
}

std::shared_ptr<iridium::LimitOrderList>
iridium::SimulationAccount::pending_limit_orders_ptr(
    const std::string &instrument) const {
  auto pending_orders_ptr = std::make_shared<LimitOrderList>();
  for (const auto &order_ptr : *orders_ptr_) {
    if (auto limit_order_ptr = std::dynamic_pointer_cast<LimitOrder>(order_ptr)) {
      if ((limit_order_ptr->instrument_ptr()->name() == instrument) &&
          (limit_order_ptr->order_state() == OrderState::kPending)) {
        pending_orders_ptr->push_back(limit_order_ptr);
      }
    }
  }
  return pending_orders_ptr;
}

int iridium::SimulationAccount::open_position_size(const std::string &instrument) const {
  auto trades_ptr = open_trades_ptr(instrument);
  auto size = 0;
  for (const auto &trade_ptr : *trades_ptr) {
    size += trade_ptr->current_units();
  }
  return size;
}

std::optional<double>
iridium::SimulationAccount::net_asset_value(const iridium::data::TickDataMap &tick_data_map) const {
  auto net_asset_value = balance();
  auto open_trades = open_trades_ptr();
  for (const auto &trade : *open_trades) {
    auto current_data = tick_data_map.at(trade->instrument_ptr()->name());
    auto quote = trade->instrument_ptr()->quote_name();
    auto acc_quote_rate = data::account_currency_rate(
        account_currency(),
        quote,
        tick_data_map);
    if (current_data.has_value() && acc_quote_rate.has_value()) {
      auto unrealized_profit_loss = CalculateUnrealizedProfitLoss(
          *trade,
          acc_quote_rate.value(),
          current_data->close);
      net_asset_value += unrealized_profit_loss;
    } else {
      return std::nullopt;
    }
  }
  return net_asset_value;
}

std::optional<double>
iridium::SimulationAccount::margin_used(const iridium::data::TickDataMap &tick_data_map) const {
  auto margin_used = 0.00;
  auto open_trades = open_trades_ptr();
  for (const auto &trade : *open_trades) {
    auto current_data = tick_data_map.at(trade->instrument_ptr()->name());
    auto base = trade->instrument_ptr()->base_name();
    auto acc_base_rate = data::account_currency_rate(
        account_currency(),
        base,
        tick_data_map);
    if (current_data.has_value() && acc_base_rate.has_value()) {
      margin_used += CalculateMarginUsed(
          *trade,
          acc_base_rate.value(),
          leverage());
    } else {
      return std::nullopt;
    }
  }
  return margin_used;
}

void iridium::SimulationAccount::CreateLimitOrder(
    std::time_t create_time,
    const std::string &instrument,
    int units,
    double price,
    std::optional<double> take_profit_price,
    std::optional<double> stop_loss_price,
    std::optional<double> trailing_stop_loss_distance) {
  auto order = std::make_shared<LimitOrder>(
      create_time,
      instrument,
      units,
      price,
      take_profit_price,
      stop_loss_price,
      trailing_stop_loss_distance);
  orders_ptr_->push_back(order);
}

void iridium::SimulationAccount::CloserPosition(
    const std::string &instrument,
    double acc_quote_rate,
    double current_price,
    std::time_t time) {
  auto trades_ptr = open_trades_ptr(instrument);
  for (const auto &trade_ptr : *trades_ptr) {
    CloseTrade(trade_ptr, acc_quote_rate, current_price, time);
  }
}

void iridium::SimulationAccount::UpdateTradeStopLossPrice(
    const std::shared_ptr<Trade> &trade_ptr,
    double stop_loss_price,
    std::time_t time) {
  trade_ptr->UpdateStopLossOrder(stop_loss_price, time);
}

void iridium::SimulationAccount::UpdateTradeTakeProfitPrice(
    const std::shared_ptr<Trade> &trade_ptr,
    double take_profit_price,
    std::time_t time) {
  trade_ptr->UpdateTakeProfitOrder(take_profit_price, time);
}

void iridium::SimulationAccount::UpdateTrailingStopDistance(
    const std::shared_ptr<Trade> &trade_ptr,
    double distance,
    std::time_t time) {
  trade_ptr->UpdateTrailingStopLossOrder(distance, time);
}

void iridium::SimulationAccount::CancelLimitOrder(
    const std::shared_ptr<LimitOrder> &order_ptr) {
  order_ptr->set_order_state(OrderState::kCancelled);
}

bool iridium::SimulationAccount::HasOpenTrades(const std::string &instrument) const {
  return !open_trades_ptr(instrument)->empty();
}

bool iridium::SimulationAccount::HasPendingOrders(const std::string &instrument) const {
  return !pending_limit_orders_ptr(instrument)->empty();
}

iridium::SimulationAccount::SimulationAccount(
    const std::string &account_currency,
    int leverage,
    double capital_base,
    double spread) :
    account_currency_(account_currency),
    leverage_(leverage),
    capital_base_(capital_base),
    balance_(capital_base),
    spread_(spread),
    trades_ptr_(std::make_shared<TradeList>()),
    orders_ptr_(std::make_shared<OrderList>()),
    logger_(iridium::logger()) {
}

std::shared_ptr<iridium::TradeList> iridium::SimulationAccount::trades_ptr() const {
  return trades_ptr_;
}

std::string iridium::SimulationAccount::string() {
  std::ostringstream ss;
  ss << *this;
  return ss.str();
}

std::string iridium::SimulationAccount::summary(std::time_t tick, const iridium::data::TickDataMap &data_map) const {
  auto nav = net_asset_value(data_map);
  auto account_margin_used = margin_used(data_map);
  if (nav.has_value() && account_margin_used.has_value()) {
    auto margin_available = iridium::CalculateMarginAvailable(nav.value(), account_margin_used.value());
    auto summary = ("time: " +  TimeToLocalTimeString(tick) +
        " NAV: " + std::to_string(nav.value()) +
        " Balance: " + std::to_string(balance()) +
        " Margin Used: " +  std::to_string(account_margin_used.value()) +
        " Margin Available: " + std::to_string(margin_available));
    return summary;
  }
  return std::string();
}

void
iridium::SimulationAccount::ProcessOrders(
    std::time_t time,
    const iridium::data::TickDataMap &tick_data_map) {
  auto acc_pending_orders = pending_orders_ptr();
  for (const auto &order_ptr : *acc_pending_orders) {
    if (auto limit_order_ptr = std::dynamic_pointer_cast<LimitOrder>(order_ptr)) {
      ProcessLimitOrder(limit_order_ptr, time, tick_data_map);
    } else if (auto price_trigger_order_ptr = std::dynamic_pointer_cast<TriggerOrder>(order_ptr)) {
      ProcessTriggerOrder(price_trigger_order_ptr, time, tick_data_map);
    }
  }
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::SimulationAccount &account) {
  os << "Balance: " << account.balance_ << std::endl
     << "Capital Base: " << account.capital_base_ << std::endl
     << "Account Currency: " << account.account_currency_ << std::endl
     << "Leverage: " << account.leverage_ << std::endl;
  auto all_trades_ptr = account.trades_ptr_;
  for (const auto &trade_ptr : *all_trades_ptr) {
    os << "Trade: " << std::endl
       << *(trade_ptr) << std::endl;
  }
  return os;
}

std::shared_ptr<iridium::TradeList> iridium::SimulationAccount::open_trades_ptr() const {
  auto open_trades_ptr = std::make_shared<TradeList>();
  std::copy_if(
      trades_ptr_->begin(),
      trades_ptr_->end(),
      std::back_inserter(*open_trades_ptr),
      [](const auto &trade) {
        return trade->trade_state() == TradeState::kOpen;
      });
  return open_trades_ptr;
}

std::shared_ptr<iridium::OrderList> iridium::SimulationAccount::pending_orders_ptr() const {
  auto pending_orders_ptr = std::make_shared<OrderList>();
  std::copy_if(
      orders_ptr_->begin(),
      orders_ptr_->end(),
      std::back_inserter(*pending_orders_ptr),
      [](const auto &order) {
        return order->order_state() == OrderState::kPending;
      });
  return pending_orders_ptr;
}

std::shared_ptr<iridium::Trade>
iridium::SimulationAccount::acc_trade_ptr(
    const iridium::TriggerOrder &order) const {
  auto trades_ptr = std::make_shared<TradeList>();
  std::copy_if(
      trades_ptr_->begin(),
      trades_ptr_->end(),
      std::back_inserter(*trades_ptr),
      [order](const auto &trade) {
        return trade->trade_id() == order.trade_id();
      });
  return trades_ptr->front();
}

std::optional<std::tuple<std::string, double, double, double, double, double, double, double>>
iridium::SimulationAccount::instrument_market_info(
    const iridium::Instrument &instrument,
    const iridium::data::TickDataMap &tick_data_map) {
  auto instrument_name = instrument.name();
  auto base = instrument.base_name();
  auto quote = instrument.quote_name();
  // price data
  auto data = tick_data_map.at(instrument_name);
  auto acc_quote_rate = data::account_currency_rate(
      account_currency(),
      quote,
      tick_data_map);
  auto acc_base_rate = iridium::data::account_currency_rate(
      account_currency(),
      base,
      tick_data_map);
  if (data.has_value() &&
      acc_quote_rate.has_value() &&
      acc_base_rate.has_value()) {
    auto low = data.value().low;
    auto high = data.value().high;
    auto current_price = data.value().close;
    auto spread_value = this->spread_ * pow(10, -pip_point(instrument));
    // ask range
    auto ask_low = low + spread_value / 2.0;
    auto ask_high = high + spread_value / 2.0;
    // bid range
    auto bid_low = low - spread_value / 2.0;
    auto bid_high = high - spread_value / 2.0;
    // account vs quote && account vs base
    auto acc_quote_rate_value = acc_quote_rate.value();
    auto acc_base_rate_value = acc_base_rate.value();
    return std::make_tuple(
        instrument_name,
        ask_low,
        ask_high,
        bid_low,
        bid_high,
        acc_quote_rate_value,
        acc_base_rate_value,
        current_price);
  } else {
    return std::nullopt;
  }
}

void iridium::SimulationAccount::PartiallyCloseTrade(
    const std::shared_ptr<Trade> &trade_ptr,
    double acc_quote_rate,
    double current_price,
    int units) {
  auto instrument = trade_ptr->instrument_ptr();
  auto spread_value = this->spread_ * pow(10, -pip_point(*instrument));
  auto current_ask = current_price + spread_value / 2.0;
  auto current_bid = current_price - spread_value / 2.0;
  auto profit_loss = trade_ptr->PartiallyCloseTrade(
      acc_quote_rate,
      trade_ptr->current_units() > 0 ? current_bid : current_ask,
      units);
  balance_ += profit_loss;
}

void iridium::SimulationAccount::CloseTrade(
    const std::shared_ptr<Trade> &trade_ptr,
    double acc_quote_rate,
    double current_price,
    std::time_t time) {
  auto instrument = trade_ptr->instrument_ptr();
  auto spread_value = this->spread_ * pow(10, -pip_point(*instrument));
  auto current_ask = current_price + spread_value / 2.0;
  auto current_bid = current_price - spread_value / 2.0;
  auto profit_loss = trade_ptr->CloseTrade(
      acc_quote_rate,
      trade_ptr->current_units() > 0 ? current_bid : current_ask,
      time);
  balance_ += profit_loss;
}

void
iridium::SimulationAccount::ProcessLimitOrder(
    const std::shared_ptr<LimitOrder> &order_ptr,
    std::time_t time,
    const iridium::data::TickDataMap &tick_data_map) {
  // instrument info
  auto instrument = order_ptr->instrument_ptr();
  auto instrument_info = instrument_market_info(*instrument, tick_data_map);
  if (instrument_info.has_value()) {
    auto[instrument_name,
    ask_low,
    ask_high,
    bid_low,
    bid_high,
    acc_quote_rate,
    acc_base_rate, _] = instrument_info.value();
    // order units & price
    auto order_units = order_ptr->units();
    auto order_price = order_ptr->price();
    // limit order trigger condition
    if ((order_price >= bid_low && order_price <= bid_high && order_units < 0) ||
        (order_price >= ask_low && order_price <= ask_high && order_units > 0)) {
      auto existing_trades_ptr = open_trades_ptr(instrument_name);
      auto existing_units = open_position_size(instrument_name);
      // handle existing positions
      if (order_units * existing_units < 0) {
        for (auto &trade : *existing_trades_ptr) {
          if (abs(order_units) >= abs(trade->current_units())) {
            order_units += trade->current_units();
            CloseTrade(trade, acc_quote_rate, order_price, time);
          } else {
            PartiallyCloseTrade(trade, acc_quote_rate, order_price, order_units);
            order_units = 0;
          }
        }
      }
      if (order_units == 0) {
        return;
      }
      auto initial_margin = CalculateMarginUsed(
          abs(order_units),
          acc_base_rate,
          leverage());
      auto nav = net_asset_value(tick_data_map);
      auto account_margin_used = margin_used(tick_data_map);
      if (nav.has_value() && account_margin_used.has_value()) {
        auto margin_available = CalculateMarginAvailable(nav.value(), account_margin_used.value());
        if (margin_available >= initial_margin) {
          auto take_profit_price = order_ptr->take_profit_price();
          auto stop_loss_price = order_ptr->stop_loss_price();
          auto trailing_stop_loss_distance = order_ptr->trailing_stop_loss_distance();
          auto trade = std::make_shared<Trade>(
              instrument_name,
              order_price,
              time,
              order_units,
              initial_margin,
              take_profit_price,
              stop_loss_price,
              trailing_stop_loss_distance);
          order_ptr->set_order_state(OrderState::kFilled);
          trades_ptr_->push_back(trade);
          auto stop_loss_order_ptr = trade->stop_loss_order_ptr();
          auto take_profit_order_ptr = trade->take_profit_order_ptr();
          auto trailing_stop_loss_order_ptr = trade->trailing_stop_loss_order_ptr();
          if (stop_loss_order_ptr) {
            orders_ptr_->push_back(stop_loss_order_ptr);
          }
          if (take_profit_order_ptr) {
            orders_ptr_->push_back(take_profit_order_ptr);
          }
          if (trailing_stop_loss_order_ptr) {
            orders_ptr_->push_back(trailing_stop_loss_order_ptr);
          }
          logger_->info(
              "limit order filled - instrument: {}, time: {}, units: {}, order price: {}, take profit price: {}, stop loss price: {}",
              instrument_name,
              TimeToLocalTimeString(time),
              order_units,
              order_price,
              take_profit_price.has_value() ? std::to_string(take_profit_price.value()) : "",
              stop_loss_price.has_value() ? std::to_string(stop_loss_price.value()) : "");
        }
      }
    }
  }
}

void
iridium::SimulationAccount::ProcessTriggerOrder(
    const std::shared_ptr<TriggerOrder> &order_ptr,
    std::time_t time,
    const iridium::data::TickDataMap &tick_data_map) {
  // instrument info
  auto trade_ptr = acc_trade_ptr(*order_ptr);
  auto instrument = trade_ptr->instrument_ptr();
  auto instrument_info = instrument_market_info(*instrument, tick_data_map);
  if (instrument_info.has_value()) {
    auto[instrument_name,
    ask_low,
    ask_high,
    bid_low,
    bid_high,
    acc_quote_rate,
    acc_base_rate,
    current_price] = instrument_info.value();
    if (auto price_trigger_order_ptr = std::dynamic_pointer_cast<PriceTriggerOrder>(order_ptr)) {
      ProcessPriceTriggerOrder(
          price_trigger_order_ptr,
          trade_ptr,
          ask_low,
          ask_high,
          bid_low,
          bid_high,
          acc_quote_rate,
          time);
    } else if (auto trailing_stop_loss_order_ptr = std::dynamic_pointer_cast<TrailingStopLossOrder>(order_ptr)) {
      ProcessTrailingStopLossOrder(
          trailing_stop_loss_order_ptr,
          trade_ptr,
          ask_low,
          ask_high,
          bid_low,
          bid_high,
          acc_quote_rate,
          current_price,
          time);
    }
  }
}

void
iridium::SimulationAccount::ProcessPriceTriggerOrder(
    const std::shared_ptr<PriceTriggerOrder> &order_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    double ask_low,
    double ask_high,
    double bid_low,
    double bid_high,
    double acc_quote_rate,
    std::time_t time) {
  auto trade_units = trade_ptr->current_units();
  auto order_price = order_ptr->price();
  if ((order_price >= bid_low && order_price <= bid_high && trade_units > 0) ||
      (order_price >= ask_low && order_price <= ask_high && trade_units < 0)) {
    order_ptr->set_order_state(OrderState::kTriggered);
    auto profit_loss = trade_ptr->CloseTrade(
        acc_quote_rate,
        order_price,
        time);
    balance_ += profit_loss;
    logger_->info(
        "price order triggered - instrument: {}, time: {}, units: {}, order price: {}",
        trade_ptr->instrument_ptr()->name(),
        TimeToLocalTimeString(time),
        trade_units,
        order_price);
  }
}

void
iridium::SimulationAccount::ProcessTrailingStopLossOrder(
    const std::shared_ptr<TrailingStopLossOrder> &order_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    double ask_low,
    double ask_high,
    double bid_low,
    double bid_high,
    double acc_quote_rate,
    double current_price,
    std::time_t time) {
  auto trade_units = trade_ptr->current_units();
  auto distance = order_ptr->distance();
  auto trailing_stop_loss_price = order_ptr->trailing_stop_price();
  if ((trailing_stop_loss_price >= bid_low && trailing_stop_loss_price <= bid_high && trade_units > 0) ||
      (trailing_stop_loss_price >= ask_low && trailing_stop_loss_price <= ask_high && trade_units < 0)) {
    order_ptr->set_order_state(OrderState::kTriggered);
    auto profit_loss = trade_ptr->CloseTrade(
        acc_quote_rate,
        trailing_stop_loss_price,
        time);
    balance_ += profit_loss;
  }
  if ((trade_units < 0 && (trailing_stop_loss_price - current_price > distance))
      || (trade_units > 0 && (current_price - trailing_stop_loss_price > distance))) {
    order_ptr->UpdateTrailingStopPrice(current_price);
  }
}



