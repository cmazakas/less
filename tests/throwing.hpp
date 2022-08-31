#ifndef LESS_TEST_THROWING_HPP_
#define LESS_TEST_THROWING_HPP_

#include "lwt_helper.hpp"

static int       tcount     = 0;
static int const limit      = 128;
static bool      was_thrown = false;

static int num_constructions     = 0;
static int num_copy_constructors = 0;
static int num_move_constructors = 0;
static int num_copy_assignments  = 0;
// static int num_move_assignments  = 0;
static int num_destructions = 0;

inline void reset_counts()
{
  tcount     = 0;
  was_thrown = false;
}

inline void reset_tracking()
{
  num_constructions     = 0;
  num_copy_constructors = 0;
  num_move_constructors = 0;
  num_copy_assignments  = 0;
  num_destructions      = 0;
}

struct throwing {
  int* x_;

  throwing()
      : x_{}
  {
    ++num_constructions;

    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = new int{1};
  }

  throwing(throwing const& rhs)
  {
    ++num_copy_constructors;

    ++tcount;
    if (tcount > limit) { throw 42; }

    x_ = rhs.x_;
    *x_ += 1;
  }

  throwing(throwing&& rhs) noexcept(false)
  {
    ++num_move_constructors;

    x_     = rhs.x_;
    rhs.x_ = nullptr;
  }

  ~throwing()
  {
    ++num_destructions;

    if (!x_) { return; }
    *x_ -= 1;
    if (*x_ == 0) {
      delete x_;
      x_ = nullptr;
    }
  }

  auto operator=(throwing const& rhs) -> throwing&
  {
    ++num_copy_assignments;

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

#define LESS_TRY(F)     \
  try {                 \
    F();                \
    was_thrown = false; \
  }                     \
  catch (...) {         \
    was_thrown = true;  \
  }                     \
  BOOST_TEST(was_thrown);

#endif    // LESS_TEST_THROWING_HPP_
