#ifndef LESS_TESTS_LWT_HELPER_HPP_
#define LESS_TESTS_LWT_HELPER_HPP_

#include <boost/core/lightweight_test.hpp>

#define BOOST_TEST_ASSERT(expr) \
  if (!BOOST_TEST(expr)) { return; }

#define BOOST_TEST_ASSERT_EQ(expr1, expr2) \
  if (!BOOST_TEST_EQ(expr1, expr2)) { return; }

#endif    // LESS_TESTS_LWT_HELPER_HPP_
