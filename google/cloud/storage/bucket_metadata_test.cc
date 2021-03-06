// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/storage/bucket_metadata.h"
#include "google/cloud/storage/storage_class.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace {
using google::cloud::internal::make_optional;
using google::cloud::internal::optional;
using ::testing::HasSubstr;
using ::testing::Not;

BucketMetadata CreateBucketMetadataForTest() {
  // This metadata object has some impossible combination of fields in it. The
  // goal is to fully test the parsing, not to simulate valid objects.
  std::string text = R"""({
      "acl": [{
        "kind": "storage#bucketAccessControl",
        "id": "acl-id-0",
        "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket/acl/user-test-user",
        "bucket": "test-bucket",
        "entity": "user-test-user",
        "role": "OWNER",
        "email": "test-user@example.com",
        "entityId": "user-test-user-id-123",
        "domain": "example.com",
        "projectTeam": {
          "projectNumber": "4567",
          "team": "owners"
        },
        "etag": "AYX="
      }, {
        "kind": "storage#objectAccessControl",
        "id": "acl-id-1",
        "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket/acl/user-test-user2",
        "bucket": "test-bucket",
        "entity": "user-test-user2",
        "role": "READER",
        "email": "test-user2@example.com",
        "entityId": "user-test-user2-id-123",
        "domain": "example.com",
        "projectTeam": {
          "projectNumber": "4567",
          "team": "viewers"
        },
        "etag": "AYX="
      }
      ],
      "billing": {
        "requesterPays": true
      },
      "cors": [{
        "maxAgeSeconds": 3600,
        "method": ["GET", "HEAD"],
        "origin": ["cross-origin-example.com"]
      }, {
        "method": ["GET", "HEAD"],
        "origin": ["another-example.com"],
        "responseHeader": ["Content-Type"]
      }],
      "defaultObjectAcl": [{
        "kind": "storage#objectAccessControl",
        "id": "default-acl-id-0",
        "bucket": "test-bucket",
        "entity": "user-test-user-3",
        "role": "OWNER",
        "email": "test-user-1@example.com",
        "entityId": "user-test-user-1-id-123",
        "domain": "example.com",
        "projectTeam": {
          "projectNumber": "123456789",
          "team": "owners"
        },
        "etag": "AYX="
      }],
      "encryption": {
        "defaultKmsKeyName": "projects/test-project-name/locations/us-central1/keyRings/test-keyring-name/cryptoKeys/test-key-name"
      },
      "etag": "XYZ=",
      "id": "test-bucket",
      "kind": "storage#bucket",
      "labels": {
        "label-key-1": "label-value-1",
        "label-key-2": "label-value-2"
      },
      "lifecycle": {
        "rule": [{
          "condition": {
            "age": 30,
            "matchesStorageClass": [ "STANDARD" ]
          },
          "action": {
            "type": "SetStorageClass",
            "storageClass": "NEARLINE"
          }
        }]
      },
      "location": "US",
      "logging": {
        "logBucket": "test-log-bucket",
        "logPrefix": "test-log-prefix"
      },
      "metageneration": "4",
      "name": "test-bucket",
      "owner": {
        "entity": "project-owners-123456789",
        "entityId": "test-owner-id-123"
      },
      "projectNumber": "123456789",
      "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket",
      "storageClass": "STANDARD",
      "timeCreated": "2018-05-19T19:31:14Z",
      "updated": "2018-05-19T19:31:24Z",
      "versioning": {
        "enabled": true
      },
      "website": {
        "mainPageSuffix": "index.html",
        "notFoundPage": "404.html"
      }
})""";
  return BucketMetadata::ParseFromString(text);
}

