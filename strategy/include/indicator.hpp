//
// Created by Evan Su on 3/6/21.
//

#ifndef IRIDIUM_INCLUDE_IRIDIUM_INDICATOR_HPP_
#define IRIDIUM_INCLUDE_IRIDIUM_INDICATOR_HPP_

#include "ta_func.h"
#include <memory>
#include <vector>
#include <optional>
#include <tuple>
#include <iridium/algorithm.hpp>

namespace iridium::indicator {

enum MarketImpulse {
  green, red, blue
};

enum TradeAction {
  buy, sell, aside
};

std::unique_ptr<std::vector<TA_Real>>
ema(const std::vector<double> &closes, int period);

/*
  * check moving average crossover & the distance to the last data
  */
std::tuple<std::optional<bool>, std::optional<int>>
CheckMACrossOver(
    const std::vector<double> &fast_ma,
    const std::vector<double> &slow_ma);

/*
  * return macd, macd signal, macd hist
  */
std::tuple<
    std::unique_ptr<std::vector<TA_Real>>,
    std::unique_ptr<std::vector<TA_Real>>,
    std::unique_ptr<std::vector<TA_Real>>>
macd(
    const std::vector<double> &closes,
    int fast_period,
    int slow_period,
    int signal_period);

std::unique_ptr<std::vector<TA_Real>>
adx(
    const std::vector<double> &highs,
    const std::vector<double> &lows,
    const std::vector<double> &closes,
    int period);

std::unique_ptr<std::vector<TA_Real>>
rsi(
    const std::vector<double> &closes,
    int period);

std::unique_ptr<std::vector<TA_Real>>
atr(
    const std::vector<double> &highs,
    const std::vector<double> &lows,
    const std::vector<double> &closes,
    int period);

std::optional<double>
nearest_peak_high_before_ma_cross(
    const std::vector<double> &highs,
    int ma_cross_distance_to_last,
    int peak_half_size);

std::optional<double>
nearest_peak_low_before_ma_cross(
    const std::vector<double> &lows,
    int ma_cross_distance_to_last,
    int peak_half_size);

double
highest_after_ma_cross(
    const std::vector<double> &highs,
    int ma_cross_distance_to_last);

double
lowest_after_ma_cross(
    const std::vector<double> &lows,
    int ma_cross_distance_to_last);

std::unique_ptr<std::vector<MarketImpulse>>
market_impulses(
    const std::vector<double> &closes,
    int ema_period,
    int macd_fast_period,
    int macd_slow_period,
    int macd_signal_period);

void ProcessOutputData(
    std::vector<TA_Real> &out,
    TA_Integer out_beg,
    int count
    );

}  // namespace iridium
#endif //IRIDIUM_INCLUDE_IRIDIUM_INDICATOR_HPP_
