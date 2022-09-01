#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void swap()
{
  {
    auto vec1 = less::vector<int>();
    auto vec2 = less::vector<int>();

    BOOST_TEST(vec1.empty());
    BOOST_TEST(vec2.empty());

    vec1.swap(vec2);

    BOOST_TEST(vec1.empty());
    BOOST_TEST(vec2.empty());
  }

  {
    auto vec1 = less::vector<int>{1, 2, 3, 4, 5};
    auto vec2 = less::vector<int>();

    BOOST_TEST_NOT(vec1.empty());
    BOOST_TEST(vec2.empty());

    auto p = vec1.data();

    vec1.swap(vec2);

    BOOST_TEST(vec1.empty());
    BOOST_TEST_NOT(vec2.empty());
    BOOST_TEST((vec2 == less::vector<int>{1, 2, 3, 4, 5}));
    BOOST_TEST_EQ(vec2.data(), p);
    BOOST_TEST_EQ(vec1.data(), nullptr);
  }

  {
    auto vec1 = less::vector<int>();
    auto vec2 = less::vector<int>{1, 2, 3, 4, 5};

    auto p = vec2.data();

    BOOST_TEST(vec1.empty());
    BOOST_TEST_NOT(vec2.empty());

    vec1.swap(vec2);

    BOOST_TEST(vec2.empty());
    BOOST_TEST_NOT(vec1.empty());
    BOOST_TEST((vec1 == less::vector<int>{1, 2, 3, 4, 5}));
    BOOST_TEST_EQ(vec1.data(), p);
    BOOST_TEST_EQ(vec2.data(), nullptr);
  }

  {
    auto vec1 = less::vector<int>{5, 4, 3, 2, 1};
    auto vec2 = less::vector<int>{1, 2, 3, 4, 5};

    auto p1 = vec1.data();
    auto p2 = vec2.data();

    vec1.swap(vec2);

    BOOST_TEST((vec1 == less::vector<int>{1, 2, 3, 4, 5}));
    BOOST_TEST((vec2 == less::vector<int>{5, 4, 3, 2, 1}));
    BOOST_TEST_EQ(vec1.data(), p2);
    BOOST_TEST_EQ(vec2.data(), p1);
  }
}

int main()
{
  swap();
  return boost::report_errors();
}
