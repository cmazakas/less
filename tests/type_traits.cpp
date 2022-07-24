#include "lwt_helper.hpp"

#include <less/vector.hpp>

struct A1 {
  A1(int)
  {
  }

  A1(A1&&) = default;
};

static_assert(less::detail::is_constructible<A1, int>::value);
static_assert(!less::detail::is_constructible<A1, void>::value);
static_assert(!less::detail::is_nothrow_constructible<A1, int>::value);

static_assert(less::detail::is_constructible<A1, A1&&>::value);
static_assert(less::detail::is_nothrow_constructible<A1, A1&&>::value);

static_assert(less::detail::is_nothrow_move_constructible<A1>::value);

int main()
{
}
