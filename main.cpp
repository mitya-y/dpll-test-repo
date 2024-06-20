#include <iostream>
#include <ostream>

#include "tests/googletest/googletest/include/gtest/gtest.h"

#include "utils/dpll.hpp"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
