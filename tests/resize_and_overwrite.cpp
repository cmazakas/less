/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void empty()
{
  auto const size = 1024u;

  less::vector<int> v;
  BOOST_TEST_EQ(v.capacity(), 0u);
  BOOST_TEST_ASSERT(v.empty());

  v.resize_and_overwrite(size, [](int* p, auto n) {
    for (auto i = 0u; i < n; ++i) {
      p[i] = 1337;
    }
    return n;
  });

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto i = 0u; i < size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i], 1337);
  }
}

static void empty_raii()
{
  auto const size = 1024u;

  less::vector<less::vector<int>> v;
  BOOST_TEST_EQ(v.capacity(), 0u);
  BOOST_TEST_ASSERT(v.empty());

  v.resize_and_overwrite(size, [](auto* vecs, auto new_len) {
    for (auto i = 0u; i < new_len; ++i) {
      vecs[i] = less::vector<int>(128u);
    }
    return new_len;
  });

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto i = 0u; i < size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i].size(), 128u);
  }
}

static void empty_throws_in_functor()
{
  auto const size = 1024u;

  less::vector<less::vector<int>> v;
  BOOST_TEST_EQ(v.capacity(), 0u);
  BOOST_TEST_ASSERT(v.empty());

  try {
    v.resize_and_overwrite(size, [](auto* vecs, auto new_len) {
      for (auto i = 0u; i < new_len / 2; ++i) {
        vecs[i] = less::vector<int>(128u);
      }
      throw 42;
      return new_len;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto i = 0u; i < size / 2; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i].size(), 128u);
  }

  for (auto i = size / 2; i < size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i].size(), 0u);
  }
}

static void prepopulated_grow()
{
  auto const size = 512u;

  less::vector<unsigned> v(less::with_capacity, size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(i);
  }

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  v.resize_and_overwrite(size * 2, [=](auto* p, auto n) {
    for (auto i = size; i < n; ++i) {
      p[i] = i;
    }
    return n;
  });

  BOOST_TEST_EQ(v.capacity(), 2 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), 2 * size);

  for (auto i = 0u; i < 2 * size; ++i) {
    BOOST_TEST_EQ(v[i], i);
  }
}

int main()
{
  empty();
  empty_raii();
  empty_throws_in_functor();
  prepopulated_grow();
  return boost::report_errors();
}
