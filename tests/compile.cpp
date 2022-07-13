/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <less/vector.hpp>

static void default_construct()
{
  auto v = less::vector<int>();
  (void)v.size();
  (void)v.capacity();
  (void)v.begin();
  (void)v.data();
}

static void default_init_construct()
{
  auto v = less::vector<int>(less::default_init, 1337u);
  (void)v.size();
  (void)v.capacity();
  (void)(v.end() - v.begin());
  (void)v.data();
}

static void size_construct()
{
  auto v = less::vector<int>(1337u);
  (void)v.capacity();
  (void)v.data();
  (void)(v.end() - v.begin());

  for (auto i = 0u; i < v.size(); ++i) {
    (void)v[i];
  }
}

static void with_capacity_construct()
{
  auto v = less::vector<int>(less::with_capacity, 1337u);
  (void)v.size();
  (void)v.capacity();
  (void)v.data();
  (void)(v.end() - v.begin());
}

static void size_value_construct()
{
  auto v = less::vector<int>(1337u, 7331);
  (void)v.size();
  (void)v.capacity();
  (void)v.data();
  (void)(v.end() - v.begin());

  for (auto i = 0u; i < v.size(); ++i) {
    (void)v[i];
  }
}

static void copy_construct()
{
  auto v  = less::vector<int>(1337u, 7331);
  auto v2 = less::vector<int>(v);
  (void)v2.size();
  (void)v2.capacity();
  (void)v2.data();
  (void)(v2.end() - v2.begin());
  for (unsigned i = 0u; i < v2.size(); ++i) {
    (void)v2[i];
  }
}

static void move_construct()
{
  auto v = less::vector<int>(1337u, 7331);

  auto v2 = less::vector<int>(less::detail::move(v));
  (void)v2.size();
  (void)v2.capacity();
  (void)v2.data();
  (void)(v2.end() - v2.begin());
  for (unsigned i = 0u; i < v2.size(); ++i) {
    (void)v2[i];
  }
}

int main()
{
  default_construct();
  default_init_construct();
  size_construct();
  with_capacity_construct();
  size_value_construct();
  copy_construct();
  move_construct();
}
