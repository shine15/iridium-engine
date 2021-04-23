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

iridium::SimulationAccount::SimulationAccount(
    const std::string &account_currency,
    int leverage,
    double capital_base) :
    account_currency_(account_currency),
    leverage_(leverage),
    capital_base_(capital_base),
    balance_(capital_base),
    trades_ptr_(std::make_shared<TradeList>()) {
}

void iridium::SimulationAccount::CreateMarketOrder(
    std::time_t create_time,
    double margin_available,
    const std::string &instrument,
    int units,
    double market_price,
    double account_quote_rate,
    double account_base_rate,
    double spread,
    std::optional<double> take_profit_price,
    std::optional<double> stop_loss_price,
    std::optional<double> trailing_stop_loss_distance,
    double financing,
    double commission) {
  auto request_ptr = std::make_shared<MarketOrderRequest>(
      create_time,
      instrument,
      units,
      market_price,
      take_profit_price,
      stop_loss_price,
      trailing_stop_loss_distance);
  auto existing_trades_ptr = open_trades_ptr(instrument);
  auto order_units = request_ptr->units();
  auto existing_units = 0;
  for_each(existing_trades_ptr->begin(),
           existing_trades_ptr->end(),
           [&existing_units](const auto &trade) {
             existing_units += trade->current_units();
           });
  if (order_units * existing_units < 0) {
    for (auto &trade : *existing_trades_ptr) {
      if (abs(order_units) >= abs(trade->current_units())) {
        order_units += trade->current_units();
        UpdateBalance(trade->CloseTrade(
            account_quote_rate,
            market_price,
            create_time));
      } else {
        UpdateBalance(trade->PartiallyCloseTrade(
            account_quote_rate,
            market_price,
            order_units));
        order_units = 0;
      }
    }
  }
  if (order_units == 0) {
    return;
  }
  auto initial_margin = CalculateMarginUsed(
      abs(order_units),
      account_base_rate,
      leverage());
  if (margin_available >= initial_margin) {
    auto trade_ptr = std::make_shared<Trade>(
        instrument,
        market_price,
        create_time,
        order_units,
        initial_margin,
        spread,
        financing,
        commission,
        take_profit_price,
        stop_loss_price,
        trailing_stop_loss_distance);
    trades_ptr_->push_back(trade_ptr);
  }
}

void iridium::SimulationAccount::PartiallyCloseTrade(
    const std::shared_ptr<Trade> &trade_ptr,
    double rate,
    double current_price,
    int units) {
  auto profit_loss = trade_ptr->PartiallyCloseTrade(
      rate,
      current_price,
      units);
  balance_ += profit_loss;
}

void iridium::SimulationAccount::CloseTrade(
    const std::shared_ptr<Trade> &trade_ptr,
    double rate,
    double current_price,
    std::time_t time) {
  auto profit_loss = trade_ptr->CloseTrade(
      rate,
      current_price,
      time);
  balance_ += profit_loss;
}

void iridium::SimulationAccount::CloserPosition(
    const std::string &instrument,
    double rate,
    double current_price,
    std::time_t time) {
  auto trades_ptr = open_trades_ptr(instrument);
  for (const auto &trade_ptr : *trades_ptr) {
    CloseTrade(trade_ptr, rate, current_price, time);
  }
}

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
        return trade->instrument()->name() == instrument &&
            trade->trade_state() == TradeState::kOpen;
      });
  return open_trades_ptr;
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

void iridium::SimulationAccount::UpdateBalance(double profit_loss) {
  balance_ += profit_loss;
}

int iridium::SimulationAccount::position_size(const std::string &instrument) const {
  auto trades_ptr = open_trades_ptr(instrument);
  auto size = 0;
  for (const auto &trade_ptr : *trades_ptr) {
    size += trade_ptr->current_units();
  }
  return size;
}
const std::shared_ptr<iridium::TradeList> &iridium::SimulationAccount::all_trades_ptr() const {
  return trades_ptr_;
}

