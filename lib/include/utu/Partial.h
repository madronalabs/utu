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

constexpr char kTimeName[] = "time";
constexpr char kFrequencyName[] = "frequency";
constexpr char kAmplitudeName[] = "amplitude";
constexpr char kBandwidthName[] = "bandwidth";
constexpr char kPhaseName[] = "phase";

namespace utu
{

struct Partial {
  using Samples = std::vector<double>;
  using Parameters = std::unordered_map<std::string, Samples>;

  std::optional<std::string> label;
  Parameters parameters;
};

}  // namespace utu
