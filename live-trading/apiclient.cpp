//
// Created by Evan Su on 20/3/21.
//

#include "apiclient.hpp"

const std::string iridium::Oanda::kPracticeBaseURL("https://api-fxpractice.oanda.com/v3");
const std::string iridium::Oanda::kLiveBaseURL("https://api-fxtrade.oanda.com/v3");

double iridium::Oanda::balance() const {
  return account_details_->balance;
}

const std::string &iridium::Oanda::account_currency() const {
  return account_details_->currency;
}

int iridium::Oanda::leverage() const {
  return account_details_->leverage;
}

std::shared_ptr<iridium::TradeList>
iridium::Oanda::open_trades_ptr(const std::string &instrument) const {
  // TODO: impl
  auto open_trades_ptr = std::make_shared<TradeList>();
  return open_trades_ptr;
}

std::shared_ptr<iridium::LimitOrderList>
iridium::Oanda::pending_limit_orders_ptr(const std::string &instrument) const {
  // TODO: impl
  auto pending_limit_orders = std::make_shared<LimitOrderList>();
  return pending_limit_orders;
}

int
iridium::Oanda::open_position_size(const std::string &instrument) const {
  // TODO: impl
  return 0;
}

std::optional<double> iridium::Oanda::net_asset_value(const iridium::data::TickDataMap &tick_data_map) const {
  return account_details_->nav;
}
std::optional<double> iridium::Oanda::margin_used(const iridium::data::TickDataMap &tick_data_map) const {
  return account_details_->margin_used;
}

void iridium::Oanda::CreateLimitOrder(
    std::time_t create_time,
    const std::string &instrument,
    int units,
    double price,
    std::optional<double> take_profit_price,
    std::optional<double> stop_loss_price,
    std::optional<double> trailing_stop_loss_distance) {
  auto path = "/accounts/" + account_id_ + "/orders";
  auto pip_num = pip_point(Instrument(instrument));
  Poco::JSON::Object order_obj;
  order_obj.set("units", units);
  order_obj.set("price", price);
  order_obj.set("instrument", instrument);
  order_obj.set("type", "LIMIT");
  order_obj.set("timeInForce", "GTC");
  order_obj.set("positionFill", "DEFAULT");
  if (stop_loss_price.has_value()) {
    Poco::JSON::Object stop_loss_obj;
    stop_loss_obj.set("price", To_String_With_Precision(stop_loss_price.value(), pip_num));
    stop_loss_obj.set("timeInForce", "GTC");
    order_obj.set("stopLossOnFill", stop_loss_obj);
  }
  if (take_profit_price.has_value()) {
    Poco::JSON::Object take_profit_obj;
    take_profit_obj.set("price", To_String_With_Precision(take_profit_price.value(), pip_num));
    take_profit_obj.set("timeInForce", "GTC");
    order_obj.set("takeProfitOnFill", take_profit_obj);
  }
  if (trailing_stop_loss_distance.has_value()) {
    Poco::JSON::Object trailing_stop_loss_obj;
    trailing_stop_loss_obj.set("distance", To_String_With_Precision(trailing_stop_loss_distance.value(), pip_num));
    trailing_stop_loss_obj.set("timeInForce", "GTC");
    order_obj.set("trailingStopLossOnFill", trailing_stop_loss_obj);
  }
  Poco::JSON::Object req_body;
  req_body.set("order", order_obj);
  auto resp = SendRequest(path, Poco::Net::HTTPRequest::HTTP_POST, std::nullopt, req_body);
  logger_->info("create a trade, status code: {0}, response: {1}", resp->status, resp->body);
}

void iridium::Oanda::CloserPosition(const std::string &instrument,
                                    double rate,
                                    double current_price,
                                    std::time_t time) {
  auto trades = account_details_->trades;
  for (const auto &trade : *trades) {
    if ((trade->instrument == instrument)
        && (trade->state == "OPEN")) {
      CloseTrade(trade->trade_id);
    }
  }
}

