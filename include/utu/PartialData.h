//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
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
  struct Source {
    std::string location;
    std::optional<std::string> fingerprint;
  };

  std::optional<std::string> description;
  std::optional<Source> source;

  std::vector<std::string> parameters;
  std::vector<Partial> partials;

  bool push_back(Partial&& partial)
  {
    // Ensure the incoming partial has all the expected parameters
    for (const auto& param : parameters) {
      if (partial.envelopes.find(param) == partial.envelopes.end()) {
        return false;
      }
    }
    partials.push_back(partial);
    return true;
  }
};

}  // namespace utu