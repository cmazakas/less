#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void iterators()
{
  auto vec = less::vector<int>();
  BOOST_TEST(vec.begin() == vec.end());
  BOOST_TEST(vec.cbegin() == vec.cend());

  vec.assign({1, 2, 3, 4});
  BOOST_TEST_EQ(*vec.begin(), 1);
  BOOST_TEST_EQ(*(vec.end() - 1), 4);
  BOOST_TEST_EQ(*vec.cbegin(), 1);
  BOOST_TEST_EQ(*(vec.cend() - 1), 4);

  BOOST_TEST(vec.begin() + vec.size() == vec.end());

  *vec.begin() = 1337;
  BOOST_TEST_EQ(vec[0], 1337);
}

int main()
{
  iterators();
  return boost::report_errors();
}
