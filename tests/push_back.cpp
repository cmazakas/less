#include "lwt_helper.hpp"
#include "throwing.hpp"

#include <memory>
#include <less/vector.hpp>

template <class T>
using vector_type = less::vector<T>;

static void push_back()
{
  auto vec = vector_type<int>{1, 2, 3, 4, 5};
  vec.push_back(1337);
  BOOST_TEST_EQ(vec.back(), 1337);
}

static void push_back_value()
{
  auto const& value = vector_type<int>{0, 1, 2, 3, 4};

  {
    // empty, resize
    //
    auto vec = vector_type<vector_type<int>>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.capacity(), 0u);

    vec.push_back(value);

    BOOST_TEST_EQ(vec.size(), 1u);
  }

  {
    // empty, no resize
    //
    auto vec = vector_type<vector_type<int>>();
    vec.reserve(16);
    BOOST_TEST(vec.empty());
    BOOST_TEST_GE(vec.capacity(), 16u);

    vec.push_back(value);

    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST_GE(vec.capacity(), 16u);
  }

  {
    // non-empty, resize
    //
    auto vec = vector_type<vector_type<int>>(5);
    BOOST_TEST_NOT(vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    vec.push_back(value);

    BOOST_TEST_EQ(vec.size(), 6u);
    BOOST_TEST_GE(vec.capacity(), vec.size());
  }

  {
    // empty, no resize
    //
    auto vec = vector_type<vector_type<int>>(5);
    vec.reserve(16);
    BOOST_TEST_NOT(vec.empty());
    BOOST_TEST_GE(vec.capacity(), 16u);

    vec.push_back(value);

    BOOST_TEST_EQ(vec.size(), 6u);
    BOOST_TEST_GE(vec.capacity(), 16u);
  }
}

static void push_back_rvalue()
{
  {
    // empty, resize
    //
    auto vec = vector_type<std::unique_ptr<int>>();
    BOOST_TEST(vec.empty());
    BOOST_TEST_EQ(vec.capacity(), 0u);

    vec.push_back(std::make_unique<int>(1337));

    BOOST_TEST_EQ(vec.size(), 1u);
  }

  {
    // empty, no resize
    //
    auto vec = vector_type<std::unique_ptr<int>>();
    vec.reserve(16);
    BOOST_TEST(vec.empty());
    BOOST_TEST_GE(vec.capacity(), 16u);

    vec.push_back(std::make_unique<int>(1337));

    BOOST_TEST_EQ(vec.size(), 1u);
    BOOST_TEST_GE(vec.capacity(), 16u);
  }

  {
    // non-empty, resize
    //
    auto vec = vector_type<std::unique_ptr<int>>(5);
    BOOST_TEST_NOT(vec.empty());
    BOOST_TEST_EQ(vec.capacity(), vec.size());

    vec.push_back(std::make_unique<int>(1337));

    BOOST_TEST_EQ(vec.size(), 6u);
    BOOST_TEST_GE(vec.capacity(), vec.size());
  }

  {
    // empty, no resize
    //
    auto vec = vector_type<std::unique_ptr<int>>(5);
    vec.reserve(16);
    BOOST_TEST_NOT(vec.empty());
    BOOST_TEST_GE(vec.capacity(), 16u);

    vec.push_back(std::make_unique<int>(1337));

    BOOST_TEST_EQ(vec.size(), 6u);
    BOOST_TEST_GE(vec.capacity(), 16u);
  }
}

static void push_back_throwing()
{
  {
    // empty
    //

    reset_counts();

    auto vec = vector_type<throwing>();

    auto const value = throwing();

    tcount = limit;

    auto const size     = vec.size();
    auto const capacity = vec.capacity();

    try {
      vec.push_back(value);
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.size(), size);
    BOOST_TEST_EQ(vec.capacity(), capacity);
  }

  {
    // non-empty
    //

    reset_counts();

    auto vec = vector_type<throwing>(limit / 2);

    auto const value = throwing();

    tcount = limit;

    auto const size     = vec.size();
    auto const capacity = vec.capacity();

    try {
      vec.push_back(value);
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.size(), size);
    BOOST_TEST_EQ(vec.capacity(), capacity);
  }
}

int main()
{
  push_back();
  push_back_value();
  push_back_rvalue();
  push_back_throwing();
  return boost::report_errors();
}
