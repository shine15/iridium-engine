//
// Created by Evan Su on 13/3/21.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "apiclient.hpp"
#include "../strategy/include/simulate.hpp"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;

class IridiumLive: public Application {
 public:
  IridiumLive() {}

 protected:
  void defineOptions(OptionSet& options)
  {
    Application::defineOptions(options);

    options.addOption(
        Option("help", "h", "Display help information on command line arguments.")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<IridiumLive>(this, &IridiumLive::HandleHelp)));

    options.addOption(
        Option("env", "e", "Specify the Oanda environment - practice or live")
            .required(true)
            .repeatable(false)
            .argument("token")
            .callback(OptionCallback<IridiumLive>(this, &IridiumLive::HandleEnv)));

    options.addOption(
        Option("token", "t", "Specify the Oanda application token.")
            .required(true)
            .repeatable(false)
            .argument("token")
            .callback(OptionCallback<IridiumLive>(this, &IridiumLive::HandleToken)));

    options.addOption(
        Option("account", "a", "Specify the account id.")
            .required(true)
            .repeatable(false)
            .argument("account id")
            .callback(OptionCallback<IridiumLive>(this, &IridiumLive::HandleAccount)));
  }

  void HandleHelp(const std::string& name, const std::string& value) {
    DisplayHelp();
    stopOptionsProcessing();
  }

  void DisplayHelp() {
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Iridium live command line for oanda live trading");
    helpFormatter.format(std::cout);
  }

  void HandleEnv(const std::string& name, const std::string& value) {
    env_ = ((value == "live") ? iridium::Oanda::Env::live : iridium::Oanda::Env::practice);
  }

  void HandleToken(const std::string& name, const std::string& value) {
    token_ = value;
  }

  void HandleAccount(const std::string& name, const std::string& value) {
    account_id_ = value;
  }

  int main(const std::vector<std::string>& args) {
    // Logging
    auto logger = iridium::logger();

    try {
      using iridium::instrument_list;
      auto instruments = instrument_list({
                                             "AUD_CAD", "AUD_JPY", "AUD_NZD", "AUD_SGD", "AUD_USD",
                                             "CAD_JPY", "CAD_SGD",
                                             "EUR_AUD", "EUR_CAD", "EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_SGD", "EUR_USD",
                                             "GBP_AUD", "GBP_CAD", "GBP_JPY", "GBP_NZD", "GBP_SGD", "GBP_USD",
                                             "NZD_CAD", "NZD_JPY", "NZD_SGD", "NZD_USD",
                                             "SGD_JPY",
                                             "USD_CAD", "USD_JPY", "USD_SGD"});
      const auto kHistDataCount = 90;

      auto client = std::make_shared<iridium::Oanda>(env_, token_, account_id_);
      while (true) {
        auto [hist_data_map, tick_data_map] = iridium::trade_data_thread_pool(
            env_, token_, account_id_, *instruments, kHistDataCount + 1, iridium::data::DataFreq::m15);
        auto spreads = client->spread(*instruments);
        for (auto const &[name, data] : *tick_data_map) {
          client->FetchAccountDetails();
          SimulateTrade(
              name,
              std::time(nullptr),
              *hist_data_map->at(name),
              *tick_data_map,
              kHistDataCount,
              client,
              spreads->at(name));
        }
        client->FetchAccountDetails();
        std::this_thread::sleep_for(std::chrono::seconds (30));
      }
    } catch (Poco::Exception &exc) {
      logger->error(exc.displayText());
      return Application::EXIT_SOFTWARE;
    } catch (const std::exception &exc) {
      logger->error(exc.what());
      return Application::EXIT_SOFTWARE;
    }
    return Application::EXIT_OK;
  }

 private:
  std::string token_;
  std::string account_id_;
  iridium::Oanda::Env env_;
};

POCO_APP_MAIN(IridiumLive)