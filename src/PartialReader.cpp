//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <nlohmann/json.hpp>

#include <utu/PartialReader.h>

#include "SerializerImpl.h"

namespace utu
{

using json = nlohmann::json;

PartialData&& PartialReader::read(const std::string& jsonData)
{
  json j = json::parse(jsonData, nullptr, false /* allow exceptions */, true /* allow comments */);

  FileInfo info = j["file_info"].get<FileInfo>();
  // TODO: validate header

  PartialData data{
      j["description"].get<std::optional<std::string>>(),
      j["source"].get<std::optional<PartialData::Source>>(),
      j["parameters"].get<std::vector<std::string>>(),
      j["partials"].get<std::vector<Partial>>()
  };

  return std::move(data);
}

}  // namespace utu