#include "lwt_helper.hpp"
#include "throwing.hpp"

#include <less/vector.hpp>

static void emplace_back_empty()
{
  auto vec = less::vector<int>();
  BOOST_TEST(vec.empty());
  BOOST_TEST_EQ(vec.emplace_back(1337), 1337);
  BOOST_TEST_EQ(vec.size(), 1u);
  BOOST_TEST(vec == less::vector<int>{1337});
}

static void emplace_back_nonempty_resize()
{
  auto vec  = less::vector<int>{1, 2, 3, 4, 5};
  auto size = vec.size();

  BOOST_TEST_EQ(vec.size(), vec.capacity());
  BOOST_TEST(!vec.empty());
  BOOST_TEST_EQ(vec.emplace_back(1337), 1337);
  BOOST_TEST_EQ(vec.size(), size + 1);
  BOOST_TEST((vec == less::vector<int>{1, 2, 3, 4, 5, 1337}));
}

static void emplace_back_nonempty_no_resize()
{
  auto vec  = less::vector<int>{1, 2, 3, 4, 5};
  auto size = vec.size();
  vec.reserve(32);

  BOOST_TEST_GT(vec.capacity(), vec.size());
  BOOST_TEST(!vec.empty());
  BOOST_TEST_EQ(vec.emplace_back(1337), 1337);
  BOOST_TEST_EQ(vec.size(), size + 1);
  BOOST_TEST((vec == less::vector<int>{1, 2, 3, 4, 5, 1337}));
}

struct moveonly {
  moveonly(int, int, int)
  {
  }
  moveonly(moveonly const&) = delete;
  moveonly(moveonly&&)      = default;
};

static void emplace_back_moveonly()
{
  auto vec = less::vector<moveonly>();
  vec.emplace_back(1, 2, 3);
}

static void emplace_back_throwing()
{
  {
    // empty
    //
    reset_counts();

    auto vec = less::vector<throwing>();
    BOOST_TEST(vec.empty());
    tcount = limit;
    try {
      vec.emplace_back();
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST(vec.empty());
  }

  {
    // non-empty
    //
    reset_counts();

    auto vec = less::vector<throwing>(limit);
    BOOST_TEST_EQ(vec.size(), limit);

    auto cap  = vec.capacity();
    auto size = vec.size();

    try {
      vec.emplace_back();
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.size(), size);
    BOOST_TEST_EQ(vec.capacity(), cap);
  }
}

int main()
{
  emplace_back_empty();
  emplace_back_nonempty_resize();
  emplace_back_nonempty_no_resize();
  emplace_back_moveonly();
  emplace_back_throwing();
  return boost::report_errors();
}
