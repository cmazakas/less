/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/core/lightweight_test.hpp>

#include <less/vector.hpp>

static void empty()
{
  auto const size = 1024u;

  less::vector<int> v;
  v.resize_and_overwrite(size, [](int* p, auto n) {
    for (auto i = 0u; i < n; ++i) {
      p[i] = 1337;
    }
    return n;
  });

  BOOST_TEST_EQ(v.capacity(), size);
  if (!BOOST_TEST_EQ(v.size(), size)) { return; }

  for (auto i = 0u; i < size; ++i) {
    if (!BOOST_TEST_EQ(v[i], 1337)) { return; }
  }
}

int main()
{
  empty();
  return boost::report_errors();
}