/// @test Verify that we parse JSON objects into BucketMetadata objects.
TEST(BucketMetadataTest, Parse) {
  auto actual = CreateBucketMetadataForTest();

  EXPECT_EQ(2U, actual.acl().size());
  EXPECT_EQ("acl-id-0", actual.acl().at(0).id());
  EXPECT_EQ("acl-id-1", actual.acl().at(1).id());
  EXPECT_TRUE(actual.billing().requester_pays);
  EXPECT_EQ(2U, actual.cors().size());
  auto expected_cors_0 = CorsEntry{make_optional<std::int64_t>(3600),
                                   {"GET", "HEAD"},
                                   {"cross-origin-example.com"},
                                   {}};
  EXPECT_EQ(expected_cors_0, actual.cors().at(0));
  auto expected_cors_1 = CorsEntry{optional<std::int64_t>(),
                                   {"GET", "HEAD"},
                                   {"another-example.com"},
                                   {"Content-Type"}};
  EXPECT_EQ(expected_cors_1, actual.cors().at(1));
  EXPECT_EQ(1U, actual.default_acl().size());
  EXPECT_EQ("user-test-user-3", actual.default_acl().at(0).entity());
  EXPECT_EQ(
      "projects/test-project-name/locations/us-central1/keyRings/"
      "test-keyring-name/cryptoKeys/test-key-name",
      actual.encryption().default_kms_key_name);
  EXPECT_EQ("XYZ=", actual.etag());
  EXPECT_EQ("test-bucket", actual.id());
  EXPECT_EQ("storage#bucket", actual.kind());
  EXPECT_EQ(2U, actual.label_count());
  EXPECT_TRUE(actual.has_label("label-key-1"));
  EXPECT_EQ("label-value-1", actual.label("label-key-1"));
  EXPECT_FALSE(actual.has_label("not-a-label-key"));
#if GOOGLE_CLOUD_CPP_HAVE_EXCEPTIONS
  EXPECT_THROW(actual.label("not-a-label-key"), std::exception);
#else
  // We accept any output here because the actual output depends on the
  // C++ library implementation.
  EXPECT_DEATH_IF_SUPPORTED(actual.label("not-a-label-key"), "");
#endif  // GOOGLE_CLOUD_CPP_EXCEPTIONS

  EXPECT_TRUE(actual.has_lifecycle());
  EXPECT_EQ(1U, actual.lifecycle().rule.size());
  LifecycleRuleCondition expected_condition =
      LifecycleRule::ConditionConjunction(
          LifecycleRule::MaxAge(30),
          LifecycleRule::MatchesStorageClassStandard());
  EXPECT_EQ(expected_condition, actual.lifecycle().rule.at(0).condition());

  LifecycleRuleAction expected_action =
      LifecycleRule::SetStorageClassNearline();
  EXPECT_EQ(expected_action, actual.lifecycle().rule.at(0).action());

  EXPECT_EQ("US", actual.location());

  EXPECT_EQ("test-log-bucket", actual.logging().log_bucket);
  EXPECT_EQ("test-log-prefix", actual.logging().log_prefix);
  EXPECT_EQ(4, actual.metageneration());
  EXPECT_EQ("test-bucket", actual.name());
  EXPECT_EQ("project-owners-123456789", actual.owner().entity);
  EXPECT_EQ("test-owner-id-123", actual.owner().entity_id);
  EXPECT_EQ(123456789, actual.project_number());
  EXPECT_EQ("https://www.googleapis.com/storage/v1/b/test-bucket",
            actual.self_link());
  EXPECT_EQ(storage_class::Standard(), actual.storage_class());
  // Use `date -u +%s --date='2018-05-19T19:31:14Z'` to get the magic number:
  auto magic_timestamp = 1526758274L;
  using std::chrono::duration_cast;
  EXPECT_EQ(magic_timestamp, duration_cast<std::chrono::seconds>(
                                 actual.time_created().time_since_epoch())
                                 .count());
  EXPECT_EQ(magic_timestamp + 10, duration_cast<std::chrono::seconds>(
                                      actual.updated().time_since_epoch())
                                      .count());

  EXPECT_EQ("index.html", actual.website().main_page_suffix);
  EXPECT_EQ("404.html", actual.website().not_found_page);
}

