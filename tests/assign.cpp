/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lwt_helper.hpp"
#include <vector>
#include <list>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <less/vector.hpp>

// These tests are largely complete but could use some fleshing out.
// They by and large check that assign() works when growing or shrinking the
// allocation, including when exceptions are thrown during this process but
// could use some extra robustness measures thrown in.
// * Explicitly test exceptions being thrown when regrowing the allocation
// * Explicitly test exceptions being thrown when appending extra elements after
// reallocation
// * Better test that the proper range (or subset of) is written to the array
// for assign(fiirst, last)
//
// What's currently tested is assign(count, value) and assign(first, last) for
// the cases when the vector is empty, pre-populated but growing, pre-populated
// but shrinking and pre-populated while keeping the same size.
//

template <class T>
using vector = less::vector<T>;

static int       count      = 0;
static int const limit      = 128 + 48;
static bool      was_thrown = false;

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

static int count_constructions      = 0;
static int count_copy_constructions = 0;
static int count_move_constructions = 0;
static int count_copy_assignments   = 0;
static int count_move_assignments   = 0;
static int count_destructions       = 0;

struct raii_tracker {
  int* x_;
  int  v_ = 0;

  raii_tracker()
  {
    ++count_constructions;
    x_ = new int{1};
  }

  raii_tracker(int v)
      : raii_tracker()
  {
    v_ = v;
  }

  raii_tracker(raii_tracker const& rhs)
  {
    ++count_copy_constructions;

    x_ = rhs.x_;
    *x_ += 1;

    v_ = rhs.v_;
  }

  raii_tracker(raii_tracker&& rhs) noexcept
  {
    ++count_move_constructions;

    x_     = rhs.x_;
    rhs.x_ = nullptr;

    v_ = rhs.v_;
  }

  ~raii_tracker()
  {
    ++count_destructions;
    if (!x_) { return; }
    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }
  }

  auto operator=(raii_tracker const& rhs) -> raii_tracker&
  {
    ++count_copy_assignments;
    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }

    x_ = rhs.x_;
    *x_ += 1;

    v_ = rhs.v_;

    return *this;
  }

  auto operator=(raii_tracker&& rhs) -> raii_tracker&
  {
    ++count_move_assignments;

    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }

    x_     = rhs.x_;
    rhs.x_ = nullptr;

    v_ = rhs.v_;

    return *this;
  }

  auto operator==(raii_tracker const& rhs) const noexcept -> bool
  {
    return v_ == rhs.v_;
  }

  auto operator!=(raii_tracker const& rhs) const noexcept -> bool
  {
    return v_ == rhs.v_;
  }
};

auto operator<<(std::ostream& os, raii_tracker const& rt) -> std::ostream&
{
  os << rt.v_;
  return os;
}

static void reset_counts()
{
  count      = 0;
  was_thrown = false;

  count_constructions      = 0;
  count_copy_constructions = 0;
  count_move_constructions = 0;
  count_copy_assignments   = 0;
  count_move_assignments   = 0;
  count_destructions       = 0;
}

static void assign_value_empty()
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v;
    BOOST_TEST_ASSERT(v.empty());
    BOOST_TEST_EQ(v.data(), nullptr);

    v.assign(size, value);
    BOOST_TEST_ASSERT_EQ(v.size(), size);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();
    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v;
    BOOST_TEST_ASSERT(v.empty());
    BOOST_TEST_EQ(v.data(), nullptr);

    v.assign(size, value);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_EQ(count_copy_constructions, size);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

