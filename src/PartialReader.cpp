//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <utu/PartialReader.h>

#include <nlohmann/json.hpp>

#include "SerializerImpl.h"

namespace utu
{

using json = nlohmann::json;

std::optional<PartialData> PartialReader::read(const std::string& jsonData)
{
  json j = json::parse(jsonData, nullptr, false /* allow exceptions */, true /* allow comments */);

  FileInfo info = j["file_info"].get<FileInfo>();

  // TODO: validate header and choose the appropriate version of the PartialData
  // structure to read.

  PartialData data = j;
  std::optional<PartialData> result(std::move(data));

  return result;
}

}  // namespace utu