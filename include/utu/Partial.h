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

namespace utu
{

struct Partial
{
  using Envelopes = std::unordered_map<std::string, std::vector<double>>;

  std::optional<std::string> label;
  Envelopes envelopes;
};

}  // namespace utu