/// @test Verify that the IOStream operator works as expected.
TEST(BucketMetadataTest, IOStream) {
  auto meta = CreateBucketMetadataForTest();

  std::ostringstream os;
  os << meta;
  auto actual = os.str();
  EXPECT_THAT(actual, HasSubstr("BucketMetadata"));

  // acl()
  EXPECT_THAT(actual, HasSubstr("acl-id-0"));
  EXPECT_THAT(actual, HasSubstr("acl-id-1"));
  // billing()
  EXPECT_THAT(actual, HasSubstr("enabled=true"));

  // bucket()
  EXPECT_THAT(actual, HasSubstr("bucket=test-bucket"));

  // labels()
  EXPECT_THAT(actual, HasSubstr("labels.label-key-1=label-value-1"));
  EXPECT_THAT(actual, HasSubstr("labels.label-key-2=label-value-2"));

  // default_acl()
  EXPECT_THAT(actual, HasSubstr("user-test-user-3"));

  // encryption()
  EXPECT_THAT(actual,
              HasSubstr("projects/test-project-name/locations/us-central1/"
                        "keyRings/test-keyring-name/cryptoKeys/test-key-name"));

  // lifecycle()
  EXPECT_THAT(actual, HasSubstr("age=30"));

  // logging()
  EXPECT_THAT(actual, HasSubstr("test-log-bucket"));
  EXPECT_THAT(actual, HasSubstr("test-log-prefix"));

  // name()
  EXPECT_THAT(actual, HasSubstr("name=test-bucket"));

  // project_team()
  EXPECT_THAT(actual, HasSubstr("project-owners-123456789"));
  EXPECT_THAT(actual, HasSubstr("test-owner-id-123"));

  // versioning()
  EXPECT_THAT(actual, HasSubstr("versioning.enabled=true"));

  // website()
  EXPECT_THAT(actual, HasSubstr("index.html"));
  EXPECT_THAT(actual, HasSubstr("404.html"));
}

/// @test Verify we can convert a BucketMetadata object to a JSON string.
TEST(BucketMetadataTest, ToJsonString) {
  auto tested = CreateBucketMetadataForTest();
  auto actual_string = tested.ToJsonString();
  // Verify that the produced string can be parsed as a JSON object.
  internal::nl::json actual = internal::nl::json::parse(actual_string);
  ASSERT_EQ(1U, actual.count("acl")) << actual;
  EXPECT_TRUE(actual["acl"].is_array()) << actual;
  EXPECT_EQ(2U, actual["acl"].size()) << actual;
  EXPECT_EQ("user-test-user", actual["acl"][0].value("entity", ""));
  EXPECT_EQ("user-test-user2", actual["acl"][1].value("entity", ""));

  ASSERT_EQ(1U, actual.count("billing")) << actual;
  EXPECT_TRUE(actual["billing"].value("requesterPays", false));

  ASSERT_EQ(1U, actual.count("cors")) << actual;
  EXPECT_TRUE(actual["cors"].is_array()) << actual;
  EXPECT_EQ(2U, actual["cors"].size()) << actual;
  EXPECT_EQ(3600, actual["cors"][0].value("maxAgeSeconds", 0));

  ASSERT_EQ(1U, actual.count("defaultObjectAcl")) << actual;
  EXPECT_TRUE(actual["defaultObjectAcl"].is_array()) << actual;
  EXPECT_EQ(1U, actual["defaultObjectAcl"].size()) << actual;
  EXPECT_EQ("user-test-user-3",
            actual["defaultObjectAcl"][0].value("entity", ""));

  ASSERT_EQ(1U, actual.count("encryption"));
  EXPECT_EQ(
      "projects/test-project-name/locations/us-central1/keyRings/"
      "test-keyring-name/cryptoKeys/test-key-name",
      actual["encryption"].value("defaultKmsKeyName", ""));

  ASSERT_EQ(1U, actual.count("labels")) << actual;
  EXPECT_TRUE(actual["labels"].is_object()) << actual;
  EXPECT_EQ("label-value-1", actual["labels"].value("label-key-1", ""));
  EXPECT_EQ("label-value-2", actual["labels"].value("label-key-2", ""));

  EXPECT_EQ("test-bucket", actual.value("name", std::string{}));
}