static void assign_value_empty_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = limit + 10;
  auto const value = value_type{};

  vector<value_type> v;
  BOOST_TEST_ASSERT(v.empty());
  BOOST_TEST_EQ(v.data(), nullptr);

  try {
    v.assign(size, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST(!v.empty());
  BOOST_TEST_NE(v.data(), nullptr);
  BOOST_TEST_GE(v.capacity(), size);
  BOOST_TEST_GE(v.size(), 0);
  BOOST_TEST_LT(v.size(), size);
}

static void assign_value_empty_no_resize()
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v;
    v.reserve(2 * size);
    BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
    BOOST_TEST_ASSERT(v.empty());

    auto const p = v.data();

    v.assign(size, value);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v;
    v.reserve(2 * size);
    BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
    BOOST_TEST_ASSERT(v.empty());

    auto const p = v.data();

    v.assign(size, value);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_EQ(v.data(), p);
    BOOST_TEST_ASSERT_EQ(count_copy_constructions, size);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

static void assign_value_empty_no_resize_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = 2 * limit;
  auto const value = value_type{};

  vector<value_type> v;
  v.reserve(2 * size);
  BOOST_TEST_ASSERT_GE(v.capacity(), v.size());
  BOOST_TEST_ASSERT(v.empty());

  auto const p = v.data();

  auto const old_count = count;
  try {
    v.assign(size, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_EQ(v.size(), limit - old_count);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

static void assign_value_nonempty_resize_grows()
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = v.capacity() + 2;

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);

    for (auto i = 0u; i < v.size(); ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = v.capacity() + 2;

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);
    BOOST_TEST_ASSERT_EQ(count_destructions, size);
    BOOST_TEST_ASSERT_EQ(count_copy_constructions, new_cap);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

static void assign_value_nonempty_resize_grows_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = limit / 2;
  auto const value = value_type{};

  vector<value_type> v(size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_GE(v.capacity(), size);

  auto const p = v.data();

  auto const new_cap = v.capacity() + 2;

  try {
    v.assign(new_cap, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_EQ(v.size(), limit - size - 1);
  BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);
}

static void assign_value_nonempty_resize_shrinks()
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = v.size() / 2;

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = v.size() / 2;

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);
    BOOST_TEST_ASSERT_EQ(count_copy_assignments, new_cap);
    BOOST_TEST_ASSERT_EQ(count_destructions, size - new_cap);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

static void assign_value_nonempty_resize_shrinks_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = limit - 20;
  auto const value = value_type{};

  vector<value_type> v(size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_GE(v.capacity(), size);

  auto const p = v.data();

  auto const new_cap = v.size() / 2;

  try {
    v.assign(new_cap, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

static void assign_value_nonempty_resize_same()
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = size;

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = v.size();

    v.assign(new_cap, value);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);
    BOOST_TEST_ASSERT_EQ(count_copy_assignments, size);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

static void assign_value_nonempty_resize_same_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = 128u;
  auto const value = value_type{};

  vector<value_type> v(size);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_GE(v.capacity(), size);

  auto const p = v.data();

  auto const new_cap = size;

  try {
    v.assign(new_cap, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_empty(AssignOp assign_op)
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v;
    BOOST_TEST_ASSERT(v.empty());
    BOOST_TEST_EQ(v.data(), nullptr);

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v;
    BOOST_TEST_ASSERT(v.empty());
    BOOST_TEST_EQ(v.data(), nullptr);

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_empty_throws(AssignOp assign_op)
{
  reset_counts();

  using value_type = throwing;
  using iterator   = typename Container<value_type>::iterator;
  using category   = typename std::iterator_traits<iterator>::iterator_category;

  auto const size  = limit / 2;
  auto const value = value_type{};

  Container<value_type> init(size);
  vector<value_type>    v;
  BOOST_TEST_ASSERT(v.empty());
  BOOST_TEST_EQ(v.data(), nullptr);

  try {
    assign_op(v, init);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST(!v.empty());
  BOOST_TEST_NE(v.data(), nullptr);
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
    BOOST_TEST_GE(v.capacity(), size);
  }
  BOOST_TEST_GE(v.size(), 0);
  BOOST_TEST_LT(v.size(), size);
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_empty_no_resize(AssignOp assign_op)
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v;
    v.reserve(2 * size);
    BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
    BOOST_TEST_ASSERT(v.empty());

    auto const p = v.data();

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v;
    v.reserve(2 * size);
    BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
    BOOST_TEST_ASSERT(v.empty());

    auto const p = v.data();

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_EQ(v.data(), p);
    BOOST_TEST_ASSERT_EQ(count_copy_constructions, v.size() + init.size());

    for (auto i = 0u; i < size; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_empty_no_resize_throws(AssignOp assign_op)
{
  reset_counts();

  using value_type = throwing;

  auto const size = limit / 2;

  vector<value_type> v;
  v.reserve(2 * size);
  BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
  BOOST_TEST_ASSERT(v.empty());

  auto const p = v.data();

  auto const new_cap = size + 48 + 5;
  BOOST_TEST_ASSERT_GT(count + limit + 10, limit);
  BOOST_TEST_ASSERT_LE(new_cap, v.capacity());

  Container<value_type> init(new_cap);
  try {
    assign_op(v, init);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_GE(v.size(), 0);
  BOOST_TEST_ASSERT_LT(v.size(), new_cap);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_grows(AssignOp assign_op)
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const            p       = v.data();
    auto const            new_cap = v.capacity() + 2;
    Container<value_type> init(new_cap, value);

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_NE(v.data(), p);
    BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    vector<value_type> v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const            p       = v.data();
    auto const            new_cap = v.capacity() + 2;
    Container<value_type> init(new_cap, value);

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_NE(v.data(), p);
    BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_grows_throws(AssignOp assign_op)
{
  using value_type = throwing;
  using iterator   = typename Container<value_type>::iterator;
  using category   = typename std::iterator_traits<iterator>::iterator_category;

  reset_counts();

  auto const size = limit / 2;

  vector<value_type>  v(size - 10);
  Container<throwing> init(size + 10);
  BOOST_TEST_ASSERT_EQ(v.size(), size - 10);
  BOOST_TEST_ASSERT_GT(init.size(), v.capacity());

  auto const cap = v.capacity();
  (void)cap;

  try {
    assign_op(v, init);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_GE(v.size(), 0);
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
    BOOST_TEST_ASSERT_NE(v.capacity(), cap);
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_shrinks(AssignOp assign_op)
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size - 10, value);
    vector<value_type>    v(size + 10);
    BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
    BOOST_TEST_ASSERT_GE(v.capacity(), size + 10);

    auto const p = v.data();

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size - 10);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < v.size(); ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size - 10, value);
    vector<value_type>    v(size + 10);
    BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
    BOOST_TEST_ASSERT_GE(v.capacity(), size + 10);

    auto const p = v.data();

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), size - 10);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < v.size(); ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_shrinks_raii(AssignOp assign_op)
{
  using value_type = raii_tracker;
  using iterator   = typename Container<value_type>::iterator;
  using category   = typename std::iterator_traits<iterator>::iterator_category;

  reset_counts();

  auto const size = limit / 2;

  vector<value_type>    v(size + 10);
  Container<value_type> init(size - 10);
  BOOST_TEST_ASSERT_EQ(count_constructions, v.size() + init.size());
  BOOST_TEST_ASSERT_EQ(count_destructions, 0);
  BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
  BOOST_TEST_ASSERT_GE(v.capacity(), size + 10);

  auto const p = v.data();

  assign_op(v, init);
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
    BOOST_TEST_EQ(count_destructions, 20);
    BOOST_TEST_EQ(count_copy_assignments, init.size());
  }
  else {
    BOOST_TEST_EQ(count_destructions, size + 10);
    BOOST_TEST_GE(count_copy_constructions, init.size());
  }
  BOOST_TEST_ASSERT_EQ(v.size(), init.size());
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_shrinks_throws(AssignOp assign_op)
{
  reset_counts();

  using value_type = throwing;
  using iterator   = typename Container<value_type>::iterator;
  using category   = typename std::iterator_traits<iterator>::iterator_category;

  auto const size = limit / 2;

  vector<value_type>    v(size + 10);
  Container<value_type> init(size - 10);
  count -= 10;

  BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
  BOOST_TEST_ASSERT_GE(v.capacity(), size + 10);

  auto const p = v.data();

  try {
    assign_op(v, init);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
    BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
  }
  else {
    BOOST_TEST_EQ(v.size(), 10u);
  }
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_same(AssignOp assign_op)
{
  {
    using value_type = int;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = size;

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }

  {
    reset_counts();

    using value_type = raii_tracker;

    auto const size  = 128u;
    auto const value = value_type{1337};

    Container<value_type> init(size, value);
    vector<value_type>    v(size);
    BOOST_TEST_ASSERT_EQ(v.size(), size);
    BOOST_TEST_ASSERT_GE(v.capacity(), size);

    auto const p = v.data();

    auto const new_cap = size;

    assign_op(v, init);
    BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
    BOOST_TEST_ASSERT_EQ(v.data(), p);

    for (auto i = 0u; i < new_cap; ++i) {
      auto const& x = v[i];
      BOOST_TEST_ASSERT_EQ(x, value);
    }
  }
}

template <template <class, class...> class Container, class AssignOp>
static void assign_range_nonempty_resize_same_throws(AssignOp assign_op)
{
  reset_counts();

  using value_type = throwing;
  using iterator   = typename Container<value_type>::iterator;
  using category   = typename std::iterator_traits<iterator>::iterator_category;

  auto const size = limit / 2;

  vector<value_type> v(size + 10);
  reset_counts();

  Container<value_type> init(size + 10);
  BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
  BOOST_TEST_ASSERT_GE(v.capacity(), size + 10);

  auto const p = v.data();

  try {
    assign_op(v, init);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
    BOOST_TEST_ASSERT_EQ(v.size(), size + 10);
  }
  else {
    BOOST_TEST_ASSERT_GT(v.size(), 0u);
  }
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

static void assign_list_empty()
{
  using value_type = int;

  vector<value_type> v;
  BOOST_TEST_ASSERT(v.empty());
  BOOST_TEST_EQ(v.data(), nullptr);

  v.assign({1, 2, 3, 4, 5, 6});
  BOOST_TEST_ASSERT_EQ(v.size(), 6);

  for (auto i = 0u; i < v.size(); ++i) {
    auto const& x = v[i];
    BOOST_TEST_ASSERT_EQ(x, i + 1);
  }
}

int main()
{
  auto const assign_range = [](auto& vec, auto const& c) {
    vec.assign(c.begin(), c.end());
  };
  auto const copy_assign_vector = [](auto& vec, auto const& c) { vec = c; };

  assign_value_empty();
  assign_value_empty_throws();
  assign_value_empty_no_resize();
  assign_value_empty_no_resize_throws();
  assign_value_nonempty_resize_grows();
  assign_value_nonempty_resize_grows_throws();
  assign_value_nonempty_resize_shrinks();
  assign_value_nonempty_resize_shrinks_throws();
  assign_value_nonempty_resize_same();
  assign_value_nonempty_resize_same_throws();

  assign_range_empty<vector>(assign_range);
  assign_range_empty_throws<vector>(assign_range);
  assign_range_empty_no_resize<vector>(assign_range);
  assign_range_empty_no_resize_throws<vector>(assign_range);
  assign_range_nonempty_resize_grows<vector>(assign_range);
  assign_range_nonempty_resize_grows_throws<vector>(assign_range);
  assign_range_nonempty_resize_shrinks<vector>(assign_range);
  assign_range_nonempty_resize_shrinks_raii<vector>(assign_range);
  assign_range_nonempty_resize_shrinks_throws<vector>(assign_range);
  assign_range_nonempty_resize_same<vector>(assign_range);
  assign_range_nonempty_resize_same_throws<vector>(assign_range);

  assign_range_empty<std::list>(assign_range);
  assign_range_empty_throws<std::list>(assign_range);
  assign_range_empty_no_resize<std::list>(assign_range);
  assign_range_empty_no_resize_throws<std::list>(assign_range);
  assign_range_nonempty_resize_grows<std::list>(assign_range);
  assign_range_nonempty_resize_grows_throws<std::list>(assign_range);
  assign_range_nonempty_resize_shrinks<std::list>(assign_range);
  assign_range_nonempty_resize_shrinks_raii<std::list>(assign_range);
  assign_range_nonempty_resize_shrinks_throws<std::list>(assign_range);
  assign_range_nonempty_resize_same<std::list>(assign_range);
  assign_range_nonempty_resize_same_throws<std::list>(assign_range);

  assign_range_empty<vector>(copy_assign_vector);
  assign_range_empty_throws<vector>(copy_assign_vector);
  assign_range_empty_no_resize<vector>(copy_assign_vector);
  assign_range_empty_no_resize_throws<vector>(copy_assign_vector);
  assign_range_nonempty_resize_grows<vector>(copy_assign_vector);
  assign_range_nonempty_resize_grows_throws<vector>(copy_assign_vector);
  assign_range_nonempty_resize_shrinks<vector>(copy_assign_vector);
  assign_range_nonempty_resize_shrinks_raii<vector>(copy_assign_vector);
  assign_range_nonempty_resize_shrinks_throws<vector>(copy_assign_vector);
  assign_range_nonempty_resize_same<vector>(copy_assign_vector);
  assign_range_nonempty_resize_same_throws<vector>(copy_assign_vector);

  assign_list_empty();

  return boost::report_errors();
}
