#ifndef LESS_TEST_THROWING_HPP_
#define LESS_TEST_THROWING_HPP_

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

#endif    // LESS_TEST_THROWING_HPP_
