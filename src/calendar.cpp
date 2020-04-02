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

#include <iridium/calendar.hpp>

using boost::gregorian::Jan;
using boost::gregorian::Dec;
using boost::gregorian::Saturday;
using boost::gregorian::Sunday;
using boost::gregorian::Monday;
using boost::gregorian::date;
using boost::gregorian::days;
using boost::gregorian::day_iterator;
using boost::gregorian::partial_date;
using boost::posix_time::time_duration;
using boost::posix_time::hours;
using boost::local_time::local_date_time;
using boost::local_time::tz_database;

constexpr auto NOT_DATE_TIME_ON_ERROR =  boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR;

// Day Iterator
iridium::calendar::DayIterator::DayIterator(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region) :
    all_holidays_({partial_date(1, Jan), partial_date(25, Dec)}) {
  auto tz = TimeZoneFromRegion(region);
  time_duration td(0, 0, 0);
  local_date_time begin(date(begin_year, begin_month, begin_day),
                        td,
                        tz,
                        NOT_DATE_TIME_ON_ERROR);
  local_date_time end(date(end_year, end_month, end_day),
                      td,
                      tz,
                      NOT_DATE_TIME_ON_ERROR);
  auto nyc_tz = TimeZoneFromRegion("America/New_York");
  auto begin_date = begin.local_time_in(nyc_tz).date();
  if (!is_trading_day(begin_date)) {
    do {
      begin_date += days(1);
    } while (!is_trading_day(begin_date));
  }
  cur_itr_ = day_iterator(begin_date);
  end_date_ = *(day_iterator(end.local_time_in(nyc_tz).date()));
}

boost::gregorian::date iridium::calendar::DayIterator::operator*() {
  return *cur_itr_;
}

iridium::calendar::DayIterator iridium::calendar::DayIterator::operator++(int junk) {
  if (*cur_itr_ > end_date_) throw std::out_of_range("Out of range.");
  auto temp = *this;
  do {
    ++cur_itr_;
  } while (!is_trading_day(*cur_itr_));
  return temp;
}

bool iridium::calendar::DayIterator::is_trading_day(const boost::gregorian::date &date) {
  auto pdate = partial_date(date.day(), date.month());
  if (all_holidays_.find(pdate) != all_holidays_.end()) return false;
  auto day_of_week = date.day_of_week();
  if (day_of_week == Saturday || day_of_week == Sunday) return false;
  if (day_of_week == Monday) {
    for (int i = 1; i < 3; ++i) {
      auto before_day = date - days(i);
      auto before_pdate = partial_date(before_day.day(), before_day.month());
      if (all_holidays_.find(before_pdate) != all_holidays_.end()) return false;
    }
  }
  return true;
}

std::time_t iridium::calendar::DayIterator::trade_start() {
  auto nyc_tz = TimeZoneFromRegion("America/New_York");
  time_duration td(17, 0, 0);
  local_date_time trade_end(*cur_itr_, td, nyc_tz, local_date_time::NOT_DATE_TIME_ON_ERROR);
  return to_time_t((trade_end - hours(24)).utc_time());
}

// Clock
iridium::calendar::Clock::Clock(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region,
    iridium::data::DataFreq freq) :
    trade_starts_ptr_(trade_start_times_ptr(
        begin_year,
        begin_month,
        begin_day,
        end_year,
        end_month,
        end_day,
        region)),
    freq_(freq) {}

// Utility methods
boost::local_time::time_zone_ptr
iridium::calendar::TimeZoneFromRegion(const std::string &region) {
  tz_database tz_db;
  tz_db.load_from_file("../resources/date_time_zonespec.csv");
  return tz_db.time_zone_from_region(region);
}

std::shared_ptr<std::vector<std::time_t>>
iridium::calendar::trade_start_times_ptr(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region) {
  DayIterator itr(begin_year, begin_month, begin_day, end_year, end_month, end_day, region);
  auto trade_start_times = std::make_shared<std::vector<std::time_t>>();
  while (true) {
    try {
      trade_start_times->push_back(itr.trade_start());
      itr++;
    } catch (std::out_of_range const &err) {
      break;
    }
  }
  return trade_start_times;
}

std::shared_ptr<std::vector<int>>
iridium::calendar::all_ticks_ptr(
    int begin_year,
    int begin_month,
    int begin_day,
    int end_year,
    int end_month,
    int end_day,
    const std::string &region,
    iridium::data::DataFreq freq) {
  Clock clock(begin_year, begin_month, begin_day, end_year, end_month, end_day, region, freq);
  auto ticks = std::make_shared<std::vector<int>>();
  for (auto it = clock.begin(); it != clock.end(); ++it) {
    ticks->push_back(*it);
  }
  return ticks;
}
