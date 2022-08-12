#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void front()
{
  auto vec = less::vector<int>{1, 2, 3, 4};

  BOOST_TEST_EQ(vec.front(), 1);
  vec.front() = 7331;
  BOOST_TEST_EQ(vec.front(), 7331);
}

int main()
{
  front();
  return boost::report_errors();
}
