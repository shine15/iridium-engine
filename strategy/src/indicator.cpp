//
// Created by Evan Su on 3/6/21.
//

#include "../include/indicator.hpp"

std::unique_ptr<std::vector<TA_Real>>
iridium::indicator::ema(const std::vector<double> &closes, int period) {
  auto count = closes.size();
  auto out = std::make_unique<std::vector<TA_Real>>(count);
  TA_Integer out_beg;
  TA_Integer out_num;
  TA_MA(
      0,
      count - 1,
      closes.data(),
      period,
      TA_MAType_EMA,
      &out_beg,
      &out_num,
      out->data());
  ProcessOutputData(*out, out_beg, count);
  return out;
}

std::tuple<std::optional<bool>, std::optional<int>>
iridium::indicator::CheckMACrossOver(
    const std::vector<double> &fast_ma,
    const std::vector<double> &slow_ma) {
  std::optional<bool> ma_cross_is_long;
  std::optional<int> ma_cross_distance_to_last;
  auto count = fast_ma.size();
  auto diffs = std::make_unique<std::vector<TA_Real>>();
  for (int i = 0; i < count; ++i) {
    if (fast_ma.at(i) == 0 || slow_ma.at(i) == 0) {
      diffs->push_back(0);
    } else {
      diffs->push_back(fast_ma.at(i) - slow_ma.at(i));
    }
  }
  for (int i = count - 1; i >= 1; --i) {
    if (diffs->at(i - 1) < 0 && diffs->at(i) > 0) {
      ma_cross_is_long = true;
      ma_cross_distance_to_last = count - 1 - i;
      break;
    } else if (diffs->at(i - 1) > 0 && diffs->at(i) < 0) {
      ma_cross_is_long = false;
      ma_cross_distance_to_last = count - 1 - i;
      break;
    }
  }
  return std::make_tuple(ma_cross_is_long, ma_cross_distance_to_last);
}

std::tuple<
    std::unique_ptr<std::vector<TA_Real>>,
    std::unique_ptr<std::vector<TA_Real>>,
    std::unique_ptr<std::vector<TA_Real>>>
    iridium::indicator::macd(
        const std::vector<double> &closes,
        int fast_period,
        int slow_period,
        int signal_period) {
  auto count = closes.size();
  auto out = std::make_unique<std::vector<TA_Real>>(count);
  auto signal_out = std::make_unique<std::vector<TA_Real>>(count);
  auto hist_out = std::make_unique<std::vector<TA_Real>>(count);
  TA_Integer out_beg;
  TA_Integer out_num;
  TA_MACD(
      0,
      count - 1,
      closes.data(),
      fast_period,
      slow_period,
      signal_period,
      &out_beg,
      &out_num,
      out->data(),
      signal_out->data(),
      hist_out->data());
  ProcessOutputData(*out, out_beg, count);
  ProcessOutputData(*signal_out, out_beg, count);
  ProcessOutputData(*hist_out, out_beg, count);
  return std::make_tuple(std::move(out), std::move(signal_out), std::move(hist_out));
}

std::unique_ptr<std::vector<TA_Real>>
iridium::indicator::adx(
    const std::vector<double> &highs,
    const std::vector<double> &lows,
    const std::vector<double> &closes,
    int period) {
  auto count = closes.size();
  auto out = std::make_unique<std::vector<TA_Real>>(count);
  TA_Integer out_beg;
  TA_Integer out_num;
  TA_ADX(
      0,
      count - 1,
      highs.data(),
      lows.data(),
      closes.data(),
      period,
      &out_beg,
      &out_num,
      out->data());
  ProcessOutputData(*out, out_beg, count);
  return out;
}

std::unique_ptr<std::vector<TA_Real>>
iridium::indicator::rsi(
    const std::vector<double> &closes,
    int period) {
  auto count = closes.size();
  auto out = std::make_unique<std::vector<TA_Real>>(count);
  TA_Integer out_beg;
  TA_Integer out_num;
  TA_RSI(
      0, count - 1,
      closes.data(),
      period,
      &out_beg,
      &out_num,
      out->data());
  ProcessOutputData(*out, out_beg, count);
  return out;
}

