#include <algorithm>
#include <set>
#include <unordered_map>
#include <iostream>
#include <ranges>

#include "dpll.h"

struct Variable
{
  uint index;
  bool value = false;
  bool used = false;
  // indexes of conjustions in which this variable occur and is positive in this
  std::vector<std::pair<uint, bool>> occurance;
};

static void set_satisfied_flag(const std::vector<Variable> &variables, Conjunction &con)
{
  con.satisfied = std::any_of(con.disjunctions.begin(), con.disjunctions.end(),
    [&variables](const auto &v) { return (!(v.first ^ variables[v.second].value)) && variables[v.second].used; });
}

static bool unit_propagation(DimacsFormat &sat,
  std::vector<Variable> &variables,
  std::set<uint> &positive, std::set<uint> &negative, std::set<uint> &all)
{
  uint cnt = 0;
  bool changed = false;
  for (auto [i, con] : std::views::enumerate(sat.conjunctions))
    if (con.number_of_free_elements == 1 && !con.satisfied)
    {
      cnt++;
      auto &dis = con.disjunctions;
      auto index_iter = std::find_if(dis.begin(), dis.end(),
        [&](auto &pair) { return !variables[pair.second].used; });

      Variable &var = variables[index_iter->second];
      var.used = true;
      var.value = index_iter->first;

      con.satisfied = true;

      (var.value ? positive : negative).insert(index_iter->second);
      all.erase(index_iter->second);

      for (auto &[con, _] : var.occurance)
      {
        sat.conjunctions[con].number_of_free_elements--;
        // if (sat.conjunctions[conjs].number_of_free_elements == 0)
        //   sat.conjunctions[conjs].satisfied = true;
        set_satisfied_flag(variables, sat.conjunctions[con]);
      }

      changed = true;
    }
  // std::cout << "cnt: " << cnt << "\n";
  return changed;
}

static void pure_variable_elimination(DimacsFormat &sat,
  std::vector<Variable> &variables,
  std::set<uint> &positive, std::set<uint> &negative, std::set<uint> &all)
{
  for (auto const [index, variable] : std::views::enumerate(variables))
  {
    auto &occur = variable.occurance;
    bool all_positive =
      std::all_of(occur.begin(), occur.end(), [](auto &pair) { return pair.second; });
    bool all_negative =
      std::all_of(occur.begin(), occur.end(), [](auto &pair) { return !pair.second; });

    if (all_positive || all_negative)
    {
      variable.value = all_positive;
      variable.used = true;

      all.erase(index);
      (all_positive ? positive : negative).insert(index);

      for (auto &&[con, _] : occur)
      {
        sat.conjunctions[con].number_of_free_elements--;
        // sat.conjunctions[con].satisfied = true;
        set_satisfied_flag(variables, sat.conjunctions[con]);
        // if (sat.conjunctions[con].number_of_free_elements == 0)
      }
    }
  }
}

static bool check_satisfaing(const DimacsFormat &sat, const std::vector<Variable> &variables)
{
  for (auto &con : sat.conjunctions)
  {
    // if (con.satisfied)
    //   continue;

    bool value = std::all_of(con.disjunctions.begin(), con.disjunctions.end(),
      [&variables](const auto &v) { return (v.first ^ variables[v.second].value) && variables[v.second].used; });
    if (value)
      return false;
  }
  return true;
}

static bool check_success(const DimacsFormat &sat)
{
  return std::all_of(sat.conjunctions.begin(), sat.conjunctions.end(),
    [](const auto &con) { return con.satisfied; });
}

