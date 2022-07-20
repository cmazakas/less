#include "lwt_helper.hpp"

#include <limits>

#include <less/vector.hpp>

static void max_size()
{
  using difference_type = less::vector<int>::difference_type;

  less::vector<int> v;
  BOOST_TEST_ASSERT_EQ(v.max_size(), std::numeric_limits<difference_type>::max());
}

int main()
{
  max_size();
  return boost::report_errors();
}
