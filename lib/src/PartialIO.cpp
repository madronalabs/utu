//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <utu/PartialIO.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "SerializerImpl.h"

constexpr uint8_t kIndentWidth = 2;

constexpr uint16_t kFileVersion = 1;
constexpr char kFileKind[] = "utu-partial-data";

namespace
{

using json = nlohmann::json;
using namespace utu;

std::optional<PartialData> _read(json& j)
{
  FileInfo info = j["file_info"].get<FileInfo>();

  // TODO: validate header and choose the appropriate version of the PartialData
  // structure to read.

  PartialData data = j;
  std::optional<PartialData> result(data);

  return result;
}

void _addFileInfo(json& j)
{
  FileInfo info({kFileKind, kFileVersion});
  j["file_info"] = info;
}

}  // namespace

namespace utu
{

using json = nlohmann::json;

template <>
std::optional<PartialData> PartialReader::read(const std::string& jsonData)
{
  json j = json::parse(jsonData, nullptr, true /* allow exceptions */, true /* allow comments */);
  return _read(j);
}

template <>
std::optional<PartialData> PartialReader::read(std::istream& is)
{
  json j;
  is >> j;
  return _read(j);
}

template <>
std::optional<std::string> PartialWriter::write(const PartialData& value)
{
  json j = value;
  _addFileInfo(j);
  std::optional<std::string> result(j.dump(kIndentWidth));
  return result;
}

template <>
void PartialWriter::write(const PartialData& value, std::ostream& os)
{
  // TODO: better error reporting
  json j = value;
  _addFileInfo(j);
  os << std::setw(kIndentWidth) << j << std::endl;
}

}  // namespace utu