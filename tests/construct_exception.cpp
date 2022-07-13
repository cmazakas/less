/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/core/lightweight_test.hpp>

#include <less/vector.hpp>
#include <iterator>
#include <list>

static int construct_count = 0;
static int destruct_count  = 0;

constexpr static int const limit = 10;

static void reset_counts()
{
  construct_count = 0;
  destruct_count  = 0;
}

struct throwing {
  throwing()
  {
    if (construct_count >= limit) { throw 42; }
    ++construct_count;
  }

  throwing(throwing const&)
  {
    if (construct_count >= limit) { throw 42; }
    ++construct_count;
  }

  ~throwing()
  {
    ++destruct_count;
  }
};

static void default_construct()
{
  reset_counts();

  auto v = less::vector<throwing>();
  BOOST_TEST_EQ(construct_count, 0);
  BOOST_TEST_EQ(destruct_count, 0);
}

static void default_init_construct()
{
  reset_counts();

  try {
    auto v = less::vector<throwing>(less::default_init, limit + 5);
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, limit);
}

static void size_construct()
{
  reset_counts();

  try {
    auto v = less::vector<throwing>(limit + 5);
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, limit);
}

static void with_capacity_construct()
{
  reset_counts();

  auto v = less::vector<throwing>(less::with_capacity, limit + 5);
  BOOST_TEST_EQ(construct_count, 0);
  BOOST_TEST_EQ(destruct_count, 0);
}

static void size_value_construct()
{
  reset_counts();

  try {
    auto v = less::vector<throwing>(limit - 1, throwing{});
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, limit);
}

static void copy_construct()
{
  reset_counts();

  auto v = less::vector<throwing>(limit - 3);
  try {
    auto v2 = less::vector<throwing>(v);
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, 3);
}

static void move_construct()
{
  reset_counts();

  auto v  = less::vector<throwing>(limit - 1);
  auto v2 = less::vector<throwing>(std::move(v));
  BOOST_TEST_EQ(construct_count, limit - 1);
  BOOST_TEST_EQ(destruct_count, 0);
}

static void iterator_construct_random_access()
{
  reset_counts();

  auto v = less::vector<throwing>(limit - 4);

  try {
    auto v2 = less::vector<throwing>(v.begin(), v.end());
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, 4);
}

static void iterator_construct_bidirectional()
{
  reset_counts();

  auto v = std::list<throwing>(limit - 4);

  try {
    auto v2 = less::vector<throwing>(v.begin(), v.end());
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, 4);
}

static void initializer_list_construct()
{
  reset_counts();

  auto il = std::initializer_list<throwing>{throwing{}, throwing{}, throwing{}, throwing{}, throwing{}, throwing{}};

  try {
    auto v = less::vector<throwing>(il);
  }
  catch (...) {
  }

  BOOST_TEST_EQ(construct_count, limit);
  BOOST_TEST_EQ(destruct_count, 4);
}

int main()
{
  default_construct();
  default_init_construct();
  size_construct();
  with_capacity_construct();
  size_value_construct();
  copy_construct();
  move_construct();
  iterator_construct_random_access();
  iterator_construct_bidirectional();
  initializer_list_construct();

  return boost::report_errors();
}
