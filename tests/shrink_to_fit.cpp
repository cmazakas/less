/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lwt_helper.hpp"

#include <memory>

#include <less/vector.hpp>

template <class T>
using vector = less::vector<T>;

static int       count      = 0;
static int const limit      = 128;
static bool      was_thrown = false;

static void reset_counts()
{
  count      = 0;
  was_thrown = false;
}

struct throwing {
  int* x_;

  throwing()
      : x_{}
  {
    ++count;
    if (count > limit) { throw 42; }

    x_ = new int{1};
  }

  throwing(throwing const& rhs)
  {
    ++count;
    if (count > limit) { throw 42; }

    x_ = rhs.x_;
    *x_ += 1;
  }

  throwing(throwing&&) = delete;

  ~throwing()
  {
    if (!x_) { return; }
    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }
  }

  auto operator=(throwing const& rhs) -> throwing&
  {
    ++count;
    if (count > limit) { throw 42; }

    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }

    x_ = rhs.x_;
    *x_ += 1;

    return *this;
  }
};

template <class T>
static void empty()
{
  reset_counts();

  auto vec = vector<T>();
  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.size(), 0u);
  BOOST_TEST_EQ(vec.capacity(), 0u);

  vec.shrink_to_fit();

  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.size(), 0u);
  BOOST_TEST_EQ(vec.capacity(), 0u);
}

template <class T>
static void empty_with_capacity()
{
  reset_counts();

  auto vec = vector<T>();
  vec.reserve(limit);

  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.size(), 0u);
  BOOST_TEST_EQ(vec.capacity(), 128u);

  vec.shrink_to_fit();

  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.size(), 0u);
  BOOST_TEST_EQ(vec.capacity(), 128u);
}

template <class T>
static void shrink()
{
  reset_counts();

  auto vec = vector<T>(limit / 2);
  reset_counts();

  vec.reserve(limit * 2);
  reset_counts();

  BOOST_TEST_EQ(vec.size(), limit / 2);
  BOOST_TEST_EQ(vec.capacity(), limit * 2);

  vec.shrink_to_fit();
  BOOST_TEST_EQ(vec.size(), limit / 2);
  BOOST_TEST_EQ(vec.capacity(), vec.size());
}

static void shrink_throws()
{
  reset_counts();

  auto const count = (limit * 3) / 4;

  auto vec = vector<throwing>(count);
  reset_counts();

  vec.reserve(limit * 2);

  BOOST_TEST_EQ(vec.size(), count);
  BOOST_TEST_EQ(vec.capacity(), limit * 2);

  try {
    vec.shrink_to_fit();
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_EQ(vec.size(), count);
  BOOST_TEST_EQ(vec.capacity(), limit * 2);
}

template <class T>
static void shrink_same()
{
  reset_counts();

  auto vec = vector<T>(limit);
  reset_counts();

  BOOST_TEST_EQ(vec.size(), limit);
  BOOST_TEST_EQ(vec.capacity(), vec.size());

  vec.shrink_to_fit();

  BOOST_TEST_EQ(vec.size(), limit);
  BOOST_TEST_EQ(vec.capacity(), vec.size());
}

int main()
{
  empty<int>();
  empty<throwing>();
  empty<std::unique_ptr<int>>();

  shrink<int>();
  shrink<throwing>();
  shrink<std::unique_ptr<int>>();

  shrink_same<int>();
  shrink_same<throwing>();
  shrink_same<std::unique_ptr<int>>();

  shrink_throws();

  return boost::report_errors();
}
