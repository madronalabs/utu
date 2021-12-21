//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <nlohmann/json.hpp>

#include <utu/PartialReader.h>

namespace utu
{
struct FileInfo
{
  std::string kind;
  uint16_t version;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileInfo, kind, version)
};
}  // namespace utu


//
// JSON serialize/deserialize helpers, implemented here to above exposing the
// chosen JSON library to API consumers and to speed compilation.
//
// https://json.nlohmann.me/features/arbitrary_types/#how-do-i-convert-third-party-types
//

namespace nlohmann
{

template<typename T>
struct adl_serializer<std::optional<T>>
{
  static void to_json(json& j, const std::optional<T>& v)
  {
    if (v) {
      j = *v;
    }
    else {
      j = nullptr;
    }
  }

  static void from_json(const json& j, std::optional<T>& v)
  {
    if (j.is_null()) {
      v = {};
    } else {
      v = j.get<T>();
    }
  }
};

template <>
struct adl_serializer<utu::Partial>
{
  static void to_json(json& j, const utu::Partial& p)
  {
    if (p.label)
    {
      j["label"] = *p.label;
    }
    j["envelopes"] = p.envelopes;
  }

  static void from_json(const json& j, utu::Partial& p)
  {
    p.label = j.value("label", std::optional<std::string>({}));
    p.envelopes = j["envelopes"].get<utu::Partial::Envelopes>();
    // TODO: ensure there is at least one envelope
  }
};

template <>
struct adl_serializer<utu::PartialData::Source>
{
  static void to_json(json& j, const utu::PartialData::Source& s)
  {
    j["location"] = s.location;
    if (s.fingerprint)
    {
      j["fingerprint"] = *s.fingerprint;
    }
  }

  static void from_json(const json& j, utu::PartialData::Source& s)
  {
    s.location = j["location"].get<std::string>();
    s.fingerprint = j.value("fingerprint", std::optional<std::string>({}));
  }
};

template <>
struct adl_serializer<utu::PartialData>
{
  static void to_json(json& j, const utu::PartialData& d)
  {
    if (d.description)
    {
      j["description"] = *d.description;
    }
    if (d.source)
    {
      j["source"] = *d.source;
    }
    j["parameters"] = d.parameters;
    j["partials"] = d.partials;
  }

  static void from_json(const json& j, utu::PartialData& d)
  {
    d.description = j.value("description", std::optional<std::string>({}));
    d.source = j.value("source", std::optional<utu::PartialData::Source>({}));
    d.parameters = j["parameters"].get<std::vector<std::string>>();

    // FIXME: should validate that parameters match up
    d.partials = j["partials"].get<std::vector<utu::Partial>>();
  }
};

}  // namespace nlohmann
