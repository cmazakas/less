#include "lwt_helper.hpp"

#include <memory>
#include <vector>
#include <less/vector.hpp>

template <class T>
using vector_type = less::vector<T>;

static int       tcount     = 0;
static int const limit      = 128;
static bool      was_thrown = false;

static void reset_counts()
{
  tcount     = 0;
  was_thrown = false;
}

struct throwing {
  int* x_;

  throwing()
      : x_{}
  {
    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = new int{1};
  }

  throwing(throwing const& rhs)
  {
    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = rhs.x_;
    *x_ += 1;
  }

  throwing(throwing&& rhs) noexcept(false)
  {
    x_     = rhs.x_;
    rhs.x_ = nullptr;
  }

  ~throwing()
  {
    if (!x_) { return; }
    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }
  }

  auto operator=(throwing const& rhs) -> throwing&
  {
    ++tcount;
    if (tcount > limit) { throw 42; }

    if (x_) {
      *x_ -= 1;
      if (*x_ == 0) {
        delete x_;
        x_ = nullptr;
      }
    }

    x_ = rhs.x_;
    *x_ += 1;

    return *this;
  }
};

static void erase_range()
{
  {
    // empty range
    //
    auto vec = vector_type<int>();
    auto pos = vec.erase(vec.begin(), vec.end());
    BOOST_TEST(vec.empty());
    BOOST_TEST(pos == vec.begin());
  }

  {
    // erase middle
    //
    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin() + 1, vec.begin() + 4);

    auto const expected = vector_type<int>{1, 5};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST_EQ(*pos, 5);
  }

  {
    // erase from offset to end
    //
    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin() + 2, vec.end());

    auto const expected = vector_type<int>{1, 2};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST(pos == vec.end());
  }

  {
    // erase from begin to offset
    //
    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin(), vec.begin() + 3);

    auto const expected = vector_type<int>{4, 5};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST(pos == vec.begin());
  }

  {
    // erase from begin to end
    //
    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin(), vec.end());

    BOOST_TEST(vec.empty());
    BOOST_TEST(pos == vec.begin());
    BOOST_TEST(pos == vec.end());
  }
}

static void erase_single()
{
  {
    // erase begin
    //

    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin());

    auto const expected = vector_type<int>{2, 3, 4, 5};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST(pos == vec.begin());
  }

  {
    // erase middle
    //

    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.begin() + 2);

    auto const expected = vector_type<int>{1, 2, 4, 5};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST_EQ(*pos, 4);
  }

  {
    // erase last
    //

    auto vec = vector_type<int>{1, 2, 3, 4, 5};
    auto pos = vec.erase(vec.end() - 1);

    auto const expected = vector_type<int>{1, 2, 3, 4};
    BOOST_TEST_ALL_EQ(vec.begin(), vec.end(), expected.begin(), expected.end());
    BOOST_TEST(pos == vec.end());
  }

  {
    // erase entirety (i.e. size() == 1)
    //

    auto vec = vector_type<int>{1};
    auto pos = vec.erase(vec.begin());

    BOOST_TEST(vec.empty());
    BOOST_TEST(pos == vec.end());
  }
}

static void erase_raii()
{
  {
    // range
    //
    auto vec = vector_type<std::unique_ptr<int>>(10);
    for (auto& v : vec) {
      v = std::make_unique<int>(1337);
    }

    vec.erase(vec.begin() + 1, vec.begin() + 8);
    BOOST_TEST_EQ(vec.size(), 3);
  }

  {
    // single
    //
    auto vec = vector_type<std::unique_ptr<int>>(10);
    for (auto& v : vec) {
      v = std::make_unique<int>(1337);
    }

    vec.erase(vec.begin() + 5);
    BOOST_TEST_EQ(vec.size(), 9);
  }
}

static void erase_throws()
{
  {
    // range
    //

    reset_counts();

    auto vec = vector_type<throwing>(limit / 2);

    tcount = limit - 10;
    try {
      vec.erase(vec.begin() + 3, vec.begin() + 10);
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.size(), limit / 2);
  }

  {
    // single
    //

    reset_counts();

    auto vec = vector_type<throwing>(limit / 2);

    tcount = limit - 10;
    try {
      vec.erase(vec.begin() + 3);
    }
    catch (...) {
      was_thrown = true;
    }

    BOOST_TEST(was_thrown);
    BOOST_TEST_EQ(vec.size(), limit / 2);
  }
}

int main()
{
  erase_range();
  erase_single();
  erase_raii();
  erase_throws();
  return boost::report_errors();
}
