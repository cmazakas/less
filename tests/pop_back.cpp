#include "lwt_helper.hpp"

#include <initializer_list>
#include <less/vector.hpp>

static void pop_back()
{
  auto vec = less::vector<int>{1, 2, 3, 4, 5};
  while (vec.size() > 0u) {
    vec.pop_back();
  }
  BOOST_TEST_EQ(vec.size(), 0u);
}

int main()
{
  pop_back();
  return boost::report_errors();
}
