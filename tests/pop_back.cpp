#include "lwt_helper.hpp"

#include <initializer_list>
#include <less/vector.hpp>

struct raii_tracker {
  static unsigned num_destruction_calls;

  ~raii_tracker()
  {
    ++num_destruction_calls;
  }
};

unsigned raii_tracker::num_destruction_calls = 0;

void reset_tracking()
{
  raii_tracker::num_destruction_calls = 0;
}

static void pop_back()
{
  {
    auto vec = less::vector<int>{1, 2, 3, 4, 5};
    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{1, 2, 3, 4}));

    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{1, 2, 3, 4}));

    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{1, 2, 3}));

    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{1, 2}));

    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{1}));

    vec.pop_back();
    BOOST_TEST((vec == less::vector<int>{}));
  }

  {
    reset_tracking();

    auto vec = less::vector<raii_tracker>(5, raii_tracker{});
    for (auto i = 0u; i < 5u; ++i) {
      vec.pop_back();
      BOOST_TEST_ASSERT_EQ(raii_tracker::num_destruction_calls, i + 1);
    }
    BOOST_TEST(vec.empty());
  }
}

int main()
{
  pop_back();
  return boost::report_errors();
}
