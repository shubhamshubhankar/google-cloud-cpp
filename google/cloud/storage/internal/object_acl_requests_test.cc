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

#include "google/cloud/storage/internal/object_acl_requests.h"
#include "google/cloud/storage/internal/curl_request_builder.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace internal {
namespace {
using ::testing::HasSubstr;

TEST(CreateObjectAclRequestTest, Simple) {
  CreateObjectAclRequest request("my-bucket", "my-object", "user-testuser",
                                 "READER");
  EXPECT_EQ("my-bucket", request.bucket_name());
  EXPECT_EQ("my-object", request.object_name());
  EXPECT_EQ("user-testuser", request.entity());
  EXPECT_EQ("READER", request.role());
}

TEST(CreateObjectAclRequestTest, Stream) {
  CreateObjectAclRequest request("my-bucket", "my-object", "user-testuser",
                                 "READER");
  request.set_multiple_options(UserProject("my-project"), Generation(7));
  std::ostringstream os;
  os << request;
  auto str = os.str();
  EXPECT_THAT(str, HasSubstr("userProject=my-project"));
  EXPECT_THAT(str, HasSubstr("generation=7"));
  EXPECT_THAT(str, HasSubstr("my-bucket"));
  EXPECT_THAT(str, HasSubstr("my-object"));
  EXPECT_THAT(str, HasSubstr("user-testuser"));
  EXPECT_THAT(str, HasSubstr("READER"));
}

TEST(ObjectAclRequestTest, Simple) {
  ObjectAclRequest request("my-bucket", "my-object", "user-test-user");
  EXPECT_EQ("my-bucket", request.bucket_name());
  EXPECT_EQ("my-object", request.object_name());
  EXPECT_EQ("user-test-user", request.entity());
}

TEST(ObjectAclRequestTest, Stream) {
  ObjectAclRequest request("my-bucket", "my-object", "user-test-user");
  request.set_multiple_options(UserProject("my-project"), Generation(7));
  std::ostringstream os;
  os << request;
  auto str = os.str();
  EXPECT_THAT(str, HasSubstr("userProject=my-project"));
  EXPECT_THAT(str, HasSubstr("generation=7"));
  EXPECT_THAT(str, HasSubstr("my-bucket"));
  EXPECT_THAT(str, HasSubstr("my-object"));
  EXPECT_THAT(str, HasSubstr("user-test-user"));
}

ObjectAccessControl CreateObjectAccessControlForTest() {
  std::string text = R"""({
      "bucket": "foo-bar",
      "domain": "example.com",
      "email": "foobar@example.com",
      "entity": "user-foobar",
      "entityId": "user-foobar-id-123",
      "etag": "XYZ=",
      "generation": 42,
      "id": "object-foo-bar-baz-acl-234",
      "kind": "storage#objectAccessControl",
      "object": "baz",
      "projectTeam": {
        "projectNumber": "3456789",
        "team": "a-team"
      },
      "role": "OWNER"
})""";
  return ObjectAccessControl::ParseFromString(text);
}

TEST(PatchObjectAclRequestTest, ReadModifyWrite) {
  ObjectAccessControl original = CreateObjectAccessControlForTest();
  ObjectAccessControl new_acl =
      CreateObjectAccessControlForTest().set_role("READER");

  PatchObjectAclRequest request("my-bucket", "my-object", "user-test-user",
                                original, new_acl);
  nl::json expected = {
      {"role", "READER"},
  };
  nl::json actual = nl::json::parse(request.payload());
  EXPECT_EQ(expected, actual);
}

TEST(PatchObjectAclRequestTest, Patch) {
  PatchObjectAclRequest request(
      "my-bucket", "my-object", "user-test-user",
      ObjectAccessControlPatchBuilder().set_role("READER").delete_entity());
  nl::json expected = {{"role", "READER"}, {"entity", nullptr}};
  nl::json actual = nl::json::parse(request.payload());
  EXPECT_EQ(expected, actual);
}

TEST(PatchObjectAclRequestTest, PatchStream) {
  ObjectAccessControl original = CreateObjectAccessControlForTest();
  ObjectAccessControl new_acl =
      CreateObjectAccessControlForTest().set_role("READER");

  PatchObjectAclRequest request("my-bucket", "my-object", "user-test-user",
                                original, new_acl);
  request.set_multiple_options(UserProject("my-project"), Generation(7),
                               IfMatchEtag("ABC="));
  std::ostringstream os;
  os << request;
  auto str = os.str();
  EXPECT_THAT(str, HasSubstr("userProject=my-project"));
  EXPECT_THAT(str, HasSubstr("If-Match: ABC="));
  EXPECT_THAT(str, HasSubstr("generation=7"));
  EXPECT_THAT(str, HasSubstr("my-bucket"));
  EXPECT_THAT(str, HasSubstr("my-object"));
  EXPECT_THAT(str, HasSubstr("user-test-user"));
  EXPECT_THAT(str, HasSubstr(request.payload()));
}

}  // namespace
}  // namespace internal
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google