void
iridium::Oanda::UpdateTradeStopLossPrice(
    const std::shared_ptr<Trade> &trade_ptr,
    double stop_loss_price,
    std::time_t time) {
  // TODO: impl
}
void
iridium::Oanda::UpdateTradeTakeProfitPrice(
    const std::shared_ptr<Trade> &trade_ptr,
    double take_profit_price,
    std::time_t time) {
  // TODO: impl
}
void
iridium::Oanda::UpdateTrailingStopDistance(
    const std::shared_ptr<Trade> &trade_ptr,
    double distance,
    std::time_t time) {
  // TODO: impl
}

void iridium::Oanda::CancelLimitOrder(const std::shared_ptr<LimitOrder> &order_ptr) {
  // TODO: impl

}

bool iridium::Oanda::HasOpenTrades(const std::string &instrument) const {
  return !open_trades_ptr(instrument)->empty();
}

bool iridium::Oanda::HasPendingOrders(const std::string &instrument) const {
  // TODO: impl
  return false;
}

iridium::Oanda::Oanda(
    iridium::Oanda::Env env,
    const std::string &token,
    const std::string &account_id) :
    token_(token),
    account_id_(account_id),
    base_url_(env == Env::practice ? kPracticeBaseURL : kLiveBaseURL),
    session_(std::make_unique<Poco::Net::HTTPSClientSession>(Poco::URI(base_url_).getHost())),
    logger_(iridium::logger()) {
  session_->setKeepAlive(true);
}

std::unique_ptr<iridium::data::DataList> iridium::Oanda::instrument_data(
    const std::string &instrument_name,
    int count,
    iridium::data::DataFreq freq) const {
  auto path = "/instruments/" + instrument_name + "/candles";
  std::map<std::string, std::string> query_params;
  query_params.insert_or_assign("count", std::to_string(count));
  query_params.insert_or_assign("granularity", iridium::data::DataFreqToString(freq));
  std::map<std::string, std::string> headers;
  headers.insert_or_assign("Accept-Datetime-Format", "UNIX");
  auto resp = SendRequest(path, Poco::Net::HTTPRequest::HTTP_GET, query_params, std::nullopt, headers);
  auto data_list = std::make_unique<iridium::data::DataList>();
  if (resp->status == Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::JSON::Parser parser;
    auto result = parser.parse(resp->body);
    auto obj = result.extract<Poco::JSON::Object::Ptr>();
    auto candles = obj->getArray("candles");
    for (const auto &candle : *candles) {
      auto candle_data = candle.extract<Poco::JSON::Object::Ptr>();
      auto time = std::stoi(candle_data->getValue<std::string>("time"));
      auto volume = candle_data->getValue<int>("volume");
      auto mid = candle_data->getObject("mid");
      auto open = std::stod(mid->getValue<std::string>("o"));
      auto high = std::stod(mid->getValue<std::string>("h"));
      auto low = std::stod(mid->getValue<std::string>("l"));
      auto close = std::stod(mid->getValue<std::string>("c"));
      iridium::data::Candlestick candlestick;
      candlestick.time = time;
      candlestick.open = open;
      candlestick.close = close;
      candlestick.high = high;
      candlestick.low = low;
      candlestick.volume = volume;
      data_list->push_back(candlestick);
    }
  }
  return data_list;
}

std::unique_ptr<std::map<std::string, double>>
iridium::Oanda::spread(const iridium::InstrumentList &instruments) const {
  auto path = "/accounts/" + account_id_ + "/pricing";
  std::map<std::string, std::string> query_params;
  query_params.insert_or_assign("instruments", InstrumentListToString(instruments));
  auto resp = SendRequest(path, Poco::Net::HTTPRequest::HTTP_GET, query_params);
  auto spread = std::make_unique<std::map<std::string, double >>();
  if (resp->status == Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::JSON::Parser parser;
    auto result = parser.parse(resp->body);
    auto obj = result.extract<Poco::JSON::Object::Ptr>();
    auto prices = obj->getArray("prices");
    for (const auto &price : *prices) {
      auto price_data = price.extract<Poco::JSON::Object::Ptr>();
      auto instrument = price_data->getValue<std::string>("instrument");
      auto bid = std::stod(price_data->getArray("bids")->getObject(0)->getValue<std::string>("price"));
      auto ask = std::stod(price_data->getArray("asks")->getObject(0)->getValue<std::string>("price"));
      spread->insert_or_assign(instrument, (ask - bid) * pow(10, iridium::pip_point(Instrument(instrument))));
    }
  }
  return spread;
}