std::unique_ptr<std::vector<TA_Real>>
iridium::indicator::atr(
    const std::vector<double> &highs,
    const std::vector<double> &lows,
    const std::vector<double> &closes,
    int period) {
  auto count = closes.size();
  auto out = std::make_unique<std::vector<TA_Real>>(count);
  TA_Integer out_beg;
  TA_Integer out_num;
  TA_ATR(
      0,
      count - 1,
      highs.data(),
      lows.data(),
      closes.data(),
      period,
      &out_beg,
      &out_num,
      out->data());
  ProcessOutputData(*out, out_beg, count);
  return out;
}

std::optional<double>
iridium::indicator::nearest_peak_high_before_ma_cross(
    const std::vector<double> &highs,
    int ma_cross_distance_to_last,
    int peak_half_size) {
  auto sliced_highs_before_cross =
      iridium::algorithm::SliceFirstN(highs, highs.size() - ma_cross_distance_to_last);
  auto nearest_peak_high_before_cross =
      iridium::algorithm::FindNearestPeakHigh(sliced_highs_before_cross, peak_half_size);
  return nearest_peak_high_before_cross;
}

std::optional<double>
iridium::indicator::nearest_peak_low_before_ma_cross(
    const std::vector<double> &lows,
    int ma_cross_distance_to_last,
    int peak_half_size) {
  auto sliced_lows_before_cross =
      iridium::algorithm::SliceFirstN(lows, lows.size() - ma_cross_distance_to_last);
  auto nearest_peak_low_before_cross =
      iridium::algorithm::FindNearestPeakLow(sliced_lows_before_cross, peak_half_size);
  return nearest_peak_low_before_cross;
}

double
iridium::indicator::highest_after_ma_cross(
    const std::vector<double> &highs,
    int ma_cross_distance_to_last) {
  auto sliced_highs_after_cross = iridium::algorithm::SliceLastN(highs, ma_cross_distance_to_last);
  auto highest_after_cross = std::max_element(sliced_highs_after_cross.begin(), sliced_highs_after_cross.end());
  return *highest_after_cross;
}

double
iridium::indicator::lowest_after_ma_cross(
    const std::vector<double> &lows,
    int ma_cross_distance_to_last) {
  auto sliced_lows_after_cross = iridium::algorithm::SliceLastN(lows, ma_cross_distance_to_last);
  auto lowest_after_cross = std::min_element(sliced_lows_after_cross.begin(), sliced_lows_after_cross.end());
  return *lowest_after_cross;
}

std::unique_ptr<std::vector<iridium::indicator::MarketImpulse>>
iridium::indicator::market_impulses(
    const std::vector<double> &closes,
    int ema_period,
    int macd_fast_period,
    int macd_slow_period,
    int macd_signal_period) {
  auto impulses = std::make_unique<std::vector<MarketImpulse>>();
  auto count = closes.size();
  auto ema = iridium::indicator::ema(closes, ema_period);
  auto macd_hist = std::get<2>(iridium::indicator::macd(
      closes, macd_fast_period, macd_slow_period, macd_signal_period));
  for (int i = 1; i < count; ++i) {
    auto ema0 = ema->at(i - 1);
    auto ema1 = ema->at(i);
    auto hist0 = macd_hist->at(i - 1);
    auto hist1 = macd_hist->at(i);
    if (ema0 != 0 && ema1 != 0 && hist0 !=0 && hist1 !=0) {
      if (ema1 > ema0 && hist1 > hist0) {
        impulses->push_back(MarketImpulse::green);
      } else if (ema1 < ema0 && hist1 < hist0) {
        impulses->push_back(MarketImpulse::red);
      } else {
        impulses->push_back(MarketImpulse::blue);
      }
    }
  }
  return impulses;
}

void iridium::indicator::ProcessOutputData(
    std::vector<TA_Real> &out,
    TA_Integer out_beg,
    int count) {
  std::vector<int> out_left_padding(out_beg, 0);
  out.insert(out.begin(), out_left_padding.begin(), out_left_padding.end());
  out.erase(out.begin() + count, out.end());
}
