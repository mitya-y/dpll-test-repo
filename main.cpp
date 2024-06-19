#include <iostream>
#include <ostream>

#include "tests/googletest/googletest/include/gtest/gtest.h"

#include "utils/dpll.h"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  return 0;
  auto result = load_cnf("files/a.cnf");
  if (!result)
  {
    std::cout << "Error in dimacs loading";
    return 0;
  }

  auto dimacs = result.value();
  print_dimacs_format(dimacs);

  auto res = dpll_algorithm(dimacs);
  if (!res)
    std::println(std::cout, "No answer");
  else
    for (auto &[var, value] : res.value())
      std::println(std::cout, "{} = {}", var, value);
}
