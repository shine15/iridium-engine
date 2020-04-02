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

#ifndef INCLUDE_IRIDIUM_CALENDAR_HPP_
#define INCLUDE_IRIDIUM_CALENDAR_HPP_

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "data.hpp"

namespace iridium {
namespace calendar {
class DayIterator {
 public:
  DayIterator(
      int begin_year,
      int begin_month,
      int begin_day,
      int end_year,
      int end_month,
      int end_day,
      const std::string &region);

  boost::gregorian::date operator*();

  DayIterator operator++(int junk);

  std::time_t trade_start();

  using date_set = std::set<boost::gregorian::partial_date>;

 private:
  boost::gregorian::date end_date_;

  boost::gregorian::day_iterator cur_itr_ =
      boost::gregorian::day_iterator(boost::gregorian::date());

  date_set all_holidays_;

  bool is_trading_day(const boost::gregorian::date &date);
};

class Clock {
 public:
  Clock(
      int begin_year,
      int begin_month,
      int begin_day,
      int end_year,
      int end_month,
      int end_day,
      const std::string &region,
      data::DataFreq freq);

  class Iterator {
   public:
    Iterator(Clock &clock, int pos) : clock_(clock), pos_(pos) {}

    Iterator operator++() {
      auto i = *this;
      pos_++;
      return i;
    }

    int operator*() {
      auto period_count = data::DataFreq::d / clock_.freq_;
      return static_cast<int>(clock_.trade_starts_ptr_->at(pos_ / period_count)
          + (pos_ % period_count) * clock_.freq_);
    }

    bool operator==(const Iterator &rhs) { return pos_ == rhs.pos_; }

    bool operator!=(const Iterator &rhs) { return pos_ != rhs.pos_; }
   private:
    int pos_;
    Clock &clock_;
  };

  Iterator begin() {
    return Iterator(*this, 0);
  }

  Iterator end() {
    return Iterator(*this, trade_starts_ptr_->size() * (data::DataFreq::d / freq_));
  }

 private:
  std::shared_ptr<std::vector<std::time_t>> trade_starts_ptr_;
  data::DataFreq freq_;
};

boost::local_time::time_zone_ptr TimeZoneFromRegion(const std::string &region);

std::shared_ptr<std::vector<std::time_t>> trade_start_times_ptr(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region);

std::shared_ptr<std::vector<int>> all_ticks_ptr(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region,
    data::DataFreq freq);
}  // namespace calendar
}  // namespace iridium
#endif  // INCLUDE_IRIDIUM_CALENDAR_HPP_
