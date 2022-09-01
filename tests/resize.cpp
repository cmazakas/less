#include "lwt_helper.hpp"
#include "throwing.hpp"

#include <memory>
#include <less/vector.hpp>

template <class ResizeOp, class T>
static void empty_resize(T const& expected_value, ResizeOp resize)
{
  {
    auto vec = less::vector<int>();
    BOOST_TEST_EQ(vec.capacity(), 0u);

    auto count = 24u;
    resize(vec, count);
    for (auto const& v : vec) {
      BOOST_TEST_ASSERT_EQ(v, expected_value);
    }
  }

  {
    auto vec = less::vector<int>();
    BOOST_TEST_EQ(vec.capacity(), 0u);

    auto count = 0u;
    resize(vec, count);
    BOOST_TEST_EQ(vec.capacity(), 0u);
  }
}

template <class ResizeOp, class T>
static void empty_resize_throws(T const&, ResizeOp resize)
{
  {
    reset_counts();

    auto vec = less::vector<throwing>();
    BOOST_TEST_EQ(vec.capacity(), 0u);

    try {
      auto count = limit + 1;
      resize(vec, count);
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.capacity(), 0u);
  }
}

template <class ResizeOp, class T>
static void empty_no_resize(T const& expected_value, ResizeOp resize)
{
  {
    auto const count = 24u;

    auto vec = less::vector<int>();
    vec.reserve(count);
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.capacity(), count);

    resize(vec, count);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST_EQ(vec.capacity(), count);
    for (auto const& v : vec) {
      BOOST_TEST_ASSERT_EQ(v, expected_value);
    }
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(12);

    auto const capacity = vec.capacity();
    BOOST_TEST(vec.empty());
    BOOST_TEST_GE(capacity, 0u);

    auto count = 0u;
    resize(vec, count);
    BOOST_TEST_EQ(vec.size(), count);
    BOOST_TEST_EQ(vec.capacity(), capacity);
  }
}

template <class ResizeOp, class T>
static void empty_no_resize_throws(T const&, ResizeOp resize)
{
  reset_counts();

  auto const count = 24u;

  auto vec = less::vector<throwing>();
  vec.reserve(count);

  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.capacity(), count);

  tcount = limit - (count / 2);

  LESS_TRY([&] { resize(vec, count); });

  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.capacity(), count);
}

template <class ResizeOp, class T>
static void non_empty_grow_resize(T const& expected_value, ResizeOp resize)
{
  auto vec = less::vector<int>{1, 2, 3, 4, 5};
  BOOST_TEST_EQ(vec.size(), vec.capacity());

  auto const count = 24u;
  BOOST_TEST_GT(count, vec.capacity());

  resize(vec, count);

  BOOST_TEST_EQ(vec.size(), count);
  for (auto i = 0u; i < 5u; ++i) {
    BOOST_TEST_ASSERT_EQ(vec[i], static_cast<int>(i + 1));
  }
  for (auto i = 5u; i < vec.size(); ++i) {
    BOOST_TEST_ASSERT_EQ(vec[i], expected_value);
  }
}

template <class ResizeOp, class T>
static void non_empty_grow_resize_throws(T const&, ResizeOp resize)
{
  {
    // throws when constructing the tail, [size(), count)
    //
    reset_counts();

    auto vec      = less::vector<throwing>((limit * 3) / 4);
    auto size     = vec.size();
    auto capacity = vec.capacity();

    auto const count     = 2 * limit;
    auto const num_extra = count - size;

    reset_tracking();
    LESS_TRY([&] { resize(vec, count); });

    BOOST_TEST_LT(num_constructions, num_extra);
    BOOST_TEST_EQ(vec.size(), size);
    BOOST_TEST_EQ(vec.capacity(), capacity);
  }

  {
    // throws when copying over the current [0, size()) elements
    //
    reset_counts();

    auto vec      = less::vector<throwing>(limit / 2);
    auto size     = vec.size();
    auto capacity = vec.capacity();

    auto const count = limit - 10;

    LESS_TRY([&] { resize(vec, count); });

    BOOST_TEST_EQ(vec.size(), size);
    BOOST_TEST_EQ(vec.capacity(), capacity);
  }
}

