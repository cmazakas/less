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
static void empty_begin()
{
  reset_counts();

  auto vec = vector<T>();
  BOOST_TEST_ASSERT(vec.empty());

  auto it = vec.insert(vec.begin(), T{});

  BOOST_TEST_ASSERT_EQ(vec.size(), 1u);
  BOOST_TEST_ASSERT_EQ(it, vec.begin());
}

template <class T>
static void empty_begin_multi()
{
  reset_counts();

  auto vec = vector<T>();
  BOOST_TEST_ASSERT(vec.empty());

  auto it = vec.insert(vec.begin(), limit / 2, T{});

  BOOST_TEST_ASSERT_EQ(vec.size(), limit / 2);
  BOOST_TEST_ASSERT_EQ(it, vec.begin());
}

template <class T>
static void empty_begin_no_resize()
{
  reset_counts();

  auto vec = vector<T>();
  vec.reserve(limit * 2);
  BOOST_TEST_ASSERT(vec.empty());

  auto it = vec.insert(vec.begin(), T{});

  BOOST_TEST_ASSERT_EQ(vec.size(), 1u);
  BOOST_TEST_ASSERT_EQ(it, vec.begin());
}

template <class T>
static void empty_begin_multi_no_resize()
{
  reset_counts();

  auto vec = vector<T>();
  vec.reserve(limit * 2);
  BOOST_TEST_ASSERT(vec.empty());

  auto it = vec.insert(vec.begin(), limit / 2, T{});

  BOOST_TEST_ASSERT_EQ(vec.size(), limit / 2);
  BOOST_TEST_ASSERT_EQ(it, vec.begin());
}

int main()
{
  empty_begin<int>();
  empty_begin<throwing>();
  empty_begin<std::unique_ptr<int>>();

  empty_begin_multi<int>();
  empty_begin_multi<throwing>();

  empty_begin_no_resize<int>();
  empty_begin_no_resize<throwing>();
  empty_begin_no_resize<std::unique_ptr<int>>();

  empty_begin_multi_no_resize<int>();
  empty_begin_multi_no_resize<throwing>();

  return boost::report_errors();
}
