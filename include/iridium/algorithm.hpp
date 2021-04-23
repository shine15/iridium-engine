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

#ifndef INCLUDE_IRIDIUM_ALGORITHM_HPP_
#define INCLUDE_IRIDIUM_ALGORITHM_HPP_

#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace iridium {
namespace algorithm {
template<class Comparable>
int BinarySearch(
    const std::vector<Comparable> &a,
    const Comparable &x,
    int low,
    int high,
    bool reversed) {
  if (low > high) return -1;
  int mid = (low + high) / 2;
  if ((a[mid] < x && !reversed) || (a[mid] > x && reversed)) {
    return BinarySearch(a, x, mid + 1, high, reversed);
  } else if ((a[mid] > x && !reversed) || (a[mid] < x && reversed)) {
    return BinarySearch(a, x, low, mid - 1, reversed);
  } else {
    return mid;
  }
}

template<class Comparable>
int BinarySearch(
    const std::vector<Comparable> &a,
    const Comparable &x,
    bool reversed = false) {
  return BinarySearch(a, x, 0, a.size() - 1, reversed);
}

template<typename T>
std::optional<T> FindNearestPeakHigh(const std::vector<T> &v, int peak_half_size) {
  if (v.size() < 2 * peak_half_size + 1) {
    throw std::out_of_range("Out of range");
  }
  for (int i = v.size() - peak_half_size - 1; i >= peak_half_size; --i) {
    auto sub_list = std::vector<T>(v.begin() + i - peak_half_size, v.begin() + i + peak_half_size + 1);
    auto max_value = std::max_element(sub_list.begin(), sub_list.end());
    auto mid_value = sub_list.at(peak_half_size);
    if (mid_value == *max_value) {
      return mid_value;
    }
  }
  return std::nullopt;
}

template<typename T>
std::optional<T> FindNearestPeakLow(const std::vector<T> &v, int peak_half_size) {
  if (v.size() < 2 * peak_half_size + 1) {
    throw std::out_of_range("Out of range");
  }
  for (int i = v.size() - peak_half_size - 1; i >= peak_half_size; --i) {
    auto sub_list = std::vector<T>(v.begin() + i - peak_half_size, v.begin() + i + peak_half_size + 1);
    auto min_value = std::min_element(sub_list.begin(), sub_list.end());
    auto mid_value = sub_list.at(peak_half_size);
    if (mid_value == *min_value) {
      return mid_value;
    }
  }
  return std::nullopt;
}

template<typename T>
std::vector<T> Slice(const std::vector<T> &v, int m, int n) {
  std::vector<T> vec(n - m + 1);
  std::copy(v.begin() + m, v.begin() + n + 1, vec.begin());
  return vec;
}

template<typename T>
std::vector<T> SliceFirstN(const std::vector<T> &v, int n) {
  return Slice(v, 0, n - 1);
}

template<typename T>
std::vector<T> SliceLastN(const std::vector<T> &v, int n) {
  return Slice(v, v.size() - n, v.size() - 1);
}
}  // namespace algorithm
}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_ALGORITHM_HPP_