std::optional<double>
iridium::SimulationAccount::net_asset_value(const iridium::data::TickDataMap &tick_data_map) const {
  auto net_asset_value = balance();
  for (const auto &trade : *open_trades_ptr()) {
    auto current_data = tick_data_map.at(trade->instrument()->name());
    auto quote = trade->instrument()->quote_name();
    auto account_quote_rate = data::account_currency_rate(
        account_currency(),
        quote,
        tick_data_map);
    if (current_data.has_value() && account_quote_rate.has_value()) {
      auto unrealized_profit_loss = CalculateUnrealizedProfitLoss(
          *trade,
          account_quote_rate.value(),
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
  for (const auto &trade : *open_trades_ptr()) {
    auto current_data = tick_data_map.at(trade->instrument()->name());
    auto base = trade->instrument()->base_name();
    auto account_base_rate = data::account_currency_rate(
        account_currency(),
        base,
        tick_data_map);
    if (current_data.has_value() && account_base_rate.has_value()) {
      margin_used += CalculateMarginUsed(
          *trade,
          account_base_rate.value(),
          leverage());
    } else {
      return std::nullopt;
    }
  }
  return margin_used;
}

bool iridium::SimulationAccount::HasOpenTrades(const std::string &instrument) const {
  return !open_trades_ptr(instrument)->empty();
}

void iridium::ProcessTriggerOrders(
    const std::shared_ptr<SimulationAccount> &account_ptr,
    std::time_t time,
    const data::TickDataMap &tick_data_map) {
  auto open_trades_ptr = account_ptr->open_trades_ptr();
  for (const auto &trade_ptr : *open_trades_ptr) {
    auto instrument = trade_ptr->instrument();
    auto quote = instrument->quote_name();
    auto current_data = tick_data_map.at(instrument->name());
    auto account_quote_rate = data::account_currency_rate(
        account_ptr->account_currency(),
        quote,
        tick_data_map);
    if (current_data.has_value() && account_quote_rate.has_value()) {
      auto account_quote_rate_value = account_quote_rate.value();
      if (auto order_ptr = trade_ptr->take_profit_order_ptr()) {
        ProcessTakeProfitOrder(
            order_ptr,
            account_ptr,
            trade_ptr,
            time,
            account_quote_rate_value,
            *current_data);
      }
      if (auto order_ptr = trade_ptr->stop_loss_order_ptr()) {
        ProcessStopLossOrder(
            order_ptr,
            account_ptr,
            trade_ptr,
            time,
            account_quote_rate_value,
            *current_data);
      }
      if (auto order_ptr = trade_ptr->trailing_stop_loss_order_ptr()) {
        ProcessTrailingStopLossOrder(
            order_ptr,
            account_ptr,
            trade_ptr,
            time,
            account_quote_rate_value,
            *current_data);
      }
    }
  }
}

void
iridium::ProcessStopLossOrder(
    const std::shared_ptr<StopLossOrder> &order_ptr,
    const std::shared_ptr<SimulationAccount> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    const iridium::data::Candlestick &current_data) {
  auto current_low = current_data.low;
  auto current_high = current_data.high;
  auto stop_loss_price = order_ptr->price();
  if (stop_loss_price >= current_low && stop_loss_price <= current_high) {
    order_ptr->set_order_state(OrderState::kTriggered);
    account_ptr->UpdateBalance(trade_ptr->CloseTrade(rate, order_ptr->price(), time));
  }
}

void iridium::ProcessTakeProfitOrder(
    const std::shared_ptr<TakeProfitOrder> &order_ptr,
    const std::shared_ptr<SimulationAccount> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    const iridium::data::Candlestick &current_data) {
  auto current_low = current_data.low;
  auto current_high = current_data.high;
  auto stop_loss_price = order_ptr->price();
  if (stop_loss_price >= current_low && stop_loss_price <= current_high) {
    order_ptr->set_order_state(OrderState::kTriggered);
    account_ptr->UpdateBalance(trade_ptr->CloseTrade(rate, order_ptr->price(), time));
  }
}

void iridium::ProcessTrailingStopLossOrder(
    const std::shared_ptr<TrailingStopLossOrder> &order_ptr,
    const std::shared_ptr<SimulationAccount> &account_ptr,
    const std::shared_ptr<Trade> &trade_ptr,
    std::time_t time,
    double rate,
    const iridium::data::Candlestick &current_data) {
  auto current_low = current_data.low;
  auto current_high = current_data.high;
  auto current_price = current_data.close;
  auto is_short = (trade_ptr->current_units() < 0);
  auto trailing_stop_loss_price = order_ptr->trailing_stop_price();
  auto distance = order_ptr->distance();
  if (trailing_stop_loss_price >= current_low && trailing_stop_loss_price <= current_high) {
    order_ptr->set_order_state(OrderState::kTriggered);
    account_ptr->UpdateBalance(trade_ptr->CloseTrade(rate, trailing_stop_loss_price, time));
  }
  if ((is_short && (trailing_stop_loss_price - current_price > distance))
      || (!is_short && (current_price - trailing_stop_loss_price > distance))) {
    order_ptr->UpdateTrailingStopPrice(current_price);
  }
}

void iridium::SimulationAccount::PrintAccountInfo(std::time_t tick, const iridium::data::TickDataMap &data_map) const {
  auto nav = net_asset_value(data_map);
  auto account_margin_used = margin_used(data_map);
  if (nav.has_value() && account_margin_used.has_value()) {
    auto margin_available = iridium::CalculateMarginAvailable(nav.value(), account_margin_used.value());
    std::cout << "time: " << TimeToLocalTimeString(tick)
              << " NAV: " << nav.value()
              << " Balance: " << balance()
              << " Margin Used: " << account_margin_used.value()
              << " Margin Available: " << margin_available
              << std::endl;
  }
}

std::ostream &iridium::operator<<(std::ostream &os, const iridium::SimulationAccount &account) {
  os << "Balance: " << account.balance_ << std::endl
     << "Capital Base: " << account.capital_base_ << std::endl
     << "Account Currency: " << account.account_currency_ << std::endl
     << "Leverage: " << account.leverage_ << std::endl;
  auto all_trades_ptr = account.all_trades_ptr();
  for (const auto &trade_ptr : *all_trades_ptr) {
    os << "Trade: " << std::endl
       << *(trade_ptr) << std::endl;
  }
  return os;
}




