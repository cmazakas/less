#include "lwt_helper.hpp"

#include <memory>
#include <less/vector.hpp>

static void emplace_empty_resize()
{
  {
    auto vec = less::vector<int>();
    BOOST_TEST(vec.empty());
    auto pos = vec.emplace(vec.begin(), 1337);
    BOOST_TEST_EQ(vec.size(), 1);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }

  {
    auto vec = less::vector<int>();
    BOOST_TEST(vec.empty());
    auto pos = vec.emplace(vec.end(), 1337);
    BOOST_TEST_EQ(vec.size(), 1);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }
}

static void emplace_empty_no_resize()
{
  {
    auto vec = less::vector<int>();
    vec.reserve(128);
    BOOST_TEST(vec.empty());
    auto pos = vec.emplace(vec.begin(), 1337);
    BOOST_TEST_EQ(vec.size(), 1);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(128);
    BOOST_TEST(vec.empty());
    auto pos = vec.emplace(vec.end(), 1337);
    BOOST_TEST_EQ(vec.size(), 1);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }
}

static void emplace_nonempty_resize()
{
  {
    auto vec = less::vector<int>();
    vec.reserve(4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_EQ(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.begin(), 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_EQ(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.begin() + 2, 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec[2], 1337);
    BOOST_TEST(pos == vec.begin() + 2);
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_EQ(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.end(), 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec.back(), 1337);
    BOOST_TEST(pos == vec.end() - 1);
  }
}

static void emplace_nonempty_no_resize()
{
  {
    auto vec = less::vector<int>();
    vec.reserve(4 * 4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_GT(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.begin(), 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec[0], 1337);
    BOOST_TEST(pos == vec.begin());
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(4 * 4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_GT(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.begin() + 2, 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec[2], 1337);
    BOOST_TEST(pos == vec.begin() + 2);
  }

  {
    auto vec = less::vector<int>();
    vec.reserve(4 * 4);
    vec.assign({1, 2, 3, 4});
    BOOST_TEST_GT(vec.capacity(), 4);

    BOOST_TEST_NOT(vec.empty());
    auto pos = vec.emplace(vec.end(), 1337);
    BOOST_TEST_EQ(vec.size(), 5);
    BOOST_TEST_EQ(vec.back(), 1337);
    BOOST_TEST(pos == vec.end() - 1);
  }
}

static void emplace_move_only()
{
  auto vec = less::vector<std::unique_ptr<int>>();
  vec.emplace(vec.end(), std::make_unique<int>(1337));
  BOOST_TEST_EQ(vec.size(), 1);
}

int main()
{
  emplace_empty_resize();
  emplace_empty_no_resize();
  emplace_nonempty_resize();
  emplace_nonempty_no_resize();
  emplace_move_only();
  return boost::report_errors();
}
