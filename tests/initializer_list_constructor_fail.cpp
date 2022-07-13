/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <less/vector.hpp>

void initializer_list_constructor()
{
  {
    // vector(std::initializer_list<T> list)
    auto v = less::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    (void)v;
  }
}

int main()
{
  initializer_list_constructor();
}
