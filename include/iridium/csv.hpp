//
// Created by Evan Su on 21/1/21.
//

#ifndef IRIDIUM_INCLUDE_IRIDIUM_CSV_HPP_
#define IRIDIUM_INCLUDE_IRIDIUM_CSV_HPP_

#include <string>
#include <fstream>
#include <iostream>

namespace iridium::data {
class CSV {
 public:
  CSV(const std::string &file_name) {
    file_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file_.open(file_name, std::fstream::trunc);
  }

  ~CSV() {
    file_.close();
  }

  template<class U, class... T>
  void WriteRow(const U &u, const T &... args) {
    const std::string sep = ",";
    const std::string end = "\n";
    ((file_ << u) << ... << (file_ << sep, args)) << end;
  }

 private:
  std::ofstream file_;

};

void GenerateTransactionsReport(const SimulationAccount &account, const std::string &csv_file_path) {
  auto csv_ptr = std::make_unique<iridium::data::CSV>(csv_file_path);
  auto all_trades_ptr = account.all_trades_ptr();
  csv_ptr->WriteRow(
      "instrument",
      "state",
      "realized_profit_loss",
      "open_time_local",
      "close_time_local",
      "trade_open_price",
      "trade_close_price",
      "stop_loss_price",
      "take_profit_price",
      "trailing_stop_distance",
      "trailing_stop_price",
      "initial_units",
      "initial_margin",
      "current_units",
      "open_time_timestamp",
      "close_time_timestamp",
      "spread"
  );
  for (const auto &trade_ptr : *all_trades_ptr) {
    auto round_decimal = pip_point(*trade_ptr->instrument()) + 1;
    auto instrument_name = trade_ptr->instrument()->name();
    auto trade_state = TradeStateToString(trade_ptr->trade_state());
    auto realized_profit_loss = To_String_With_Precision(trade_ptr->realized_profit_loss(), 2);
    auto trade_open_time_local = TimeToLocalTimeString(trade_ptr->open_time());
    auto trade_open_price = To_String_With_Precision(trade_ptr->price(), round_decimal);
    auto trade_close_time_local = trade_ptr->close_time().has_value()
        ? TimeToLocalTimeString(trade_ptr->close_time().value()) : std::string();
    auto trade_close_price = trade_ptr->close_price().has_value()
        ? To_String_With_Precision(trade_ptr->close_price().value(), round_decimal) : std::string();
    auto take_profit_price = trade_ptr->take_profit_price().has_value()
        ? To_String_With_Precision(trade_ptr->take_profit_price().value(), round_decimal) : std::string();
    auto stop_loss_price = trade_ptr->stop_loss_price().has_value()
        ? To_String_With_Precision(trade_ptr->stop_loss_price().value(), round_decimal) : std::string();
    auto trailing_stop_distance = trade_ptr->trailing_stop_distance().has_value()
        ? To_String_With_Precision(trade_ptr->trailing_stop_distance().value(), round_decimal) : std::string();
    auto trailing_stop_price = trade_ptr->trailing_stop_price().has_value()
        ? To_String_With_Precision(trade_ptr->trailing_stop_price().value(), round_decimal) : std::string();
    auto initial_units = trade_ptr->initial_units();
    auto initial_margin = trade_ptr->initial_margin();
    auto current_units = trade_ptr->current_units();
    auto spread = trade_ptr->spread();
    auto trade_open_time = trade_ptr->open_time();
    auto trade_close_time = trade_ptr->close_time().has_value() ? std::to_string(trade_ptr->close_time().value()) : std::string();
    csv_ptr->WriteRow(
        instrument_name,
        trade_state,
        realized_profit_loss,
        trade_open_time_local,
        trade_close_time_local,
        trade_open_price,
        trade_close_price,
        stop_loss_price,
        take_profit_price,
        trailing_stop_distance,
        trailing_stop_price,
        initial_units,
        initial_margin,
        current_units,
        trade_open_time,
        trade_close_time,
        spread
        );
  }

}
} // namespace iridium

#endif //IRIDIUM_INCLUDE_IRIDIUM_CSV_HPP_
