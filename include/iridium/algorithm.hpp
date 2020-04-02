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
}  // namespace algorithm
}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_ALGORITHM_HPP_
