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

struct Partial {
  using Envelope = std::vector<double>;
  using Envelopes = std::unordered_map<std::string, Envelope>;

  std::optional<std::string> label;
  Envelopes envelopes;
};

}  // namespace utu