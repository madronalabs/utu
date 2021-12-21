#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <utu/Partial.h>
#include <utu/PartialData.h>

#include "SerializerImpl.h"

using json = nlohmann::json;


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
    "envelopes": {
      "amplitude": [0.2, 0.4, -0.2, -0.3]
    }
  })"_json;

  utu::Partial p1 = e1;
  EXPECT_FALSE(p1.label);
  EXPECT_EQ(p1.envelopes.size(), 1);
  EXPECT_NE(p1.envelopes.find("amplitude"), p1.envelopes.end());
  EXPECT_EQ(p1.envelopes["amplitude"].size(), 4);

  json e2 = R"({
    "label": "component-1",
    "envelopes": {
      "time": [0, 0.1, 0.2, 0.3],
      "frequency": [440.0, 440.5, 462.2, 439.8]
    }
  })"_json;

  utu::Partial p2 = e2;
  EXPECT_EQ(*p2.label, "component-1");
  EXPECT_EQ(p2.envelopes.size(), 2);
  EXPECT_EQ(p2.envelopes["frequency"].size(), 4);
  EXPECT_DOUBLE_EQ(p2.envelopes["frequency"][2], 462.2);
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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