/// @test Verify we can make changes to one Acl in BucketMetadata.
TEST(BucketMetadataTest, MutableAcl) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  EXPECT_EQ(expected, copy);
  copy.mutable_acl().at(0).set_role(BucketAccessControl::ROLE_READER());
  copy.mutable_acl().at(1).set_role(BucketAccessControl::ROLE_OWNER());
  EXPECT_EQ("READER", copy.acl().at(0).role());
  EXPECT_EQ("OWNER", copy.acl().at(1).role());
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the full acl in BucketMetadata.
TEST(BucketMetadataTest, SetAcl) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  auto acl = expected.acl();
  acl.at(0).set_role(BucketAccessControl::ROLE_READER());
  acl.at(1).set_role(BucketAccessControl::ROLE_OWNER());
  copy.set_acl(std::move(acl));
  EXPECT_NE(expected, copy);
  EXPECT_EQ("READER", copy.acl().at(0).role());
}

/// @test Verify we can change the billing configuration in BucketMetadata.
TEST(BucketMetadataTest, SetBilling) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  auto billing = copy.billing();
  billing.requester_pays = not billing.requester_pays;
  copy.set_billing(billing);
  EXPECT_NE(expected, copy);
}

/// @test Verify we can reset the billing configuration in BucketMetadata.
TEST(BucketMetadataTest, ResetBilling) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.has_billing());
  auto copy = expected;
  copy.reset_billing();
  EXPECT_FALSE(copy.has_billing());
  EXPECT_NE(expected, copy);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("billing")));
}

/// @test Verify we can make changes to one CORS entry in BucketMetadata.
TEST(BucketMetadataTest, MutableCors) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  EXPECT_EQ(expected, copy);
  copy.mutable_cors().at(0).max_age_seconds = 3 * 3600;
  EXPECT_NE(expected, copy);
  EXPECT_EQ(3600, *expected.cors().at(0).max_age_seconds);
  EXPECT_EQ(3 * 3600, *copy.cors().at(0).max_age_seconds);
}

/// @test Verify we can change the full CORS configuration in BucketMetadata.
TEST(BucketMetadataTest, SetCors) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  auto cors = copy.cors();
  cors.at(0).response_header.emplace_back("Content-Encoding");
  copy.set_cors(std::move(cors));
  EXPECT_NE(expected, copy);
  EXPECT_EQ("Content-Encoding", copy.cors().at(0).response_header.back());
}

/// @test Verify we can make changes to one DefaultObjectAcl in BucketMetadata.
TEST(BucketMetadataTest, MutableDefaultObjectAcl) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_EQ("OWNER", expected.default_acl().at(0).role());
  auto copy = expected;
  EXPECT_EQ(expected, copy);
  copy.mutable_default_acl().at(0).set_role(BucketAccessControl::ROLE_READER());
  EXPECT_EQ("READER", copy.default_acl().at(0).role());
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the full DefaultObjectAcl in BucketMetadata.
TEST(BucketMetadataTest, SetDefaultObjectAcl) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_FALSE(expected.default_acl().empty());
  auto copy = expected;
  auto default_acl = expected.default_acl();
  auto access = default_acl.at(0);
  access.set_entity("allAuthenticatedUsers");
  access.set_role("READER");
  default_acl.push_back(access);
  copy.set_default_acl(std::move(default_acl));
  EXPECT_EQ(2U, copy.default_acl().size());
  EXPECT_EQ("allAuthenticatedUsers", copy.default_acl().at(1).entity());
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the full DefaultObjectAcl in BucketMetadata.
TEST(BucketMetadataTest, SetEncryption) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  std::string fake_key_name =
      "projects/test-project-name/locations/us-central1/keyRings/"
      "test-keyring-name/cryptoKeys/another-test-key-name";
  copy.set_encryption(BucketEncryption{fake_key_name});
  EXPECT_EQ(fake_key_name, copy.encryption().default_kms_key_name);
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the full DefaultObjectAcl in BucketMetadata.
TEST(BucketMetadataTest, ResetEncryption) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.has_encryption());
  auto copy = expected;
  copy.reset_encryption();
  EXPECT_FALSE(copy.has_encryption());
  EXPECT_NE(expected, copy);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("encryption.")));
}