void iridium::Oanda::FetchAccountDetails() {
  auto path = "/accounts/" + account_id_;
  std::map<std::string, std::string> headers;
  headers.insert_or_assign("Accept-Datetime-Format", "UNIX");
  auto resp = SendRequest(path, Poco::Net::HTTPRequest::HTTP_GET, std::nullopt, std::nullopt, headers);
  logger_->info("account details: {0}", resp->body);
  auto account_details = std::make_unique<AccountDetails>();
  if (resp->status == Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::JSON::Parser parser;
    auto result = parser.parse(resp->body);
    auto obj = result.extract<Poco::JSON::Object::Ptr>()->getObject("account");
    account_details->currency = obj->getValue<std::string>("currency");
    account_details->balance = std::stod(obj->getValue<std::string>("balance"));
    account_details->nav = std::stod(obj->getValue<std::string>("NAV"));
    account_details->margin_used = std::stod(obj->getValue<std::string>("marginUsed"));
    account_details->margin_available = std::stod(obj->getValue<std::string>("marginAvailable"));
    account_details->leverage = static_cast<int>(1 / std::stod(obj->getValue<std::string>("marginRate")));
    auto trades = obj->getArray("trades");
    auto trades_list = std::make_shared<TradeSummaryList>();
    for (const auto &trade : *trades) {
      auto trade_data = trade.extract<Poco::JSON::Object::Ptr>();
      auto trade_summary = std::make_shared<TradeSummary>();
      trade_summary->trade_id = trade_data->getValue<std::string>("id");
      trade_summary->instrument = trade_data->getValue<std::string>("instrument");
      trade_summary->price = std::stod(trade_data->getValue<std::string>("price"));
      trade_summary->open_time = std::stoi(trade_data->getValue<std::string>("openTime"));
      trade_summary->state = trade_data->getValue<std::string>("state");
      trade_summary->initial_units = std::stoi(trade_data->getValue<std::string>("initialUnits"));
      trade_summary->initial_margin = std::stod(trade_data->getValue<std::string>("initialMarginRequired"));
      trade_summary->current_units = std::stoi(trade_data->getValue<std::string>("currentUnits"));
      trade_summary->realized_pl = std::stod(trade_data->getValue<std::string>("realizedPL"));
      trade_summary->unrealized_pl = std::stod(trade_data->getValue<std::string>("unrealizedPL"));
      trade_summary->margin_used = std::stod(trade_data->getValue<std::string>("marginUsed"));
      if (trade_data->has("averageClosePrice")) {
        trade_summary->close_price = std::stod(trade_data->getValue<std::string>("averageClosePrice"));
      }
      if (trade_data->has("closeTime")) {
        trade_summary->close_time = std::stoi(trade_data->getValue<std::string>("closeTime"));
      }
      if (trade_data->has("stopLossOrderID")) {
        trade_summary->stop_loss_order_id = trade_data->getValue<std::string>("stopLossOrderID");
      }
      if (trade_data->has("takeProfitOrderID")) {
        trade_summary->take_profit_order_id = trade_data->getValue<std::string>("takeProfitOrderID");
      }
      if (trade_data->has("guaranteedStopLossOrderID")) {
        trade_summary->guaranteed_stop_loss_order_id = trade_data->getValue<std::string>("guaranteedStopLossOrderID");
      }
      if (trade_data->has("trailingStopLossOrderID")) {
        trade_summary->trailing_stop_loss_order_id = trade_data->getValue<std::string>("trailingStopLossOrderID");
      }
      trades_list->push_back(trade_summary);
    }
    account_details->trades = trades_list;
  }
  account_details_ = std::move(account_details);
}