static std::optional<std::vector<Variable>> recursive_dpll(DimacsFormat sat,
  std::vector<Variable> variables,
  std::set<uint> positive, std::set<uint> negative, std::set<uint> all)
{
  while (unit_propagation(sat, variables, positive, negative, all)) {}

  if (!check_satisfaing(sat, variables))
  {
    // std::copy(std::ostream_iterator<int>(std::cout, " "), positive.begin(), positive.end());

    // for (int v : positive)
    //   std::cout << v << " ";
    // std::cout << "\n";
    // for (int v : negative)
    //   std::cout << "-" << v << " ";
    // std::cout << "\n\n";
    return std::nullopt;
  }

  if (check_success(sat))
    return variables;

  int var = *all.begin();
  all.erase(all.begin());

  for (uint i = 0; i < 2; i++)
  {
    bool is_positive = i == 0;

    // change variable
    (is_positive ? positive : negative).insert(var);
    auto &variable = variables[var];
    variable.used = true;
    variable.value = is_positive;
    for (auto &&[con, _] : variable.occurance)
    {
      sat.conjunctions[con].number_of_free_elements--;
      // if (sat.conjunctions[con].number_of_free_elements == 0)
      //   sat.conjunctions[con].satisfied = true;
      // sat.conjunctions[con].satisfied = true;
      set_satisfied_flag(variables, sat.conjunctions[con]);
    }

    // TODO : dont copy, make instuctions for backup (set of chaned variables)
    auto result = recursive_dpll(sat, variables, positive, negative, all);
    if (result)
      return result.value();

    // restore value for next iteration
    (is_positive ? positive : negative).erase(var);
    variable.used = false;
    for (auto &&[con, _] : variable.occurance)
    {
      sat.conjunctions[con].number_of_free_elements++;
      // TODO : here error!
      // sat.conjunctions[con].satisfied = false;
      set_satisfied_flag(variables, sat.conjunctions[con]);
    }
  }

  return std::nullopt;
}

std::optional<DPLLResult> dpll_algorithm(const DimacsFormat &sat)
{
  // array of variables
  size_t n = sat.variables.size();
  auto iter = sat.variables.begin();
  std::vector<Variable> variables(n);
  for (uint i = 0; i < n; i++)
    variables[i].index = *(iter++);

  // mapping from all natural numbers to [0; N) for indexing in array
  std::unordered_map<uint, uint> variables_map;
  uint id = 0;
  for (int v : sat.variables)
    if (!variables_map.contains(v))
      variables_map[v] = id++;

  auto sat_copy = sat;
  auto &formula = sat_copy.conjunctions;
  for (auto &con : formula)
    for (auto &[_, v] : con.disjunctions)
      v = variables_map[v];

  // calculate occurance of each variable
  // handled variant if in one conjucion variable occur twice and more
  
  for (auto const [index, con] : std::views::enumerate(formula))
  {
    std::unordered_map<uint, std::pair<uint, uint>> counts;
    for (auto &&[positive, v] : con.disjunctions)
    {
      if (!counts.contains(v))
        counts[v] = {0, 0};
      (positive ? counts[v].first : counts[v].second)++;
    }

    for (auto &&[v, pair] : counts)
    {
      auto [poscnt, negcnt] = pair;
      if (poscnt + negcnt == 1)
        variables[v].occurance.push_back({index, poscnt != 0});
      else
      {
        auto &dis = con.disjunctions;
        dis.erase(std::remove_if(dis.begin(), dis.end(),
          [v](auto &pair) { return pair.second == v; }), dis.end());

        if (poscnt == 0 || negcnt == 0)
        {
          dis.push_back({poscnt != 0, v});
          variables[v].occurance.push_back({index, poscnt != 0});
        }

        con.number_of_free_elements = dis.size();

        if (con.number_of_free_elements == 0)
          con.satisfied = true;
      }
    }
  }

  std::set<uint> positive, negative, all;
  for (uint i = 0; i < n; i++)
    all.insert(i);

  pure_variable_elimination(sat_copy, variables, positive, negative, all);
  if (!check_satisfaing(sat_copy, variables))
    return std::nullopt;

  auto result = recursive_dpll(sat_copy, variables, positive, negative, all);
  if (!result)
    return std::nullopt;
  std::map<uint, bool> answer;
  // TODO : reverse mappimg
  for (auto const [index, var] : std::views::enumerate(result.value()))
    answer[var.index] = var.value; // todo:

  return answer;
}