/// @test Verify we can reset the Object Lifecycle in BucketMetadata.
TEST(BucketMetadataTest, ResetLifecycle) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  EXPECT_TRUE(copy.has_lifecycle());
  copy.reset_lifecycle();
  EXPECT_FALSE(copy.has_lifecycle());
  EXPECT_NE(expected, copy);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("lifecycle.")));
}

/// @test Verify we can change the Object Lifecycle in BucketMetadata.
TEST(BucketMetadataTest, SetLifecycle) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  EXPECT_TRUE(copy.has_lifecycle());
  auto updated = copy.lifecycle();
  updated.rule.emplace_back(
      LifecycleRule(LifecycleRule::MaxAge(365), LifecycleRule::Delete()));
  copy.set_lifecycle(std::move(updated));
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the Logging configuration in BucketMetadata.
TEST(BucketMetadataTest, SetLogging) {
  auto expected = CreateBucketMetadataForTest();
  BucketLogging new_logging{"another-test-bucket", "another-test-prefix"};
  auto copy = expected;
  copy.set_logging(new_logging);
  EXPECT_EQ(new_logging, copy.logging());
  EXPECT_NE(expected, copy);
}

/// @test Verify we can change the Logging configuration in BucketMetadata.
TEST(BucketMetadataTest, ResetLogging) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.has_logging());
  auto copy = expected;
  copy.reset_logging();
  EXPECT_FALSE(copy.has_logging());
  EXPECT_NE(expected, copy);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("logging.")));
}

/// @test Verify we can clear the versioning field in BucketMetadata.
TEST(BucketMetadataTest, ClearVersioning) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.versioning().has_value());
  auto copy = expected;
  copy.clear_versioning();
  EXPECT_FALSE(copy.versioning().has_value());
  EXPECT_NE(copy, expected);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("versioning.")));
}

/// @test Verify we can set the versioning field in BucketMetadata.
TEST(BucketMetadataTest, DisableVersioning) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.versioning().has_value());
  EXPECT_TRUE(expected.versioning()->enabled);
  auto copy = expected;
  copy.disable_versioning();
  EXPECT_TRUE(copy.versioning().has_value());
  EXPECT_FALSE(copy.versioning()->enabled);
  EXPECT_NE(copy, expected);
}

/// @test Verify we can set the versioning field in BucketMetadata.
TEST(BucketMetadataTest, EnableVersioning) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.versioning().has_value());
  EXPECT_TRUE(expected.versioning()->enabled);
  auto copy = expected;
  copy.clear_versioning();
  copy.enable_versioning();
  EXPECT_TRUE(copy.versioning().has_value());
  EXPECT_TRUE(copy.versioning()->enabled);
  EXPECT_EQ(copy, expected);
}

/// @test Verify we can set the versioning field in BucketMetadata.
TEST(BucketMetadataTest, SetVersioning) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.versioning().has_value());
  EXPECT_TRUE(expected.versioning()->enabled);
  auto copy = expected;
  copy.set_versioning(optional<BucketVersioning>(BucketVersioning{false}));
  EXPECT_TRUE(copy.versioning().has_value());
  EXPECT_FALSE(copy.versioning()->enabled);
  EXPECT_NE(copy, expected);
}

/// @test Verify we can set the website field in BucketMetadata.
TEST(BucketMetadataTest, SetWebsite) {
  auto expected = CreateBucketMetadataForTest();
  auto copy = expected;
  copy.set_website(BucketWebsite{"main.html", "not-found.html"});
  EXPECT_EQ("main.html", copy.website().main_page_suffix);
  EXPECT_EQ("not-found.html", copy.website().not_found_page);
  EXPECT_NE(copy, expected);
}

/// @test Verify we can set the website field in BucketMetadata.
TEST(BucketMetadataTest, ResetWebsite) {
  auto expected = CreateBucketMetadataForTest();
  EXPECT_TRUE(expected.has_website());
  auto copy = expected;
  copy.reset_website();
  EXPECT_FALSE(copy.has_website());
  EXPECT_NE(copy, expected);
  std::ostringstream os;
  os << copy;
  EXPECT_THAT(os.str(), Not(HasSubstr("website.")));
}

}  // namespace
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google
