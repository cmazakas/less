#include "lwt_helper.hpp"
#include <vector>
#include <less/vector.hpp>

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

  ~throwing()
  {
    *x_ -= 1;
    if (*x_ == 0) { delete x_; }
  }

  auto operator=(throwing const&) -> throwing&
  {
    ++count;
    if (count > limit) { throw 42; }
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
  raii_tracker()
  {
    ++count_constructions;
  }

  raii_tracker(raii_tracker const&)
  {
    ++count_copy_constructions;
  }

  raii_tracker(raii_tracker&&) noexcept
  {
    ++count_move_constructions;
  }

  ~raii_tracker()
  {
    ++count_destructions;
  }

  auto operator=(raii_tracker const&) -> raii_tracker&
  {
    ++count_copy_assignments;
    return *this;
  }

  auto operator=(raii_tracker&&) -> raii_tracker&
  {
    ++count_move_assignments;
    return *this;
  }
};

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

static void assign_value_empty_no_resize_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = 128u;
  auto const value = value_type{};

  vector<value_type> v;
  v.reserve(2 * size);
  BOOST_TEST_ASSERT_GE(v.capacity(), 2 * size);
  BOOST_TEST_ASSERT(v.empty());

  auto const p = v.data();

  auto const new_cap = size + 48 + 5;
  BOOST_TEST_ASSERT_LE(new_cap, v.capacity());
  try {
    v.assign(new_cap, value);
  }
  catch (...) {
    was_thrown = true;
  }

  BOOST_TEST_ASSERT(was_thrown);
  BOOST_TEST_ASSERT_GE(v.size(), 0);
  BOOST_TEST_ASSERT_LT(v.size(), new_cap);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

static void assign_value_nonempty_resize_grows()
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
  BOOST_TEST_ASSERT_NE(v.data(), p);
  BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);

  for (auto i = 0u; i < new_cap; ++i) {
    auto const& x = v[i];
    BOOST_TEST_ASSERT_EQ(x, value);
  }
}

static void assign_value_nonempty_resize_grows_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = 128u;
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
  BOOST_TEST_ASSERT_GE(v.size(), 0);
  BOOST_TEST_ASSERT_NE(v.data(), p);
  BOOST_TEST_ASSERT_GE(v.capacity(), new_cap);
}

static void assign_value_nonempty_resize_shrinks()
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

static void assign_value_nonempty_resize_shrinks_raii()
{
  reset_counts();

  using value_type = raii_tracker;

  auto const size  = 128u;
  auto const value = value_type{};

  vector<value_type> v(size);
  BOOST_TEST_EQ(count_constructions, 1 + size);
  BOOST_TEST_EQ(count_destructions, 0);
  BOOST_TEST_ASSERT_EQ(v.size(), size);
  BOOST_TEST_ASSERT_GE(v.capacity(), size);

  auto const p = v.data();

  auto const new_cap = v.size() / 2;

  v.assign(new_cap, value);
  BOOST_TEST_EQ(count_destructions, size - new_cap);
  BOOST_TEST_EQ(count_copy_assignments, new_cap);
  BOOST_TEST_ASSERT_EQ(v.size(), new_cap);
  BOOST_TEST_ASSERT_EQ(v.data(), p);
}

static void assign_value_nonempty_resize_shrinks_throws()
{
  reset_counts();

  using value_type = throwing;

  auto const size  = 128u;
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

int main()
{
  assign_value_empty();
  assign_value_empty_throws();
  assign_value_empty_no_resize();
  assign_value_empty_no_resize_throws();
  assign_value_nonempty_resize_grows();
  assign_value_nonempty_resize_grows_throws();
  assign_value_nonempty_resize_shrinks();
  assign_value_nonempty_resize_shrinks_raii();
  assign_value_nonempty_resize_shrinks_throws();
  assign_value_nonempty_resize_same();
  assign_value_nonempty_resize_same_throws();
  return boost::report_errors();
}
