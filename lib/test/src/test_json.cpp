//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: MIT
//

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <utu/Partial.h>
#include <utu/PartialData.h>
#include <utu/PartialIO.h>

#include "SerializerImpl.h"

using json = nlohmann::json;

// TODO: Add tests for non-happy path cases

TEST(json, CanReadOptional)
{
  json j1 = {{"key", "value"}};
  auto v1 = j1["key"].get<std::optional<std::string>>();
  EXPECT_TRUE(v1.has_value());
  EXPECT_EQ(*v1, "value");

  json j2 = {{"key", {}}};
  auto v2 = j2["key"].get<std::optional<std::string>>();
  EXPECT_FALSE(v2.has_value());
}

TEST(json, CanReadPartial)
{
  json e1 = R"({
    "parameters": {
      "amplitude": [0.2, 0.4, -0.2, -0.3]
    }
  })"_json;

  utu::Partial p1 = e1;
  EXPECT_FALSE(p1.label);
  EXPECT_EQ(p1.parameters.size(), 1);
  EXPECT_NE(p1.parameters.find("amplitude"), p1.parameters.end());
  EXPECT_EQ(p1.parameters["amplitude"].size(), 4);

  json e2 = R"({
    "label": "component-1",
    "parameters": {
      "time": [0, 0.1, 0.2, 0.3],
      "frequency": [440.0, 440.5, 462.2, 439.8]
    }
  })"_json;

  utu::Partial p2 = e2;
  EXPECT_EQ(*p2.label, "component-1");
  EXPECT_EQ(p2.parameters.size(), 2);
  EXPECT_EQ(p2.parameters["frequency"].size(), 4);
  EXPECT_DOUBLE_EQ(p2.parameters["frequency"][2], 462.2);
}

TEST(json, CanReadPartialDataSource)
{
  json s1 = {
    {"location", "file://foo/bar/baz"}
  };

  utu::PartialData::Source v1 = s1;
  EXPECT_EQ(v1.location, "file://foo/bar/baz");
  EXPECT_FALSE(v1.fingerprint);

  json s2 = {
    {"location", "path/to/source.aiff"},
    {"fingerprint", "aa3d7a675a011631f8a1c5402403496245fd822cdc15a8c385e1bdb5e6f1f0a8"}
  };

  utu::PartialData::Source v2 = s2;
  EXPECT_EQ(v2.location, "path/to/source.aiff");
  EXPECT_TRUE(v2.fingerprint.has_value());
  EXPECT_EQ(*v2.fingerprint, "aa3d7a675a011631f8a1c5402403496245fd822cdc15a8c385e1bdb5e6f1f0a8");
}

TEST(json, CanReadPartialData)
{
  json j1 = R"({
    "parameters": ["foo", "bar", "baz"],
    "partials": [
      {
        "parameters": {
          "time": [0, 0.1, 0.2, 0.3],
          "frequency": [440.0, 440.5, 462.2, 439.8]
        }
      }
    ]
  })"_json;

  utu::PartialData minimal = j1;
  EXPECT_FALSE(minimal.source);
  EXPECT_FALSE(minimal.description);
  EXPECT_EQ(minimal.partials.size(), 1);

  json j2 = R"({
    "description": "something",
    "source": {
      "location": "file.wav"
    },
    "parameters": ["foo", "bar", "baz"],
    "partials": [
      {
        "parameters": {
          "time": [0, 0.1, 0.2, 0.3],
          "frequency": [440.0, 440.5, 462.2, 439.8]
        }
      },
      {
        "label": "a component label",
        "parameters": {
          "foo": [0, 0.1, 0.2, 0.3],
          "bar": [1, 2]
        }
      }
    ]
  })"_json;

  utu::PartialData maximal = j2;
  EXPECT_EQ(*maximal.description, "something");
  EXPECT_TRUE(maximal.source);
  EXPECT_EQ(maximal.parameters.size(), 3);
  EXPECT_EQ(maximal.parameters[2], "baz");

  const utu::Partial& p2 = maximal.partials[1];
  EXPECT_NE(p2.parameters.find("bar"), p2.parameters.end());

  const utu::Partial::Samples& e = p2.parameters.find("bar")->second;
  EXPECT_DOUBLE_EQ(e[1], 2);
}

TEST(json, PartialReader)
{
  std::string data = R"({
    "file_info": {
        "kind": "utu-partial-data",
        "version": 1
    },
    "source": {
        "location": "/some/path/on/disk.aiff"
    },
    "parameters": ["time", "frequency", "amplitude", "bandwidth", "phase"],
    "partials": [
        {
            "label": "component-1",
            "parameters": {
                "time": [0, 440, 0.3, 0.2, 0],
                "frequency": [0.2, 440, 0.3, 0.2, 0],
                "amplitude": [0.5, 440, 0.3, 0.2, 0],
                "bandwith": [1.5, 440, 0.3, 0.2, 0],
                "phase": [3.0, 440, 0.3, 0.2, 0]
            }
        },
        {
            "parameters": {
                "time": [0.3, 440, 0.3, 0.2, 0],
                "frequency": [3.0, 440, 0.3, 0.2, 0],
                "amplitude": [4.2, 440, 0.3, 0.2, 0]
            }
        }
    ]
  })";

  std::optional<utu::PartialData> d = utu::PartialReader::read(data);

}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
