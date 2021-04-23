//
// Created by Evan Su on 22/2/21.
//

#include <gtest/gtest.h>
#include <iridium/algorithm.hpp>
#include <vector>

TEST(FindNearestPeakHighTest, Success) {
  std::vector<int> v{1, 2, 3, 1, 2, 5, 4, 3, 9};
  EXPECT_EQ(iridium::algorithm::FindNearestPeakHigh(v, 2), 5);
}

TEST(FindNearestPeakHighTest, Failure1) {
  try {
    std::vector<int> v{1, 2, 3, 1, 2};
    iridium::algorithm::FindNearestPeakHigh(v, 3);
    FAIL() << "Expected std::out_of_range";
  } catch (std::out_of_range const &err) {
    EXPECT_EQ(
        err.what(),
        std::string("Out of range"));
  }
}

TEST(FindNearestPeakHighTest, Failure2) {
  std::vector<int> v{1, 2, 3, 4, 5};
  EXPECT_EQ(iridium::algorithm::FindNearestPeakHigh(v, 2), std::nullopt);
}

TEST(FindNearestPeakLowTest, Success) {
  std::vector<int> v{4, 3, 2, 4, 3, 1, 4, 5, 6};
  EXPECT_EQ(iridium::algorithm::FindNearestPeakLow(v, 2), 1);
}

TEST(FindNearestPeakLowTest, Failure1) {
  try {
    std::vector<int> v{3, 2, 1, 2, 3};
    iridium::algorithm::FindNearestPeakLow(v, 3);
    FAIL() << "Expected std::out_of_range";
  } catch (std::out_of_range const &err) {
    EXPECT_EQ(
        err.what(),
        std::string("Out of range"));
  }
}

TEST(FindNearestPeakLowTest, Failure2) {
  std::vector<int> v{5, 4, 3, 2, 1};
  EXPECT_EQ(iridium::algorithm::FindNearestPeakLow(v, 2), std::nullopt);
}

TEST(VectorSliceTest, Success) {
  std::vector<int> v{1, 2, 3, 4, 5};
  std::vector<int> sliced_v{1, 2, 3};
  EXPECT_EQ(iridium::algorithm::Slice(v, 0, 2), sliced_v);
}

TEST(VectorSliceFirstNTest, Success) {
  std::vector<int> v{1, 2, 3, 4, 5};
  std::vector<int> sliced_v{1, 2, 3};
  EXPECT_EQ(iridium::algorithm::SliceFirstN(v, 3), sliced_v);
}

TEST(VectorSliceLastNTest, Success) {
  std::vector<int> v{1, 2, 3, 4, 5};
  std::vector<int> sliced_v{3, 4, 5};
  EXPECT_EQ(iridium::algorithm::SliceLastN(v, 3), sliced_v);
}