template <class ResizeOp, class T>
static void non_empty_grow_no_resize(T const& expected_value, ResizeOp resize)
{
  auto const count = 24u;

  auto vec = less::vector<int>{1, 2, 3, 4, 5};
  vec.reserve(count);

  auto const capacity = vec.capacity();
  BOOST_TEST_GE(capacity, count);

  resize(vec, count);

  BOOST_TEST_EQ(vec.size(), count);
  BOOST_TEST_EQ(vec.capacity(), capacity);
  for (auto i = 0u; i < 5u; ++i) {
    BOOST_TEST_ASSERT_EQ(vec[i], static_cast<int>(i + 1));
  }
  for (auto i = 5u; i < vec.size(); ++i) {
    BOOST_TEST_ASSERT_EQ(vec[i], expected_value);
  }
}

template <class ResizeOp, class T>
static void non_empty_grow_no_resize_throws(T const&, ResizeOp resize)
{
  reset_counts();
  auto const count = static_cast<unsigned>(limit + 1);

  auto vec = less::vector<throwing>(5);
  vec.reserve(count);

  auto const capacity = vec.capacity();
  BOOST_TEST_GE(capacity, count);

  LESS_TRY([&] { resize(vec, count); });

  BOOST_TEST_EQ(vec.size(), 5);
  BOOST_TEST_EQ(vec.capacity(), capacity);
}

template <class ResizeOp, class T>
static void non_empty_shrink(T const&, ResizeOp resize)
{
  {
    auto vec = less::vector<int>{1, 2, 3, 4, 5};

    resize(vec, vec.size() - 1);
    BOOST_TEST((vec == less::vector<int>{1, 2, 3, 4}));

    resize(vec, vec.size() - 1);
    BOOST_TEST((vec == less::vector<int>{1, 2, 3}));

    resize(vec, vec.size() - 1);
    BOOST_TEST((vec == less::vector<int>{1, 2}));

    resize(vec, vec.size() - 1);
    BOOST_TEST((vec == less::vector<int>{1}));

    resize(vec, vec.size() - 1);
    BOOST_TEST((vec == less::vector<int>{}));

    BOOST_TEST(vec.empty());
  }

  {
    reset_counts();
    reset_tracking();
    auto vec = less::vector<throwing>(5);
    BOOST_TEST_EQ(num_destructions, 0);
    for (auto i = 0; i < 5; ++i) {
      vec.resize(vec.size() - 1);
      BOOST_TEST_EQ(num_destructions, i + 1);
    }
  }

  {
    reset_counts();
    reset_tracking();
    auto vec = less::vector<throwing>(5);
    BOOST_TEST_EQ(num_destructions, 0);
    vec.resize(vec.size());
    BOOST_TEST_EQ(vec.size(), 5u);
    BOOST_TEST_EQ(num_destructions, 0);
  }
}

int main()
{
  auto f = [](auto& vec, auto count) { vec.resize(count); };
  auto g = [](auto& vec, auto count) { vec.resize(count, 1337); };
  auto h = [](auto& vec, auto count) { vec.resize(count, throwing()); };

  empty_resize(0, f);
  empty_resize_throws(0, f);
  empty_no_resize(0, f);
  empty_no_resize_throws(0, f);
  non_empty_grow_resize(0, f);
  non_empty_grow_resize_throws(0, f);
  non_empty_grow_no_resize(0, f);
  non_empty_grow_no_resize_throws(0, f);
  non_empty_shrink(0, f);

  empty_resize(1337, g);
  empty_resize_throws(1337, h);
  empty_no_resize(1337, g);
  empty_no_resize_throws(1337, h);
  non_empty_grow_resize(1337, g);
  non_empty_grow_resize_throws(1337, h);
  non_empty_grow_no_resize(1337, g);
  non_empty_grow_no_resize_throws(1337, h);
  non_empty_shrink(1337, g);
  return boost::report_errors();
}
