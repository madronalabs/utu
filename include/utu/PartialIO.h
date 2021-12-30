//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#pragma once

#include <utu/PartialData.h>

#include <iostream>
#include <optional>
#include <string>

namespace utu
{

template <typename T>
struct Reader {
  using ValueType = T;
  static std::optional<T> read(const std::string& jsonData);
  static std::optional<T> read(std::istream& is);
};

template <typename T>
struct Writer {
  using ValueType = T;
  static std::optional<std::string> write(const T& value);
  static void write(const T& value, std::ostream& os);
};

typedef Reader<PartialData> PartialReader;
typedef Writer<PartialData> PartialWriter;

}  // namespace utu