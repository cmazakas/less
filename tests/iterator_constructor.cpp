/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <less/vector.hpp>

static int exit_status = 0;

void iterator_constructor()
{
  auto v  = less::vector<int>(1337u);
  auto v2 = less::vector<int>(v.begin(), v.end());

  if (v2.end() - v2.begin() != static_cast<long>(v2.size())) {
    exit_status = 1;
    return;
  }

  if (v2.data() == nullptr) {
    exit_status = 2;
    return;
  }

  if (v2.capacity() < v2.size()) {
    exit_status = 3;
    return;
  }

  if (v2.size() != v.size()) {
    exit_status = 4;
    return;
  }

  for (auto i = 0u; i < v.size(); ++i) {
    if (v2[i] != v[i]) {
      exit_status = 5;
      return;
    }
  }
}

static int construct_count = 0;
static int destruct_count  = 0;

constexpr static int const limit = 10;

struct throwing {
  throwing()
  {
    if (construct_count >= limit) { throw 42; }
    ++construct_count;
  }

  throwing(throwing const&)
  {
    if (construct_count >= limit) { throw 42; }
    ++construct_count;
  }

  ~throwing()
  {
    ++destruct_count;
  }
};

static void iterator_construct_random_access()
{
  auto v = less::vector<throwing>(limit - 4);

  try {
    auto v2 = less::vector<throwing>(v.begin(), v.end());
  }
  catch (...) {
  }

  if (construct_count != limit) {
    exit_status = 10;
    return;
  }

  if (destruct_count != 4) {
    exit_status = 11;
    return;
  }
}

int main()
{
  iterator_constructor();
  if (exit_status != 0) { return exit_status; }

  iterator_construct_random_access();
  if (exit_status != 0) { return exit_status; }

  return exit_status;
}
