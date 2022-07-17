/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lwt_helper.hpp"

#include <less/vector.hpp>

static auto       count = 0u;
static auto const limit = 750u;

static auto throw_during_copy_construction = false;

struct throwing {
  unsigned  x_ = 0u;
  unsigned* p_ = nullptr;

  throwing()
  {
    ++count;
    if (count >= limit) { throw 42; }

    p_  = static_cast<unsigned*>(::operator new(sizeof(*p_)));
    *p_ = 1;
  }

  throwing(unsigned x)
      : x_(x)
  {
    ++count;
    if (count >= limit) { throw 42; }

    p_  = static_cast<unsigned*>(::operator new(sizeof(*p_)));
    *p_ = 1;
  }

  throwing(throwing const& rhs)
      : x_(rhs.x_)
      , p_(rhs.p_)
  {
    if (throw_during_copy_construction) { throw 42; }

    *p_ += 1;
  }

  throwing(throwing&&) = delete;

  ~throwing()
  {
    *p_ -= 1;
    if (*p_ == 0) { ::operator delete(p_); }
  }
};

void reset_counts()
{
  count = 0u;

  throw_during_copy_construction = false;
}

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
    BOOST_TEST_ASSERT_EQ(v[i], i);
  }
}

static void prepopulated_grow_throws_in_functor()
{
  auto const size = 512u;

  less::vector<unsigned> v(less::with_capacity, size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(i);
  }

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  try {
    v.resize_and_overwrite(size * 2, [=](auto* p, auto n) {
      for (auto i = size; i < n; ++i) {
        p[i] = i;
        if (i == size + 10) { throw 42; }
      }
      return n;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), 2 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), 2 * size);

  for (auto i = 0u; i < 10 + size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i], i);
  }
}

static void prepopulated_grow_throws_in_resize()
{
  reset_counts();

  auto const size = 512u;

  less::vector<throwing> v(size);
  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  try {
    v.resize_and_overwrite(2 * size, [](auto* p, auto n) {
      for (auto i = 0u; i < n; ++i) {
        p[i].x_ = i;
      }
      return n;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto t : v) {
    BOOST_TEST_ASSERT_EQ(t.x_, 0u);
  }
}

static void prepopulated_grow_throws_in_resize_during_copy_construction()
{
  reset_counts();

  auto const size = 256u;

  less::vector<throwing> v(size);
  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  throw_during_copy_construction = true;

  try {
    v.resize_and_overwrite(2 * size, [](auto* p, auto n) {
      for (auto i = 0u; i < n; ++i) {
        p[i].x_ = i;
      }
      return n;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto const& t : v) {
    BOOST_TEST_ASSERT_EQ(t.x_, 0u);
  }
}

static void prepopulated_grow_no_realloc()
{
  auto const size = 512u;

  less::vector<unsigned> v(less::with_capacity, 4 * size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(i);
  }

  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  v.resize_and_overwrite(size * 2, [=](auto* p, auto n) {
    for (auto i = size; i < n; ++i) {
      p[i] = i;
    }
    return n;
  });

  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), 2 * size);

  for (auto i = 0u; i < 2 * size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i], i);
  }
}

static void prepopulated_grow_throws_in_functor_no_realloc()
{
  auto const size = 512u;

  less::vector<unsigned> v(less::with_capacity, 4 * size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(i);
  }

  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  try {
    v.resize_and_overwrite(size * 2, [=](auto* p, auto n) {
      for (auto i = size; i < n; ++i) {
        p[i] = i;
        if (i == size + 10) { throw 42; }
      }
      return n;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), 2 * size);

  for (auto i = 0u; i < 10 + size; ++i) {
    BOOST_TEST_ASSERT_EQ(v[i], i);
  }
}

static void prepopulated_grow_throws_in_resize_norealloc()
{
  reset_counts();

  auto const size = 512u;

  less::vector<throwing> v(less::with_capacity, 4 * size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(throwing());
  }
  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  try {
    v.resize_and_overwrite(2 * size, [](auto* p, auto n) {
      for (auto i = 0u; i < n; ++i) {
        p[i].x_ = i;
      }
      return n;
    });
  }
  catch (...) {
  }

  BOOST_TEST_EQ(v.capacity(), 4 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);

  for (auto const& t : v) {
    BOOST_TEST_ASSERT_EQ(t.x_, 0u);
  }
}

static void clear()
{
  auto const size = 512u;

  less::vector<int> v(size);
  v.resize_and_overwrite(2 * size, [](auto* p, auto n) {
    (void)p;
    (void)n;
    return 0u;
  });

  BOOST_TEST_GE(v.capacity(), 2 * size);
  BOOST_TEST_ASSERT_EQ(v.size(), 0u);
}

int main()
{
  empty();
  empty_raii();
  empty_throws_in_functor();
  prepopulated_grow();
  prepopulated_grow_throws_in_functor();
  prepopulated_grow_throws_in_resize();
  prepopulated_grow_throws_in_resize_during_copy_construction();
  prepopulated_grow_no_realloc();
  prepopulated_grow_throws_in_functor_no_realloc();
  prepopulated_grow_throws_in_resize_norealloc();
  clear();
  return boost::report_errors();
}
