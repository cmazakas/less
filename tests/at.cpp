#include "lwt_helper.hpp"

#include <less/vector.hpp>

static void at_test()
{
  auto vec = less::vector<int>{1, 2, 3, 4};

  auto& x = vec.at(0);
  x       = 1337;

  BOOST_TEST_EQ(vec[0], 1337);

  BOOST_TEST_THROWS(vec.at(vec.size() + 1), less::out_of_range);
}

int main()
{
  at_test();
  return boost::report_errors();
}