std::tuple<std::unique_ptr<iridium::data::DataListMap>, std::unique_ptr<iridium::data::TickDataMap>>
iridium::Oanda::trade_data(
    const iridium::InstrumentList &instruments,
    int count,
    iridium::data::DataFreq freq) const {
  auto hist_data_map = std::make_unique<iridium::data::DataListMap>();
  auto tick_data_map = std::make_unique<iridium::data::TickDataMap>();
  for (const auto &instrument : instruments) {
    auto data = instrument_data(instrument->name(), count, freq);
    tick_data_map->insert({instrument->name(), data->back()});
    data->pop_back();
    hist_data_map->insert({instrument->name(), std::move(data)});
  }
  return std::make_tuple(std::move(hist_data_map), std::move(tick_data_map));
}

std::unique_ptr<iridium::Oanda::Resp> iridium::Oanda::SendRequest(
    const std::string &path,
    const std::string &http_method,
    const std::optional<std::map<std::string, std::string>> &query_params,
    const std::optional<Poco::JSON::Object> &req_body,
    const std::optional<std::map<std::string, std::string>> &headers) const {
  // URL
  Poco::URI uri(base_url_ + path);
  if (query_params.has_value()) {
    for (auto const &param : query_params.value()) {
      uri.addQueryParameter(param.first, param.second);
    }
  }
  // request init
  auto req =
      std::make_unique<Poco::Net::HTTPRequest>(http_method, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
  // headers
  req->setContentType("application/json");
  if (headers.has_value()) {
    for (auto const &header : headers.value()) {
      req->add(header.first, header.second);
    }
  }
  // authentication
  auto creds = std::make_unique<Poco::Net::OAuth20Credentials>(token_);
  creds->authenticate(*req);
  // request body
  if (req_body.has_value()) {
    auto ss = std::make_unique<std::stringstream>();
    req_body.value().stringify(*ss);
    req->setContentLength(ss->str().length());
    session_->sendRequest(*req) << ss->str();
  } else {
    session_->sendRequest(*req);
  }
  // response
  auto resp = std::make_unique<Poco::Net::HTTPResponse>();
  auto &is = session_->receiveResponse(*resp);
  auto resp_body = std::make_unique<std::stringstream>();
  Poco::StreamCopier::copyStream(is, *resp_body);
  auto response = std::make_unique<Resp>();
  response->status = resp->getStatus();
  response->body = resp_body->str();
  // logging error
  if ((response->status != Poco::Net::HTTPResponse::HTTP_OK) &&
      (response->status != Poco::Net::HTTPResponse::HTTP_CREATED)) {
    logger_->error("path: {0}, error code: {1}, error response: {2}", path, response->status, response->body);
  }

  return response;
}

void iridium::Oanda::CloseTrade(const std::string &trade_id) {
  auto path = "/accounts/" + account_id_ + "/trades/" + trade_id + "/close";
  auto resp = SendRequest(path, Poco::Net::HTTPRequest::HTTP_PUT);
  logger_->info("close a trade, status code: {0}, resp: {1}", path, resp->status, resp->body);
}

std::tuple<std::shared_ptr<iridium::data::DataListMap>, std::shared_ptr<iridium::data::TickDataMap>>
iridium::trade_data_thread_pool(
    iridium::Oanda::Env env,
    const std::string &token,
    const std::string &account_id,
    const iridium::InstrumentList &instruments,
    int count,
    iridium::data::DataFreq freq) {
  auto hist_data_map = std::make_shared<iridium::data::DataListMap>();
  auto tick_data_map = std::make_shared<iridium::data::TickDataMap>();
  boost::asio::thread_pool pool(instruments.size());
  for (const auto &instrument : instruments) {
    boost::asio::post(pool, [env, token, account_id, instrument, count, freq, tick_data_map, hist_data_map]() {
      auto client = std::make_unique<iridium::Oanda>(env, token, account_id);
      auto data = client->instrument_data(instrument->name(), count, freq);
      tick_data_map->insert({instrument->name(), data->back()});
      data->pop_back();
      hist_data_map->insert({instrument->name(), std::move(data)});
    });
  }
  pool.join();
  return std::make_tuple(hist_data_map, tick_data_map);
}

