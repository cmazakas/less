/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lwt_helper.hpp"

#include <memory>
#include <vector>
#include <list>
#include <iterator>

#include <less/vector.hpp>

template <class T>
using vector = less::vector<T>;

static int       tcount     = 0;
static int const limit      = 128;
static bool      was_thrown = false;

static void reset_counts()
{
  tcount     = 0;
  was_thrown = false;
}

struct throwing {
  int* x_;

  throwing()
      : x_{}
  {
    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = new int{1};
  }

  throwing(throwing const& rhs)
  {
    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = rhs.x_;
    *x_ += 1;
  }

  throwing(throwing&& rhs) noexcept(false)
  {
    x_     = rhs.x_;
    rhs.x_ = nullptr;
  }

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
    ++tcount;
    if (tcount > limit) { throw 42; }

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

static void assign_int_single()
{
  {
    // empty, insert begin, resize
    auto const& value = 1337;

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);

    auto it = vec.insert(vec.begin(), value);
    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337}));
  }

  {
    // empty, insert begin, no resize
    auto const& value = 1337;

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);

    auto it = vec.insert(vec.begin(), value);
    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337}));
  }

  {
    // empty, insert end, resize
    auto const& value = 1337;

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);

    auto it = vec.insert(vec.end(), value);
    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337}));
  }

  {
    // empty, insert end, no resize
    auto const& value = 1337;

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);

    auto it = vec.insert(vec.end(), value);
    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337}));
  }

  {
    // non-empty, insert begin, resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert begin, no resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.begin());
    auto const expected = vector<int>{1337, 1, 2, 3, 4};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
  }

  {
    // non-empty, insert middle, resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 3, 4}));
  }

  {
    // non-empty, insert middle, no resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 3, 4}));
  }

  {
    // non-empty, insert end, resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.end() - 1);
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337}));
  }

  {
    // non-empty, insert end, no resize
    auto const& value = 1337;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), value);
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST(it == vec.end() - 1);
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337}));
  }
}

static void assign_int_multi()
{
  {
    // empty, insert begin, resize
    auto const& value = 1337;
    auto const  count = 32u;

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), count);

    auto it = vec.insert(vec.begin(), count, value);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>(count, value)));
  }

  {
    // empty, insert begin, no resize
    auto const& value = 1337;
    auto const  count = 32u;

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GT(vec.capacity(), count);

    auto it = vec.insert(vec.begin(), count, value);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>(count, value)));
  }

  {
    // empty, insert end, resize
    auto const& value = 1337;
    auto const  count = 32u;

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), count);

    auto it = vec.insert(vec.end(), count, value);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>(count, value)));
  }

  {
    // empty, insert end, no resize
    auto const& value = 1337;
    auto const  count = 32u;

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GT(vec.capacity(), count);

    auto it = vec.insert(vec.end(), count, value);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>(count, value)));
  }

  {
    // non-empty, insert begin, resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1337, 1337, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert begin, no resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1337, 1337, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert middle, resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1337, 1337, 3, 4}));
  }

  {
    // non-empty, insert middle, no resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1337, 1337, 3, 4}));
  }

  {
    // non-empty, insert end, resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.end() - count);
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1337, 1337}));
  }

  {
    // non-empty, insert end, no resize
    auto const& value = 1337;
    auto const  count = 3u;

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), count, value);
    BOOST_TEST_EQ(vec.size(), 7u);
    BOOST_TEST(it == vec.end() - count);
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1337, 1337}));
  }
}

static void assign_int_range_random_access()
{
  {
    // empty, insert begin, resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), range.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == range));
  }

  {
    // empty, insert begin, no resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GE(vec.capacity(), range.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == range));
  }

  {
    // empty, insert end, resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), range.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == range));
  }

  {
    // empty, insert end, no resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GT(vec.capacity(), range.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == range));
  }

  {
    // non-empty, insert begin, resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1338, 1339, 1340, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert begin, no resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1338, 1339, 1340, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert middle, resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1338, 1339, 1340, 3, 4}));
  }

  {
    // non-empty, insert middle, no resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1338, 1339, 1340, 3, 4}));
  }

  {
    // non-empty, insert end, resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.end() - range.size());
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1338, 1339, 1340}));
  }

  {
    // non-empty, insert end, no resize
    auto const range = vector<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.end() - range.size());
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1338, 1339, 1340}));
  }
}

static void assign_int_range_bidirectional()
{
  {
    // empty, insert begin, resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), range.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), range.begin(), range.end());
  }

  {
    // empty, insert begin, no resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GE(vec.capacity(), range.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), range.begin(), range.end());
  }

  {
    // empty, insert end, resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.data(), nullptr);
    BOOST_TEST_LT(vec.capacity(), range.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), range.begin(), range.end());
  }

  {
    // empty, insert end, no resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>();
    vec.reserve(64);
    BOOST_TEST(vec.empty());
    BOOST_TEST_NE(vec.data(), nullptr);
    BOOST_TEST_GT(vec.capacity(), range.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), range.size());
    BOOST_TEST(it == vec.begin());
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), range.begin(), range.end());
  }

  {
    // non-empty, insert begin, resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1338, 1339, 1340, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert begin, no resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin());
    BOOST_TEST((vec == vector<int>{1337, 1338, 1339, 1340, 1, 2, 3, 4}));
  }

  {
    // non-empty, insert middle, resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1338, 1339, 1340, 3, 4}));
  }

  {
    // non-empty, insert middle, no resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.begin() + 2, range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.begin() + 2);
    BOOST_TEST((vec == vector<int>{1, 2, 1337, 1338, 1339, 1340, 3, 4}));
  }

  {
    // non-empty, insert end, resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    BOOST_TEST(!vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.end() - range.size());
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1338, 1339, 1340}));
  }

  {
    // non-empty, insert end, no resize
    auto const range = std::list<int>{1337, 1338, 1339, 1340};

    auto vec = vector<int>{1, 2, 3, 4};
    vec.reserve(64);
    BOOST_TEST(!vec.empty());
    BOOST_TEST_GT(vec.capacity(), vec.size());

    auto it = vec.insert(vec.end(), range.begin(), range.end());
    BOOST_TEST_EQ(vec.size(), 8u);
    BOOST_TEST(it == vec.end() - range.size());
    BOOST_TEST((vec == vector<int>{1, 2, 3, 4, 1337, 1338, 1339, 1340}));
  }
}

int main()
{
  assign_int_single();
  assign_int_multi();
  assign_int_range_random_access();
  assign_int_range_bidirectional();

  return boost::report_errors();
}
