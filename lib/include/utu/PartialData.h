//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "utu/Partial.h"

namespace utu
{

struct PartialData {
  using Parameters = std::vector<std::string>;
  using Partials = std::vector<Partial>;

  struct Source {
    std::string location;
    std::optional<std::string> fingerprint;
  };

  std::optional<std::string> description;
  std::optional<Source> source;

  Parameters parameters;
  Partials partials;

  bool push_back(Partial& partial)
  {
    // Ensure the incoming partial has all the expected parameters
    for (const auto& param : parameters) {
      if (partial.parameters.find(param) == partial.parameters.end()) {
        return false;
      }
    }
    partials.push_back(partial);
    return true;
  }
};

}  // namespace utu