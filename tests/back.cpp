#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void front()
{
  auto vec = less::vector<int>{1, 2, 3, 4};

  BOOST_TEST_EQ(vec.back(), 4);
  vec.back() = 7331;
  BOOST_TEST_EQ(vec.back(), 7331);
  BOOST_TEST_EQ(vec[vec.size() - 1], 7331);
}

int main()
{
  front();
  return boost::report_errors();
}
