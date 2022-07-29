/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/core/lightweight_test.hpp>

#include <less/vector.hpp>

#include <initializer_list>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

static void default_construct()
{
  {
    auto v = less::vector<int>();
    BOOST_TEST_EQ(v.size(), 0);
    BOOST_TEST_EQ(v.capacity(), 0);
    BOOST_TEST(v.begin() == v.end());
    BOOST_TEST_EQ(v.data(), nullptr);
  }

  {
    auto v = less::vector<std::unique_ptr<int>>();
    BOOST_TEST_EQ(v.size(), 0);
    BOOST_TEST_EQ(v.capacity(), 0);
    BOOST_TEST(v.begin() == v.end());
    BOOST_TEST_EQ(v.data(), nullptr);
  }
}

static void default_init_construct()
{
  {
    auto v = less::vector<int>(less::default_init, 1337u);
    BOOST_TEST_EQ(v.size(), 1337u);
    BOOST_TEST_GE(v.capacity(), 1337u);
    BOOST_TEST_EQ(v.end() - v.begin(), v.size());
    BOOST_TEST_NE(v.data(), nullptr);
  }

  {
    auto v = less::vector<std::unique_ptr<int>>(less::default_init, 1337u);
    BOOST_TEST_EQ(v.size(), 1337u);
    BOOST_TEST_GE(v.capacity(), 1337u);
    BOOST_TEST_EQ(v.end() - v.begin(), v.size());
    BOOST_TEST_NE(v.data(), nullptr);
  }
}

static void size_construct()
{
  auto v = less::vector<int>(1337u);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);

  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  for (auto i = 0u; i < v.size(); ++i) {
    if (!BOOST_TEST_EQ(v[i], 0)) { break; }
  }
}

static void with_capacity_construct()
{
  auto v = less::vector<int>(less::with_capacity, 1337u);
  BOOST_TEST_EQ(v.size(), 0);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
}

static void size_value_construct()
{
  auto v = less::vector<int>(1337u, 7331);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  for (auto i = 0u; i < v.size(); ++i) {
    if (!BOOST_TEST_EQ(v[i], 7331)) { break; }
  }
}

static void size_value_construct_raii()
{
  auto const value = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto v = less::vector<std::vector<int>>(1337u, value);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  for (auto i = 0u; i < v.size(); ++i) {
    BOOST_TEST_EQ(v[i].size(), value.size());
    if (!BOOST_TEST(v[i] == value)) {
      std::cerr << "Comparison failed at index " << i << "\n";
      std::cerr << std::endl;
      break;
    }
  }
}

static void copy_construct()
{
  auto v = less::vector<int>(1337u, 7331);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  auto v2 = less::vector<int>(v);
  BOOST_TEST_GE(v2.capacity(), 1337u);
  BOOST_TEST_EQ(v2.end() - v2.begin(), v2.size());
  BOOST_TEST_NE(v2.data(), nullptr);
  if (!BOOST_TEST_EQ(v2.size(), 1337u)) { return; }

  for (unsigned i = 0u; i < v.size(); ++i) {
    if (!BOOST_TEST_EQ(v2[i], v[i])) { break; }
  }
}

static void copy_construct_raii()
{
  auto const value = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto v = less::vector<std::vector<int>>(1337u, value);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  auto v2 = less::vector<std::vector<int>>(v);
  BOOST_TEST_GE(v2.capacity(), 1337u);
  BOOST_TEST_EQ(v2.end() - v2.begin(), v2.size());
  BOOST_TEST_NE(v2.data(), nullptr);
  if (!BOOST_TEST_EQ(v2.size(), 1337u)) { return; }

  for (auto i = 0u; i < v.size(); ++i) {
    BOOST_TEST_EQ(v2[i].size(), value.size());
    if (!BOOST_TEST(v2[i] == v[i])) {
      std::cerr << "Comparison failed at index " << i << "\n";
      std::cerr << std::endl;
      break;
    }
  }
}

static void move_construct()
{
  auto v = less::vector<int>(1337u, 7331);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  auto old_data = v.data();

  auto v2 = less::vector<int>(std::move(v));
  BOOST_TEST_EQ(v2.data(), old_data);
  BOOST_TEST_GE(v2.capacity(), 1337u);
  BOOST_TEST_EQ(v2.end() - v2.begin(), v2.size());
  if (!BOOST_TEST_EQ(v2.size(), 1337u)) { return; }

  for (auto i = 0u; i < v2.size(); ++i) {
    if (!BOOST_TEST_EQ(v2[i], 7331)) { break; }
  }
}

static void move_construct_raii()
{
  auto const value = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto v = less::vector<std::vector<int>>(1337u, value);
  BOOST_TEST_GE(v.capacity(), 1337u);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 1337u)) { return; }

  auto old_data = v.data();

  auto v2 = less::vector<std::vector<int>>(std::move(v));
  BOOST_TEST_EQ(v2.data(), old_data);
  BOOST_TEST_GE(v2.capacity(), 1337u);
  BOOST_TEST_EQ(v2.end() - v2.begin(), v2.size());
  if (!BOOST_TEST_EQ(v2.size(), 1337u)) { return; }

  for (auto i = 0u; i < v2.size(); ++i) {
    if (!BOOST_TEST(v2[i] == value)) {
      std::cerr << "Comparison failed at index " << i << "\n";
      std::cerr << std::endl;
      break;
    }
  }
}

static void iterator_construct_random_access()
{
  auto const value = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto v = less::vector<int>(value.begin(), value.end());
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  BOOST_TEST_GE(v.capacity(), value.size());
  if (!BOOST_TEST_EQ(v.size(), value.size())) { return; }

  BOOST_TEST_ALL_EQ(v.begin(), v.end(), value.begin(), value.end());
}

static void iterator_construct_bidirectional()
{
  auto const value = std::list<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto v = less::vector<int>(value.begin(), value.end());
  BOOST_TEST_GE(v.capacity(), value.size());
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), value.size())) { return; }

  BOOST_TEST_ALL_EQ(v.begin(), v.end(), value.begin(), value.end());
}

static void initializer_list_construct()
{
  auto v = less::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  BOOST_TEST_GE(v.capacity(), 10);
  BOOST_TEST_EQ(v.end() - v.begin(), v.size());
  BOOST_TEST_NE(v.data(), nullptr);
  if (!BOOST_TEST_EQ(v.size(), 10)) { return; }

  for (auto i = 0u; i < v.size(); ++i) {
    if (!BOOST_TEST_EQ(v[i], 1 + i)) { break; }
  }
}

static void over_aligned_construct()
{
  struct alignas(512) overaligned {
    int x = 0;
  };

  static_assert(alignof(overaligned) == 512);

  auto v = less::vector<overaligned>(1337u);

  auto addr = reinterpret_cast<std::uintptr_t>(v.data());
  BOOST_TEST_EQ(addr % 512, 0);

  v[0] = overaligned{7331};
  BOOST_TEST_EQ(v[0].x, 7331);
}

int main()
{
  default_construct();
  default_init_construct();
  size_construct();
  with_capacity_construct();
  size_value_construct();
  size_value_construct_raii();
  copy_construct();
  copy_construct_raii();
  move_construct();
  move_construct_raii();
  iterator_construct_random_access();
  iterator_construct_bidirectional();
  initializer_list_construct();
  over_aligned_construct();
  return boost::report_errors();
}
