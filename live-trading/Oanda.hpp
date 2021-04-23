//
// Created by Evan Su on 20/3/21.
//

#ifndef IRIDIUM_LIVE_TRADING_OANDA_HPP_
#define IRIDIUM_LIVE_TRADING_OANDA_HPP_

#include <string>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <tuple>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/OAuth20Credentials.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <iridium/data.hpp>
#include <iridium/account.hpp>

namespace iridium {
class Oanda: public Account {
 public:
  enum Env { practice, live };

  struct Resp {
    Poco::Net::HTTPResponse::HTTPStatus status;
    std::string body;
  };

  struct TradeSummary {
    std::string trade_id;
    std::string instrument;
    double price;
    std::time_t open_time;
    std::string state;
    int initial_units;
    double initial_margin;
    int current_units;
    double realized_pl;
    double unrealized_pl;
    double margin_used;
    std::optional<double> close_price;
    std::optional<std::time_t> close_time;
    std::optional<std::string> take_profit_order_id;
    std::optional<std::string> stop_loss_order_id;
    std::optional<std::string> guaranteed_stop_loss_order_id;
    std::optional<std::string> trailing_stop_loss_order_id;
  };

  using TradeSummaryList = std::vector<std::shared_ptr<TradeSummary>>;

  struct AccountDetails {
    std::string currency;
    double balance;
    double nav;
    double margin_used;
    double margin_available;
    int leverage;
    std::shared_ptr<TradeSummaryList> trades;
  };

  Oanda(
      Env env,
      const std::string &token,
      const std::string &account_id);

  std::unique_ptr<Resp> SendRequest(
      const std::string &path,
      const std::string &http_method,
      const std::optional<std::map<std::string, std::string>> &query_params = std::nullopt,
      const std::optional<Poco::JSON::Object> &req_body = std::nullopt,
      const std::optional<std::map<std::string, std::string>> &headers = std::nullopt) const;

  [[nodiscard]]
  std::unique_ptr<iridium::data::DataList>
  instrument_data(
      const std::string &instrument_name,
      int count,
      iridium::data::DataFreq freq) const;

  [[nodiscard]]
  std::tuple<std::unique_ptr<iridium::data::DataListMap>, std::shared_ptr<iridium::data::TickDataMap>>
  trade_data(
      const InstrumentList &instruments,
      int count,
      iridium::data::DataFreq freq) const;

  [[nodiscard]]
  std::unique_ptr<std::map<std::string, double>>
  spread(const InstrumentList &instruments) const;

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
      double commission = 0.0) override;

  void CloserPosition(
      const std::string &instrument,
      double rate,
      double current_price,
      std::time_t time) override;

  void FetchAccountDetails();

  [[nodiscard]]
  double balance() const override;

  [[nodiscard]]
  const std::string &account_currency() const override;

  [[nodiscard]]
  int leverage() const override;

  [[nodiscard]]
  std::optional<double>
  net_asset_value(const data::TickDataMap &tick_data_map) const override;

  [[nodiscard]]
  std::optional<double>
  margin_used(const data::TickDataMap &tick_data_map) const override;

  bool HasOpenTrades(const std::string &instrument) const override;

  [[nodiscard]]
  std::shared_ptr<TradeSummaryList>
  open_trades_ptr(const std::string &instrument) const;

  int position_size(const std::string &instrument) const override;

  void PrintAccountInfo(std::time_t tick, const iridium::data::TickDataMap &data_map) const override;

 private:
  std::string base_url_;
  std::string token_;
  std::string account_id_;
  std::unique_ptr<Poco::Net::HTTPSClientSession> session_;
  std::unique_ptr<AccountDetails> account_details_;
  static const std::string kPracticeBaseURL;
  static const std::string kLiveBaseURL;
};
}  // namespace iridium


#endif //IRIDIUM_LIVE_TRADING_OANDA_HPP_
